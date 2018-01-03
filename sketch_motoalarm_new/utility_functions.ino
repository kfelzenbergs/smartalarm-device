void powerCycle() {
  SerialUSB.println("Powercycle..");
  showPixel(5000, "red");

  boolean is_healthy = false;

  if (!gpsTracker.Check_If_Power_On()) {
    gpsTracker.Power_On();  
    delay(7000);
  }
  
  if (gpsTracker.Check_If_Power_On()) {
    boolean status;
    status = gpsTracker.AT_PowerDown();
    SerialUSB.println("at power down " + String(status));
    status = gpsTracker.Check_If_Power_On();
    SerialUSB.println("check if power on " + String(status));

    SerialUSB.println("powering on");
    gpsTracker.Power_On();
    delay(7000);

    if (gpsTracker.Check_If_Power_On() && gpsTracker.checkSIMStatus()) {
      is_healthy = true;
    }

    while(!gpsTracker.waitForNetworkRegister())
    {
      logger("Network error!");
      powerCycle();
    }

    gpsTracker.GSM_work_mode(1);

    if(!gnss.open_GNSS_default_mode()){
    //if(!gnss.open_GNSS(EPO_QUICK_MODE)){
    //if(!gnss.open_GNSS_EPO_LP_mode()){
    
      logger("Open GNSS failed!");
    }
    else {
      logger("open_GNSS_EPO_LP_mode OK.");
    }

    getIMEI();
    if (numbers_only(IMEI)) {
  
      identity_imei = true;
      identity_uuid = false;
  
      logger("Sim initialized");
      logger("IMEI [" + String(IMEI) + "] is usable for identity and is of length " + String(String(IMEI).length()));
    }

    status = gnss.isNetworkRegistered();
    SerialUSB.println("isNetworkRegistered " + String(status));
    status = gnss.isTimeSynchronized();
    SerialUSB.println("isTimeSynchronized " + String(status));
  }

  gprsSetupCompleted = false;

  
  SerialUSB.println("is healthy" + String(is_healthy));

  //gpsTracker.powerReset();
  SerialUSB.println("Powercycle completed");
}

void showPixel(int timeout, String color) {
  int r = 0;
  int g = 0;
  int b = 0;
  
  if (color == "blue")
    b = 100;
  else if (color == "red")
    r = 100;
  else if (color =="green")
    g = 100;
  else if (color == "yellow") {
    r = 255;
    g = 255;
  }
    
  
  pixels.setPixelColor(0, pixels.Color(r, g, b));
  pixels.show();
  delay(timeout);
}

void getIMEI()
{
    char mc20_Buffer[64];
    int count = 0;
    MC20_clean_buffer(mc20_Buffer,64);
    MC20_send_cmd("AT+CGSN\r\n");
    MC20_read_buffer(mc20_Buffer,64,DEFAULT_TIMEOUT);

    char * pch;

    pch = strtok (mc20_Buffer,"\n");
    while (pch != NULL)
    {
      
      if (strstr(pch, "AT+CGSN") && strstr(pch, "OK")) {
        size_t i = 0;
        size_t len = strlen(pch);
        while(i < len){
          if (!isdigit(pch[i])){
              delete_char(pch, i);
              len--;
          }else
              i++; 
        }
      }
         
      if (numbers_only(pch)) {      
        if (strlen(pch) < 20) {
          strcpy(IMEI, pch);
          logger("Got IMEI: " + String(pch));
        }  
      }
      pch = strtok (NULL, " ,.-");
    }
}

static unsigned char getComma(unsigned char num,const char *str)
{
  unsigned char i,j = 0;
  int len=strlen(str);
  for(i = 0;i < len;i ++)
  {
     if(str[i] == ',')
      j++;
     if(j == num)
      return i + 1; 
  }
  return 0; 
}

static double getDoubleNumber(const char *s)
{
  char buf[10];
  unsigned char i;
  double rev;
  
  i=getComma(1, s);
  i = i - 1;
  strncpy(buf, s, i);
  buf[i] = 0;
  rev=atof(buf);
  return rev; 
}

static double getIntNumber(const char *s)
{
  char buf[10];
  unsigned char i;
  double rev;
  
  i=getComma(1, s);
  i = i - 1;
  strncpy(buf, s, i);
  buf[i] = 0;
  rev=atoi(buf);
  return rev; 
}

int numbers_only(const char *s)
{

    if (String(*s).length() == 0) return 0;
    
    while (*s) {
      char x = *s++;
        if (isdigit(x) == 0) return 0;
    }

    return 1;
}

void delete_char(char *str, int i) {
    int len = strlen(str);

    for (; i < len - 1 ; i++)
    {
       str[i] = str[i+1];
    }

    str[i] = '\0';
}

void logger(String msg) {
  if (DEBUG_MODE) {
    SerialUSB.println(msg);
  }
  if (SDLOG_MODE) {
    
    File dataFile = SD.open("tracker.log", FILE_WRITE);
    
    // if the file is available, write to it:
    if (dataFile) {
     
      // write to file
      dataFile.println(msg);
      dataFile.close();
    }
  }
}

