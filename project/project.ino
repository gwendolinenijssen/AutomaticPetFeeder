#include <ESP32Servo.h>
#include <TFT_eSPI.h>
#include "config.h"

#define IR1_PIN 2
#define IR2_PIN 12
#define SERVO_PIN 15
#define BUTTON_PIN 13

#define OBSTACLE_DETECTED LOW

TFT_eSPI tft = TFT_eSPI(); // Constructor for the TFT library
/* The daily limit for amount of 'food droppings' per day */
int dailyLimit = 100;
int totalDroppingsToday = 0;

/* Information about servo */
Servo servo;
int angleStep = 5;
int angleMin = 0; 
int angleMax = 90;
int droppingTime = 1000; // specified in millisecond

/* Information about button/debouncing the physical button */
int buttonState = LOW;
long lastDebounceTime = 0;
long debounceDelay = 200;

// Set up the feeds (ADAFRUIT IO)
AdafruitIO_Feed *feedBtnFeed = io.feed("feed_btn");
AdafruitIO_Feed *indicatorFeed = io.feed("eating");
AdafruitIO_Feed *statusTextFeed = io.feed("status_text");
AdafruitIO_Feed *maxLimitFeed = io.feed("max_limit_text");

void setup() {
  pinMode(IR1_PIN, INPUT);
  pinMode(BUTTON_PIN, INPUT_PULLDOWN);
  Serial.begin(115200);
  servo.attach(SERVO_PIN);

  io.connect();

  feedBtnFeed->onMessage(handleAdafruitButtonPress);

  // wait for a connection
  while (io.status() < AIO_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  // we are connected
  Serial.println();
  Serial.println(io.statusText());

  feedBtnFeed->get();
  indicatorFeed->get();
  statusTextFeed->get();
  maxLimitFeed->get();

  // TO DO: Work with idicator/status and max lim
  // TO DO: mss daily limit aanpasbaar maken via adafruit io
}

void loop() {
  io.run();

  /* Checks if button was pressed and handles debouncing. */
  buttonState = digitalRead(BUTTON_PIN);
  //Serial.println(buttonState);

  if ((millis() - lastDebounceTime) > debounceDelay) {
    /* If button was pressed, drop food. */
    if (buttonState == HIGH) {
      triggerFoodDrop();

      lastDebounceTime = millis(); //set the current time
    }
  }

  // TEST
  Serial.println(digitalRead(IR1_PIN));

  /* Checks the reading from the IR sensor. */
  int IR_Reading = digitalRead(IR1_PIN);
  if (IR_Reading == OBSTACLE_DETECTED) {
    triggerFoodDrop();
  }
}

/* Handles the data received by adafruit the feed of the button.*/
void handleAdafruitButtonPress(AdafruitIO_Data *data) {
  Serial.println("Button pressed through AdafruitIO");
  if (data->toString() == "1"){
      triggerFoodDrop();
  }
}

void triggerFoodDrop() {
  // TO DO: Check for a new day, reset limit

  /* If the daily limit was not exceeded, drop the food. */
  if (totalDroppingsToday < dailyLimit) {
    dropFood();
    totalDroppingsToday++;
  }
}

/* Resets the total droppings back to 0. */
void resetTotalDropingsToday() {
  totalDroppingsToday = 0;
}

/* If button was pressed, rotate servo to amount of degrees declared by angleMax,
   wait for a given time (droppingTime) to drop the food and turn it back to the begin position (angleMin). */
void dropFood() {
  servo.write(0);
  for (int angle = angleMin; angle <= angleMax; angle += angleStep) {
    servo.write(angle);
    Serial.println(angle);
    delay(20);
  }

  delay(droppingTime);

  for (int angle = angleMax; angle >= angleMin; angle -= angleStep) {
    servo.write(angle);
    Serial.println(angle);
    delay(20);
  }
}
