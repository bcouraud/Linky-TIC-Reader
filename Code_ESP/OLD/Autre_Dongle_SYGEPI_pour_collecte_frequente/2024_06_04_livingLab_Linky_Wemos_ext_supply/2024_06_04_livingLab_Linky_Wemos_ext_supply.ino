
// #################################  compile it for LOLIN (WeMO) D1 R1 ##############################
// Install driver CH34x_Install_Windows_v3_4
/* this code comes from https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266WiFi/examples/HTTPSRequest
 *  Interesting also: https://maakbaas.com/esp8266-iot-framework/logs/https-requests/
 *  if error uploading the code: https://sparks.gogo.co.nz/ch340.html  download and install the driver.
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
#define STASSID "livinglab"
#define STAPSK "2021#Bowl@LivingLab"

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


int indice = 0;  // index for the recording of the power
int debug = 1;  // true when we are in debug mode
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


const char* ssid = STASSID;
const char* password = STAPSK;

X509List cert(cert_DigiCert_Global_Root_CA);

   // ################ set up wifi + send message function, is triggered only 

void send_wifi_TIP() {

  



        time_t now = time(nullptr);
        while (now < 8 * 3600 * 2) {
          delay(500);
          now = time(nullptr);
        }
        struct tm timeinfo;
        gmtime_r(&now, &timeinfo);
      int epoch_time = now;


  
          
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

        String httpRequestData = "linkySensor,sensor_id=LinkyLivingLab power="+String(puissanceCommuniquee)+",danger="+String(Danger)+",voltage=234.231,frequency=50.2,pf=0.9 "+String(epoch_time)+"000000000";
        Serial.println(httpRequestData); //
        client.print(String("POST ") + url + " HTTP/1.0\r\n" + "Host: " + TIP_host + "\r\n" + "User-Agent: BuildFailureDetectorESP8266\r\n" +  "Content-Length: " + httpRequestData.length()   + "\r\n"+"Connection: close\r\n\r\n"+httpRequestData);



}






















































  
   // ################ listen to Linky, every 15 seconds ##################### 
void readLinky1200(){
 //Serial.println("reading TIC!");
//digitalWrite(0, LOW); // initialisation
//delay(50);               
// digitalWrite(0, HIGH); // initialisation
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
        send_wifi_TIP();
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
  pinMode(0, OUTPUT);
  digitalWrite(0, HIGH); // initialisation
  delay(10);
  digitalWrite(0, LOW);
    delay(100);
   digitalWrite(0, HIGH); // initialisation
  pinMode(5, OUTPUT);
  digitalWrite(5, LOW);
        Serial.begin(1200,SERIAL_7E1);    // Define and start serial monitoring of the Linky TIC
//Serial.swap();

    // commit 512 bytes of ESP8266 flash (for "EEPROM" emulation)
  // this step actually loads the content (512 bytes) of flash into 
  // a 512-byte-array cache in RAM

  // read bytes (i.e. sizeof(data) from "EEPROM"),
  // in reality, reads from byte-array cache
  

  
finished_recording = false;

     //WiFi.mode(WIFI_STA);

    // WiFi.config(ip, gateway_dns, gateway_dns); 
      WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(50);
  }

           if (debug==1){
 Serial.println("connected");
 Serial.println("IP address: "); //
Serial.println(WiFi.localIP());//
           }


  configTime(3 * 3600, 0,"134.59.1.5", "pool.ntp.org", "time.nist.gov");

 // ESP.deepSleep(20e6); // we go to sleep 10 seconds
 // ESP.deepSleep(20e6); // we go to sleep 10 seconds
    }
    void loop() { // nothing here, we go to sleep and do not enter an infinite loop
   

puissance_int = 0; // power measured, in int
puissance_Uint = 0; // power measured, in int
puissance_moyenne = 0; // averagepower measured, in int
firststartup = 0;


indice = 0;  // index for the recording of the power
 counter_moyenne = 0; // counter to compute the average
 counter_moyennetemp = 0; // used to store temporary counter to compute the average
 puissanceCommuniquee = 0; // puissance que l'on communique via wifi
 finished_recording = false; //specifies if we can go back to sleep (= we received the signal from Linky)
 timeoutReadLinky = 0; // timeout to stop trying to read linky if there is no data to read
//const int A0 = A0;  // Analog input pin that the potentiometer is attached to
 BatteryVoltage = 0;
 Danger = 0; // indicate if Power > Powermax
 Powermax = 10000; // to be read from Linky, and stored in EEPROM
t1 = 0;
t2=0;








         if (debug==1){
          Serial.println("New loop");
         }
     timeoutReadLinky = 0;
     int counter = 1;
       while(finished_recording == false && counter<10000){  
            readLinky1200(); // we read Linky data until we have found a data (finished_recording) or if the timeout is reached 
           counter = counter +1;
           //  t2 = millis();
           //  timeoutReadLinky = t2-t1; 
     //             if (debug==1){
     //     Serial.print(".");
          //Serial.println(counter);
     //    }      
        }

       if(debug==1 || finished_recording) {  // if we have enough voltage, we send the data and reinitialise the values
         puissanceCommuniquee=puissance_moyenne; 
         // puissanceCommuniquee=puissance_Uint;
         if (debug==1){
          Serial.println("ready to send");
         }
  
            send_wifi_TIP();
            counterwakeup = 0;
            counter_moyenne = 0;
            puissance_moyenne = 0;
        } 






    }
  
