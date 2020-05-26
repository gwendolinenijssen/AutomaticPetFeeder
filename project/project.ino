/**
 * Author: Gwendoline Nijssen
 * Certains portions of this code were originally written by other authors and taken from other sources,
 * such as the Digital Electronics labs, the Arduino documentation (examples/tutorials), or other sources that are stated below.  
 * 
 * References: 
 * - https://www.arduino.cc/en/Tutorial/HomePage
 * - https://iotdesignpro.com/projects/google-assistant-controlled-led-using-ESP32-and-adafruit-io
 * - https://www.youtube.com/watch?v=dqr-AT5HvyM
 */

#include <ESP32Servo.h>
#include <SPI.h>
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

/* Set up the feeds (ADAFRUIT IO) */
AdafruitIO_Feed *feedBtnFeed = io.feed("feed_btn");
AdafruitIO_Feed *indicatorFeed = io.feed("eating");
AdafruitIO_Feed *statusTextFeed = io.feed("status_text");
AdafruitIO_Feed *maxLimitFeed = io.feed("max_limit_text");
AdafruitIO_Feed *max_limit_slider = io.feed("max_limit_slider");

/* Values to store and handle capsense reading */
uint16_t ReadTouchVal = 0;
uint16_t emptyReading = 0;
uint16_t filledReading = 0;
uint16_t finalReading = 0;

/* Values to handle resetting the limit per day */
int dailyLimit = 3;
int totalDroppingsToday = 0;
struct tm startingDateTime;
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;

/* Keeps track of the previous message displayed on the display. */
String previousMessage = "Your pet has not eaten yet today.";

/**
 * Initializes important variables and makes sure an internet connection is possible.
 * Connects to Adafruit IO if Wifi connection is succesful, and will initialize
 * all important fields needed to make the pet feeder work.
 */
