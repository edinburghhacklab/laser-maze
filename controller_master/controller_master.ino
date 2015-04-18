const int tx_enable_pin = 4;
const int max_node = 20;

byte node = 0;
byte command = 0;
int current_node = 1;
unsigned long cycle_start;

void setup() {
  pinMode(tx_enable_pin, OUTPUT);
  digitalWrite(tx_enable_pin, LOW);

  // initialize USB serial  
  Serial.begin(57600);
  while (!Serial);

  // initialize RS485 serial
  Serial1.begin(9600);
  Serial1.flush();
  
  Serial.println("ready");
  
  // track the length of a full cycle over all of the nodes
  cycle_start = millis();
}

void loop() {
  int response;
  
  if (Serial.available() > 0) {
    if (node==0) {
      node = Serial.read();
    } else {
      // we have a received a node address already, and it wasn't zero
      if (Serial.available() > 0) {
        command = Serial.read();
      }
      if (command!=0) {
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

  unsigned long start = millis();
  response = send_and_receive(current_node, 1);
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

}

int send_and_receive(byte node, byte command) {
  int response;
  
  digitalWrite(tx_enable_pin, HIGH);
  Serial1.write(0);
  Serial1.write(current_node);
  Serial1.write(1);
  Serial1.flush();
  digitalWrite(tx_enable_pin, LOW);

  if (node!=255) {
    response = receive_byte();
    return response;
  } else {
    // no response to a broadcast
    return -1;
  }
  
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

