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

#define RF69_FREQ     433.0
#define RFM69_CS      8
#define RFM69_RST     4

#if defined (__AVR_ATmega32U4__) // Feather 32u4 w/Radio
#define RFM69_INT     7
#else
#define RFM69_INT     3
#endif

// 20 bytes max length means at maximum 2 bytes for checksum and 18 for message itself
#define MAX_MESSAGE_LENGTH 20

RH_RF69 rf69(RFM69_CS, RFM69_INT);
RHMesh rf69_manager(rf69, REPEATER1_ADDRESS);

int16_t packetnum = 0;  // packet counter, we increment per xmission
uint8_t checksum_ok[] = "CHK_OK";
uint8_t checksum_error[] = "CHK_ERROR";
uint8_t buf[RH_RF69_MAX_MESSAGE_LEN]; // Dont put this on the stack:

void setup(){
    Serial.begin(115200);
    //while (!Serial) { delay(1); } // wait until serial console is open, remove if not tethered to computer

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
    Serial.println("RFM69 radio init OK!");

    // Manually define the routes for this network
    rf69_manager.addRouteTo(CLIENT_ADDRESS, CLIENT_ADDRESS);
    rf69_manager.addRouteTo(MASK1_ADDRESS, MASK1_ADDRESS);
    rf69_manager.addRouteTo(MASK2_ADDRESS, MASK2_ADDRESS);
    rf69_manager.addRouteTo(MASK3_ADDRESS, MASK3_ADDRESS);
    rf69_manager.addRouteTo(MASK4_ADDRESS, MASK4_ADDRESS);
    rf69_manager.addRouteTo(BASE1_ADDRESS, BASE1_ADDRESS);
    rf69_manager.addRouteTo(BASE2_ADDRESS, BASE2_ADDRESS);
    rf69_manager.addRouteTo(BASE3_ADDRESS, BASE3_ADDRESS);
    rf69_manager.addRouteTo(BASE4_ADDRESS, BASE4_ADDRESS);    

    // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM (for low power module)
    // No encryption
    if (!rf69.setFrequency(RF69_FREQ)) {
        Serial.println("setFrequency failed");
    }

    // If you are using a high power RF69 eg RFM69HW, you *must* set a Tx power with the
    // ishighpowermodule flag set like this:
    rf69.setTxPower(20, true);  // range from 14-20 for power, 2nd arg must be true for 69HCW

    // The encryption key has to be the same on both ends
    uint8_t key[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    rf69.setEncryptionKey(key);
}

void loop() {
    
}
