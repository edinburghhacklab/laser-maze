/*
 *
 */
#include "RS485_protocol.h"
#include <SoftwareSerial.h>
#include <EEPROM.h>

// Pin defs
const byte laser_broken   = 6;      // From safety processor (HIGH=beam-broken)
const byte laser_enable   = 5;      // Tell safety to enable laser
const byte laser_override = 4;      // Tell safety to enter test mode
const byte bus_tx         = 9;
const byte bus_rx         = 8;
const byte bus_rf         = 7;
const byte bus_de         = 16;

const byte MASTER_ADDR = 200;

// Enums and constants for states, commands etc
typedef enum state
{
  MODE_TEST          = 1,
  MODE_NORMAL        = 2,
  MODE_NORMAL_BROKEN = 3,
  MODE_OFF           = 4
};

typedef enum command
{
  SEND_STATE       = 1,
  SET_MODE_NORMAL  = 2,
  SET_MODE_OFF     = 3,
  SET_MODE_TEST    = 4,
  RESPONSE_STATE   = 5
};

typedef enum dir
{
  RECEIVE,
  TRANSMIT
};

const byte ACK = 1;
const unsigned long time_allowed_in_test_mode = 5000;

// Globals
byte my_address                     = 88; // temporary until serial read from EEPROM
unsigned long time_entered_test_mode = 0;
state mode = MODE_OFF;
byte input_src_address                  = 0;
byte input_dst_address                  = 0;
byte input_command                  = 0;
byte input_command_arg              = 0;

#define SERIAL_DEBUG 1

SoftwareSerial bus(bus_rx, bus_tx);

void fWrite (const byte what)
  {
  bus.write (what);  
  }
  
int fAvailable ()
  {
  return bus.available ();  
  }

int fRead ()
  {
  return bus.read ();  
  }
  

void set_bus_direction(byte value)
{
  if (TRANSMIT == value)
  {
    digitalWrite(bus_de, HIGH);
    digitalWrite(bus_rf, HIGH);
  }
  else if (RECEIVE == value)
  {
    digitalWrite(bus_de, LOW);
    digitalWrite(bus_rf, LOW);
  }
}


void setup()
{
  pinMode(laser_enable,   OUTPUT);
  pinMode(laser_override, OUTPUT);
  pinMode(bus_rf,         OUTPUT);
  pinMode(bus_de,         OUTPUT);
  
  digitalWrite(laser_enable, HIGH); // laser OFF
  digitalWrite(laser_override, HIGH); // Laser Test mode OFF

  my_address = EEPROM.read(0);

  set_bus_direction(RECEIVE);
  bus.begin(14400);

#ifdef SERIAL_DEBUG
  Serial.begin(57600);
  //while (!Serial)
  //{
    //; // wait for serial port to connect. Needed for Leonardo only
  //}

  Serial.println("LAZORS!!");
  Serial.print("Serial: ");
  Serial.print(my_address);
#endif


}

// the loop function runs over and over again forever
void loop()
{

#ifdef SERIAL_DEBUG  
  if (Serial.available())
  {
    char thing = Serial.read();
    if('n' == thing)
    {
      Serial.println("->normal (console)");
      mode = MODE_NORMAL;
    }
    else if ('t' == thing)
    {
            Serial.println("->test (console)");
      mode = MODE_TEST;
      time_entered_test_mode = millis();
    }
    else if ('o' == thing)
    {
            Serial.println("->off (console)");
      mode = MODE_OFF;
    }
  }
#endif  
  
  
  byte buf [4];
  
  byte received = recvMsg (fAvailable, fRead, buf, sizeof (buf),10);
 
/* Serial.print("Rec = ");
 Serial.print(received);
    Serial.print(" ,");
    Serial.print(buf[0]);
    Serial.print(',');
    Serial.println(buf[1]);
*/
  if (received)
    {
      input_src_address = buf [0];
      input_dst_address = buf [1]; /////
      input_command = buf [2]; 
      input_command_arg = buf [3]; 
    
    // We have an address and a command
    if ((input_dst_address > 0) && (input_command > 0))
    {
      if (input_dst_address == my_address ||  input_dst_address == 255)
      {
        if (SEND_STATE == input_command)
        {
          
        }
        else if (SET_MODE_NORMAL == input_command)
        {
          #ifdef SERIAL_DEBUG
          Serial.println("->normal (bus)");
          #endif
          mode = MODE_NORMAL;
        }
        else if (SET_MODE_TEST == input_command)
        {
          #ifdef SERIAL_DEBUG
          Serial.println("->test (bus)");
          #endif
          mode = MODE_TEST;
          time_entered_test_mode = millis();
        }
        else if (SET_MODE_OFF == input_command)
        {
          #ifdef SERIAL_DEBUG
          Serial.println("->off (bus)");
          #endif
          mode = MODE_OFF;
        }
      }

      // reply with status, but not for broadcasts      
      if (input_dst_address == my_address) {
 
        // assemble message
        byte msg [] = { 
           my_address,        //src
           MASTER_ADDR,    //dest // master device
           RESPONSE_STATE,
           mode  
        };
        #ifdef SERIAL_DEBUG
        Serial.print("sending status ");
        Serial.println(mode);
        #endif
        set_bus_direction(TRANSMIT);
        sendMsg (fWrite, msg, sizeof msg);
        set_bus_direction(RECEIVE);
      }
      
      input_src_address = 0;
      input_dst_address = 0;
      input_command = 0;
      input_command_arg = 0;

    }
  } // if recived


  // Handle operating modes
  if (MODE_TEST == mode)
  {
    // If we have been in test mode too long, drop into normal mode
    if ((millis() - time_entered_test_mode) > time_allowed_in_test_mode)
    {
      #ifdef SERIAL_DEBUG
      Serial.println("test->normal (timeout)");
      #endif
      mode = MODE_NORMAL;
    }
    digitalWrite(laser_override, LOW);
    digitalWrite(laser_enable, LOW);
  }
  else
  {
    digitalWrite(laser_override, HIGH);
  }
  
  if(MODE_NORMAL == mode)
  {
    if(digitalRead(laser_broken))
    {
      #ifdef SERIAL_DEBUG
      Serial.println("normal->broken");
      #endif
      mode = MODE_NORMAL_BROKEN;
    }
    else
    {
      digitalWrite(laser_enable, LOW); // switch laser ON
    }
  }
  
  if(MODE_NORMAL_BROKEN == mode)
  {
    digitalWrite(laser_enable, HIGH); // switch laser OFF
  }
  
  if(MODE_OFF == mode)
  {
    digitalWrite(laser_enable, HIGH); // switch laser OFF
  }
}


