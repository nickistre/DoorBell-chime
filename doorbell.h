/*
 Base code and constants for doorbell functions
 */
 
#ifndef __DOORBELL_H__
#define __DOORBELL_H__

// Different types of communications.  See RF24NetworkHeader::type descriptions
#define TYPE_ECHO 0         // Echos back what was sent to it
#define TYPE_BUTTON_PRESS 1 // No payload needed, announces a button was pressed

#endif // __DOORBELL_H__

