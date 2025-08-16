
// #################################  compile it for LOLIN (WeMO) D1 R1 ##############################
/* this code comes from https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266WiFi/examples/HTTPSRequest
 *  Interesting also: https://maakbaas.com/esp8266-iot-framework/logs/https-requests/
 *  if error uploading the code: https://sparks.gogo.co.nz/ch340.html  download and install the driver CH34x_Install_Windows_v3_4
*/

#include <NTPClient.h> // to get a clock from a synchronisation server
#include <WiFiUdp.h> // needed for the ntp client 
//https://lastminuteengineers.com/esp8266-ntp-server-date-time-tutorial/
#include <TimeLib.h> // to convert epoch into days. requires to install library: 

const long utcOffsetInSeconds = 3600;
String currentDate = "";



#include <ctype.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include "certs.h"
#ifndef STASSID


#define STASSID "VM9478242"
#define STAPSK "xbeubowf6ceNiJjx"
#define PIN_Resistance_RX D6    // GPIO12

#endif
#include <EEPROM.h>


//const long utcOffsetInSeconds = 3600;

//##############   variables definition   ###########################
int counterwakeup = 0; // counter to check how many times we wen to wake up before to send requests
String StringToDetect = "SINSTS";
char detectSINSTS[7]; // array that records the characters received
char detectPAPP[5]; // array that records the characters received

bool state = HIGH; // state for the LED
bool start_record = false;  // boolean triggered to start recording the power measurement
char puissance[5]; // power measured, in char
int puissance_int = 0; // power measured, in int
uint puissance_Uint = 0; // power measured, in int
uint puissance_moyenne = 0; // averagepower measured, in int
uint firststartup = 0;
uint timeoutconnectWifi = 0;

int indice = 0;  // index for the recording of the power
int debug = 0;  // true when we are in debug mode
uint counter_moyenne = 0; // counter to compute the average
uint counter_moyennetemp = 0; // used to store temporary counter to compute the average
int puissanceCommuniquee = 0; // puissance que l'on communique via wifi
bool finished_recording = false; //specifies if we can go back to sleep (= we received the signal from Linky)
long int timeoutReadLinky = 0; // timeout to stop trying to read linky if there is no data to read
//const int A0 = A0;  // Analog input pin that the potentiometer is attached to
int BatteryVoltage = 0;
int Danger = 0; // indicate if Power > Powermax
int Powermax = 10000; // to be read from Linky, and stored in EEPROM
long int t1 = 0;
long int t2=0;
uint voltageThreshold = 190; //207; // voltage limit to startup the communication
uint voltageThreshold2 = 170; // voltage limit to startup the process
const char* TIP_host = "tip-imredd.unice.fr";
int counterWifi = 0;

const char* ssid = STASSID;
const char* password = STAPSK;

X509List cert(cert_DigiCert_Global_Root_CA);

   // ################ set up wifi + send message function, is triggered only 

   void setup_wifi_TIP() {

   digitalWrite(LED_BUILTIN, LOW);
   delay(50);
   digitalWrite(LED_BUILTIN, HIGH); // initialisation
   delay(100);
      //WiFi.mode(WIFI_STA);
    // WiFi.config(ip, gateway_dns, gateway_dns); 
      WiFi.begin(ssid, password);





             t1 = millis();
             timeoutconnectWifi = 0;




  while (WiFi.status() != WL_CONNECTED && timeoutconnectWifi<10000) {
    delay(50);
                 t2 = millis();
             timeoutconnectWifi = t2-t1;
  }


   digitalWrite(LED_BUILTIN, LOW);
   delay(50);
   digitalWrite(LED_BUILTIN, HIGH); // initialisation
   delay(100);
           if (debug==1){
 Serial.println("connected");
 Serial.println("IP address: "); //
Serial.println(WiFi.localIP());//
           }
  configTime(3 * 3600, 0,"134.59.1.5", "pool.ntp.org", "time.nist.gov");

  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2) {
    delay(500);
    now = time(nullptr);
  }
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
 int epoch_time = now;


  
 /* 
// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);
timeClient.update();
int hourvalue = timeClient.getHours();
int minutevalue = timeClient.getMinutes();
int secondvalue = timeClient.getSeconds();*/
//int epoch_time = now;

// define starting date
String monthstringstart = "";
String yearstringstart = "";
String daystringstart = "";
String hourstringstart = "";
String minutestringstart = "";
String secondstringstart = "";

String monthstringend = "";
String yearstringend = "";
String daystringend = "";
String hourstringend = "";
String minutestringend = "";
String secondstringend = "";


if (month(epoch_time)>9) {
   monthstringstart = String(month(epoch_time));
}else {
   monthstringstart = "0"+String(month(epoch_time));
}
if (day(epoch_time)>9) {
   daystringstart = String(day(epoch_time));
}else {
    daystringstart = "0"+String(day(epoch_time));
}
if (hour(epoch_time)>9) {
   hourstringstart = String(hour(epoch_time));
}else {
    hourstringstart = "0"+String(hour(epoch_time));
}
if (minute(epoch_time)>9) {
   minutestringstart = String(minute(epoch_time));
}else {
   minutestringstart = "0"+String(minute(epoch_time));
}
if (second(epoch_time)>9) {
   secondstringstart = String(second(epoch_time));
}else {
   secondstringstart = "0"+String(second(epoch_time));
}
 yearstringstart = String(year(epoch_time));
