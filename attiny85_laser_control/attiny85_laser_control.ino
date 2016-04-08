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
#define LASER_TEST_PIN        1              // attiny pin 6
#define LASER_OUTPUT          0 // IR sensor - ATtiny Pin 5 

#define LASER_ON       1
#define LASER_BROKEN   2
#define LASER_OFF      3

#define BEAM_BREAK_MIN    20 // ms

//volatile unsigned long lastRiseTime = 0;
volatile long lastFallTime = 0;
/*volatile*/ int laserState = LASER_OFF;
volatile long lastGoodTime = 0;

void setup(){
  
  pinMode(LASER_OUTPUT,OUTPUT);
  pinMode(LASER_ENABLE_PIN, INPUT_PULLUP);
  pinMode(LASER_TEST_PIN, INPUT_PULLUP);  
  pinMode(SENSOR_PIN,INPUT_PULLUP);
  pinMode(LASER_BROKEN_OUT_PIN, OUTPUT);           
  // switch laser off
  //digitalWrite(LASER_OUTPUT, LOW);
  //TCCR0A |= (1<<COM0A1) | (1<<WGM00); // PWM Phase correct mode 1

TCCR0A = 3<<COM0A0 | 3<<WGM00;
TCCR0B = 0<<WGM02 | 3<<CS00;

laserOff();
// TCCR0B = 0<<WGM02 | 3<<CS00; // Optional; already set

//  laserOn();
  //analogWrite(LASER_OUTPUT,255*0.90);
  // Catch change of laser enable pin on attiny
  
  //delay(500);
  //lastRiseTime= micros();

//attachInterrupt(SENSOR_PIN, interrupt, RISING);
  attachPcInterrupt(SENSOR_PIN,laser_detect_isr,FALLING);
  //delay(100);
  laserState = LASER_OFF;
}

void laserOn() {
  OCR0A = 255 - (255*0.85);    
}
void laserOff() {
  OCR0A = 255 ;    
}

void loop(){
  
  long currentMillis = millis();

  if (currentMillis - lastGoodTime > 50 && laserState == LASER_ON) {
    laserState = LASER_BROKEN;
    laserOff();
    //digitalWrite(LASER_OUTPUT, LOW);
    digitalWrite(LASER_BROKEN_OUT_PIN, HIGH);
    delay(100);
    digitalWrite(LASER_BROKEN_OUT_PIN, LOW);
    delay(10);
    laserState = LASER_OFF;
  }

  if (digitalRead(LASER_ENABLE_PIN) == LOW && digitalRead(LASER_TEST_PIN) == LOW) {
    // we are in test mode switch laser on
    laserState = LASER_ON;
    lastGoodTime=millis();
    laserOn();
    //delay(10);
  }
  
  
  if (digitalRead(LASER_ENABLE_PIN) == LOW) { 
    if (laserState == LASER_OFF) {
      // switch laser on
      laserState = LASER_ON;
      lastGoodTime=millis();
      laserOn();
    }
  } else {
   // arduino switched us off
   laserOff();
   laserState = LASER_OFF;
  }
  
 
}

void laser_detect_isr(){ 
  // This ISR is called for EACH change of the laser enable pin
  long currentMillis = millis(); 
  if ((currentMillis - lastFallTime > 10) && (currentMillis - lastFallTime < 20)) {
    lastGoodTime = currentMillis;
  }
  lastFallTime = currentMillis;

}

