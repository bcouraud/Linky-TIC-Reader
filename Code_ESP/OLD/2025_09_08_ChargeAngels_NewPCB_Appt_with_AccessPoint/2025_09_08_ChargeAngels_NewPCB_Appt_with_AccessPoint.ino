
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
#include <DNSServer.h>  // for the accesspoint
#include <ESP8266WebServer.h> // for the accesspoint
/*Specifying the SSID and Password of the AP*/
const char* ap_ssid = "LinkyDongle"; //Access Point SSID
const char* ap_password= "password123"; //Access Point Password
uint8_t max_connections=1;//Maximum Connection Limit for AP
int current_stations=0, new_stations=0;
String WiFi_Name = "";
String WiFi_Password = "";
char value;

const byte DNS_PORT = 53;
DNSServer dnsServer;
//Specifying the Webserver instance to connect with HTTP Port: 80
ESP8266WebServer server(80);
IPAddress apIP(192, 168, 0, 1);  // Custom AP IP

uint addr = 0;
int length_Wifiname_EEPROM = 40;

bool WiFi_status=false; 

struct { 
    uint puissancemoy = 0; // we store the average power (but we could store all the previous power)
    uint countermoy = 0;  // counter of tnhe number of times we went to sleep without being able to send data
    bool sent_last_time = false;  // Was data sent last time?
    char wifinameEEPROM[32];
    char wifipasswordEEPROM[32];
  } 
  dataEEPROM;

String header = 
"<!DOCTYPE html><html><head>"
"<meta name='viewport' content='width=device-width, initial-scale=1.0'>"
"<title>WiFi Control</title>"
"<style>"
"body{margin:0;font-family:Arial,sans-serif;color:#bdbdbd;text-align:center;"
"background:radial-gradient(circle,#202126,#161924);}"
"h1,h2{margin:20px;}"
"form{margin-top:20px;}"
"select,input[type=text],input[type=submit]{"
"font-size:1rem;padding:10px;margin:10px;border:2px solid #00acfb;"
"border-radius:10px;background:transparent;color:#bdbdbd;"
"box-shadow:0 0 8px rgba(0,172,251,.6);transition:.3s;}"
"select:focus,input[type=text]:focus,input[type=submit]:focus{"
"outline:none;box-shadow:0 0 20px rgba(0,172,251,1);}"
"input[type=submit]{cursor:pointer;color:#00acfb;}"
"input[type=submit]:hover{background:rgba(0,172,251,.2);}"
"</style></head><body>";

String footer = "</body></html>";

#include "certs.h"
#define PIN_Resistance_RX D6    // GPIO12

//#ifndef STASSID
//#define STASSID "VM9478242" // "Livebox-CB4A" //
//#define STAPSK "xbeubowf6ceNiJjx" // "FZNdu4aWShWEp3NSox" //
//#endif
#include <EEPROM.h>


//const long utcOffsetInSeconds = 3600;

//##############   variables definition   ###########################
int counterwakeup = 0; // counter to check how many times we wen to wake up before to send requests
String StringToDetect = "SINSTS";
char detectSINSTS[7]; // array that records the characters received
char detectPAPP[5]; // array that records the characters received
bool sent_last_time_code = false; // boolean that states that the data was sent through wifi on the last wake up (which seems to create issues, maybe due to a false ADC measure)
bool state = HIGH; // state for the LED
bool start_record = false;  // boolean triggered to start recording the power measurement
char puissance[5]; // power measured, in char
int puissance_int = 0; // power measured, in int
uint puissance_Uint = 0; // power measured, in int
uint puissance_moyenne = 0; // averagepower measured, in int
uint firststartup = 0;
uint timeoutconnectWifi = 0;
float real_voltage = 0;
int indice = 0;  // index for the recording of the power
int debug = 0;  // true when we are in debug mode
uint counter_moyenne = 0; // counter to compute the average
uint counter_moyennetemp = 0; // used to store temporary counter to compute the average
int puissanceCommuniquee = 0; // puissance que l'on communique via wifi
bool finished_recording = false; //specifies if we can go back to sleep (= we received the signal from Linky)
long int timeoutReadLinky = 0; // timeout to stop trying to read linky if there is no data to read
//const int A0 = A0;  // Analog input pin that the potentiometer is attached to
int rawADC = 0;
int Danger = 0; // indicate if Power > Powermax
int Powermax = 10000; // to be read from Linky, and stored in EEPROM
long int t1 = 0;
long int t2=0;
float voltageThreshold = 6.4; //190; //207; // voltage limit to startup the communication
float voltageThreshold2 = 5.4; //170; // voltage limit to startup the process
int epoch_time = 0;
int last_epoch_time = 0;
//uint voltageThreshold = 200; //207; // voltage limit to startup the communication
//uint voltageThreshold2 = 180; // voltage limit to startup the process
//const char* host_url = "tip-imredd.unice.fr";
const char* host_url = "imredd.charge-angels.com";