int counter_moyenne_int = counter_moyenne;
// define end date:
epoch_time=epoch_time + 1;//15*max(1,counter_moyenne_int); // IF WE HAVE SLEEP EVERY 10-15 SECONDS
if (month(epoch_time)>9) {
   monthstringend = String(month(epoch_time));
}else {
   monthstringend = "0"+String(month(epoch_time));
}
if (day(epoch_time)>9) {
   daystringend = String(day(epoch_time));
}else {
    daystringend = "0"+String(day(epoch_time));
}
if (hour(epoch_time)>9) {
   hourstringend = String(hour(epoch_time));
}else {
    hourstringend = "0"+String(hour(epoch_time));
}
if (minute(epoch_time)>9) {
   minutestringend = String(minute(epoch_time));
}else {
   minutestringend = "0"+String(minute(epoch_time));
}
if (second(epoch_time)>9) {
   secondstringend = String(second(epoch_time));
}else {
   secondstringend = "0"+String(second(epoch_time));
}
 yearstringend = String(year(epoch_time));









  WiFiClientSecure client;
  //client.setTrustAnchors(&cert);
  client.setInsecure(); //the magic line, use with caution

  
  if (!client.connect(TIP_host, github_port)) {
    //Connection failed
    return;
  }

  String url = "/nodes/imredd/energyconso/linky";

String httpRequestData = "linkySensor,sensor_id=LinkyHouse4 power="+String(puissanceCommuniquee)+",danger="+String(Danger)+",voltage=234.231,frequency=50.2,pf=0.9 "+String(epoch_time)+"000000000";
 client.print(String("POST ") + url + " HTTP/1.0\r\n" + "Host: " + TIP_host + "\r\n" + "User-Agent: BuildFailureDetectorESP8266\r\n" +  "Content-Length: " + httpRequestData.length()   + "\r\n"+"Connection: close\r\n\r\n"+httpRequestData);


