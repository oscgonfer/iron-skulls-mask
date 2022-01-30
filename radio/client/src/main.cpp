// This code will forward anything received via serial 
// to a server with a destination defined as
// the first char in the string received via serial
// The rest of the message is forwarded to the server until 
// a new message is received via serial, or the server 
// acknowledges reception

#include "Arduino.h"
#include <SPI.h>
#include <RH_RF69.h>
#include <RHReliableDatagram.h>

/************ Radio Setup ***************/
#define RF69_FREQ 433.0
#define MY_ADDRESS     1

#define RFM69_CS      8

#define MAX_MESSAGE_LENGTH 20
// 20 bytes max length means at maximum 2 bytes for checksum and 18 for message itself
char checksum_ok[] = "CHK_OK";

#if defined (__AVR_ATmega32U4__) // Feather 32u4 w/Radio
#define RFM69_INT     7
#else
#define RFM69_INT     3
#endif

#define RFM69_RST     4
#define LED           13

// Singleton instance of the radio driver
RH_RF69 rf69(RFM69_CS, RFM69_INT);

// Class to manage message delivery and receipt, using the driver declared above
RHReliableDatagram rf69_manager(rf69, MY_ADDRESS);

int16_t packetnum = 0;  // packet counter, we increment per xmission

void setup()
{
    Serial.begin(115200);
    while (!Serial) { delay(1); } // wait until serial console is open, remove if not tethered to computer

    pinMode(LED, OUTPUT);
    pinMode(RFM69_RST, OUTPUT);
    digitalWrite(RFM69_RST, LOW);

    Serial.println("Feather Addressed RFM69 TX Test!");

    // manual reset
    digitalWrite(RFM69_RST, HIGH);
    delay(10);
    digitalWrite(RFM69_RST, LOW);
    delay(10);

    if (!rf69_manager.init()) {
        Serial.println("RFM69 radio init failed");
        while (1);
    }

    Serial.println("RFM69 radio init OK!");
    // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM (for low power module)
    // No encryption
    if (!rf69.setFrequency(RF69_FREQ)) {
    Serial.println("setFrequency failed");
    }

    // If you are using a high power RF69 eg RFM69HW, you *must* set a Tx power with the
    // ishighpowermodule flag set like this:
    rf69.setTxPower(20, true);  // range from 14-20 for power, 2nd arg must be true for 69HCW

    // The encryption key has to be the same as the one in the server
    uint8_t key[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                      0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    rf69.setEncryptionKey(key);

    pinMode(LED, OUTPUT);

    Serial.print("RFM69 radio @");  Serial.print((int)RF69_FREQ);  Serial.println(" MHz");
}

uint8_t buf[RH_RF69_MAX_MESSAGE_LEN];

void loop() {
  
    static char message[MAX_MESSAGE_LENGTH];
    static unsigned int message_pos = 0;

    while (Serial.available() > 0) {

        // read the incoming byte:
        byte inByte = Serial.read();
        if (inByte != '\n') {

            message[message_pos] = inByte;
            message_pos++;

        } else {

            message[message_pos] = '\0';
            message_pos = 0;

            Serial.println("Got new message to send");

            char destination = String(message[0]).toInt();
            Serial.print("Destination mask: ");
            Serial.println(String(message[0]));
            
            char packet[MAX_MESSAGE_LENGTH]{};
            strcpy(packet, &message[1]);

            int packet_length = strlen(packet);

            Serial.print("Packet lenght ");
            Serial.println(packet_length);

            uint16_t checksum = 0;

            for (int i = 0; i < packet_length; i++) {
                // Serial.println(packet[i], HEX);
                checksum += packet[i];
            }
            
            Serial.print("Checksum: ");
            Serial.println(checksum, HEX);

            packet[packet_length++] = (uint8_t)(checksum >> 8);
            packet[packet_length++] = (uint8_t)(checksum & 0xFF);
            packet[packet_length] =  '\0';
            Serial.print("New packet length: ");
            Serial.println(strlen(packet));

            // for (int i = 0; i < strlen(packet); i++) {
            //     Serial.println(packet[i], HEX);
            // }

            bool responseReceived = false;

            while (!responseReceived) {

                // Check if we got another message to send
                if (Serial.available()>0){
                    break;
                }

                Serial.print("Sending ");
                Serial.println(packet);

                Serial.println("Sending it...");

                if (rf69_manager.sendtoWait((uint8_t *)packet, strlen(packet), destination)) {
                    // Now wait for a reply from the server
                    uint8_t len = sizeof(buf);
                    uint8_t from;

                    if (rf69_manager.recvfromAckTimeout(buf, &len, 2000, &from)) {
                        buf[len] = 0; // zero out remaining string

                        Serial.print("Got reply from #"); Serial.print(from);
                        Serial.print(" [RSSI :");
                        Serial.print(rf69.lastRssi());
                        Serial.print("] : ");
                        Serial.println((char*)buf);

                        if (strstr((char*)buf, checksum_ok)) {
                            // Checksum OK
                            responseReceived = true;
                        } else {
                            Serial.println((char*) buf);
                        }

                    } else {
                        Serial.println("No reply, is anyone listening?");
                    }
                } else {
                    Serial.println("Sending failed (no ack)");
                }
            }
        }
    }
}