const char* TIP_host = "tip-imredd.unice.fr";
int counterWifi = 0;

const char* ssid_Wifi;
const char* password_Wifi;

X509List cert(cert_DigiCert_Global_Root_CA);



float mapVoltageToSleep(float voltage) {
  const float Vmax = 6.1;
  const float Vmin = 5.5;
  const float Tmin = 20.0; // seconds
  const float Tmax = 60.0; // seconds

  if (voltage >= Vmax) return Tmin;
  //if (voltage <= Vmin) return Tmax;

  // Linear interpolation
  float t = Tmin + (Vmax - voltage) * (Tmax - Tmin) / (Vmax - Vmin);
  return t;
}







   // ################ set up wifi + send message function, is triggered only 

   void setup_wifi(const char* host_parameter) {

   digitalWrite(LED_BUILTIN, LOW);
   delay(20);
   digitalWrite(LED_BUILTIN, HIGH); // initialisation
      //WiFi.mode(WIFI_STA);
    // WiFi.config(ip, gateway_dns, gateway_dns); 
      WiFi.begin(ssid_Wifi, password_Wifi);





             t1 = millis();
             timeoutconnectWifi = 0;
                              int retries = 0;
                              const int maxRetries = 20; // About 10 seconds
                              while (WiFi.status() != WL_CONNECTED && retries < maxRetries) {
                                delay(200);
                                Serial.print(".");
                                retries++;
                              }


   digitalWrite(LED_BUILTIN, LOW);
   delay(20);
   digitalWrite(LED_BUILTIN, HIGH); // initialisation
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
 epoch_time = now;


  
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








if (host_parameter == "imredd.charge-angels.com"){
  WiFiClientSecure client;
  client.setTrustAnchors(&cert);



               if (debug==1){
 Serial.print("Connecting to:");
Serial.println(host_parameter);
           }


   String httpRequestData = "{\"assetID\": \"65290bc4d788d24fc7982ebe\",\"startedAt\": \""+yearstringstart+"-"+monthstringstart+"-"+daystringstart+"T"+hourstringstart+":"+minutestringstart+":"+secondstringstart+".000Z\",\"endedAt\": \""+yearstringend+"-"+monthstringend+"-"+daystringend+"T"+hourstringend+":"+minutestringend+":"+secondstringend+".000Z\",\"instantWatts\": "+String(puissanceCommuniquee)+",\"instantWattsL1\": 0,\"instantWattsL2\": 0,\"instantWattsL3\": 0,\"instantAmps\": 0,\"instantAmpsL1\": 0,\"instantAmpsL2\": 0,\"instantAmpsL3\": 0,\"instantVolts\": 0,\"instantVoltsL1\": 0,\"instantVoltsL2\": 0,\"instantVoltsL3\": 0,\"consumptionWh\": 0,\"consumptionAmps\": 0,\"stateOfCharge\": 0}";



  if (!client.connect(host_url_certif, url_port)) {
    //Connection failed
               if (debug==1){
 Serial.println("First Connection Failed to url:");
Serial.println(host_parameter);
           }

    return;
  }

  String url = "/v1/api/assets/65290bc4d788d24fc7982ebe/consumptions"; //"/a/check";

// https://wokwi.com/projects/327948646817464914
//https://forum.arduino.cc/t/esp32-https-post-request/964599/2
client.stop();
    if (client.connect(host_url_certif, url_port)) {
      client.println("POST " + url + " HTTP/1.0");
      client.println("Host: " + (String)host_url);
      client.println(F("User-Agent: ESP"));
      client.println(F("Connection: close"));
      client.println(F("Content-Type: application/json"));
      client.println(F("Authorization: Bearer eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpZCI6IjY0MjU2MGNmNTY2ODlkNzk5MDBkZGU3MSIsInJvbGUiOiJBIiwicm9sZXNBQ0wiOlsiYWRtaW4iXSwibmFtZSI6IklNUkVERCIsIm1vYmlsZSI6IiIsImVtYWlsIjoiYmVub2l0LmNvdXJhdWRAdW5pY2UuZnIiLCJmaXJzdE5hbWUiOiJBUEkiLCJsb2NhbGUiOiJlbl9VUyIsImxhbmd1YWdlIjoiZW4iLCJjdXJyZW5jeSI6IkVVUiIsInRlbmFudElEIjoiNWMzZGU5MTA2NmMwM2YwMDA5OGRhNDBhIiwidGVuYW50TmFtZSI6IklNUkVERCAoQ2l0eSBvZiBOaWNlKSIsInRlbmFudFN1YmRvbWFpbiI6ImltcmVkZCIsInVzZXJIYXNoSUQiOiJmZTZhMjBjYTIzOWY2MjdhZTNkY2NhN2QwNWFmMDYzODZiNzY2ZGFiYTEyNmZmNzBkYTg4NmYwNTgzMjkxOTU3IiwidGVuYW50SGFzaElEIjoiNWEzZDM1YzlkNWNlNDE2MGRhMmQ5ZGFhYjA4NTA1ODc5ODFlNTExM2MyYzMyOTA0Yzc4YmU2NGEzNWU2ODliYyIsInNjb3BlcyI6WyJBc3NldDpJbkVycm9yIiwiQXNzZXQ6TGlzdCIsIkJpbGxpbmdBY2NvdW50OkJpbGxpbmdBY2NvdW50T25ib2FyZCIsIkJpbGxpbmdBY2NvdW50Okxpc3QiLCJCaWxsaW5nVHJhbnNmZXI6TGlzdCIsIkNhcjpMaXN0IiwiQ2FyQ2F0YWxvZzpMaXN0IiwiQ2hhcmdpbmdQcm9maWxlOkxpc3QiLCJDaGFyZ2luZ1N0YXRpb246SW5FcnJvciIsIkNoYXJnaW5nU3RhdGlvbjpMaXN0IiwiQ2hhcmdpbmdTdGF0aW9uQ2VydGlmaWNhdGU6TGlzdCIsIkNvbXBhbnk6TGlzdCIsIkNvbm5lY3Rpb246TGlzdCIsIkludm9pY2U6TGlzdCIsIkxvZ2dpbmc6TGlzdCIsIk9jcGlFbmRwb2ludDpMaXN0IiwiT2ljcEVuZHBvaW50Okxpc3QiLCJQYXltZW50TWV0aG9kOkxpc3QiLCJQbGFubmluZzpMaXN0IiwiUHJpY2luZ0RlZmluaXRpb246TGlzdCIsIlJlZ2lzdHJhdGlvblRva2VuOkxpc3QiLCJSZWxlYXNlTm90ZXM6TGlzdCIsIlNldHRpbmc6TGlzdCIsIlNpdGU6TGlzdCIsIlNpdGVBcmVhOkxpc3QiLCJTaXRlVXNlcnM6TGlzdCIsIlNvdXJjZTpMaXN0IiwiU3Vic2NyaXB0aW9uOkxpc3QiLCJTdWJzY3JpcHRpb246VXBkYXRlIiwiVGFnOkxpc3QiLCJUYWc6VXBkYXRlIiwiVGF4Okxpc3QiLCJUcmFuc2FjdGlvbjpJbkVycm9yIiwiVHJhbnNhY3Rpb246TGlzdCIsIlVzZXI6SW5FcnJvciIsIlVzZXI6TGlzdCIsIlVzZXI6VXBkYXRlIiwiVXNlckdyb3VwOkxpc3QiLCJVc2VyU2l0ZXM6TGlzdCIsIlVzZXJTdWJzY3JpcHRpb246TGlzdCJdLCJhY3RpdmVDb21wb25lbnRzIjpbInByaWNpbmciLCJiaWxsaW5nIiwib3JnYW5pemF0aW9uIiwiY2FyIiwiYXNzZXQiLCJzdGF0aXN0aWNzIiwic21hcnRDaGFyZ2luZyJdLCJhY3RpdmVGZWF0dXJlcyI6WyJjaGFyZ2luZ1N0YXRpb25NYXAiLCJjaGFyZ2luZ1N0YXRpb25QbGFubmluZyIsInVzZXJHcm91cCIsInVzZXJQcmljaW5nIiwiY29tcGFueVByaWNpbmciLCJzaXRlQXJlYVByaWNpbmciLCJ1c2VyR3JvdXBQcmljaW5nIiwiZGVncmVzc2l2ZVByaWNpbmciXSwiZGlzdGFuY2VVbml0Ijoia21zIiwiaWF0IjoxNzUxNTc5NTQxLCJleHAiOjE3NjcxMzE1NDF9.U54OuAHPW2GmCDo9aCVFqQoIXNUq00UA9jggR6ixqHo"));
      client.print(F("Content-Length: "));
      client.println(httpRequestData.length());
      client.println();
      client.println(httpRequestData);

    } 
      digitalWrite(LED_BUILTIN, HIGH); // initialisation
  delay(10);
  digitalWrite(LED_BUILTIN, LOW);
    delay(50);
   digitalWrite(LED_BUILTIN, HIGH); // initialisation
     delay(10);


               if (debug==1){
 Serial.println("Request Sent:");
 Serial.println(httpRequestData);
           }



  
}


else {



         WiFiClientSecure client;
         //client.setTrustAnchors(&cert);
         client.setInsecure(); //the magic line, use with caution

         
         if (!client.connect(TIP_host, url_port)) {
            //Connection failed
            return;
         }

         String url = "/nodes/imredd/energyconso/linky";

         String httpRequestData = "linkySensor,sensor_id=LinkyHouse4 power="+String(puissanceCommuniquee)+",danger="+String(Danger)+",voltage="+String(real_voltage)+",frequency=50.2,pf=0.9 "+String(epoch_time)+"000000000";
         client.print(String("POST ") + url + " HTTP/1.0\r\n" + "Host: " + TIP_host + "\r\n" + "User-Agent: BuildFailureDetectorESP8266\r\n" +  "Content-Length: " + httpRequestData.length()   + "\r\n"+"Connection: close\r\n\r\n"+httpRequestData);

      }

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
        setup_wifi(host_url);
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
        setup_wifi(host_url);
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
   pinMode(LED_BUILTIN, OUTPUT);
   pinMode(PIN_Resistance_RX, OUTPUT);
   digitalWrite(PIN_Resistance_RX, LOW); // Deactivate the resistance after the optocoupler to ensure the 1200baud is by default

   digitalWrite(LED_BUILTIN, HIGH); // initialisation
   delay(10);
   digitalWrite(LED_BUILTIN, LOW);
   delay(50);
   digitalWrite(LED_BUILTIN, HIGH); // initialisation
      delay(50);

 // Serial.begin(115200);
 // Serial.println();
 // Serial.println("Hello");
   pinMode(12, INPUT); // reading Voltage (digital) on D2 from the USB
  int connection_USB = digitalRead(12);  // will be 0 (LOW) or 1 (HIGH)

 // Serial.print("Voltage = ");
 // Serial.println(connection_USB);



  if (connection_USB == HIGH) {
  //   Serial.println("USB Mode");
               Serial.begin(115200);
               Serial.println(connection_USB);
                                          // an USB cable is connected
                     digitalWrite(LED_BUILTIN, HIGH); // initialisation
                     delay(500);
                     digitalWrite(LED_BUILTIN, LOW);
                     delay(2000);
                     digitalWrite(LED_BUILTIN, HIGH); // initialisation
                        delay(500);
                     digitalWrite(LED_BUILTIN, LOW);
                     delay(1000);

                     digitalWrite(LED_BUILTIN, HIGH); // initialisation
                        delay(500);
                     digitalWrite(LED_BUILTIN, LOW);
                     delay(3000);
                     digitalWrite(LED_BUILTIN, HIGH); // initialisation
                        delay(500);                             
                     int n = WiFi.scanNetworks();
                      Serial.println("Scan completed");

                     if (n == 0) {
                        Serial.println("No networks found");
                     } else {
                        Serial.print(n);
                        Serial.println(" networks found");
                        for (int i = 0; i < n; ++i) {
                           Serial.print(i + 1);
                           Serial.print(": ");
                           Serial.println(WiFi.SSID(i));
                        }
                     }




                     WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));

                     //Setting the AP Mode with SSID, Password, and Max Connection Limit
                     if(WiFi.softAP(ap_ssid,ap_password,1,false,max_connections)==true)
                     {
                           
                        // Start DNS server to redirect all domains to apIP
                        dnsServer.start(DNS_PORT, "*", apIP);

                        Serial.print("Access Point is Created with SSID: ");
                        Serial.println(ap_ssid);
                        Serial.print("Max Connections Allowed: ");
                        Serial.println(max_connections);
                        Serial.print("Access Point IP: ");
                        Serial.println(WiFi.softAPIP());
                     }

                     else
                     {
                        Serial.println("Unable to Create Access Point");
                     }

                     //Specifying the functions which will be executed upon corresponding GET request from the client
                     server.on("/",handle_OnConnect);

                     //server.on("/ledon",handle_ledon);
                     //server.on("/ledoff",handle_ledoff);
                     server.on("/WiFion",handle_WiFion);
                     server.on("/WiFioff",handle_WiFioff);

                     server.onNotFound(handle_NotFound);
                     server.on("/submitted",handle_submitted);

                     delay(5000);
              //       uint addr = 0; //address of eeprom memory

                     // read bytes (i.e. sizeof(data) from "EEPROM"),
                     // in reality, reads from byte-array cache
                     // cast bytes into structure called data
                     EEPROM.begin(512);

                     EEPROM.get(addr,dataEEPROM);
                     WiFi_Name = dataEEPROM.wifinameEEPROM; // we retrieve wifiname from eeprom

                     Serial.print("Data stored for Wifi Name in EEPROM: ");
                     Serial.println(WiFi_Name);

                     //Starting the Server
                     server.begin();
                     Serial.println("HTTP Server Started");

   while(1){
                  //Assign the server to handle the clients
               dnsServer.processNextRequest();
               server.handleClient();


               //Continuously check how many stations are connected to Soft AP and notify whenever a new station is connected or disconnected
               new_stations=WiFi.softAPgetStationNum();
                  
               if(current_stations<new_stations)//Device is Connected
               {
                  current_stations=new_stations;
                  Serial.print("New Device Connected to SoftAP... Total Connections: ");
                  Serial.println(current_stations);
               }
                  
               if(current_stations>new_stations)//Device is Disconnected
               {
                  current_stations=new_stations;
                  Serial.print("Device disconnected from SoftAP... Total Connections: ");
                  Serial.println(current_stations);
               }
               
   }


  } else {
       // We are connected to the Linky

            digitalWrite(LED_BUILTIN, HIGH); // initialisation
            delay(10);
            digitalWrite(LED_BUILTIN, LOW);
            delay(20);
            digitalWrite(LED_BUILTIN, HIGH); // initialisation
             delay(20);



                     pinMode(5, OUTPUT);
                     digitalWrite(5, LOW);
                     Serial.begin(1200,SERIAL_7E1);    // Define and start serial monitoring of the Linky TIC
                     //Serial.swap();

                     rawADC = analogRead(A0); // we read voltage
                     real_voltage = 0.0312*rawADC-0.4873;  //5.24V --> 181  or 5.06V -->174  or 4.90 -->170 or 4.18 --> 147

                                    if (debug==1){
                     rawADC = 255;}

                     if (real_voltage>voltageThreshold2)
                     //if (rawADC>0)
                     {
                   //  uint addr = 0; //address of eeprom memory
                     // we use EEPROM in order to keep the values stored even when the module is sleeping





                     digitalWrite(LED_BUILTIN, LOW);
                     delay(20);
                     digitalWrite(LED_BUILTIN, HIGH); // initialisation
                     delay(10);

                     // commit 512 bytes of ESP8266 flash (for "EEPROM" emulation)
                     // this step actually loads the content (512 bytes) of flash into 
                     // a 512-byte-array cache in RAM
                     EEPROM.begin(512);

                     // read bytes (i.e. sizeof(data) from "EEPROM"),
                     // in reality, reads from byte-array cache
                     // cast bytes into structure called data
                     EEPROM.get(addr, dataEEPROM);
                     // temporary String holders
                     String ssid_tmp     = dataEEPROM.wifinameEEPROM;
                     String password_tmp = dataEEPROM.wifipasswordEEPROM;

                     if (ssid_tmp.length() > 0 && password_tmp.length() > 0) {
                     // Use EEPROM values
                     ssid_Wifi     = ssid_tmp.c_str();
                     password_Wifi = password_tmp.c_str();
                     } else {
                     // Fallback to hardcoded defaults
                     ssid_Wifi     = "VM9478242";
                     password_Wifi = "xbeubowf6ceNiJjx";
                     digitalWrite(LED_BUILTIN, LOW);
                     delay(50);
                     digitalWrite(LED_BUILTIN, HIGH); // initialisation
                     delay(80);
                     digitalWrite(LED_BUILTIN, LOW);
                     delay(50);
                     digitalWrite(LED_BUILTIN, HIGH); // initialisation
                     delay(80);

                     }
                     puissance_moyenne = dataEEPROM.puissancemoy; // we retrieve average power from eeprom
                     counter_moyenne = dataEEPROM.countermoy;
                     // firststartup = dataEEPROM.booleenfirststartup;




                     pinMode(A0, INPUT); // we define the analog pin (ADC PIN below RST)
                     counterwakeup = counterwakeup+1; // we count the number of times the dongle woke up and stored data in the power average
                     t1 = millis(); // we start to record time to use the timeout to go to sleep if we do not read any data from Linky
                     rawADC = analogRead(A0); // we read voltage
                     real_voltage = 0.0312*rawADC-0.4873;  //5.24V --> 181  or 5.06V -->174  or 4.90 -->170 or 4.18 --> 147
                              if (debug==1){
                     Serial.println("Starting Reading loop");
                     rawADC = 255;}

                     timeoutReadLinky = 0;
                     while(finished_recording == false && timeoutReadLinky<5000){  
                           readLinky1200(); // we read Linky data until we have found a data (finished_recording) or if the timeout is reached 
                           t2 = millis();
                           timeoutReadLinky = t2-t1; 
                     }
                     if(finished_recording){
                        digitalWrite(LED_BUILTIN, HIGH); // initialisation
                     delay(10);
                     digitalWrite(LED_BUILTIN, LOW);
                     delay(20);
                     digitalWrite(LED_BUILTIN, HIGH); // initialisation
                     delay(10);


                     }
                     if (debug==1){
                     Serial.println("End of Reading loop");}
                     //       if(rawADC>voltageThreshold && finished_recording) {  // if we have enough voltage, we send the data and reinitialise the values real_voltage = (rawADC/ 1024) * 2.97 * 10;  //5.24V --> 181  or 5.06V -->174  or 4.90 -->170 or 4.18 --> 147
                     if(debug==1 || (real_voltage>voltageThreshold && finished_recording && !dataEEPROM.sent_last_time)) {  // if we have enough voltage, we send the data and reinitialise the values
                     puissanceCommuniquee=puissance_moyenne/counter_moyenne; 
                     // puissanceCommuniquee=puissance_Uint;
                     if (debug==1){
                        Serial.println("ready to send");
                     }


                        dataEEPROM.sent_last_time = true;
                    //      if (ssid_Wifi.length() > 0) {
                     if (ssid_Wifi != nullptr && strlen(ssid_Wifi) > 0) {
                     digitalWrite(LED_BUILTIN, LOW);
                     delay(20);
                     digitalWrite(LED_BUILTIN, HIGH); // initialisation
                     delay(20);
          
                              setup_wifi(host_url);
                              counterwakeup = 0;
                              counter_moyenne = 0;
                              puissance_moyenne = 0;
                      }
                     } else {
                     dataEEPROM.sent_last_time = false;
                     }


                     // now we store the updated values back in the eeprom before going to sleep
                     dataEEPROM.puissancemoy =   puissance_moyenne ;
                     dataEEPROM.countermoy= counter_moyenne ;

                     //dataEEPROM.booleenfirststartup = 1; 

                     // replace values in byte-array cache with modif   ied data
                     // no changes made to flash, all in local byte-array cache
                     EEPROM.put(addr,dataEEPROM);
                     // actually write the content of byte-array cache to
                     // hardware flash.  flash write occurs if and only if one or more byte
                     // in byte-array cache has been changed, but if so, ALL 512 bytes are 
                     // written to flash
                     EEPROM.commit();  

                     finished_recording = false;
                     }
                     digitalWrite(LED_BUILTIN, LOW);
                     delay(20);
                     digitalWrite(LED_BUILTIN, HIGH); // initialisation

                    // Serial.println("going to sleep... good night!");
                     float sleepSec = mapVoltageToSleep(real_voltage);
                     //sleepSec = 10;
                     ESP.deepSleep((uint64_t)(sleepSec * 1000000.0)); // convert to microseconds

                     // ESP.deepSleep(20e6); // we go to sleep 10 seconds
                     //ESP.deepSleep(20e6); // we go to sleep 10 seconds
            }


    }
    void loop() { // nothing here, we go to sleep and do not enter an infinite loop
    }
  


