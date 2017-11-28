#define APN "internet.lmt.lv"
#define HOST "46.101.247.231"
#define PORT 4242
#define URL_STATS "/stats_gateway/"
#define URL_EVENT "/event_gateway/"
#define IDENTITY "d39a8c02-effa-449d-80be-50dbd6315ce3"

GPRS gprs = GPRS();

char payload[180];
char postContent[280];
bool gprsSetupCompleted = false;
int connectionFailedCounter = 0;

void setupGPRS() {
  int gprsInitErrorCounter = 0;
  boolean initComplete = false;

  

  while (!initComplete) {
    logger("Trying to initiate gprs..");
    int ret = 0;
    if(!(ret = gprs.init(APN))) {
      logger("GPRS init error: " + String(ret));
      gprsInitErrorCounter +=1;
      
      if (gprsInitErrorCounter == 3) {
        powerCycle();
        return;
      }
    }
    else {
      SerialUSB.println("gprs init complete");
      initComplete = true;
    }

    gprs.join();

//    logger("\n\rIP: [" + String(gprs.ip_string) + "]");
//    if (strlen(gprs.ip_string) < 4) {
//      gprsInitErrorCounter +=1;
//
//      if (gprsInitErrorCounter == 3) {
//        powerCycle();
//        return;
//      }
//    }
//    else {
//      SerialUSB.println("gprs init complete");
//      initComplete = true;
//    }
    
  }
  

  gprsSetupCompleted = true;
}

void send_stats_update(float lat, float lon, float alt, int satelites, int current_speed, boolean acc_on, float car_voltage, float battery_voltage){
  showPixel(500, "green");
  if (!gprsSetupCompleted) {
    setupGPRS();  
  }

//  sprintf(payload,"%s", "{");
//  sprintf(payload,"%s%s", payload, "\"lat\":");
//  sprintf(payload,"%s%f", payload, lat);
//  sprintf(payload,"%s%s", payload, ",\"lon\":");
//  sprintf(payload,"%s%f", payload, lon);
//  sprintf(payload,"%s%s", payload, ",\"alt\":");
//  sprintf(payload,"%s%f", payload, alt);
//  sprintf(payload,"%s%s", payload, ",\"satelites\":");
//  sprintf(payload,"%s%d", payload, satelites);
//  sprintf(payload,"%s%s", payload, ",\"speed\":");
//  sprintf(payload,"%s%d", payload, current_speed);
//  sprintf(payload,"%s%s", payload, ",\"bat_level\":");
//  sprintf(payload,"%s%d", payload, currentBatLevel);
//  sprintf(payload,"%s%s", payload, ",\"is_charging\":");
//  sprintf(payload,"%s%d", payload, is_charging);
//  sprintf(payload,"%s%s", payload, ",\"battery_voltage\":");
//  sprintf(payload,"%s%f", payload, battery_voltage);

  if (identity_imei) {
    //sprintf(payload,"%s%s", payload, ",\"imei\":\"");
    //sprintf(payload,"%s%s\"", payload, IMEI);

    sprintf(payload, "%s", IMEI);
  }
  else {
    //sprintf(payload,"%s%s", payload, ",\"identity\":\"" IDENTITY "\"");
    sprintf(payload, "%s", IDENTITY);
  }
  //sprintf(payload,"%s%s", payload, "}");
  
  if(gprs.connectTCP(HOST, PORT)) {  
    logger("STATS API Connected!");
    connectionFailedCounter = 0;

    MC20_clean_buffer(postContent, 280);
    // post json
    // sprintf(postContent, "POST %s HTTP/1.1\r\nContent-Type: application/json\r\nHost: %s\r\nContent-Length: %d\r\n\r\n%s", URL_STATS, HOST, strlen(payload), payload);
    
    // send tcp
    sprintf(postContent, "%s,%f,%f,%f,%d,%d,%d,%f,%f", payload, lat, lon, alt, satelites, current_speed, acc_on, car_voltage, battery_voltage);

    logger(postContent);
    
    gprs.sendTCPData(postContent);   // Send HTTP request
    int ret = 0;
    ret = MC20_wait_for_resp("CLOSED\r\n", CMD, 100, 3000, DEBUG_MODE);
    gprs.closeTCP();

    logger("Wait for request: ");
    logger(String(ret));

  } 
  else {
    logger("STATS API Connection failed");
    connectionFailedCounter +=1;

    if (connectionFailedCounter == 3) {
      connectionFailedCounter = 0;
      powerCycle();
    }
  }
}

void send_event_update(String event_type){

  if (!gprsSetupCompleted) {
    setupGPRS();  
  }
  
  sprintf(payload,"%s", "{");
  sprintf(payload,"%s%s", payload, "\"identity\":\"" IDENTITY "\""); 

  if (identity_imei) {
    sprintf(payload,"%s%s", payload, ",\"imei\":\"");
    sprintf(payload,"%s%s\"", payload, IMEI);
  }
  else {
    sprintf(payload,"%s%s", payload, ",\"identity\":\"" IDENTITY "\"");
  }  
  
  sprintf(payload,"%s%s", payload, ",\"event_type\":\"");
  sprintf(payload,"%s%s", payload, event_type.c_str());  
  sprintf(payload,"%s%s", payload, "\"}");

  if(gprs.connectTCP(HOST, PORT)) {
    logger("STATS API Connected!");

    MC20_clean_buffer(postContent, 380);
    sprintf(postContent, "POST %s HTTP/1.1\r\nContent-Type: application/json\r\nHost: %s\r\nContent-Length: %d\r\n\r\n%s", URL_EVENT, HOST, strlen(payload), payload);
    
    logger(postContent);
    
    gprs.sendTCPData(postContent);   // Send HTTP request
    int ret = 0;
    ret = MC20_wait_for_resp("CLOSED\r\n", CMD, 100, 3000, DEBUG_MODE);
    gprs.closeTCP();

    logger("Wait for request: ");
    logger(String(ret));

  } else {
    logger("STATS API Connection failed");
  }
}
