// Use http://www.gammon.com.au/Arduino/RS485_protocol.zip
#include "RS485_protocol.h"


// callback routines
  
void fWrite (const byte what)
  {
  Serial1.write (what);  
  }
  
int fAvailable ()
  {
  return Serial1.available ();  
  }

int fRead ()
  {
  return Serial1.read ();  
  }

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

const int tx_enable_pin = 4;
const int max_node = 20;
const int relay1_pin = 10;
const int relay2_pin = 16;
const int game_btn_pin = A3;

byte node = 0;
byte command = 0;
int current_node = 1;
unsigned long cycle_start;

void setup() {
  pinMode(relay1_pin, OUTPUT);
  pinMode(relay2_pin, OUTPUT);
  pinMode(tx_enable_pin, OUTPUT);
  
  pinMode(game_btn_pin, INPUT_PULLUP);

  digitalWrite(relay1_pin, LOW);
  digitalWrite(relay2_pin, LOW);
  digitalWrite(tx_enable_pin, LOW);

  // initialize USB serial  
  /* Serial.begin(57600); */
  /* while (!Serial); */

  // initialize RS485 serial
  Serial1.begin(14400);
  Serial1.flush();
  
  //Serial.println("ready");
  /*
  // track the length of a full cycle over all of the nodes
  cycle_start = millis();

  for (int i = 2; i < 15; i++) {
      digitalWrite(tx_enable_pin, HIGH);
      Serial1.write(0);
      Serial1.write(255); // broadcast
      Serial1.write(4); // 'set test mode' cmd.
      Serial1.flush();
      digitalWrite(tx_enable_pin, LOW);
  }
*/
}

void loop() {

  int response;

  // assemble message
  byte msg [] = { 
     200,
     current_node,    // device
     1,    // command, 1 get status, 2 = normal, 3 off, test 4,
     0 // command arg 0
  };


  // send to slave  
  //Serial.println("sending staus request");
  
  digitalWrite (tx_enable_pin, HIGH);  // enable sending
  sendMsg (fWrite, msg, sizeof msg);
  Serial1.flush();
  //delay (1);
  digitalWrite (tx_enable_pin, LOW);  // disable sending
  
  
  // receive response  
  byte buf [4];
  byte received = recvMsg (fAvailable, fRead, buf, sizeof buf, 20);
  
  if (received)
    {
      Serial.print("response = ");
      Serial.print(buf[0]); //src
      Serial.print(" ,");
      Serial.print(buf[1]); // dst
      Serial.print(" ,");
      Serial.print(buf[2]); // comand
      Serial.print(" ,");
      Serial.println(buf[3]); // command arg
      if (buf[2] == RESPONSE_STATE && buf[3] == MODE_NORMAL_BROKEN)  
      {
        digitalWrite(relay1_pin, HIGH);
        delay(2000);
        digitalWrite(relay1_pin, LOW);
        send_command(buf[0],SET_MODE_OFF);
      }

    }

  

  delay(20);

  // assemble message
  byte msg2 [] = { 
     200, // src address
     current_node,    // device
     4,    // command, 1 get status, 2 = normal, test 4
     0 // arg
  };


  // send to slave  
  //Serial.println("sending Switch laser on command request");
  
  if (!digitalRead(game_btn_pin)) // spin
  {
    digitalWrite (tx_enable_pin, HIGH);  // enable sending
    sendMsg (fWrite, msg2, sizeof msg2);
    Serial1.flush();
    delay (1);
    digitalWrite (tx_enable_pin, LOW);  // disable sending
    delay(15);
  }

  current_node++;
  if (current_node > max_node) {
    current_node = 1;
    //Serial.println(millis()-cycle_start);
    cycle_start = millis();

    
  }
  //delay(15);
  

  //delay(500);
  //digitalWrite(relay1_pin, !digitalRead(relay1_pin));
//}
  
  return;
  
 
  if (Serial.available() > 0) {
    if (node==0) {
      node = Serial.read();
    } else {
      // we have a received a node address already, and it wasn't zero
      if (Serial.available() > 0) {
        command = Serial.read();
      }
      if (command!=0) {
          if (node == 253) {
              digitalWrite(relay1_pin, command == '0'?LOW:HIGH);
              Serial.print("relay1 ");
              Serial.println(digitalRead(relay1_pin));
              node = 0;
              command = 0;
              
          } else if (node == 254) {
              digitalWrite(relay2_pin, command == '0'?LOW:HIGH);
              Serial.print("relay2 ");
              Serial.println(digitalRead(relay2_pin));
              node = 0;
              command = 0;
          } else {
              Serial.print("send ");
              Serial.print(node);
              Serial.print(" ");
              Serial.println(command);
              digitalWrite(tx_enable_pin, HIGH);
              Serial1.write(0);
              Serial1.write(node);
              Serial1.write(command);
              Serial1.flush();  
              digitalWrite(tx_enable_pin, LOW);
              node = 0;
              command = 0;
              response = receive_byte();
              Serial.print("reply ");
              Serial.println(response, DEC);
          }
      }
    }
  }

  unsigned long start = millis();
  //response = send_and_receive(current_node, 1);
  if (response!=-1) {
    //Serial.println(millis()-start);
    Serial.print("qreply ");
    Serial.print(current_node);
    Serial.print(" ");
    Serial.println(response, DEC);
  }
  
  current_node++;
  if (current_node > max_node) {
    current_node = 1;
    //Serial.println(millis()-cycle_start);
    cycle_start = millis();
  }
  delay(1000);
}

int send_command(byte node, byte command) {
  int response;
 
  // assemble message
  byte msg [] = { 
     200,
     node,    // device
     command,    // command, 1 get status, 2 = normal, 3 off, test 4,
     0 // command arg 0
  };

  digitalWrite (tx_enable_pin, HIGH);  // enable sending
  sendMsg (fWrite, msg, sizeof msg);
  Serial1.flush();
  digitalWrite (tx_enable_pin, LOW);  // disable sending
  delay(10);
  return 0;
  
}

int receive_byte() {
  unsigned long start;
  int response = -1;

  // keep trying to read a byte until the response is not -1 or 0
  // or until we run out of time
  start = millis();
  while (millis()-start < 5 && (response==-1 || response==0)) {
    response = Serial1.read();
  }
  
  // always return -1 for invalid, or a postive number
  if (response==0) {
    response = -1;
  }
  
  return response;
}