/*client.stop();
    if (client.connect(github_host, 443)) {
 
      client.println("POST " + url + " HTTP/1.0");
      client.println("Host: " + (String)github_host);
      client.println(F("User-Agent: ESP"));
      client.println(F("Connection: close"));
      client.println(F("Content-Type: application/json"));
      client.println(F("Authorization: Bearer eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpZCI6IjY0MjU2MGNmNTY2ODlkNzk5MDBkZGU3MSIsInJvbGUiOiJBIiwicm9sZXNBQ0wiOlsiYWRtaW4iXSwibmFtZSI6IklNUkVERCIsIm1vYmlsZSI6IiIsImVtYWlsIjoiYmVub2l0LmNvdXJhdWRAdW5pY2UuZnIiLCJmaXJzdE5hbWUiOiJBUEkiLCJsb2NhbGUiOiJlbl9VUyIsImxhbmd1YWdlIjoiZW4iLCJjdXJyZW5jeSI6IkVVUiIsInRlbmFudElEIjoiNWMzZGU5MTA2NmMwM2YwMDA5OGRhNDBhIiwidGVuYW50TmFtZSI6IklNUkVERCAoQ2l0eSBvZiBOaWNlKSIsInRlbmFudFN1YmRvbWFpbiI6ImltcmVkZCIsInVzZXJIYXNoSUQiOiJmZTZhMjBjYTIzOWY2MjdhZTNkY2NhN2QwNWFmMDYzODZiNzY2ZGFiYTEyNmZmNzBkYTg4NmYwNTgzMjkxOTU3IiwidGVuYW50SGFzaElEIjoiNWEzZDM1YzlkNWNlNDE2MGRhMmQ5ZGFhYjA4NTA1ODc5ODFlNTExM2MyYzMyOTA0Yzc4YmU2NGEzNWU2ODliYyIsInNjb3BlcyI6WyJBc3NldDpJbkVycm9yIiwiQXNzZXQ6TGlzdCIsIkJpbGxpbmdBY2NvdW50OkJpbGxpbmdBY2NvdW50T25ib2FyZCIsIkJpbGxpbmdBY2NvdW50Okxpc3QiLCJCaWxsaW5nVHJhbnNmZXI6TGlzdCIsIkNhcjpMaXN0IiwiQ2FyQ2F0YWxvZzpMaXN0IiwiQ2hhcmdpbmdQcm9maWxlOkxpc3QiLCJDaGFyZ2luZ1N0YXRpb246SW5FcnJvciIsIkNoYXJnaW5nU3RhdGlvbjpMaXN0IiwiQ2hhcmdpbmdTdGF0aW9uQ2VydGlmaWNhdGU6TGlzdCIsIkNvbXBhbnk6TGlzdCIsIkNvbm5lY3Rpb246TGlzdCIsIkludm9pY2U6TGlzdCIsIkxvZ2dpbmc6TGlzdCIsIk9jcGlFbmRwb2ludDpMaXN0IiwiT2ljcEVuZHBvaW50Okxpc3QiLCJQYXltZW50TWV0aG9kOkxpc3QiLCJQbGFubmluZzpMaXN0IiwiUHJpY2luZ0RlZmluaXRpb246TGlzdCIsIlJlZ2lzdHJhdGlvblRva2VuOkxpc3QiLCJSZWxlYXNlTm90ZXM6TGlzdCIsIlNldHRpbmc6TGlzdCIsIlNpdGU6TGlzdCIsIlNpdGVBcmVhOkxpc3QiLCJTaXRlVXNlcnM6TGlzdCIsIlNvdXJjZTpMaXN0IiwiU3Vic2NyaXB0aW9uOkxpc3QiLCJTdWJzY3JpcHRpb246VXBkYXRlIiwiVGFnOkxpc3QiLCJUYWc6VXBkYXRlIiwiVGF4Okxpc3QiLCJUcmFuc2FjdGlvbjpJbkVycm9yIiwiVHJhbnNhY3Rpb246TGlzdCIsIlVzZXI6SW5FcnJvciIsIlVzZXI6TGlzdCIsIlVzZXI6VXBkYXRlIiwiVXNlckdyb3VwOkxpc3QiLCJVc2VyU2l0ZXM6TGlzdCIsIlVzZXJTdWJzY3JpcHRpb246TGlzdCJdLCJhY3RpdmVDb21wb25lbnRzIjpbInByaWNpbmciLCJiaWxsaW5nIiwib3JnYW5pemF0aW9uIiwiY2FyIiwiYXNzZXQiLCJzdGF0aXN0aWNzIiwic21hcnRDaGFyZ2luZyJdLCJhY3RpdmVGZWF0dXJlcyI6WyJjaGFyZ2luZ1N0YXRpb25NYXAiLCJjaGFyZ2luZ1N0YXRpb25QbGFubmluZyIsInVzZXJHcm91cCIsInVzZXJQcmljaW5nIiwiY29tcGFueVByaWNpbmciLCJzaXRlQXJlYVByaWNpbmciLCJ1c2VyR3JvdXBQcmljaW5nIiwiZGVncmVzc2l2ZVByaWNpbmciXSwiZGlzdGFuY2VVbml0Ijoia21zIiwiaWF0IjoxNzUwNDA4MDY5LCJleHAiOjE3NjU5NjAwNjl9.Ui9eNJG6OFNBJboMzF2Wrew6J9Au-hkgSVfLBXnDGQ8"));
      client.print(F("Content-Length: "));
      client.println(httpRequestData.length());
      client.println();
      client.println(httpRequestData);

    } */

}





























   
   // when voltage is high enough##################### 
 /*   void setup_wifiChargeAngels() {

  
      //WiFi.mode(WIFI_STA);

     WiFi.config(ip, gateway_dns, gateway_dns); 
      WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(50);
  }

  
 
  configTime(3 * 3600, 0,"134.59.1.5", "pool.ntp.org", "time.nist.gov");

  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2) {
    delay(500);
    now = time(nullptr);
  }
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
 int epoch_time = now;


  
 
//int epoch_time = now;

// define starting date
String monthstringstart = "";
String yearstringstart = "";
String daystringstart = "";
String hourstringstart = "";
String minutestringstart = "";
String secondstringstart = "";

String monthstringend = "";
String yearstringend = "";
String daystringend = "";
String hourstringend = "";
String minutestringend = "";
String secondstringend = "";


if (month(epoch_time)>9) {
   monthstringstart = String(month(epoch_time));
}else {
   monthstringstart = "0"+String(month(epoch_time));
}
if (day(epoch_time)>9) {
   daystringstart = String(day(epoch_time));
}else {
    daystringstart = "0"+String(day(epoch_time));
}
if (hour(epoch_time)>9) {
   hourstringstart = String(hour(epoch_time));
}else {
    hourstringstart = "0"+String(hour(epoch_time));
}
if (minute(epoch_time)>9) {
   minutestringstart = String(minute(epoch_time));
}else {
   minutestringstart = "0"+String(minute(epoch_time));
}
if (second(epoch_time)>9) {
   secondstringstart = String(second(epoch_time));
}else {
   secondstringstart = "0"+String(second(epoch_time));
}
 yearstringstart = String(year(epoch_time));
int counter_moyenne_int = counter_moyenne;
// define end date:
epoch_time=epoch_time + 1;//15*max(1,counter_moyenne_int); // IF WE HAVE SLEEP EVERY 10-15 SECONDS
if (month(epoch_time)>9) {
   monthstringend = String(month(epoch_time));
}else {
   monthstringend = "0"+String(month(epoch_time));
}
if (day(epoch_time)>9) {
   daystringend = String(day(epoch_time));
}else {
    daystringend = "0"+String(day(epoch_time));
}
if (hour(epoch_time)>9) {
   hourstringend = String(hour(epoch_time));
}else {
    hourstringend = "0"+String(hour(epoch_time));
}
if (minute(epoch_time)>9) {
   minutestringend = String(minute(epoch_time));
}else {
   minutestringend = "0"+String(minute(epoch_time));
}
if (second(epoch_time)>9) {
   secondstringend = String(second(epoch_time));
}else {
   secondstringend = "0"+String(second(epoch_time));
}
 yearstringend = String(year(epoch_time));









  WiFiClientSecure client;
  client.setTrustAnchors(&cert);

  
//String currentDate = String(currentYear) + "-" + String(currentMonth) + "-" + String(monthDay);
 // Serial.println("Date: "+ String(year(epoch_time))+"-"+ String(month(epoch_time))+"-"+String(day(epoch_time))+" "+ String(hour(epoch_time))+"-"+ String(minute(epoch_time))+"-"+ String(second(epoch_time)));
  //Serial.println(currentDate);
  // HTTPClient http;
   //String serverPath = serverName;   // + "?temperature=24.37";
 String httpRequestData = "{\"assetID\": \"6365122519b9aa99b068c09a\",\"startedAt\": \""+yearstringstart+"-"+monthstringstart+"-"+daystringstart+"T"+hourstringstart+":"+minutestringstart+":"+secondstringstart+".000Z\",\"endedAt\": \""+yearstringend+"-"+monthstringend+"-"+daystringend+"T"+hourstringend+":"+minutestringend+":"+secondstringend+".000Z\",\"instantWatts\": "+String(puissanceCommuniquee)+",\"instantWattsL1\": 0,\"instantWattsL2\": 0,\"instantWattsL3\": 0,\"instantAmps\": 0,\"instantAmpsL1\": 0,\"instantAmpsL2\": 0,\"instantAmpsL3\": 0,\"instantVolts\": 0,\"instantVoltsL1\": 0,\"instantVoltsL2\": 0,\"instantVoltsL3\": 0,\"consumptionWh\": 0,\"consumptionAmps\": 0,\"stateOfCharge\": 0}";



  if (!client.connect(github_host, github_port)) {
    //Connection failed
    return;
  }

  String url = "/v1/api/assets/6365122519b9aa99b068c09a/consumptions"; //"/a/check";


// https://wokwi.com/projects/327948646817464914
//https://forum.arduino.cc/t/esp32-https-post-request/964599/2
client.stop();
    if (client.connect(github_host, 443)) {
 
      client.println("POST " + url + " HTTP/1.0");
      client.println("Host: " + (String)github_host);
      client.println(F("User-Agent: ESP"));
      client.println(F("Connection: close"));
      client.println(F("Content-Type: application/json"));
      client.println(F("Authorization: Bearer eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpZCI6IjY0MjU2MGNmNTY2ODlkNzk5MDBkZGU3MSIsInJvbGUiOiJBIiwicm9sZXNBQ0wiOlsiYWRtaW4iXSwibmFtZSI6IklNUkVERCIsIm1vYmlsZSI6IiIsImVtYWlsIjoiYmVub2l0LmNvdXJhdWRAdW5pY2UuZnIiLCJmaXJzdE5hbWUiOiJBUEkiLCJsb2NhbGUiOiJlbl9VUyIsImxhbmd1YWdlIjoiZW4iLCJjdXJyZW5jeSI6IkVVUiIsInRlbmFudElEIjoiNWMzZGU5MTA2NmMwM2YwMDA5OGRhNDBhIiwidGVuYW50TmFtZSI6IklNUkVERCAoQ2l0eSBvZiBOaWNlKSIsInRlbmFudFN1YmRvbWFpbiI6ImltcmVkZCIsInVzZXJIYXNoSUQiOiJmZTZhMjBjYTIzOWY2MjdhZTNkY2NhN2QwNWFmMDYzODZiNzY2ZGFiYTEyNmZmNzBkYTg4NmYwNTgzMjkxOTU3IiwidGVuYW50SGFzaElEIjoiNWEzZDM1YzlkNWNlNDE2MGRhMmQ5ZGFhYjA4NTA1ODc5ODFlNTExM2MyYzMyOTA0Yzc4YmU2NGEzNWU2ODliYyIsInNjb3BlcyI6WyJBc3NldDpJbkVycm9yIiwiQXNzZXQ6TGlzdCIsIkJpbGxpbmdBY2NvdW50OkJpbGxpbmdBY2NvdW50T25ib2FyZCIsIkJpbGxpbmdBY2NvdW50Okxpc3QiLCJCaWxsaW5nVHJhbnNmZXI6TGlzdCIsIkNhcjpMaXN0IiwiQ2FyQ2F0YWxvZzpMaXN0IiwiQ2hhcmdpbmdQcm9maWxlOkxpc3QiLCJDaGFyZ2luZ1N0YXRpb246SW5FcnJvciIsIkNoYXJnaW5nU3RhdGlvbjpMaXN0IiwiQ2hhcmdpbmdTdGF0aW9uQ2VydGlmaWNhdGU6TGlzdCIsIkNvbXBhbnk6TGlzdCIsIkNvbm5lY3Rpb246TGlzdCIsIkludm9pY2U6TGlzdCIsIkxvZ2dpbmc6TGlzdCIsIk9jcGlFbmRwb2ludDpMaXN0IiwiT2ljcEVuZHBvaW50Okxpc3QiLCJQYXltZW50TWV0aG9kOkxpc3QiLCJQbGFubmluZzpMaXN0IiwiUHJpY2luZ0RlZmluaXRpb246TGlzdCIsIlJlZ2lzdHJhdGlvblRva2VuOkxpc3QiLCJSZWxlYXNlTm90ZXM6TGlzdCIsIlNldHRpbmc6TGlzdCIsIlNpdGU6TGlzdCIsIlNpdGVBcmVhOkxpc3QiLCJTaXRlVXNlcnM6TGlzdCIsIlNvdXJjZTpMaXN0IiwiU3Vic2NyaXB0aW9uOkxpc3QiLCJTdWJzY3JpcHRpb246VXBkYXRlIiwiVGFnOkxpc3QiLCJUYWc6VXBkYXRlIiwiVGF4Okxpc3QiLCJUcmFuc2FjdGlvbjpJbkVycm9yIiwiVHJhbnNhY3Rpb246TGlzdCIsIlVzZXI6SW5FcnJvciIsIlVzZXI6TGlzdCIsIlVzZXI6VXBkYXRlIiwiVXNlckdyb3VwOkxpc3QiLCJVc2VyU2l0ZXM6TGlzdCIsIlVzZXJTdWJzY3JpcHRpb246TGlzdCJdLCJhY3RpdmVDb21wb25lbnRzIjpbInByaWNpbmciLCJiaWxsaW5nIiwib3JnYW5pemF0aW9uIiwiY2FyIiwiYXNzZXQiLCJzdGF0aXN0aWNzIiwic21hcnRDaGFyZ2luZyJdLCJhY3RpdmVGZWF0dXJlcyI6WyJjaGFyZ2luZ1N0YXRpb25NYXAiLCJjaGFyZ2luZ1N0YXRpb25QbGFubmluZyIsInVzZXJHcm91cCIsInVzZXJQcmljaW5nIiwiY29tcGFueVByaWNpbmciLCJzaXRlQXJlYVByaWNpbmciLCJ1c2VyR3JvdXBQcmljaW5nIiwiZGVncmVzc2l2ZVByaWNpbmciXSwiZGlzdGFuY2VVbml0Ijoia21zIiwiaWF0IjoxNzUwNDA4MDY5LCJleHAiOjE3NjU5NjAwNjl9.Ui9eNJG6OFNBJboMzF2Wrew6J9Au-hkgSVfLBXnDGQ8"));
      client.print(F("Content-Length: "));
      client.println(httpRequestData.length());
      client.println();
      client.println(httpRequestData);

    } 




     // String serverPath = serverName+String(BatteryVoltage)+"&power="+String(puissanceCommuniquee)+"&counter="+String(counter_moyenne)+"&danger="+String(Danger)+"&wakeupnumber="+counterwakeup;// + "?temperature=24.37";
      // defines the server name with URL path
  //  http.begin(client, serverPath.c_str());
   //   http.addHeader("Content-Type", "application/json");
   //   http.addHeader("Authorization", "Bearer eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpZCI6IjY0MjU2MGNmNTY2ODlkNzk5MDBkZGU3MSIsInJvbGUiOiJBIiwicm9sZXNBQ0wiOlsiYWRtaW4iXSwibmFtZSI6IklNUkVERCIsIm1vYmlsZSI6IiIsImVtYWlsIjoiYmVub2l0LmNvdXJhdWRAdW5pY2UuZnIiLCJmaXJzdE5hbWUiOiJBUEkiLCJsb2NhbGUiOiJlbl9VUyIsImxhbmd1YWdlIjoiZW4iLCJjdXJyZW5jeSI6IkVVUiIsInRlbmFudElEIjoiNWMzZGU5MTA2NmMwM2YwMDA5OGRhNDBhIiwidGVuYW50TmFtZSI6IklNUkVERCAoQ2l0eSBvZiBOaWNlKSIsInRlbmFudFN1YmRvbWFpbiI6ImltcmVkZCIsInVzZXJIYXNoSUQiOiJmZTZhMjBjYTIzOWY2MjdhZTNkY2NhN2QwNWFmMDYzODZiNzY2ZGFiYTEyNmZmNzBkYTg4NmYwNTgzMjkxOTU3IiwidGVuYW50SGFzaElEIjoiNWEzZDM1YzlkNWNlNDE2MGRhMmQ5ZGFhYjA4NTA1ODc5ODFlNTExM2MyYzMyOTA0Yzc4YmU2NGEzNWU2ODliYyIsInNjb3BlcyI6WyJBc3NldDpJbkVycm9yIiwiQXNzZXQ6TGlzdCIsIkJpbGxpbmdBY2NvdW50OkJpbGxpbmdBY2NvdW50T25ib2FyZCIsIkJpbGxpbmdBY2NvdW50Okxpc3QiLCJCaWxsaW5nVHJhbnNmZXI6TGlzdCIsIkNhcjpMaXN0IiwiQ2FyQ2F0YWxvZzpMaXN0IiwiQ2hhcmdpbmdQcm9maWxlOkxpc3QiLCJDaGFyZ2luZ1N0YXRpb246SW5FcnJvciIsIkNoYXJnaW5nU3RhdGlvbjpMaXN0IiwiQ2hhcmdpbmdTdGF0aW9uQ2VydGlmaWNhdGU6TGlzdCIsIkNvbXBhbnk6TGlzdCIsIkNvbm5lY3Rpb246TGlzdCIsIkludm9pY2U6TGlzdCIsIkxvZ2dpbmc6TGlzdCIsIk9jcGlFbmRwb2ludDpMaXN0IiwiT2ljcEVuZHBvaW50Okxpc3QiLCJQYXltZW50TWV0aG9kOkxpc3QiLCJQbGFubmluZzpMaXN0IiwiUHJpY2luZ0RlZmluaXRpb246TGlzdCIsIlJlZ2lzdHJhdGlvblRva2VuOkxpc3QiLCJSZWxlYXNlTm90ZXM6TGlzdCIsIlNldHRpbmc6TGlzdCIsIlNpdGU6TGlzdCIsIlNpdGVBcmVhOkxpc3QiLCJTaXRlVXNlcnM6TGlzdCIsIlNvdXJjZTpMaXN0IiwiU3Vic2NyaXB0aW9uOkxpc3QiLCJTdWJzY3JpcHRpb246VXBkYXRlIiwiVGFnOkxpc3QiLCJUYWc6VXBkYXRlIiwiVGF4Okxpc3QiLCJUcmFuc2FjdGlvbjpJbkVycm9yIiwiVHJhbnNhY3Rpb246TGlzdCIsIlVzZXI6SW5FcnJvciIsIlVzZXI6TGlzdCIsIlVzZXI6VXBkYXRlIiwiVXNlckdyb3VwOkxpc3QiLCJVc2VyU2l0ZXM6TGlzdCIsIlVzZXJTdWJzY3JpcHRpb246TGlzdCJdLCJhY3RpdmVDb21wb25lbnRzIjpbInByaWNpbmciLCJiaWxsaW5nIiwib3JnYW5pemF0aW9uIiwiY2FyIiwiYXNzZXQiLCJzdGF0aXN0aWNzIiwic21hcnRDaGFyZ2luZyJdLCJhY3RpdmVGZWF0dXJlcyI6WyJjaGFyZ2luZ1N0YXRpb25NYXAiLCJjaGFyZ2luZ1N0YXRpb25QbGFubmluZyIsInVzZXJHcm91cCIsInVzZXJQcmljaW5nIiwiY29tcGFueVByaWNpbmciLCJzaXRlQXJlYVByaWNpbmciLCJ1c2VyR3JvdXBQcmljaW5nIiwiZGVncmVzc2l2ZVByaWNpbmciXSwiZGlzdGFuY2VVbml0Ijoia21zIiwiaWF0IjoxNzUwNDA4MDY5LCJleHAiOjE3NjU5NjAwNjl9.Ui9eNJG6OFNBJboMzF2Wrew6J9Au-hkgSVfLBXnDGQ8");
  
      // Send HTTP GET request
  //    int httpResponseCode = http.GET();
   //   int httpResponseCode = http.POST(httpRequestData);

      // Send HTTP GET request
     // int httpResponseCode = http.GET(); //send GET request to the server
//if (httpResponseCode>0) {            
//   digitalWrite(LED_BUILTIN, LOW); // initialisation
//delay(50);               
 // digitalWrite(LED_BUILTIN, HIGH); // initialisation
//delay(50);
// }
}*/
   // ################ listen to Linky, every 15 seconds ##################### 



   // ################ listen to Linky, every 15 seconds ##################### 
