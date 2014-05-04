#include <RF24.h>
#include <RF24_config.h>
#include <nRF24L01.h>

#include <RF24Network.h>
#include <RF24Network_config.h>
#include <Sync.h>
#include <SPI.h>

#include "nodeconfig.h"
#include "printf.h"
#include "doorbell.h"

/*
 DoorBell Chime
 */
 

#ifdef VERSION_H
#include "version.h"
#else
#define __TAG__ "Unknown"
#endif

const uint8_t radio_cepin = 9; // Pin attached to Chip Enable on RF module
const uint8_t radio_cspin = 10; // Pin attached to Chip Select on RF module

const int radioChannel = 100;  // The radio channel to use
#define serialSpeed 57600 // The serial port speed

// nRF24L01(+) radio using the Getting Started board
RF24 radio(radio_cepin, radio_cspin);
RF24Network network(radio);

// Our node address
uint16_t this_node;

void setup() {
  //
  // Print preamble
  //

  Serial.begin(serialSpeed);
  printf_begin();
  printf_P(PSTR("\r\nDoorBell Button Node\r\n"));
  printf_P(PSTR("VERSION: " __TAG__ "\r\n"));
  printf_P(PSTR("Send 'HELP' via serial to get a list of available commands\r\n"));
  
  //
  // Pull node out of eeprom
  //
  
  // Which node are we?
  if (!nodeconfig_exists()) {
    printf_P(PSTR("Node address not configured!  Use help to see about setting up address\r\n"));
    // Just look through checking serial
    while(1) {
      checkSerial();
    }
  }
  
  this_node = nodeconfig_read();
  
  //
  // Bring up the RF network
  //
  
  SPI.begin();
  radio.begin();
  network.begin(radioChannel, /*node address*/ this_node);
}

void loop(void)
{
  // Pump the network regularly
  network.update();
  checkNetwork(); 
  checkSerial();
}


void checkNetwork() {
  // Is there anything ready for us?
  while (network.available()) {
    printf_P(PSTR("Received network message..."));
    // If so, grab it and print it out.
    RF24NetworkHeader header;
    // Check what type of message it is.
    network.peek(header);
    switch (header.type) {
      case TYPE_ECHO:
        handleEcho(header);
        break;
        
      case TYPE_BUTTON_PRESS:
        handleButtonPress(header);
        break;
        
      default:
        unhandledMessageType(header);
        break;
    }
  }
}

void handleEcho(RF24NetworkHeader& header) {
  // TODO: Stub, finish function
}

void handleButtonPress(RF24NetworkHeader& header) {
  
}

void unhandledMessageType(RF24NetworkHeader& header) {
  printf_P(PSTR("Unhandled message type: %i\r\n"), header.type);
  printf_P(PSTR("Header details: \r\n%s\r\n"), header.toString());
}
