/*
 * This code will configure ESP8266 in SoftAP mode and will act as a web server for all the connecting devices. It will then turn On/Off 4 LEDs as per input from the connected station devices.
  The default IP address for an ESP8266 access point (AP) is 192.168.4.1. 
*/
 
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#define EEPROM_SIZE 1024  // or more

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

int addr = 0;
int length_Wifiname_EEPROM = 40;

//Specifying the Pins connected to LED
//uint8_t led_pin=LED_BUILTIN;
uint8_t wifi_pin=LED_BUILTIN;

struct { 
    uint puissancemoy = 0; // we store the average power (but we could store all the previous power)
    uint countermoy = 0;  // counter of tnhe number of times we went to sleep without being able to send data
    char wifinameEEPROM[32];
    char wifipasswordEEPROM[32];
  } 
  dataEEPROM;

//Specifying the boolean variables indicating the status of LED
//bool led_status=false; 
bool WiFi_status=false; 

void setup(){
  //Start the serial communication channel
  Serial.begin(115200);
  Serial.println();
  EEPROM.begin(512);

  //Output mode for the LED Pins
  //pinMode(led_pin,OUTPUT);
  pinMode(wifi_pin,OUTPUT);

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

// read bytes (i.e. sizeof(data) from "EEPROM"),
// in reality, reads from byte-array cache
// cast bytes into structure called data
  EEPROM.get(addr,dataEEPROM);
  WiFi_Name = dataEEPROM.wifinameEEPROM; // we retrieve wifiname from eeprom

  Serial.print("Data stored for Wifi Name in EEPROM: ");
  Serial.println(WiFi_Name);

  //Starting the Server
  server.begin();
  Serial.println("HTTP Server Started");
}
 
void loop() {
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
 
  if(WiFi_status==false)
  {
    digitalWrite(wifi_pin,LOW);
  }

  else
  {
    digitalWrite(wifi_pin,HIGH);
  }

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

  Serial.println("Data stored in EEPROM. Reading first 20 characters:");
  for (int i=0;i<20;i++)
  {
    value = EEPROM.read(i);
    Serial.print(i);
    Serial.print("\t");
    Serial.print(value);
    Serial.println();
  }
   Serial.println("Trying to connect to Wifi");

  // Switch to AP + STA mode
  WiFi.mode(WIFI_AP_STA);

  // Try connecting to user's WiFi
  WiFi.begin(WiFi_Name.c_str(), WiFi_Password.c_str());

  int retries = 0;
  const int maxRetries = 20; // About 10 seconds
  while (WiFi.status() != WL_CONNECTED && retries < maxRetries) {
    delay(500);
    Serial.print(".");
    retries++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConnected to WiFi!");
    Serial.print("Local IP: ");
    Serial.println(WiFi.localIP());
    message = "<h2>Success! Connected to WiFi.</h2>";
    message += "<p>IP: ";
    message += WiFi.localIP().toString();
    message += "</p>";
  } else {
    Serial.println("\nFailed to connect to WiFi.");
    message = "<h2>Failed to connect to WiFi.</h2><p>Please check your SSID and password.</p>";
  }

  server.send(200, "text/html", message);
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
 
String HTML()
{
  String msg="<!DOCTYPE html> <html>\n";
    msg+=" <head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
    msg+=" <title>WiFi Control</title>\n";
    msg+=" <style>html{font-family:Helvetica; display:inline-block; margin:0px auto; text-align:center;}\n";
    msg+=" body{margin-top: 50px;} h1{color: #444444; margin: 50px auto 30px;} h3{color:#444444; margin-bottom: 50px;}\n";
    msg+=" .button{display:block; width:80px; background-color:#f48100; border:none; color:white; padding: 13px 30px; text-decoration:none; font-size:25px; margin: 0px auto 35px; cursor:pointer; border-radius:4px;}\n";
    msg+=" .button-on{background-color:#f48100;}\n";
    msg+=" .button-on:active{background-color:#f48100;}\n";
    msg+=" .button-off{background-color:#26282d;}\n";
    msg+=" .button-off:active{background-color:#26282d;}\n";
    msg+=" </style>\n";
    msg+=" </head>\n";
    msg+=" <body>\n";
    msg+=" <h1>ESP8266 Web Server</h1>\n";
    msg+=" <h3>Using Access Point (AP) Mode</h3>\n";
    msg+="<h2>Enter your WiFi network and credentials</h2>\n";
    msg+="<form action=\"/submitted\">\n";
    msg+=" <label for=\"wifiname\">WiFi Name:</label><br>\n";
    msg+=" <input type=\"text\" id=\"wifiname\" name=\"wifiname\" value=\"\"><br>\n";
    msg+=" <label for=\"password\">WiFi Password:</label><br>\n";
    msg+=" <input type=\"text\" id=\"password\" name=\"wifipassword\" value=\"\"><br><br>\n";
    msg+=" <input type=\"submit\" value=\"Submit\"> </form>\n";
    msg+=" <p>If you click the \"Submit\" button, the Linky dongle will try to connect to your wifi until you reset it through the dedicated button (see procedure in the datasheet).</p>\n";

    if(WiFi_status==false)
    {
      msg+="<p>WiFi Status: OFF</p><a class=\"button button-on\" href=\"/WiFion\">ON</a>\n";    
    }
    
    else
    {
      msg+="<p>WiFi Status: ON</p><a class=\"button button-off\" href=\"/WiFioff\">OFF</a>\n";
    }
    msg+="</body>\n";    msg+="</html>\n";
    return msg;
}


String HTMLsubmitted()
{
  String msg="<!DOCTYPE html> <html>\n";    msg+=" <head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";    msg+=" <title>WiFi Control</title>\n";    msg+=" <style>html{font-family:Helvetica; display:inline-block; margin:0px auto; text-align:center;}\n";    msg+=" body{margin-top: 50px;} h1{color: #444444; margin: 50px auto 30px;} h3{color:#444444; margin-bottom: 50px;}\n";    msg+=" .button{display:block; width:80px; background-color:#f48100; border:none; color:white; padding: 13px 30px; text-decoration:none; font-size:25px; margin: 0px auto 35px; cursor:pointer; border-radius:4px;}\n";    msg+=" .button-on{background-color:#f48100;}\n";    msg+=" .button-on:active{background-color:#f48100;}\n";    msg+=" .button-off{background-color:#26282d;}\n";    msg+=" .button-off:active{background-color:#26282d;}\n";    msg+=" </style>\n";    msg+=" </head>\n";    msg+=" <body>\n";    msg+=" <h1>ESP8266 Web Server</h1>\n";    msg+=" <h3>Thank you for the information. We are trying to connect. Check the light status on the Linky dongle, or go back to the wifi information page</h3>\n";
     if(WiFi_status==false)
    {
      msg+="<p>WiFi Status: OFF</p><a class=\"button button-on\" href=\"/WiFion\">ON</a>\n";    
    }
    else
    {
      msg+="<p>WiFi Status: ON</p><a class=\"button button-off\" href=\"/WiFioff\">OFF</a>\n";
    }    msg+="</body>\n";    msg+="</html>\n";
    return msg;
}