#include <ESP32Servo.h>
#include <TFT_eSPI.h>
#include "time.h"
#include "config.h"

#define IR1_PIN 2
#define CAPSENSE_PIN 12
#define SERVO_PIN 15
#define BUTTON_PIN 13
#define DELAYTIMEMS 1000
#define DELAYTIMEMS30SEC 30000
#define DELAYTIMEMS3SEC 3000

#define OBSTACLE_DETECTED LOW

TFT_eSPI tft = TFT_eSPI(); // Constructor for the TFT library
void PrintEnCryptionScheme(uint8_t);

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
AdafruitIO_Feed *max_limit_slider = io.feed("max_limit_slider");

// Values to store capsense reading/ handle readings
uint16_t ReadTouchVal = 0;
uint16_t emptyReading = 0;
uint16_t filledReading = 0;
uint16_t finalReading = 0;

// Values to handle resetting the limit per day
String dayStamp;
/* The daily limit for amount of 'food droppings' per day */
int dailyLimit = 3; // TO DO: get from Adafruit slider
int totalDroppingsToday = 0;
struct tm startingDateTime;

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;

void setup() {
  pinMode(IR1_PIN, INPUT);
  pinMode(BUTTON_PIN, INPUT_PULLDOWN);
  Serial.begin(115200);
  //printPetFeederInfo(); // DISPLAY NOT WORKING

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
  max_limit_slider->get();

  touch_pad_init();
  touch_pad_config(TOUCH_PAD_NUM9, 0); //Num2==GPIO2, Threshold: 0 == not in use

  touch_pad_filter_start(10);
  touch_pad_set_cnt_mode(TOUCH_PAD_NUM8, TOUCH_PAD_SLOPE_7, TOUCH_PAD_TIE_OPT_LOW);
  touch_pad_set_voltage(TOUCH_HVOLT_2V4, TOUCH_LVOLT_0V8, TOUCH_HVOLT_ATTEN_1V5);

  touch_pad_read(TOUCH_PAD_NUM8, &emptyReading);
  Serial.println("Empty reading: ");
  Serial.println(emptyReading);

  //init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  // Sets the starting date and time to the current date and time
  getLocalTime(&startingDateTime);


  // TO DO: Work with idicator/status and max lim
  // TO DO: daily limit aanpasbaar maken via adafruit io
}

void loop() {
  checkForNewDay();

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

  /* Checks the reading from the IR sensor. */
  int IR_Reading = digitalRead(IR1_PIN);
  if (IR_Reading == OBSTACLE_DETECTED) {
    triggerFoodDrop();
  }
}

/*
    Handles the data received by adafruit the feed of the button.
*/
void handleAdafruitButtonPress(AdafruitIO_Data *data) {
  Serial.println("Button pressed through AdafruitIO");
  if (data->toString() == "1") {
    triggerFoodDrop();
  }
}

/**
   Will trigger a food drop if the daily limit hasn't been exceeded yet.
*/
void triggerFoodDrop() {
  /* If the daily limit was not exceeded, drop the food. */
  if (totalDroppingsToday < dailyLimit) {
    dropFood();
    totalDroppingsToday++;
  }
}

/*
    Resets the total droppings back to 0.
*/
void resetTotalDropingsToday() {
  totalDroppingsToday = 0;
}

/**
    If button was pressed, rotate servo to amount of degrees declared by angleMax,
    wait for a given time (droppingTime) to drop the food and turn it back to the begin position (angleMin).
*/
void dropFood() {
  // Updates empty reading of capsense
  touch_pad_read(TOUCH_PAD_NUM8, &emptyReading);
  servo.write(0);
  Serial.println("Dropping food");

  for (int angle = angleMin; angle <= angleMax; angle += angleStep) {
    servo.write(angle);
    //Serial.println(angle);
    delay(20);
  }

  delay(droppingTime);

  for (int angle = angleMax; angle >= angleMin; angle -= angleStep) {
    servo.write(angle);
    //Serial.println(angle);
    delay(20);
  }

  eatingStarted();
}

/*
   Handles the capsense reading when the food was dropped.
*/
void eatingStarted() {
  Serial.println("Started eating");
  int IR_Reading = digitalRead(IR1_PIN);

  delay(2000); // wait a little so the food has time to fall before reading capsense
  touch_pad_read(TOUCH_PAD_NUM8, &filledReading);
  Serial.println("Filled reading: ");
  Serial.println(filledReading);

  /* Checks the capsense reading for 30 seconds (3 for demo).
      If it's fairly stable for 30 seconds (3 for demo), an eating 'session' will end
      and the variables will be updated:
      If the reading is close to the empty bowl value, the message will be
      "Your pet finished it's food!". If it's too high and closer to the filled reading,
      the message will be "Your pet has stopped eating. It didn't finish everything!".
  */
  long TempTime = millis(); //Contains the time for the updaterate
  int currentIRReading;

  while (DELAYTIMEMS3SEC > (millis() - TempTime)) {
    Serial.println("Still eating");

    /* If we detect the pet, the timer starts counting again because the pet
        may still be eating. */
    currentIRReading = digitalRead(IR1_PIN);
    if (currentIRReading == OBSTACLE_DETECTED) {
      TempTime = millis();
      Serial.println("OBSTACLE DETECTED");
    }
  }

  int dataDifference = emptyReading - filledReading;
  touch_pad_read(TOUCH_PAD_NUM8, &finalReading);

  /* If the pet finished eating and the reading is closer to empty reading, pet finished its food. */
  if ((finalReading >= (filledReading + (dataDifference / 2))) || (finalReading == emptyReading)) {
    petFinishedEating();
  }

  /* If the pet stopped eating and the reading is closer to filled reading, pet stopped and didnt finish its food. */
  else if ((finalReading <= (filledReading + (dataDifference / 2))) || (finalReading == filledReading)) {
    petStoppedEating();
  }

}

void petFinishedEating() {
  Serial.println("Pet Finished eating!");
  // to do
  // send message to display and adafruit: pet finished eating
  // allow food drops again

}

void petStoppedEating() {
  Serial.println("Pet stopped eating, and didn't finish the food!");
  // to do
  // send message to display and adafruit: pet stopped eating, not finished
  // allow food drops again

}

/**
   Checks for a new day, and resets the daily limit if a day has passed.
   (By checking which day of the year it is and comparing it to the startingDateTime's day)
*/
void checkForNewDay() {
  struct tm currentDateTime;

  // Updates the current date and time
  getLocalTime(&currentDateTime);

  if (currentDateTime.tm_yday == startingDateTime.tm_yday + 1) {
    totalDroppingsToday = 0;
    getLocalTime(&startingDateTime);
  }

  // If it's January 1st
  else if (currentDateTime.tm_yday == 0) {
    if (startingDateTime.tm_yday != 0) {
      totalDroppingsToday = 0;
      getLocalTime(&startingDateTime);
    }
  }
}

/**
   Prints all the important pet feeder info to the TTGO display. (display not working!)
*/
void printPetFeederInfo()
{
  tft.init();
  tft.setRotation(3);            //setRotation: 1: Screen in landscape(USB to the right), 3:  Screen in landscape(USB connector Left)
  tft.fillScreen(TFT_YELLOW);         //Fill screen with random colour
  //  tft.setCursor(0, 0, 4);          //(cursor at 0,0; font 4, println autosets the cursor on the next line)
  //  tft.setTextColor(TFT_BLACK, TFT_YELLOW); // Textcolor, BackgroundColor; independent of the fillscreen
  //  tft.println("TESTING");        //Print on cursorpos 0,0
}
