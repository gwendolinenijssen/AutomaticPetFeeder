#include <ESP32Servo.h>
#define IR1_PIN 2
#define SERVO_PIN 15
#define BUTTON_PIN 13

/* Information about servo */
Servo servo;
int angleStep = 5;
int angleMin = 0;
int angleMax = 180;
int droppingTime = 1000; // specified in millisecond

/* Information about button/debouncing the button */
int buttonState = LOW; 
long lastDebounceTime = 0;  
long debounceDelay = 200;  

void setup() {
//    pinMode(IR1_PIN,INPUT);
    pinMode(BUTTON_PIN, INPUT_PULLDOWN);
    Serial.begin(115200);
    servo.attach(SERVO_PIN);
}

void loop() {
    /* Checks if button was pressed and handles debouncing. */
    buttonState = digitalRead(BUTTON_PIN);
    Serial.println(buttonState);

    if ((millis() - lastDebounceTime) > debounceDelay) {
      /* If button was pressed, drop food. */
      if (buttonState == HIGH) {
        dropFood();
                
        lastDebounceTime = millis(); //set the current time
      }
    }
      /* Checks the reading from the IR sensor. */
//    Serial.println(analogRead(IR1_PIN));
//    delay(1000);
}



/* If button was pressed, rotate servo to 180 degrees, 
*  wait for a given time (droppingTime) to drop the food and turn it back to the begin position (0). */
void dropFood(){
  for(int angle = 0; angle <= angleMax; angle += angleStep) {
    servo.write(angle);
//    Serial.println(angle);
    delay(20);
  }

  delay(droppingTime);
  
  for(int angle = 180; angle >= angleMin; angle -= angleStep) {
    servo.write(angle);
//    Serial.println(angle);
    delay(20);
  }
}
