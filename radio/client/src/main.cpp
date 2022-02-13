#include "Arduino.h"
#include <SPI.h>
#include <RH_RF69.h>
#include <RHMesh.h>

//RADIO
#define CLIENT_ADDRESS      1
#define REPEATER1_ADDRESS   2
#define REPEATER2_ADDRESS   3
#define BASE1_ADDRESS       4
#define BASE2_ADDRESS       5
#define BASE3_ADDRESS       6
#define BASE4_ADDRESS       7
// MASKS WILL START AT 10 TO HAVE ROOM FOR MORE REPEATERS/BASES
#define MASK1_ADDRESS       10
#define MASK2_ADDRESS       11
#define MASK3_ADDRESS       12
#define MASK4_ADDRESS       13

#define RF69_FREQ           433.0
#define RFM69_CS            8
#define RFM69_RST           4

#if defined (__AVR_ATmega32U4__) // Feather 32u4 w/Radio
#define RFM69_INT           7
#else
#define RFM69_INT           3
#endif

#define LED                 13

// 20 bytes max length means at maximum 2 bytes for checksum and 18 for message itself
#define MAX_MESSAGE_LENGTH  20

RH_RF69 rf69(RFM69_CS, RFM69_INT);
RHMesh rf69_manager(rf69, CLIENT_ADDRESS);

int16_t packetnum = 0;  // packet counter, we increment per xmission
char checksum_ok[] = "CHK_OK";
uint8_t buf[RH_RF69_MAX_MESSAGE_LEN];

void setup()
{
    Serial.begin(115200);
    // while (!Serial) { delay(1); } // wait until serial console is open, remove if not tethered to computer

    pinMode(LED, OUTPUT);
    digitalWrite(LED, HIGH);
    pinMode(RFM69_RST, OUTPUT);
    digitalWrite(RFM69_RST, LOW);

    // manual reset
    digitalWrite(RFM69_RST, HIGH);
    delay(10);
    digitalWrite(RFM69_RST, LOW);
    delay(10);

    if (!rf69_manager.init()) {
        Serial.println("RFM69 radio init failed");
        while (1);
    }

    // Manually define the routes for this network
    rf69_manager.addRouteTo(REPEATER1_ADDRESS, REPEATER1_ADDRESS);
    rf69_manager.addRouteTo(REPEATER2_ADDRESS, REPEATER2_ADDRESS);
    rf69_manager.addRouteTo(BASE1_ADDRESS, BASE1_ADDRESS);
    rf69_manager.addRouteTo(BASE2_ADDRESS, BASE2_ADDRESS);
    rf69_manager.addRouteTo(BASE3_ADDRESS, BASE3_ADDRESS);
    rf69_manager.addRouteTo(BASE4_ADDRESS, BASE4_ADDRESS);
    // rf69_manager.addRouteTo(MASK1_ADDRESS, MASK1_ADDRESS);
    // rf69_manager.addRouteTo(MASK2_ADDRESS, MASK2_ADDRESS);
    // rf69_manager.addRouteTo(MASK3_ADDRESS, MASK3_ADDRESS);
    // rf69_manager.addRouteTo(MASK4_ADDRESS, MASK4_ADDRESS);

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

    // Serial.print("RFM69 radio @");  Serial.print((int)RF69_FREQ);  Serial.println(" MHz");
}


void loop() {
  
    static char message[MAX_MESSAGE_LENGTH];
    static unsigned int message_pos = 0;
    digitalWrite(LED, HIGH);

    while (Serial.available() > 0) {

        // read the incoming byte:
        byte inByte = Serial.read();
        if (inByte != '\n') {

            message[message_pos] = inByte;
            message_pos++;

        } else {
            digitalWrite(LED, LOW);

            message[message_pos] = '\0';
            message_pos = 0;

            Serial.println("Got new message to send");

            Serial.println(message);
            char d1 = message[0];
            char d2 = message[1];
            char d[2] = {d1, d2};
            int destination = String(d).toInt();
            Serial.print("Destination: ");
            Serial.println(destination);
            
            char packet[MAX_MESSAGE_LENGTH]{};
            strcpy(packet, &message[2]);
            int packet_length = strlen(packet);

            Serial.print("Packet: ");
            Serial.println(packet);

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

                if (rf69_manager.sendtoWait((uint8_t *)packet, strlen(packet), destination) == RH_ROUTER_ERROR_NONE) {
                    digitalWrite(LED, HIGH);
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
