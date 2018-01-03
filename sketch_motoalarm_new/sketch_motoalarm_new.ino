//#include <Timer.h>
#include <Thread.h>
#include <Wire.h>
#include <SD.h>
#include "MC20_Common.h"
#include "MC20_Arduino_Interface.h"
#include "MC20_GPRS.h"
#include "MC20_GNSS.h"
#include <ADXL345.h>
#include <math.h>
#include <MemoryFree.h>;
#include <pgmStrToRAM.h>; // not needed for new way. but good to have for reference.

#define RGBPOWER     6          //To use User LED, D6 should be HIGH.
#define RGBPIN       10
#define LED_NUM      1
#include <Adafruit_NeoPixel.h>
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(LED_NUM, RGBPIN, NEO_GRB + NEO_KHZ800);

Thread batT = Thread();
Thread smsT = Thread();
Thread gpsT = Thread();
Thread gsmT = Thread();
Thread angleT = Thread();
Thread carvT = Thread();
Thread accT = Thread();

#define MIN_SATELITES 3
char IMEI[20];
boolean identity_imei = false;
boolean identity_uuid = true;

// if true then SerialUSB.print will be utilized.
boolean DEBUG_MODE = true;
boolean SDLOG_MODE = false;
 
// Alarm vars
boolean alarmEnabled = false;
boolean gpsFixed = false;

boolean alert_onGPSFixed = true;
boolean alert_onAngleChange = true;
boolean alert_onPositionChange = true;
boolean alert_onBatteryChargeStateChange = true;
boolean alert_onBatteryChargeStateChange_sent = false;

boolean alert_alreadyCalled = false;
boolean alert_alreadySentAngleChanged = false;
boolean alert_alreadySentPositionChanged = false;

// GSM vars
int sendFirstSMS = 0;

// Battery vars
char batteryInfo[256];
int lastBatLevel = 0;
int currentBatLevel = 0;
boolean is_charging = 0;
int batControl = 4;
float battery_voltage;
float car_voltage;
boolean acc_on;

// Accelerometer vars
ADXL345 adxl; //variable adxl is an instance of the ADXL345 library

//These variables will be used to hold the x,y and z axis accelerometer values.
int x_fixed, y_fixed, z_fixed; // previous values
int x,y,z; // current values

//GPS vars
double fixed_lat;
double fixed_lon;
double current_lat;
double current_lon;
double current_alt;
int current_speed = 0;
int satellites = 0;

//SD card
const int chipSelect = 4;

//sensor vars

GPSTracker gpsTracker = GPSTracker();
GNSS gnss = GNSS();

// Timer triggered functions goes here
void sendSMS(String message) {
  char *msg = &message[0u];
  gpsTracker.sendSMS("+37128765799", msg);
}

void makeCall() {
  
}

void fixAngle() {
  x_fixed = x;
  y_fixed = y;
  z_fixed = z;
}

void fixGPS() {
  
  if (satellites < MIN_SATELITES) {
    logger("Waiting for at least " + String(MIN_SATELITES) + " satellites, currenty " + String(satellites) + " visible.");
  }
  else {
    fixed_lat = current_lat;
    fixed_lon = current_lon;

    gpsFixed = true;
    logger("GPS fixed!");

    if (alert_onGPSFixed) {
      //sendSMS("GPS position locked!");
    }
  }
   
}

void sendData() {
  SerialUSB.println("sendData start");
  send_stats_update(current_lat, current_lon, current_alt, satellites, current_speed, acc_on, car_voltage, battery_voltage);
  SerialUSB.println("sendData end");
}

void setup() {

  SD.begin(chipSelect);

  pinMode(RGBPOWER, OUTPUT);
  digitalWrite(RGBPOWER, HIGH);
  pixels.begin(); // This initializes the NeoPixel library.

  gpsTracker.Power_On();
  
  if (DEBUG_MODE) {
    SerialUSB.begin(115200);
    logger("Setup sequence..");
  }

  powerCycle();
  
  //Accelerometer block
  adxl.powerOn();
  configureADXL();

  batT.onRun(checkBatteryVoltage);
  batT.setInterval(60000);

  carvT.onRun(checkCarVoltage);
  carvT.setInterval(10000);

  accT.onRun(checkAcc);
  accT.setInterval(10000);

  smsT.onRun(checkSMS);
  smsT.setInterval(30000);

  gpsT.onRun(checkGPS);
  gpsT.setInterval(30000);

  gsmT.onRun(sendData);
  gsmT.setInterval(60000);

  angleT.onRun(checkAngle);
  angleT.setInterval(30000);  
}

void loop() {
  if(batT.shouldRun())
    batT.run();

  if(carvT.shouldRun())
    carvT.run();

  if(accT.shouldRun())
    accT.run();

  if(smsT.shouldRun())
    smsT.run();

  if(gpsT.shouldRun())
    gpsT.run();

  if(gsmT.shouldRun())
    gsmT.run();

  if(angleT.shouldRun())
    angleT.run();

  showPixel(500, "blue");
  showPixel(500, "");

//  SerialUSB.println(getPSTR("Old way to force String to Flash")); // forced to be compiled into and read   
//  SerialUSB.println(F("New way to force String to Flash")); // forced to be compiled into and read  
//  SerialUSB.println(F("Free RAM = ")); //F function does the same and is now a built in library, in IDE > 1.0.0
//  SerialUSB.println(freeMemory(), DEC);  // print how much RAM is available.  
//  SerialUSB.print("Skaititajs:");
//  SerialUSB.println(skaititajs);
}