void handle_OnConnect()
{
  Serial.println("Client Connected");
  server.send(200, "text/html", HTML()); 

  if (server.method() == HTTP_POST)
  {
    WiFi_Name = server.arg("wifiname");
    WiFi_Password = server.arg ("password");

    server.send(200, "text/html", "<!doctype html lang='en'<head><meta charset='utf-8'><meta name='viewpoint' content='width=device-width, initial-scale=1'");
  }

  else
  {
    server.send(200, "text/html", "<!doctype html lang='en'<head><meta charset='utf-8'><meta name='viewpoint' content='width=device-width, initial-scale=1'");
  }
}
 
void handle_WiFion()
{
   Serial.println("WiFi ON");
  WiFi_status=true;
  server.send(200, "text/html", HTML());

  for (int i =0; i<length_Wifiname_EEPROM; i++)
  {
    Serial.print(EEPROM.read(i));
  }
}
void handle_submitted()
{
   Serial.println("Received password");
  String message = "";

  if (server.arg("wifiname") == "" || server.arg("wifipassword") == "") {
    message = "Missing WiFi credentials.";
    server.send(200, "text/html", "<h2>Missing WiFi name or password!</h2>");
    return;
  }

  WiFi_Name = server.arg("wifiname");
  WiFi_Password = server.arg("wifipassword");

  Serial.println("WiFi Name received: " + WiFi_Name);
  Serial.println("Storing in EEPROM");

  // Store in EEPROM
  strncpy(dataEEPROM.wifinameEEPROM, WiFi_Name.c_str(), sizeof(dataEEPROM.wifinameEEPROM));
dataEEPROM.wifinameEEPROM[sizeof(dataEEPROM.wifinameEEPROM)-1] = '\0';

strncpy(dataEEPROM.wifipasswordEEPROM, WiFi_Password.c_str(), sizeof(dataEEPROM.wifipasswordEEPROM));
dataEEPROM.wifipasswordEEPROM[sizeof(dataEEPROM.wifipasswordEEPROM)-1] = '\0';

 // dataEEPROM.wifinameEEPROM = WiFi_Name;
 // dataEEPROM.wifipasswordEEPROM = WiFi_Password;
  EEPROM.put(addr, dataEEPROM);
  EEPROM.commit();

  Serial.println("Stored WiFi credentials in EEPROM:");
  Serial.print("SSID: "); Serial.println(WiFi_Name);
  Serial.print("Password: "); Serial.println(WiFi_Password);

   Serial.println("Trying to connect to Wifi");



                     // read bytes (i.e. sizeof(data) from "EEPROM"),
                     // in reality, reads from byte-array cache
                     // cast bytes into structure called data
                     EEPROM.get(addr, dataEEPROM);
                     // temporary String holders
                     String ssid_tmp     = dataEEPROM.wifinameEEPROM;
                     String password_tmp = dataEEPROM.wifipasswordEEPROM;

                     if (ssid_tmp.length() > 2 && password_tmp.length() > 0) {
                     // Use EEPROM values
                     ssid_Wifi     = ssid_tmp.c_str();
                     password_Wifi = password_tmp.c_str();
                     } else {
                     // Fallback to hardcoded defaults
                        Serial.println("\nFallback to hardcoded defaults");

                     ssid_Wifi     = "VM9478242";
                     password_Wifi = "xbeubowf6ceNiJjx";
                     }

                        Serial.print("\nchecking the ssid and pwd:");
                        Serial.println(ssid_Wifi);
                     if (ssid_Wifi != nullptr && strlen(ssid_Wifi) > 0) {
                                  Serial.println("\nTrying to connect");

                        // Switch to AP + STA mode
                        WiFi.mode(WIFI_AP_STA);

                        // Try connecting to user's WiFi
                              WiFi.begin(ssid_Wifi, password_Wifi);


                           int retries = 0;
                           const int maxRetries = 20; // About 10 seconds
                           while (WiFi.status() != WL_CONNECTED && retries < maxRetries) {
                              delay(500);
                              Serial.print(".");
                              retries++;
                           }

                           if (WiFi.status() == WL_CONNECTED) {
                           message  = "<h2>Merci ! Le dongle a pu se connecter au Wifi.</h2>";
                           message += "<p>Vous pouvez desormais debrancher le câble USB et brancher le dongle Linky sur votre compteur, comme indique dans la procedure.</p>";
                           } else {
                           Serial.println("\nImpossible de se connecter au WiFi.");
                           message  = "<h2>Impossible de se connecter au WiFi.</h2>";
                           message += "<p>Veuillez selectionner un autre Wifi ou re-entrer le mot de passe de votre box internet.</p>";
                           message += "<h2>Choisissez votre Wifi, et entrez votre mot de passe.</h2>";
                           message += "<form action='/submitted'>";

                           message += "<label for='wifiname'>Reseau WiFi:</label><br>";
                           message += "<select name='wifiname' id='wifiname'>";
                           int n = WiFi.scanNetworks();
                           for (int i = 0; i < n; ++i) {
                              message += "<option value='" + WiFi.SSID(i) + "'>" + WiFi.SSID(i) + "</option>";
                           }
                           message += "</select><br>";

                           message += "<label for='password'>Mot de passe du WiFi:</label><br>";
                           message += "<input type='text' id='password' name='wifipassword'><br><br>";
                           message += "<input type='submit' value='Valider'></form>";
                           }

                           String fullPage = header + message + footer;

                           server.send(200, "text/html", fullPage);
                     }
}

