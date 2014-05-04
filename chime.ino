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


void needReset() {
  printf_P(PSTR("Press RESET to continue!\r\n"));
  while(1) {
    checkSerial();
  }
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

const unsigned int serial_buffer_length = 255;
char buffer[serial_buffer_length];  

void loadBufferLine()
{
  int bytesRead = Serial.readBytesUntil('\n', buffer, serial_buffer_length-1);
  buffer[bytesRead] = char(0); // Make sure null terminator is setup.  
}

void checkSerial() {
  // Check serial connection for any new commands
  if (Serial.available() > 0) {
    loadBufferLine();
    String command = String(buffer);
    command.trim();
    
    // Check for which command was run and handle this command
    if (command.equals("SET NODE ADDRESS")) {
      handleUpdateNodeAddress();
    }
    else if (command.equals("HELP")) {
      showHelpInfo();
    }
  }
}

void showHelpInfo() {
  printf_P(PSTR("The following commands are avaiable: \r\n"));
  
  printf_P(PSTR(" HELP\r\n"));
  printf_P(PSTR(" SET NODE ADDRESS\r\n"));
  
  printf_P(PSTR("\r\n"));
}

void handleUpdateNodeAddress() {
  // Wait until serial data is available.
  printf_P(PSTR("CHANGE NODE ADDRESS\r\n \r\n"));
  
  printf_P(PSTR("Each node must be assigned an 15-bit address by the administrator.\r\n"));
  printf_P(PSTR("This address exactly describes the position of the node within the\r\n"));
  printf_P(PSTR("tree. The address is an octal number. Each digit in the address represents a\r\n"));
  printf_P(PSTR("position in the tree further from the base.\r\n"));
  printf_P(PSTR("\r\n"));
  printf_P(PSTR("- Node 00 is the base node.\r\n"));
  printf_P(PSTR("- Nodes 01-05 are nodes whose parent is the base.\r\n"));
  printf_P(PSTR("- Node 021 is the second child of node 01.\r\n"));
  printf_P(PSTR("- Node 0321 is the third child of node 021, an so on.\r\n"));
  printf_P(PSTR("- The largest node address is 05555, so 3,125 nodes are allowed on a single channel.\r\n"));
  printf_P(PSTR("\r\n"));  
  printf_P(PSTR("Please type in the node address (from 0 to 65535)\r\n: "));
  while (Serial.available() == 0) {
    // Do nothing!
  }
  
  loadBufferLine();
  uint16_t new_node = strtoul(buffer, NULL, 8);
  printf_P(PSTR("\r\nAddress entered: %s\r\n"), buffer);
  printf_P(PSTR("Decimal address detected: %i\r\n"), new_node);
  printf_P(PSTR("Address as octal: 0%o\r\n"), new_node);
  printf_P(PSTR("Setting node address in eeprom\r\n"));
  
  // Do update of address
  nodeconfig_update(new_node);
  
  needReset();
}
