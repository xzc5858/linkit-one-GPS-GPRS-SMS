#include <LGSM.h>  
#include <LGPS.h>
#include <LGPRS.h>
#include <LGPRSClient.h>
#include <LGPRSServer.h>
#include <LBattery.h>

char buff[256];
//接收到短信后，发送GPS系统到指定服务器
gpsSentenceInfoStruct info;
char server[] = "120.27.41.213";
int port = 3000;
gpsSentenceInfoStruct gpsInfo;
LGPRSClient client;
double latitude = 0.00;
double longitude = 0.00;
float dop = 100.00; //dilution of precision
float geoid = 0.00;
int sat_num = 0; //number of visible satellites

String lat_format = "0.00000", lon_format = "0.00000";
String raw = "";

char buff2[50];
int len = 0;
char *command;
char *device;
char *textToNumber;

float convert(String str, boolean dir)
{
  double mm, dd;
  int point = str.indexOf('.');
  dd = str.substring(0, (point - 2)).toFloat();
  mm = str.substring(point - 2).toFloat() / 60.00;
  return (dir ? -1 : 1) * (dd + mm);
}


int getData(gpsSentenceInfoStruct* info)
{
//  Serial.println("Collecting GPS data.----");
  LGPS.getData(info);
//  Serial.println((char*)info->GPGGA);
  if (info->GPGGA[0] == '$')
  {
 //   Serial.print("Parsing GGA data....");
    String str = (char*)(info->GPGGA);
    str = str.substring(str.indexOf(',') + 1);
    raw = str;

    str = str.substring(str.indexOf(',') + 1);
    latitude = convert(str.substring(0, str.indexOf(',')), str.charAt(str.indexOf(',') + 1) == 'S');
//    Serial.println(latitude);
    int val = latitude * 1000000;
    String s = String(val);
    lat_format = s.substring(0, (abs(latitude) < 100) ? 2 : 3);
    lat_format += '.';
    lat_format += s.substring((abs(latitude) < 100) ? 2 : 3);
    Serial.println(lat_format);
    str = str.substring(str.indexOf(',') + 3);
    longitude = convert(str.substring(0, str.indexOf(',')), str.charAt(str.indexOf(',') + 1) == 'W');
//    Serial.println(longitude);
    val = longitude * 1000000;
    s = String(val);
    lon_format = s.substring(0, (abs(longitude) < 100) ? 3 : 3);
    lon_format += '.';
    lon_format += s.substring((abs(longitude) < 100) ? 3 : 3);
     Serial.println(lon_format);
    str = str.substring(str.indexOf(',') + 3);
    str = str.substring(2);
 //   Serial.println(str);
    sat_num = str.substring(0, 2).toInt();
 //   Serial.println(sat_num);
    str = str.substring(3);
    dop = str.substring(0, str.indexOf(',')).toFloat();
    str = str.substring(str.indexOf(',') + 1);
    str = str.substring(str.indexOf(',') + 3);
    geoid = str.substring(0, str.indexOf(',')).toFloat();
//    Serial.println("done.");
    if (info->GPRMC[0] == '$')
    {
 //     Serial.print("Parsing RMC data....");
      str = (char*)(info->GPRMC);
      int comma = 0;
      for (int i = 0; i < 60; ++i)
      {
        if (info->GPRMC[i] == ',')
        {
          comma++;
          if (comma == 7)
          {
            comma = i + 1;
            break;
          }
        }
      }
      str = str.substring(comma);
      str = str.substring(str.indexOf(',') + 1);
      str = str.substring(str.indexOf(',') + 1);
  //    Serial.println(lat_format);
 //     Serial.println(lon_format);
  //    Serial.println("done...............");
  //    Serial.println(sat_num);
      return sat_num;
    }
  }
  else
  {
    Serial.println("No GGA data");
  }
  return 0;
}

  
void setup() {
  Serial.begin(115200);
  LGPS.powerOn();
  
  // Start up the SMS listener
  while (!LSMS.ready()) {
    delay(1000);
  }

  // If there's a leftover message, flush it.
  if (LSMS.available())
  {
    Serial.println("Flushing in setup...");
    LSMS.flush();
  }
  Serial.println("SMS ready.");
  
  delay(3000);
    while (!LGPRS.attachGPRS())
  {
    delay(500);
  }
}

void loop() {
  
  if (LSMS.available())
  { 
 
        while (true)
       {
        int v = LSMS.read();
        if (v < 0)
          break;

         buff2[len++] = (char)v;
        }
       Serial.println(buff2);

         getData(&gpsInfo);
         LGPS.getData(&gpsInfo);
         if (client.connect(server, port))
         {   
             Serial.println("connecting...");
             String str = "GET /updt/";
             str += lon_format;
             str += "/";
             str += lat_format;
             str +="/";
             str +=LBattery.level();
             client.print(str);
             client.println(" HTTP/1.1");
             client.print("Host: ");
             client.println(server);
             client.println("Connection: close");
             client.println();
       //      Serial.println(str);
             }
            else 
            {
              Serial.println("connection failed");
              Serial.println();
              Serial.println("disconnecting.");
              client.stop();
            }
             LSMS.flush();
          memset(&buff2[0], 0, sizeof(buff2));
          len = 0;
    }
    delay(5000);
}
