#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <FS.h>
#include <WiFiClientSecure.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

ESP8266WebServer server(80);

// ------------ Variables ------------
String wifi_ssid, wifi_pass;
String provider;
String host_url, api_url, httpRequestData, bearer_token, method, content_type;
String sensor_id, asset_id;
String host_url_certif;
int url_port = 443;  // default HTTPS
#define TRIGGER_PIN 4   // Wemos D1 mini D2 = GPIO4

// ------------ SPIFFS helpers ------------
void writeFile(const char* path, const String& data) {
  File f = SPIFFS.open(path, "w");
  if (!f) { Serial.printf("Failed to open %s for writing\n", path); return; }
  f.print(data);
  f.close();
}

String readFile(const char* path) {
  File f = SPIFFS.open(path, "r");
  if (!f) {
    Serial.printf("File %s not found\n", path);
    return "";
  }
    String content = f.readString();
  f.close();
  content.trim();
  return content;
}

// ------------ WiFi Connect ------------
bool connectWiFi() {
  wifi_ssid = readFile("/ssid.txt");
  wifi_pass = readFile("/pass.txt");
  if (wifi_ssid.length() == 0) return false;

  WiFi.begin(wifi_ssid.c_str(), wifi_pass.c_str());
  Serial.printf("Connecting to %s", wifi_ssid.c_str());
  int tries = 0;
  while (WiFi.status() != WL_CONNECTED && tries < 20) {
    delay(500);
    Serial.print(".");
    tries++;
  }
  Serial.println();
  return WiFi.status() == WL_CONNECTED;
}

// ------------ Web Config AP ------------
void startAPMode() {
  WiFi.softAP("LinkyDongle", "password123");
  Serial.println("Started AP: LinkyDongle (pw: password123)");
  Serial.println("Go to http://192.168.4.1/");
  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());
  server.on("/", HTTP_GET, []() {
    String html = "<h2>Device Configuration</h2>";
    html += "<form action='/save' method='post'>";
    html += "<h3>WiFi</h3>";
    html += "SSID: <input name='ssid'><br>";
    html += "Password: <input name='pass' type='password'><br><br>";

    html += "<h3>Service Provider</h3>";
    html += "<select name='provider' onchange='showProviderFields(this.value)'>";
    html += "<option value='none'> </option>";
    html += "<option value='unica'>UniCA IMREDD</option>";
    html += "<option value='chargeangels'>Charge Angels</option>";
    html += "<option value='other'>Other</option>";
    html += "</select><br><br>";

    // UniCA block
    html += "<div id='unica_fields' style='display:none'>";
    html += "Sensor ID: <input name='sensor_id' value='LinkyHouse4'><br>";
    html += "</div>";

    // Charge Angels block
    html += "<div id='chargeangels_fields' style='display:none'>";
    html += "Asset ID: <input name='asset_id'><br>";
    html += "Bearer Token:<br><textarea name='token' rows='6' cols='40'></textarea><br>";
    html += "</div>";

    // Other block
    html += "<div id='other_fields' style='display:none'>";
    html += "Host: <input name='host'><br>";
    html += "URL: <input name='url'><br>";
    html += "Port: <input name='port'><br>";
     html += "Sensor ID: <input name='sensor_id' value='LinkyHouse4'><br>";

    html += "Method: <select name='method'><option>GET</option><option>POST</option><option>PATCH</option></select><br>";
    html += "Content-Type: <input name='ctype' value='application/json'><br>";
    html += "Authorization: <input name='auth'><br>";
    html += "Custom Body:<br><textarea name='body' rows='6' cols='40'></textarea><br>";
    html += "</div>";

    html += "<br><input type='submit' value='Save'>";
    html += "</form>";

    // Javascript for dynamic blocks
    html += "<script>"
            "function showProviderFields(p) {"
            "document.getElementById('unica_fields').style.display=(p=='unica')?'block':'none';"
            "document.getElementById('chargeangels_fields').style.display=(p=='chargeangels')?'block':'none';"
            "document.getElementById('other_fields').style.display=(p=='other')?'block':'none';"
            "}"
            "</script>";

    server.send(200, "text/html", html);
  });

  server.on("/save", HTTP_POST, []() {
    if (server.hasArg("ssid")) writeFile("/ssid.txt", server.arg("ssid"));
    if (server.hasArg("pass")) writeFile("/pass.txt", server.arg("pass"));
    if (server.hasArg("provider")) writeFile("/provider.txt", server.arg("provider"));

    // Save depending on provider
    if (server.arg("provider") == "unica") {
      writeFile("/host.txt", "tip-imredd.unice.fr");
      writeFile("/url.txt", "/nodes/imredd/energyconso/linkytest");
      writeFile("/sensor_id.txt", server.arg("sensor_id"));
    }
    else if (server.arg("provider") == "chargeangels") {
      writeFile("/host.txt", "imredd.charge-angels.com");
      writeFile("/url.txt", "/v1/api/assets/" + server.arg("asset_id") + "/consumptions");
      writeFile("/asset_id.txt", server.arg("asset_id"));
      writeFile("/token.txt", server.arg("token"));
    }
    else if (server.arg("provider") == "other") {
      writeFile("/host.txt", server.arg("host"));
      writeFile("/url.txt", server.arg("url"));
      writeFile("/port.txt", server.arg("port"));
      writeFile("/method.txt", server.arg("method"));
      writeFile("/ctype.txt", server.arg("ctype"));
      writeFile("/auth.txt", server.arg("auth"));
      writeFile("/body.txt", server.arg("body"));
      writeFile("/sensor_id.txt", server.arg("sensor_id"));


    }

    server.send(200, "text/plain", "Config saved. You can now Reboot the device by pressing the button on the side of the dongle (you might need to use a pen).");
  });

  server.begin();
    Serial.println("HTTP server started");

}

