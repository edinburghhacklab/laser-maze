/* ATtiny85 IR Receiver Test               BroHogan                        2/11/11
 * Tests IR receiver (using pin change interrupt) while I2C LC display is used. 
 * SETUP:
 * ATtiny Pin 1 = (RESET) N/U                      ATtiny Pin 2 = (PB3) IR OUTPUT PIN
 * ATtiny Pin 3 = (PB4) to LED1                    ATtiny Pin 4 = GND
 * ATtiny Pin 5 = PB0/SDA on GPIO                  ATtiny Pin 6 = (PB1) N/U
 * ATtiny Pin 7 = PB2/INT0/SCK on GPIO             ATtiny Pin 8 = VCC (2.7-5.5V)
 */

#include <PinChangeInterrupt.h>         // mimics attachInterrupt() but a PCI for ATtiny

#define LASER_ENABLE_PIN      4 
#define SENSOR_PIN            3              // ATtiny Pin 2
#define LASER_BROKEN_OUT_PIN  2              // attiny pin 7
#define LASER_TEST_PIN        1              // attiny pin 1
#define LASER_OUTPUT          0 // IR sensor - ATtiny Pin 5 

#define LASER_ON       1
#define LASER_BROKEN   2
#define LASER_OFF      3

#define BEAM_BREAK_MIN    20 // ms

volatile unsigned long lastRiseTime = 0;
volatile unsigned long lastFallTime = 0;
volatile int laserState = 0;

void setup(){
  pinMode(LASER_OUTPUT,OUTPUT);
  pinMode(LASER_ENABLE_PIN, INPUT);
  pinMode(LASER_TEST_PIN, INPUT);  
  pinMode(SENSOR_PIN,INPUT);
  pinMode(LASER_BROKEN_OUT_PIN, OUTPUT);           
    // switch laser off
  digitalWrite(LASER_OUTPUT, LOW);
  
  // Catch change of laser enable pin on attiny
  attachPcInterrupt(SENSOR_PIN,laser_detect_isr,CHANGE);
}

void loop(){
  
  unsigned long currentMillis = millis();  
  
  if (digitalRead(LASER_ENABLE_PIN) == HIGH && digitalRead(LASER_TEST_PIN) == HIGH) {
    laserState = LASER_ON;
    digitalWrite(LASER_OUTPUT, HIGH);
    lastFallTime = currentMillis;
  }
  
  if (digitalRead(LASER_ENABLE_PIN) == HIGH) { 
    if (laserState == LASER_OFF) {
      // switch laser on
      laserState = LASER_ON;
      lastFallTime = currentMillis;
      digitalWrite(LASER_OUTPUT, HIGH);
    }

    if(digitalRead(SENSOR_PIN) == LOW && (currentMillis > lastFallTime + BEAM_BREAK_MIN)) {
       // sensor pin, has been off for longer than BEAM_BREAK_MIN 
       digitalWrite(LASER_OUTPUT, LOW);
       digitalWrite(LASER_BROKEN_OUT_PIN, HIGH);
       laserState = LASER_BROKEN;
       delay(10);
       digitalWrite(LASER_BROKEN_OUT_PIN, LOW);
       laserState = LASER_OFF;
    }
  } else {
    // arduino switched us off
    digitalWrite(LASER_OUTPUT, LOW);
    laserState = LASER_OFF;
    delay(10);
  }
}

void laser_detect_isr(){ 
  // This ISR is called for EACH change of the laser enable pin
  int detectPinState = digitalRead(SENSOR_PIN);
  unsigned long currentMillis = millis();
  if (HIGH == detectPinState) {
    // RISING
    lastRiseTime = currentMillis;
  } else if (LOW == detectPinState){
    // FALLING
    if(digitalRead(LASER_TEST_PIN) == LOW) {
      if (currentMillis > lastFallTime + 20) { 
        // last fall time > 20ms ago, we are in low, switch off LASER 
        digitalWrite(LASER_OUTPUT, LOW);
        laserState = LASER_BROKEN;
      }
    }
    lastFallTime = currentMillis; 
  }
}


