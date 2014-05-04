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

// button Nodes to expect certain chime types from.
const uint16_t chime_1_node = 01;
const uint16_t chime_2_node = 02;

const uint8_t chime_1_pin = 2;
const uint8_t chime_2_pin = 4;

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
  printf_P(PSTR("\r\nDoorBell Chime Node\r\n"));
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
  
  // Setup Chime pin relays
  pinMode(chime_1_pin, OUTPUT);
  pinMode(chime_2_pin, OUTPUT);
  
  digitalWrite(chime_1_pin, LOW);
  digitalWrite(chime_2_pin, LOW);
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
    printf_P(PSTR("Received network message...\r\n"));
    // If so, grab it and print it out.
    RF24NetworkHeader header;
    // Check what type of message it is.
    network.peek(header);
    printf_P(PSTR("Header details: \r\n%s\r\n"), header.toString());
    printf_P(PSTR("Message Type: %i \r\n"), header.type);
    switch (header.type) {
      case TYPE_ECHO:
        handleEcho();
        break;
        
      case TYPE_BUTTON_PRESS:
        handleButtonPress();
        break;
        
      default:
        unhandledMessageType();
        break;
    }
  }
}

void handleEcho() {
  RF24NetworkHeader header;
  payload_echo payload;
  network.read(header, &payload, sizeof(payload));
  // TODO: Stub, finish function
}

void handleButtonPress() {
  RF24NetworkHeader header;
  payload_button_press payload;
  network.read(header, &payload, sizeof(payload));
  printf_P(PSTR("Recieved button press message from node: %i\r\n"), header.from_node);
  
  switch (header.from_node) {
    case chime_1_node:
      printf_P(PSTR("Ring chime 1\r\n"));
      digitalWrite(chime_1_pin, HIGH);
      delay(500);
      digitalWrite(chime_1_pin, LOW);
      break;
      
    case chime_2_node:
      printf_P(PSTR("Ring chime 2\r\n"));
      digitalWrite(chime_2_pin, HIGH);
      delay(500);
      digitalWrite(chime_2_pin, LOW);
      break;
      
    default:
      printf_P(PSTR("Unhandled button node %i\r\n"), header.from_node);
      // What do we do here?
      break;
      
  }
  
}

void unhandledMessageType() {
  RF24NetworkHeader header;
  char payload[255];
  network.read(header, &payload, 255);
  printf_P(PSTR("Unhandled message type: %i\r\n"), header.type);
  printf_P(PSTR("First 255 chars of payload: \r\n%s\r\n"), payload);
}