// ------------ Example Sending Logic ------------
 void sendRequest() {
  //WiFiClient client;

  String prov = readFile("/provider.txt");
  host_url = readFile("/host.txt");
  api_url = readFile("/url.txt");
Serial.println(prov);
Serial.println(host_url);
Serial.println(api_url);

  if (prov == "unica") {
    Serial.println("into unica section");
  WiFiClientSecure client;
  client.setInsecure(); //the magic line, use with caution

    String sensor = readFile("/sensor_id.txt");
    int puissanceCommuniquee = 123;
    int Danger = 0;
    int real_voltage = 220;
    long epoch_time = 1756939456;
    httpRequestData = "linkySensor,sensor_id=" + sensor +
                      " power=" + String(puissanceCommuniquee) +
                      ",danger=" + String(Danger) +
                      ",voltage=" + String(real_voltage) +
                      ",frequency=50.2,pf=0.9 " +
                      String(epoch_time) + "000000000";

    if (client.connect(host_url.c_str(), 443)) {
      Serial.println("Connected to Host ! Sending request");
      client.print(String("POST ") + api_url + " HTTP/1.0\r\n" +
                   "Host: " + host_url + "\r\n" +
                   "User-Agent: LinkyDongle\r\n" +
                   "Content-Length: " + httpRequestData.length() + "\r\n" +
                   "Connection: close\r\n\r\n" +
                   httpRequestData);
              Serial.println(String("POST ") + api_url + " HTTP/1.0\r\n" +
                   "Host: " + host_url + "\r\n" +
                   "User-Agent: LinkyDongle\r\n" +
                   "Content-Length: " + httpRequestData.length() + "\r\n" +
                   "Connection: close\r\n\r\n" +
                   httpRequestData);
    }


  }
  else if (prov == "chargeangels") {
      WiFiClientSecure client;
    //  client.setTrustAnchors(&cert);

    String token = readFile("/token.txt");
    String asset = readFile("/asset_id.txt");

    String yearstringstart = "2025"; // example
    String monthstringstart = "8";
    String daystringstart = "4";
    String hourstringstart = "12";
    String minutestringstart = "34";
    String secondstringstart = "56";
    String yearstringend = "2025"; // example
    String monthstringend = "8";
    String daystringend = "4";
    String hourstringend = "12";
    String minutestringend = "35";
    String secondstringend = "56";
    int puissanceCommuniquee = 123;

    httpRequestData = "{\"assetID\": \"" + asset + "\",\"startedAt\": \"" +
                      yearstringstart + "-" + monthstringstart + "-" + daystringstart + "T" +
                      hourstringstart + ":" + minutestringstart + ":" + secondstringstart + ".000Z\",\"endedAt\": \"" +
                      yearstringend + "-" + monthstringend + "-" + daystringend + "T" +
                      hourstringend + ":" + minutestringend + ":" + secondstringend +
                      ".000Z\",\"instantWatts\": " + String(puissanceCommuniquee) +
                      ",\"instantWattsL1\": 0,\"instantWattsL2\": 0,\"instantWattsL3\": 0," +
                      "\"instantAmps\": 0,\"instantAmpsL1\": 0,\"instantAmpsL2\": 0,\"instantAmpsL3\": 0," +
                      "\"instantVolts\": 0,\"instantVoltsL1\": 0,\"instantVoltsL2\": 0,\"instantVoltsL3\": 0," +
                      "\"consumptionWh\": 0,\"consumptionAmps\": 0,\"stateOfCharge\": 0}";

    if (client.connect(host_url.c_str(), 443)) {
      client.println("POST " + api_url + " HTTP/1.0");
      client.println("Host: " + host_url);
      client.println("User-Agent: LinkyDongle");
      client.println("Connection: close");
      client.println("Content-Type: application/json");
      client.println("Authorization: Bearer " + token);
      client.print("Content-Length: ");
      client.println(httpRequestData.length());
      client.println();
      client.println(httpRequestData);
    }
  }
  else if (prov == "other") {

    int puissanceCommuniquee = 123;
    int Danger = 0;
    int real_voltage = 220;
    long epoch_time = 1756976689;

    String sensor = readFile("/sensor_id.txt");

    method = readFile("/method.txt");
    content_type = readFile("/ctype.txt");
    bearer_token = readFile("/auth.txt");
    httpRequestData = readFile("/body.txt");
    String portStr = readFile("/port.txt");
    if (portStr.length() > 0) url_port = portStr.toInt();
// Replace placeholders with variables
httpRequestData.replace("{ASSETID}", String(sensor));
httpRequestData.replace("{PUISSANCE}", String(puissanceCommuniquee));
httpRequestData.replace("{EPOCH}", String(epoch_time));
httpRequestData.replace("{DANGER}", String(Danger));
//httpRequestData.replace("{EPOCH}", String(epoch_time));


Serial.println(httpRequestData);
Serial.println(content_type);
Serial.println(method);

  WiFiClientSecure client;
  client.setInsecure(); //the magic line, use with caution

    if (client.connect(host_url.c_str(), url_port)) {
      client.println(method + " " + api_url + " HTTP/1.0");
      client.println("Host: " + host_url);
      client.println("User-Agent: LinkyDongle");
      client.println("Connection: close");
      if (content_type.length() > 0)
        client.println("Content-Type: " + content_type);
      if (bearer_token.length() > 0)
        client.println("Authorization: " + bearer_token);
      client.print("Content-Length: ");
      client.println(httpRequestData.length());
      client.println();
      client.println(httpRequestData);
      Serial.println(method + " " + api_url + " HTTP/1.0");
      Serial.println("Host: " + host_url);
      Serial.println(httpRequestData);

    }
  }
}




