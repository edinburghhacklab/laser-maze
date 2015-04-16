/*
 *
 */
 
#include <SoftwareSerial.h>

// Pin defs
const byte laser_broken   = 6;
const byte laser_enabled  = 5;
const byte bus_tx         = 9;
const byte bus_rx         = 8;
const byte bus_rf         = 7;
const byte bus_de         = 16;

// Constants
const byte my_address     = 69;

typedef enum state
{
  MODE_TEST,
  MODE_NORMAL
};

typedef enum command
{
  SEND_STATE = 1
};

typedef enum dir
{
  RECEIVE,
  TRANSMIT
};

SoftwareSerial bus(bus_rx, bus_tx);
state mode = MODE_NORMAL;
byte input_address = 0;
byte input_command = 0;
byte laser_state;


void set_bus_direction(byte value)
{
  if(TRANSMIT== value)
  {
    digitalWrite(bus_de, HIGH);
    digitalWrite(bus_rf, HIGH);
  }
  else if(RECEIVE == value)
  {
    digitalWrite(bus_de, LOW);
    digitalWrite(bus_rf, LOW);
  }
}


void setup()
{
  pinMode(laser_enabled, OUTPUT);
  pinMode(bus_rf,        OUTPUT);
  pinMode(bus_de,        OUTPUT);
  set_bus_direction(RECEIVE);
  bus.begin(9600);
  
  Serial.begin(57600);
  
  while (!Serial)
  {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  
  Serial.println("LAZORS!!");
  laser_state = HIGH;
}

// the loop function runs over and over again forever
void loop()
{
  
          
  laser_state = !digitalRead(laser_broken);
  
  
  if(bus.available())
  {
    if(0 == input_address)
    {
      input_address = bus.read();
      //Serial.print("Input address: ");
      Serial.println(input_address);
    }
    else
    {
      input_command = bus.read();
      //Serial.print("Input command: ");
      Serial.println(input_command);
    }
    
    // We have an address and a command
    if((input_address > 0) && (input_command > 0))
    {
      if(input_address == my_address)
      {
        if(SEND_STATE == input_command)
        {
          //Serial.println("Command: send state");
          delay(200);
          set_bus_direction(TRANSMIT);
          bus.write('b');
          set_bus_direction(RECEIVE);
        }
      }
      
      input_address = 0;
      input_command = 0;
    }
  }
}
