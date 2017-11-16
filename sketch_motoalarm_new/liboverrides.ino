bool getGPSData(char* NMEA, char *paramHeader)
{
    int i = 0;
    int j = 0;
    int tmp = 0;
    char *p = NULL;
    char buffer[1024];
    char strLine[128];
    char *header = paramHeader; //"$GNGGA,";
    char str_longitude[16];
    char str_latitude[16];
    double longitude;
    double latitude;
    char North_or_South[2];
    char West_or_East[2];

    p = &header[0];

    MC20_clean_buffer(buffer, 1024);
    MC20_send_cmd(NMEA);
    //MC20_send_cmd("AT+QGNSSRD=”NMEA/RMC”\n\r");
    MC20_read_buffer(buffer, 1024, 2);
    if(NULL != strstr("+CME ERROR:", buffer))
    {
      logger("+CME ERROR: Failed to query GPS data");
      logger(buffer);
      return false;
    }
    while(buffer[i] != '\0'){
        if(buffer[i] ==  *(p+j)){
            j++;
            if(j >= 7) {
                p = &buffer[i];
                
                strLine[0] = '$'; // didn't exist
                i = 1; // was 0
                
                while(*(p++) != '\n'){
                    strLine[i++] = *p;
                }
                strLine[i] = '\0';

                logger(strLine); // 093359.000,2235.0189,N,11357.9816,E,2,17,0.80,35.6,M,-2.5,M,,*51
                
                if(NMEA == "AT+QGNSSRD=\"NMEA/RMC\"\n\r") {
                  logger("parsing RMC");
                  parseGPRMC((const char*)strLine);
                }
                else {
                  logger("parsing GGA");
                  parseGPGGA((const char*)strLine);
                }
                
                break;
            }
        } else {
            j = 0;
        }
        i++;
    }

    return true;
}
