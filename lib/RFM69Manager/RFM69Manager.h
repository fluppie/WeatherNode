/*

Radio

RFM69 Radio Manager for ESP8266
Based on sample code by Felix Rusu - http://LowPowerLab.com/contact
Copyright (C) 2016 by Xose Pérez <xose dot perez at gmail dot com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef RFM69Manager_h
#define RFM69Manager_h

#include <RFM69.h>
#include <RFM69_ATC.h>
#include <SPI.h>

// -----------------------------------------------------------------------------
// Configuration
// -----------------------------------------------------------------------------

#define PING_EVERY          3
#define RETRIES             2
#define REQUESTACK          1
#define RADIO_DEBUG         1
#define SEND_PACKET_ID      1

#define RF69_SPI_CS             SS // SS is the SPI slave select pin, for instance D10 on ATmega328

// INT0 on AVRs should be connected to RFM69's DIO0 (ex on ATmega328 it's D2, on ATmega644/1284 it's D2)
#if defined(__AVR_ATmega168__) || defined(__AVR_ATmega328P__) || defined(__AVR_ATmega88) || defined(__AVR_ATmega8__) || defined(__AVR_ATmega88__)
  #define RF69_IRQ_PIN          2
  #define RF69_IRQ_NUM          0
#elif defined(__AVR_ATmega644P__) || defined(__AVR_ATmega1284P__)
  #define RF69_IRQ_PIN          2
  #define RF69_IRQ_NUM          2
#elif defined(__AVR_ATmega32U4__)
  #define RF69_IRQ_PIN          3
  #define RF69_IRQ_NUM          0
#elif defined(__arm__)//Use pin 10 or any pin you want
  #define RF69_IRQ_PIN          10
  #define RF69_IRQ_NUM          10
#else
  #define RF69_IRQ_PIN          2
  #define RF69_IRQ_NUM          0
#endif

typedef struct {
    unsigned long messageID;
    unsigned char packetID;
    unsigned char senderID;
    unsigned char targetID;
    char * name;
    char * value;
    int16_t rssi;
} packet_t;

typedef void (*TMessageCallback)(packet_t *);

class RFM69Manager: public RFM69_ATC {

    public:

        RFM69Manager(uint8_t slaveSelectPin=RF69_SPI_CS, uint8_t interruptPin=RF69_IRQ_PIN, bool isRFM69HW=false, uint8_t interruptNum=RF69_IRQ_NUM):
            RFM69_ATC(slaveSelectPin, interruptPin, isRFM69HW, interruptNum) {};

        bool initialize(uint8_t frequency, uint8_t nodeID, uint8_t networkID, const char* key, uint8_t gatewayID = 0, int16_t targetRSSI = -70);
        void onMessage(TMessageCallback fn);
        bool send(uint8_t destinationID, char * name, char * value, uint8_t retries = RETRIES, bool requestACK = REQUESTACK);
        bool send(char * name, char * value, uint8_t retries = RETRIES) {
            return send(_gatewayID, name, value, retries, false);
        }
        bool send(char * name, char * value, bool requestACK = REQUESTACK) {
            return send(_gatewayID, name, value, 0, requestACK);
        }
        bool loop();
        void promiscuous(bool promiscuous);
        packet_t * getMessage() {
            return &_message;
        }

    private:

        packet_t _message;
        TMessageCallback _callback = NULL;
        uint8_t _gatewayID = 0;
        unsigned long _receiveCount = 0;
        #if SEND_PACKET_ID
            unsigned char _sendCount = 0;
        #endif
        unsigned int _ackCount = 0;

};

#endif
