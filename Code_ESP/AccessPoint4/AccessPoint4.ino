/*
 * This code will configure ESP8266 in SoftAP mode and will act as a web server for all the connecting devices. It will then turn On/Off 4 LEDs as per input from the connected station devices.
  The default IP address for an ESP8266 access point (AP) is 192.168.4.1. 
*/
 
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>

/*Specifying the SSID and Password of the AP*/
const char* ap_ssid = "LinkyDongle"; //Access Point SSID
const char* ap_password= "password"; //Access Point Password
uint8_t max_connections=1;//Maximum Connection Limit for AP
int current_stations=0, new_stations=0;
String WiFi_Name = "";
String WiFi_Password = "";
char value;

//Specifying the Webserver instance to connect with HTTP Port: 80
ESP8266WebServer server(80);
int addr = 0;
int length_Wifiname_EEPROM = 20;

//Specifying the Pins connected to LED
//uint8_t led_pin=LED_BUILTIN;
uint8_t wifi_pin=LED_BUILTIN;

struct { 
    uint puissancemoy = 0; // we store the average power (but we could store all the previous power)
    uint countermoy = 0;  // counter of tnhe number of times we went to sleep without being able to send data
    String wifinameEEPROM="";
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

  //Setting the AP Mode with SSID, Password, and Max Connection Limit
  if(WiFi.softAP(ap_ssid,ap_password,1,false,max_connections)==true)
  {
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

//to be removed !!!
  Serial.println("Let's read EEPROM");
    for (int i=0;i<20;i++)
    {
      value = EEPROM.read(i);
      Serial.print(i);
      Serial.print("\t");
      Serial.print(value);
      Serial.println();
    }
//end of to be removed

// read bytes (i.e. sizeof(data) from "EEPROM"),
// in reality, reads from byte-array cache
// cast bytes into structure called data
  EEPROM.get(addr,dataEEPROM);
  WiFi_Name = dataEEPROM.wifinameEEPROM; // we retrieve average power from eeprom

  Serial.print("Data stored for Wifi Name in EEPROM: ");
  Serial.println(WiFi_Name);

  //Starting the Server
  server.begin();
  Serial.println("HTTP Server Started");
}
 
void loop() {
  //Assign the server to handle the clients
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
 
  //Turn the LED ON/OFF as per its status set by the connected client
  
  /* //LED
  if(led_status==false)
  {
    digitalWrite(led_pin,LOW);
  }

  else
  {
    digitalWrite(led_pin,HIGH);
  } */

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
    WiFi_Password = server.arg ("pwd");

    server.send(200, "text/html", "<!doctype html lang='en'<head><meta charset='utf-8'><meta name='viewpoint' content='width=device-width, initial-scale=1'");
  }

  else
  {
    server.send(200, "text/html", "<!doctype html lang='en'<head><meta charset='utf-8'><meta name='viewpoint' content='width=device-width, initial-scale=1'");
  }
}
 
//void handle_ledon()
void handle_WiFion()
{
  /*Serial.println("LED ON");
  led_status=true;
  server.send(200, "text/html", HTML());*/

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
  Serial.println("received password");
 
  String message = "";

  if (server.arg("wifiname")== "") //Parameter not found
  {     
    message = "wifi name not found";
  }

  else  //Parameter found
  {    
    message = "WiFi Name = ";
    message += server.arg("wifiname");     //Gets the value of the query parameter
    WiFi_Name = server.arg("wifiname");
  }

  Serial.println("WiFi Name received: ");
  Serial.println(message);
  Serial.println("storing in EEPROM");

  dataEEPROM.wifinameEEPROM =   WiFi_Name ;
  EEPROM.put(addr,dataEEPROM);
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
 
  server.send(200, "text/html", HTMLsubmitted());
}

/*void handle_ledoff()
{
  Serial.println("LED OFF");
  led_status=false;
  server.send(200, "text/html", HTML());*/

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

    msg+=" <label for=\"pwd\">WiFi Password:</label><br>\n";

    msg+=" <input type=\"text\" id=\"pwd\" name=\"pwd\" value=\"\"><br><br>\n";

    msg+=" <input type=\"submit\" value=\"Submit\"> </form>\n";

    msg+=" <p>If you click the \"Submit\" button, the Linky dongle will try to connect to your wifi until you reset it through the dedicated button (see procedure in the datasheet).</p>\n";


/*
  String msg="<!DOCTYPE html> <html>\n";
    msg+="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
    msg+=" <title>LED Control</title>\n";
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
    msg+="<h2>Enter your Wifi Network and credentials</h2>\n";
    msg+="<form action="/XXXXXX.html">\n";
    msg+=" <label for="fname">Wifi name:</label><br>\n";
    msg+=" <input type="text" id="fname" name="fname" value=""><br>\n";
    msg+=" <label for="lname">Wifi Password:</label><br>\n";
    msg+=" <input type="text" id="lname" name="lname" value=""><br><br>\n";
    msg+=" <input type="submit" value="Submit"> </form>\n";
    msg+=" <p>If you click the "Submit" button, the Linky dongle will try to connect to your wifi until you reset it through the dedicated button (see procedure in the datasheet).</p>\n";

    msg+="   </body>\n";
    msg+="</html>\n";





*/





    
    /*if(led_status==false)
    {
      msg+="<p>LED Status: OFF</p><a class=\"button button-on\" href=\"/ledon\">ON</a>\n";    
    }
    
    else
    {
      msg+="<p>LED Status: ON</p><a class=\"button button-off\" href=\"/ledoff\">OFF</a>\n";
    }*/

    if(WiFi_status==false)
    {
      msg+="<p>WiFi Status: OFF</p><a class=\"button button-on\" href=\"/WiFion\">ON</a>\n";    
    }
    
    else
    {
      msg+="<p>WiFi Status: ON</p><a class=\"button button-off\" href=\"/WiFioff\">OFF</a>\n";
    }

    msg+="</body>\n";
    msg+="</html>\n";
    return msg;
}


String HTMLsubmitted()
{
  String msg="<!DOCTYPE html> <html>\n";

    msg+=" <head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";

    //msg+=" <title>LED Control</title>\n";

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

    msg+=" <h3>Thank you for the information. We are trying to connect. Check the light status on the Linky dongle, or go back to the wifi information page</h3>\n";

    /*
    if(led_status==false)
    {
      msg+="<p>LED Status: OFF</p><a class=\"button button-on\" href=\"/ledon\">ON</a>\n";    
    }
    
    else
    {
      msg+="<p>LED Status: ON</p><a class=\"button button-off\" href=\"/ledoff\">OFF</a>\n";
    }
    */

     if(WiFi_status==false)
    {
      msg+="<p>WiFi Status: OFF</p><a class=\"button button-on\" href=\"/WiFion\">ON</a>\n";    
    }
    
    else
    {
      msg+="<p>WiFi Status: ON</p><a class=\"button button-off\" href=\"/WiFioff\">OFF</a>\n";
    }

    msg+="</body>\n";
    msg+="</html>\n";
    return msg;
}