// ------------ Example Sending Logic ------------
void seeData() {

  String prov = readFile("/provider.txt");
  host_url = readFile("/host.txt");
  api_url = readFile("/url.txt");
  Serial.println(prov);
  Serial.println(host_url);
  Serial.println(api_url);

  if (prov == "unica") {
    String sensor = readFile("/sensor_id.txt");
    int puissanceCommuniquee = 123;
    int Danger = 0;
    int real_voltage = 220;
    long epoch_time = 1690000000;
    httpRequestData = "linkySensor,sensor_id=" + sensor +
                      " power=" + String(puissanceCommuniquee) +
                      ",danger=" + String(Danger) +
                      ",voltage=" + String(real_voltage) +
                      ",frequency=50.2,pf=0.9 " +
                      String(epoch_time) + "000000000";

 /*   if (client.connect(host_url.c_str(), 80)) {
      client.print(String("POST ") + api_url + " HTTP/1.0\r\n" +
                   "Host: " + host_url + "\r\n" +
                   "User-Agent: LinkyDongle\r\n" +
                   "Content-Length: " + httpRequestData.length() + "\r\n" +
                   "Connection: close\r\n\r\n" +
                   httpRequestData);
    }*/
  }
  else if (prov == "chargeangels") {
    String token = readFile("/token.txt");
    String asset = readFile("/asset_id.txt");

    String yearstringstart = "2023"; // example
    String monthstringstart = "10";
    String daystringstart = "18";
    String hourstringstart = "12";
    String minutestringstart = "34";
    String secondstringstart = "56";
    String yearstringend = "2023"; // example
    String monthstringend = "10";
    String daystringend = "18";
    String hourstringend = "12";
    String minutestringend = "35";
    String secondstringend = "56";
    int puissanceCommuniquee = 123;

    httpRequestData = "{\"assetID\": \"" + asset + "\",\"startedAt\": \"" +
                      yearstringstart + "-" + monthstringstart + "-" + daystringstart + "T" +
                      hourstringstart + ":" + minutestringstart + ":" + secondstringstart + ".000Z\",\"endedAt\": \"" +
                      yearstringend + "-" + monthstringend + "-" + daystringend + "T" +
                      hourstringend + ":" + minutestringend + ":" + secondstringend +
                      ".000Z\",\"instantWatts\": " + String(puissanceCommuniquee) +
                      ",\"instantWattsL1\": 0,\"instantWattsL2\": 0,\"instantWattsL3\": 0," +
                      "\"instantAmps\": 0,\"instantAmpsL1\": 0,\"instantAmpsL2\": 0,\"instantAmpsL3\": 0," +
                      "\"instantVolts\": 0,\"instantVoltsL1\": 0,\"instantVoltsL2\": 0,\"instantVoltsL3\": 0," +
                      "\"consumptionWh\": 0,\"consumptionAmps\": 0,\"stateOfCharge\": 0}";

  /*  if (client.connect(host_url.c_str(), 80)) {
      client.println("POST " + api_url + " HTTP/1.0");
      client.println("Host: " + host_url);
      client.println("User-Agent: LinkyDongle");
      client.println("Connection: close");
      client.println("Content-Type: application/json");
      client.println("Authorization: Bearer " + token);
      client.print("Content-Length: ");
      client.println(httpRequestData.length());
      client.println();
      client.println(httpRequestData);
    }*/
  }
  else if (prov == "other") {
    method = readFile("/method.txt");
    content_type = readFile("/ctype.txt");
    bearer_token = readFile("/auth.txt");
    httpRequestData = readFile("/body.txt");
    String portStr = readFile("/port.txt");
    if (portStr.length() > 0) url_port = portStr.toInt();

    Serial.println(httpRequestData);
    Serial.println(content_type);
    Serial.println(method);


    /*  if (client.connect(host_url.c_str(), url_port)) {
      client.println(method + " " + api_url + " HTTP/1.0");
      client.println("Host: " + host_url);
      client.println("User-Agent: LinkyDongle");
      client.println("Connection: close");
      if (content_type.length() > 0)
        client.println("Content-Type: " + content_type);
      if (bearer_token.length() > 0)
        client.println("Authorization: " + bearer_token);
      client.print("Content-Length: ");
      client.println(httpRequestData.length());
      client.println();
      client.println(httpRequestData);
    }*/
  }
}