void setup() {
  initializeDisplay();
    
  pinMode(IR1_PIN, INPUT);
  pinMode(BUTTON_PIN, INPUT_PULLDOWN);
  Serial.begin(115200);

  servo.attach(SERVO_PIN);

  io.connect();

  /* Wait for a connection */
  while (io.status() < AIO_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  /* Connected */ 
  Serial.println();
  Serial.println(io.statusText());

  //feedBtnFeed->get();
  indicatorFeed->get();
  statusTextFeed->get();
  maxLimitFeed->get();
  max_limit_slider->get();

  /* Initialize AdafruitIO feeds correctly */
  feedBtnFeed->onMessage(handleAdafruitButtonPress);
  max_limit_slider->onMessage(handleLimitChanged);
  setNewLimitMessage(dailyLimit);
  indicatorFeed->save(0);
  statusTextFeed->save("Your pet hasn't eaten yet today.");

  /* Initialze capsense */
  touch_pad_init();
  touch_pad_config(TOUCH_PAD_NUM9, 0); //Num2==GPIO2, Threshold: 0 == not in use

  touch_pad_filter_start(10);
  touch_pad_set_cnt_mode(TOUCH_PAD_NUM8, TOUCH_PAD_SLOPE_7, TOUCH_PAD_TIE_OPT_LOW);
  touch_pad_set_voltage(TOUCH_HVOLT_2V4, TOUCH_LVOLT_0V8, TOUCH_HVOLT_ATTEN_1V5);
  touch_pad_read(TOUCH_PAD_NUM8, &emptyReading);

  /* Initialize and get the time */
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  /* Sets the starting date and time to the current date and time */
  getLocalTime(&startingDateTime);
}

/**
 * Checks for activity such as a physical button press,
 * button press on Adafruit IO, change in limit through Adafruit IO, 
 * obstacle detection through IR sensor close to the bowl, or
 * Google Assistant activity (handled through Adafruit IO). 
 * 
 * Also checks for a new day, so the daily limit can be reset and
 * new food droppings will be allowed again after reaching the daily limit.
 */
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
 * Handles the data received by the feed of the button through Adafruit IO.
 */
void handleAdafruitButtonPress(AdafruitIO_Data *data) {
  if (data->toString() == "1") {
    Serial.println("Button pressed through AdafruitIO");
    triggerFoodDrop();
  }
}

/*
 * Handles the change of the max limit slider for daily food drops.
 */
void handleLimitChanged(AdafruitIO_Data *data) {
  dailyLimit = data->toInt();
  setNewLimitMessage(dailyLimit - totalDroppingsToday);
  printPetFeederInfo(dailyLimit - totalDroppingsToday, previousMessage);
}

/**
 * Will trigger a food drop if the daily limit hasn't been exceeded yet.
 */
void triggerFoodDrop() {
  /* If the daily limit was not exceeded, drop the food. */
  if (totalDroppingsToday < dailyLimit) {
    dropFood();
  }
}

/*
 * Resets the total droppings back to 0.
 */
void resetTotalDropingsToday() {
  totalDroppingsToday = 0;
  setNewLimitMessage(dailyLimit);
  printPetFeederInfo(dailyLimit, "Your pet has not eaten yet today.");
}

/**
 * Changes the message displaying the food limit on Adafruit IO page
 * to a message with the given amount.
*/
void setNewLimitMessage(int amount) {
  String limitMessage;
  limitMessage = "Your pet has ";
  if (amount <= 0) {
    limitMessage += "no portions left for today.";

  } else if (amount == 1) {
    limitMessage +=  amount;
    limitMessage += " portion left for today.";

  } else {
    limitMessage +=  amount;
    limitMessage += " portions left for today.";
  }
  maxLimitFeed->save(limitMessage);
}

/**
 * If button was pressed, rotate servo to amount of degrees declared by angleMax, 
 * wait for a given time (droppingTime) to drop the food and turn it back to the begin position (angleMin).
*/
void dropFood() {
  /* Updates empty reading of capsense */
  touch_pad_read(TOUCH_PAD_NUM8, &emptyReading);
  servo.write(0);
  Serial.println("Dropping food");

  for (int angle = angleMin; angle <= angleMax; angle += angleStep) {
    servo.write(angle);
    delay(20);
  }

  delay(droppingTime);

  for (int angle = angleMax; angle >= angleMin; angle -= angleStep) {
    servo.write(angle);
    delay(20);
  }

  totalDroppingsToday++;
  setNewLimitMessage(dailyLimit - totalDroppingsToday);
  printPetFeederInfo(dailyLimit - totalDroppingsToday, "Your pet is currently eating.");
  eatingStarted();
}

/*
 * Handles the capsense reading when the food was dropped.
 * Checks the capsense reading for 30 seconds (3 for demo).
 * If it's fairly stable for 30 seconds (3 for demo), an eating 'session' will end
 * and the variables will be updated:
 * -  If the reading is close to the empty bowl value, the message will be
 *    "Your pet finished it's food!". 
 * -  If it's too high and closer to the filled reading,
      the message will be "Your pet has stopped eating. It didn't finish everything!".
 */
void eatingStarted() {
  Serial.println("Started eating");
  int IR_Reading = digitalRead(IR1_PIN);

  delay(2000); // wait a little so the food has time to fall before reading capsense
  touch_pad_read(TOUCH_PAD_NUM8, &filledReading);
  Serial.println("Filled reading: ");
  Serial.println(filledReading);

  long TempTime = millis(); //Contains the time for the updaterate
  int currentIRReading;

  /* Start feeding session and check when pet has stopped eating.
   * DELAYTIMEMS3SEC will be the amount of time given for the feeding session to go on, without
   * detecting the pet. This is 3 seconds for the demo.
   * (Should be longer in case pet steps away for half a minute to drink but continues eating.)*/
  while (DELAYTIMEMS3SEC > (millis() - TempTime)) {
    /* If we detect the pet through the IR sensor, the timer starts counting again because the pet
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

/**
 *  Updates Adafruit IO and display with the correct message
 * for when a pet has finished eating.
*/
void petFinishedEating() {
  Serial.println("Pet Finished eating!");
  indicatorFeed->save(0);
  statusTextFeed->save("Your pet has finished eating!");
  printPetFeederInfo(dailyLimit-totalDroppingsToday, "Your pet has finished eating!");
}

/**
 *  Updates Adafruit IO and display with the correct message
 * for when a pet has stopped eating. (hasn't finished food)
*/
void petStoppedEating() {
  Serial.println("Pet stopped eating, and didn't finish the food!");
   indicatorFeed->save(1);
   statusTextFeed->save("Your pet didn't finish its food!");
   printPetFeederInfo(dailyLimit-totalDroppingsToday, "Your pet didn't finish its food!");
}

/**
 * Checks for a new day, and resets the daily limit if a day has passed.
 * (By checking which day of the year it is and comparing it to the startingDateTime's day)
*/
void checkForNewDay() {
  struct tm currentDateTime;

  /* Updates the current date and time */
  getLocalTime(&currentDateTime);

  if (currentDateTime.tm_yday == startingDateTime.tm_yday + 1) {
    totalDroppingsToday = 0;
    getLocalTime(&startingDateTime);
  }

  /* If it's January 1st (exception) */
  else if (currentDateTime.tm_yday == 0) {
    if (startingDateTime.tm_yday != 0) {
      totalDroppingsToday = 0;
      getLocalTime(&startingDateTime);
    }
  }
}

/**
 * Prints all the important pet feeder info to the TTGO display.
*/
void printPetFeederInfo(int amountLeft, String message) {
  tft.fillScreen(TFT_BLACK); //Fill screen with black colour
  tft.setCursor(0, 0, 2); //(cursor at 0,0; font 4, println autosets the cursor on the next line)
  tft.setTextColor(TFT_WHITE, TFT_RED); // Textcolor, BackgroundColor; independent of the fillscreen
  tft.println("Automatic Pet Feeder");
  tft.setTextColor(TFT_WHITE, TFT_BLACK); // Textcolor, BackgroundColor; independent of the fillscreen
  tft.println(message);
  tft.print("Portions left: ");

  if (amountLeft <= 0) {
    amountLeft = 0;
  }
  
  tft.println(amountLeft);

  previousMessage = message;
}

/** 
 * Initializes the display of the TTGO.
 */
void initializeDisplay() {
  tft.init();
  tft.setRotation(3); //setRotation: 1: Screen in landscape(USB to the right), 3:  Screen in landscape(USB connector Left)
  tft.fillScreen(TFT_BLACK); //Fill screen with black colour
  tft.setCursor(0, 0, 2); //(cursor at 0,0; font 4, println autosets the cursor on the next line)
  tft.setTextColor(TFT_WHITE, TFT_RED); // Textcolor, BackgroundColor; independent of the fillscreen
  tft.println("Automatic Pet Feeder");        //Print on cursorpos 0,0
  tft.setTextColor(TFT_WHITE, TFT_BLACK); // Textcolor, BackgroundColor; independent of the fillscreen
  tft.println("Your pet has not eaten yet today.");        //Print on cursorpos 0,0

  int portionsLeft = dailyLimit - totalDroppingsToday;
  if (portionsLeft <= 0) {
    portionsLeft = 0;
  }
  
  tft.print("Portions left: ");
  tft.println(portionsLeft);
}
