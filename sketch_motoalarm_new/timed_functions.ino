void checkGPS() {
  logger("checkGPS start");
  showPixel(500, "yellow");
  //GPS block starts

  if(getGPSData("AT+QGNSSRD=\"NMEA/GGA\"\n\r", "$GNGGA,"))
  {
    logger("getting GNGGA coordinates..");
  }
  else {
    logger("Failed to get GGA GPS data");
    powerCycle();
  }

  if(getGPSData("AT+QGNSSRD=\"NMEA/RMC\"\n\r", "$GNRMC,"))
  {
    logger("getting GNRMC coordinates..");
  }
  else {
    logger("Failed to get RMC GPS data");
  }

  if (!gpsFixed || satellites < MIN_SATELITES) {
    gpsFixed = false;
    fixGPS();
  }
  
  if (alarmEnabled && gpsFixed) {
    double lat_change = abs(current_lat - fixed_lat);
    double lon_change = abs(current_lon - fixed_lon);

    logger("lat_change: " + String(lat_change));
    logger("lon_change: " + String(lon_change));

    double alert_change = 0.01;
    if (lat_change >= alert_change || lon_change >= alert_change) {
      logger("!!! Position changed !!!");

      if (alert_onPositionChange && !alert_alreadySentPositionChanged) {
        String stats = "\nLat: " + String(current_lat);
        stats += "\nLon: " + String(current_lon);
        stats += "\nSat: " + String(satellites);
        sendSMS(stats);
        alert_alreadySentPositionChanged = true;
        send_event_update("position_changed");

        if (!alert_alreadyCalled) {
          makeCall();
          alert_alreadyCalled = true;
        }
      } 
      
    }
    
  }

  //gnss.close_GNSS();
  
  logger("checkGPS end");
}

void checkAngle() {
  logger("checkAngle start");
  adxl.readXYZ(&x, &y, &z);

  if (alarmEnabled) {
    int x_change = abs(x - x_fixed);
    int y_change = abs(y - y_fixed);
    int z_change = abs(z - z_fixed);

    int alert_change = 20;
    if (x_change >= alert_change || y_change >= alert_change || z_change >= alert_change) {
      logger("!!! Angle changed !!!");

      if (alert_onAngleChange && !alert_alreadySentAngleChanged) {
        sendSMS("Mocis kustas YO! - /Angle changed/");
        send_event_update("angle_changed");
        alert_alreadySentAngleChanged = true;
        
        if (!alert_alreadyCalled) {
          makeCall();
          alert_alreadyCalled = true;
        }
      }
    }
  }
  
  //Print the results to the terminal.
  logger("x: " + String(x) + " y: " + String(y) + " z: " + String(z));
  logger("checkAngle end");
}

void checkSMS() {
  logger("checkSMS start");
  char *s = NULL;
  char buffer[64];
  int inComing = 0;
  
  if(MC20_check_readable()){
    inComing = 1;

    MC20_read_buffer(buffer, 64);
    logger(buffer);

    if(NULL != (s = strstr(buffer,"+CMTI: \"SM\""))) { //SMS: $$+CMTI: "SM",24$$
        char phone_nr[30];
        char message[128];
        char datetime[128];
        
        int messageIndex = atoi(s+12);
        gpsTracker.readSMS(messageIndex, message, 128, phone_nr, datetime);

        logger("There is unread sms from " + String(phone_nr) + "!");
        logger("Message: " + String(message));
        

        if (strstr(phone_nr, "+37128765799")) {
          
          for(int i=0;i<strlen(message);i++){
              message[i] = toupper(message[i]);
          }

          //commands
          if (strstr(message, "ON")) {
            fixGPS();
            fixAngle();
            
            alarmEnabled = true;
            String msg = "Mocis apsargajas!";
            
            sendSMS(msg);
            send_event_update("device_armed");
    
            //reset vars
            alert_alreadyCalled = false;
            alert_alreadySentAngleChanged = false;
            alert_alreadySentPositionChanged = false;
          }
          else if (strstr(message, "OFF")) {
            alarmEnabled = false;
            sendSMS("Signalizacija atslegta!");
            send_event_update("device_disarmed");
          }
          else if (strstr(message, "STATS")) {
            //get all important stats
            String stats = "Stats\nbat: " + String(currentBatLevel);
            stats += "\nChr: " + String(is_charging);
            stats += "\nVtg: " + String(battery_voltage);
            stats += "\nLat: " + String(current_lat);
            stats += "\nLon: " + String(current_lon);
            stats += "\nSat: " + String(satellites);

            logger("sending: " + stats);
            sendSMS(stats);
          }
          
        }
        
     }
     else {
      logger("No unread sms!");
    }
     
     MC20_clean_buffer(buffer,64);  
     inComing = 0;

  }
  logger("checkSMS end");
}

void checkCarVoltage() {
  const int pin_car_voltage = A0;

  int num_samples = 10;
  int sum = 0;                    // sum of samples taken
  unsigned char sample_count = 0; // current sample number
  float voltage = 0.0;            // calculated voltage

  // take a number of analog samples and add them up
  while (sample_count < num_samples) {
      sum += analogRead(pin_car_voltage);
      sample_count++;
      delay(10);
  }
  
  voltage = ((float)sum / (float)num_samples * 3.24) / 1024.0;
  if (voltage < 0.1) {
    voltage = 0;
  }
  
  car_voltage = roundf(voltage * 11.132 * 100) / 100;
  logger("Car Voltage: " + String(car_voltage) + "V");
  
  sample_count = 0;
  sum = 0;
}

void checkBatteryVoltage() {
  const int pin_battery_voltage = A4;

  int a = analogRead(pin_battery_voltage);
  float v = a/1023.0*3.3*2.0;        // there's an 10M and 10M resistor divider
  battery_voltage = roundf(v * 100) / 100;
  logger("Battery Voltage: " + String(battery_voltage) + "V");
}

// check if car engine is on
void checkAcc() {
  const int pin_acc = A1;

  int acc = analogRead(pin_acc);
  float voltage = 0.0;
  voltage = (float)acc * 3.24 / 1024.0;
  logger("Acc: " + String(voltage));

  if (voltage > 3.0 && voltage < 4.0) {
    acc_on = true;
  }
  else {
    acc_on = false;
  }
}