void handle_WiFioff()
{
  Serial.println("WiFi OFF");
  WiFi_status=false;
  server.send(200, "text/html", HTML());

  for (int i =0; i<length_Wifiname_EEPROM; i++)
  {
    Serial.print(EEPROM.read(i));
  }
}
 
void handle_NotFound()
{
  server.send(404, "text/plain", "Not found");
}
 String msg;

String HTML()
{
    msg="<h1>Configurez votre Dongle Linky</h1>\n";
    msg+="<h2>Choisissez votre Wifi, et entrez le mot de passe de votre box internet/de votre wifi.</h2>\n";
    msg+="<form action='/submitted'>\n";

    msg+="<label for='wifiname'>Reseau WiFi:</label><br>\n";
    msg+="<select name='wifiname' id='wifiname'>\n";

    // dynamically add WiFi networks here
    int n = WiFi.scanNetworks();
    for (int i = 0; i < n; ++i) {
      msg += "<option value='" + WiFi.SSID(i) + "'>" + WiFi.SSID(i) + "</option>\n";
    }
    msg+="</select><br>\n";

    msg+="<label for='password'>Mot de passe du WiFi (Mot de passe de votre box internet):</label><br>\n";
    msg+="<input type='text' id='password' name='wifipassword'><br><br>\n";
    msg+="<input type='submit' value='Valider'>\n";
    msg+="</form>\n";
    msg+="<p>Lorsque vous cliquez sur \"Valider\" le dongle essaiera de se connecter au reseau choisi avec le mot de passe. Pour reconfigurer, rebranchez le dongle.</p>\n";
    msg+="</body></html>\n";
    String fullPage = header + msg + footer;

return fullPage;
}