void readLinky9600(){
 // Serial.println("reading TIC!");
//digitalWrite(LED_BUILTIN, LOW); // initialisation
//delay(50);               
// digitalWrite(LED_BUILTIN, HIGH); // initialisation
//delay(50);
  int communication = Serial.available(); 
  if (communication != 0)  // when we receive a character
  {
    char caracter = Serial.read(); 
    // we slide the array of detect_PAPP to keep collecting characters
    detectSINSTS[0]=detectSINSTS[1];  
    detectSINSTS[1]=detectSINSTS[2];
    detectSINSTS[2]=detectSINSTS[3];
    detectSINSTS[3]=detectSINSTS[4];
    detectSINSTS[4]=detectSINSTS[5];
    detectSINSTS[5]=detectSINSTS[6];
    detectSINSTS[6]=caracter; 
   /* detectPAPP[0]=detectPAPP[1];  
    detectPAPP[1]=detectPAPP[2];
    detectPAPP[2]=detectPAPP[3];
    detectPAPP[3]=detectPAPP[4];
    detectPAPP[4]=caracter;*/

    if (start_record == true ) // if we have received "PAPP " or "SINST ", we start recording the power
    {
     if (indice>=5 )//&& (caracter == ' ' || isdigit(caracter)==false)) // if we receive a space, the power information is over. could be more robust by considering anything above 9
      {
        indice = 0;
                 if (debug==1){
        Serial.print("got a power:");
        Serial.println(puissance);
        }
        start_record = false;
        puissance_int = String(puissance).toInt();
        puissance_Uint = (unsigned int)puissance_int;
        puissance_moyenne = puissance_moyenne + puissance_Uint;
                         if (debug==1){
         Serial.print("Puissance Uint:");
        Serial.println(puissance_Uint);                        
        Serial.print("Puissance moyenne:");
        Serial.println(puissance_moyenne);
        }
        finished_recording=true;
        if (puissance_int > Powermax)  // if the power is above 6kW
        { 
        puissanceCommuniquee = puissance_int;
        counter_moyennetemp = counter_moyenne;
        counter_moyenne = 1;
        Danger = 1;
        setup_wifi_TIP();
        Danger = 0;
        counter_moyenne = counter_moyennetemp;
        }
       /* if (puissance_int > Powermax_test_relai)  // if the power is above 6kW
        { 
          Need_to_cut = 1;
        } */        
        }
      else {  // if we have received a digit, we store it
         if (isdigit(caracter))
         {puissance[indice] = caracter;}
         else puissance[indice] = '0';   //we should discard the number  instead of putting a 0 
          indice = indice+1;
        }
    }
    else
    {
    if (detectSINSTS[0]=='S' && detectSINSTS[1]=='I' && detectSINSTS[2]=='N' && detectSINSTS[3]=='S' &&detectSINSTS[4]=='T' && detectSINSTS[5]=='S' )// && detectSINSTS[6]==' ')  
    // if (detectPAPP[0]=='P' && detectPAPP[1]=='A' && detectPAPP[2]=='P' && detectPAPP[3]=='P' && detectPAPP[4]==' ' )  
    {
       if (debug==1){
        Serial.print("We are here");
       }
       start_record = true;
          counter_moyenne = counter_moyenne+1;
          for (int i = 0; i < sizeof(puissance)/sizeof(puissance[0]); i++){
          puissance[i] = 0;}
    }
    }
  }
}




  


























  
   // ################ listen to Linky, every 15 seconds ##################### 