// ------------ Setup & Loop ------------
void setup() {
/*  // WiFi.disconnect(true);   // true = erase WiFi credentials
delay(1000);

if (SPIFFS.begin()) {
  SPIFFS.format();   // ⚠️ This erases everything in SPIFFS
  Serial.println("SPIFFS formatted!");
}*/

  pinMode(TRIGGER_PIN, INPUT); // external circuit must pull HIGH when needed

  Serial.begin(115200);
  SPIFFS.begin();
  delay(1000);
  seeData();

      if (digitalRead(TRIGGER_PIN) == HIGH) {
        Serial.println("Connecting to wifi to send data");
          if (!connectWiFi()) {
            Serial.println("WiFi not configured or failed, starting AP...");
            startAPMode();
          } else {
            Serial.println("WiFi connected!");
            Serial.println(WiFi.localIP());
            sendRequest();
          }
      } else {

                Serial.println("Start AP mode");

   startAPMode();

      }
 /* if (!connectWiFi()) {
    Serial.println("WiFi not configured or failed, starting AP...");
    startAPMode();
  } else {
    Serial.println("WiFi connected!");
    Serial.println(WiFi.localIP());
   // sendRequest();
  }*/

   



}

void loop() {
  if (WiFi.getMode() == WIFI_AP) {
    server.handleClient();

  }
}
/*
void loop() {
  server.handleClient();
}
*/