String HTMLsubmitted()
{
  String msg="<!DOCTYPE html> <html>\n";    msg+=" <head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";    msg+=" <title>WiFi Control</title>\n";    msg+=" <style>html{font-family:Helvetica; display:inline-block; margin:0px auto; text-align:center;}\n";    msg+=" body{margin-top: 50px;} h1{color: #444444; margin: 50px auto 30px;} h3{color:#444444; margin-bottom: 50px;}\n";    msg+=" .button{display:block; width:80px; background-color:#f48100; border:none; color:white; padding: 13px 30px; text-decoration:none; font-size:25px; margin: 0px auto 35px; cursor:pointer; border-radius:4px;}\n";    msg+=" .button-on{background-color:#f48100;}\n";    msg+=" .button-on:active{background-color:#f48100;}\n";    msg+=" .button-off{background-color:#26282d;}\n";    msg+=" .button-off:active{background-color:#26282d;}\n";    msg+=" </style>\n";    msg+=" </head>\n";    msg+=" <body>\n";    msg+=" <h1>ESP8266 Web Server</h1>\n";    msg+=" <h3>Thank you for the information. We are trying to connect. Check the light status on the Linky dongle, or go back to the wifi information page</h3>\n";
    // if(WiFi_status==false)
   // {
    //  msg+="<p>WiFi Status: OFF</p><a class=\"button button-on\" href=\"/WiFion\">ON</a>\n";    
    //}
    //else
   // {
   //   msg+="<p>WiFi Status: ON</p><a class=\"button button-off\" href=\"/WiFioff\">OFF</a>\n";
   // }    msg+="</body>\n";    msg+="</html>\n";
    return msg;
}