void readLinky1200(){
 // Serial.println("reading TIC!");
//digitalWrite(LED_BUILTIN, LOW); // initialisation
//delay(50);               
// digitalWrite(LED_BUILTIN, HIGH); // initialisation
//delay(50);
  int communication = Serial.available(); 
  if (communication != 0)  // when we receive a character
  {
    char caracter = Serial.read(); 
    // we slide the array of detect_PAPP to keep collecting characters
  /*  detectSINSTS[0]=detectSINSTS[1];  
    detectSINSTS[1]=detectSINSTS[2];
    detectSINSTS[2]=detectSINSTS[3];
    detectSINSTS[3]=detectSINSTS[4];
    detectSINSTS[4]=detectSINSTS[5];
    detectSINSTS[5]=detectSINSTS[6];
    detectSINSTS[6]=caracter; */
    detectPAPP[0]=detectPAPP[1];  
    detectPAPP[1]=detectPAPP[2];
    detectPAPP[2]=detectPAPP[3];
    detectPAPP[3]=detectPAPP[4];
    detectPAPP[4]=caracter;

    if (start_record == true ) // if we have received "PAPP " or "SINST ", we start recording the power
    {
     if (indice>=5 )//&& (caracter == ' ' || isdigit(caracter)==false)) // if we receive a space, the power information is over. could be more robust by considering anything above 9
      {
        indice = 0;
                 if (debug==1){
        Serial.print("got a power:");
        Serial.println(puissance);}
        start_record = false;
        puissance_int = String(puissance).toInt();
        puissance_Uint = (unsigned int)puissance_int;
        puissance_moyenne = puissance_moyenne + puissance_Uint;
        finished_recording=true;
        if (puissance_int > Powermax)  // if the power is above 6kW
        { 
        puissanceCommuniquee = puissance_int;
        counter_moyennetemp = counter_moyenne;
        counter_moyenne = 1;
        Danger = 1;
        setup_wifi_TIP();
        Danger = 0;
        counter_moyenne = counter_moyennetemp;
        }
       /* if (puissance_int > Powermax_test_relai)  // if the power is above 6kW
        { 
          Need_to_cut = 1;
        } */        
        }
      else {  // if we have received a digit, we store it
         if (isdigit(caracter))
         {puissance[indice] = caracter;}
         else puissance[indice] = '0';   //we should discard the number  instead of putting a 0 
          indice = indice+1;
        }
    }
    else
    {
 //   if (detectSINSTS[0]=='S' && detectSINSTS[1]=='I' && detectSINSTS[2]=='N' && detectSINSTS[3]=='S' &&detectSINSTS[4]=='T' && detectSINSTS[5]=='S')  
     if (detectPAPP[0]=='P' && detectPAPP[1]=='A' && detectPAPP[2]=='P' && detectPAPP[3]=='P' && detectPAPP[4]==' ' )  
    {
          start_record = true;
          counter_moyenne = counter_moyenne+1;
          for (int i = 0; i < sizeof(puissance)/sizeof(puissance[0]); i++){
          puissance[i] = 0;}
    }
    }
  }
}




















 void setup() {
     delay(1000);

 pinMode(LED_BUILTIN, OUTPUT);
  // pinMode(PIN_Resistance_RX, OUTPUT);
  // digitalWrite(PIN_Resistance_RX, LOW); // Deactivate the resistance after the optocoupler to ensure the 1200baud is by default

   digitalWrite(LED_BUILTIN, HIGH); // initialisation
   delay(10);
   digitalWrite(LED_BUILTIN, LOW);
   delay(100);
   digitalWrite(LED_BUILTIN, HIGH); // initialisation
      delay(100);


  // pinMode(5, OUTPUT);
   //digitalWrite(5, LOW);
   Serial.begin(1200,SERIAL_7E1);    // Define and start serial monitoring of the Linky TIC
//Serial.swap();

       BatteryVoltage = analogRead(A0); // we read voltage
                       if (debug==1){
BatteryVoltage = 255;}

if (BatteryVoltage>voltageThreshold2)
//if (BatteryVoltage>0)
   {
     uint addr = 0; //address of eeprom memory
    // we use EEPROM in order to keep the values stored even when the module is sleeping
  struct { 
    uint puissancemoy = 0; // we store the average power (but we could store all the previous power)
    uint countermoy = 0;  // counter of tnhe number of times we went to sleep without being able to send data
  //  uint booleenfirststartup = 0;//char str[20] = "";
  } dataEEPROM;

   digitalWrite(LED_BUILTIN, LOW);
   delay(50);
   digitalWrite(LED_BUILTIN, HIGH); // initialisation
      delay(1000);
 digitalWrite(LED_BUILTIN, LOW);
   delay(500);
      digitalWrite(LED_BUILTIN, HIGH); // initialisation
      delay(1000);

    // commit 512 bytes of ESP8266 flash (for "EEPROM" emulation)
  // this step actually loads the content (512 bytes) of flash into 
  // a 512-byte-array cache in RAM
  EEPROM.begin(512);

  // read bytes (i.e. sizeof(data) from "EEPROM"),
  // in reality, reads from byte-array cache
  // cast bytes into structure called data
  EEPROM.get(addr,dataEEPROM);
  puissance_moyenne = dataEEPROM.puissancemoy; // we retrieve average power from eeprom
  counter_moyenne = dataEEPROM.countermoy;
 // firststartup = dataEEPROM.booleenfirststartup;




        pinMode(A0, INPUT); // we define the analog pin (ADC PIN below RST)
      counterwakeup = counterwakeup+1; // we count the number of times the dongle woke up and stored data in the power average
      t1 = millis(); // we start to record time to use the timeout to go to sleep if we do not read any data from Linky
       BatteryVoltage = analogRead(A0); // we read voltage
                if (debug==1){
Serial.println("Starting Reading loop");
BatteryVoltage = 255;}
BatteryVoltage = 255;
      timeoutReadLinky = 0;
       while(finished_recording == false && timeoutReadLinky<5000){  
             readLinky1200(); // we read Linky data until we have found a data (finished_recording) or if the timeout is reached 
             t2 = millis();
             timeoutReadLinky = t2-t1; 
        }
        if(finished_recording){
            digitalWrite(LED_BUILTIN, HIGH); // initialisation
   delay(100);
   digitalWrite(LED_BUILTIN, LOW);
   delay(50);
   digitalWrite(LED_BUILTIN, HIGH); // initialisation
   delay(100);
      digitalWrite(LED_BUILTIN, LOW);
   delay(50);
   digitalWrite(LED_BUILTIN, HIGH); // initialisation
   delay(100);
      digitalWrite(LED_BUILTIN, LOW);
   delay(50);
   digitalWrite(LED_BUILTIN, HIGH); // initialisation
   delay(100);
      digitalWrite(LED_BUILTIN, LOW);
   delay(50);
   digitalWrite(LED_BUILTIN, HIGH); // initialisation
   delay(100);


        }
         if (debug==1){
Serial.println("End of Reading loop");}
  //       if(BatteryVoltage>voltageThreshold && finished_recording) {  // if we have enough voltage, we send the data and reinitialise the values
         if(debug==1 || (BatteryVoltage>voltageThreshold && finished_recording)) {  // if we have enough voltage, we send the data and reinitialise the values
         puissanceCommuniquee=puissance_moyenne/counter_moyenne; 
         // puissanceCommuniquee=puissance_Uint;
         if (debug==1){
          Serial.println("ready to send");
         }
   
          //  setup_wifi_TIP();
            counterwakeup = 0;
            counter_moyenne = 0;
            puissance_moyenne = 0;
        }
  

// now we store the updated values back in the eeprom before going to sleep
dataEEPROM.puissancemoy =   puissance_moyenne ;
dataEEPROM.countermoy= counter_moyenne ;

 //dataEEPROM.booleenfirststartup = 1; 

// replace values in byte-array cache with modified data
  // no changes made to flash, all in local byte-array cache
  EEPROM.put(addr,dataEEPROM);
    // actually write the content of byte-array cache to
  // hardware flash.  flash write occurs if and only if one or more byte
  // in byte-array cache has been changed, but if so, ALL 512 bytes are 
  // written to flash
  EEPROM.commit();  
  
finished_recording = false;
   }
 // ESP.deepSleep(20e6); // we go to sleep 10 seconds
  ESP.deepSleep(10e6); // we go to sleep 10 seconds
    }
    void loop() { // nothing here, we go to sleep and do not enter an infinite loop
    }
  
