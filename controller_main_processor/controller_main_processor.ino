/*
 *
 */

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
  SET_MODE_TEST    = 4
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
byte input_address                  = 0;
byte input_command                  = 0;

SoftwareSerial bus(bus_rx, bus_tx);

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
  
  digitalWrite(laser_enable, LOW);
  digitalWrite(laser_override, LOW);

  my_address = EEPROM.read(0);

  set_bus_direction(RECEIVE);
  bus.begin(9600);

  Serial.begin(57600);
  //while (!Serial)
  //{
    //; // wait for serial port to connect. Needed for Leonardo only
  //}

  Serial.println("LAZORS!!");
  Serial.print("Serial: ");
  Serial.print(my_address);
}

// the loop function runs over and over again forever
void loop()
{
  
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
  
  
  // Handle any messages from the bus
  if (bus.available())
  {
    if (0 == input_address)
    {
      input_address = bus.read();
      //Serial.print("Input address: ");
      //Serial.print(input_address);
    }
    else
    {
      input_command = bus.read();
      if (0 == input_command)     // reset on zeros
      {
        input_address = 0;
      }
      else
      {
        //Serial.print("Input command: ");
        //Serial.println(input_command);
      }
    }

    // We have an address and a command
    if ((input_address > 0) && (input_command > 0))
    {
      if (input_address == my_address || input_address == 255)
      {
        if (SEND_STATE == input_command)
        {
          
        }
        else if (SET_MODE_NORMAL == input_command)
        {
          Serial.println("->normal (bus)");
          mode = MODE_NORMAL;
        }
        else if (SET_MODE_TEST == input_command)
        {
          Serial.println("->test (bus)");
          mode = MODE_TEST;
          time_entered_test_mode = millis();
        }
        else if (SET_MODE_OFF == input_command)
        {
          Serial.println("->off (bus)");
          mode = MODE_OFF;
        }
      }

      // reply with status, but not for broadcasts      
      if (input_address == my_address) {
        set_bus_direction(TRANSMIT);
        bus.write(mode);
        set_bus_direction(RECEIVE);
      }
      
      input_address = 0;
      input_command = 0;
    }
  }


  // Handle operating modes
  if (MODE_TEST == mode)
  {
    // If we have been in test mode too long, drop into normal mode
    if ((millis() - time_entered_test_mode) > time_allowed_in_test_mode)
    {
      Serial.println("test->normal (timeout)");
      mode = MODE_NORMAL;
    }
    digitalWrite(laser_override, HIGH);
    digitalWrite(laser_enable, HIGH);
  }
  else
  {
    digitalWrite(laser_override, LOW);
  }
  
  if(MODE_NORMAL == mode)
  {
    if(digitalRead(laser_broken))
    {
      Serial.println("normal->broken");
      mode = MODE_NORMAL_BROKEN;
    }
    else
    {
      digitalWrite(laser_enable, HIGH);
    }
  }
  
  if(MODE_NORMAL_BROKEN == mode)
  {
    digitalWrite(laser_enable, LOW);
  }
  
  if(MODE_OFF == mode)
  {
    digitalWrite(laser_enable, LOW);
  }
}

