#include <ESP8266WiFi.h>

#include <PubSubClient.h>

#include <ESP8266HTTPClient.h>

#include <ESP8266WebServer.h>

#include <EEPROM.h>

//Variables

int i = 0;

int statusCode;

String st;

String content;

String esid;

String epass;

int PinStopNC = 13;  // rename to appr function eg: pinForward (14)

int PinCloseNO = 12;  // same here (12)

int PinOpenNO = 14;  // same here (4)

String MAC_ID = WiFi.macAddress();

String lastFiveDigits = MAC_ID.substring(12);

const char* ClientID = MAC_ID.c_str();

const char* topic1 = MAC_ID.c_str();

WiFiClient client;

PubSubClient mqtt(client);



//Function Declaration

bool testWifi(void);

void launchWeb(void);

void setupAP(void);

boolean mqttConnect();



//--------Establishing Local server at port 80 whenever required

ESP8266WebServer server(80);



void setup()

{

  WiFi.mode(WIFI_AP_STA);

  Serial.begin(115200);  //Initializing if(DEBUG)Serial Monitor

  pinMode(PinStopNC, OUTPUT);

  pinMode(PinCloseNO, OUTPUT);

  pinMode(PinOpenNO, OUTPUT);

  digitalWrite(PinStopNC, LOW);

  digitalWrite(PinCloseNO, LOW);

  digitalWrite(PinOpenNO, LOW);

  delay(1000);

  Serial.println();

  // Serial.println("Disconnecting previously connected WiFi");

  // WiFi.disconnect();

  EEPROM.begin(512);  //Initializing EEPROM

  delay(10);

  //  pinMode(LED_BUILTIN, OUTPUT);

  Serial.println();

  Serial.println();

  Serial.println("Startup");

  Serial.println(WiFi.macAddress());

  // Read eeprom for ssid and password

  Serial.println("Reading EEPROM ssid");

  //String esid;

  for (int i = 0; i < 32; ++i)

  {

    esid += char(EEPROM.read(i));
  }

  Serial.println();

  Serial.print("SSID: ");

  Serial.println(esid);

  Serial.println("Reading EEPROM pass");

  //String epass = "";

  for (int i = 32; i < 64; ++i)

  {

    epass += char(EEPROM.read(i));
  }

  Serial.print("PASS: ");

  Serial.println(epass);



  Serial.println("Reading EEPROM port");

  String eport = "";

  for (int i = 64; i < 96; ++i)

  {

    eport += char(EEPROM.read(i));
  }

  Serial.print("PORT: ");

  Serial.println(eport);

  Serial.println("Reading EEPROM broker");

  String ebroker = "";

  // const char* broker = ebroker.c_str();

  for (int i = 96; i < 128; ++i)

  {

    ebroker += char(EEPROM.read(i));
  }

  Serial.print("BROKER: ");

  Serial.println(ebroker);

  Serial.println();



  mqtt.setServer(ebroker.c_str(), eport.toInt());

  Serial.println("Turning the HotSpot On");

  launchWeb();

  setupAP();  // Setup accesspoint or HotSpot

  WiFi.begin(esid.c_str(), epass.c_str());


  if (testWifi())

  {

    Serial.println("Succesfully Connected!!!");

    while (mqttConnect() == false)

      continue;

    return;
  }

  // else

  // {

  //   Serial.println("Turning the HotSpot On");

  //   launchWeb();

  //   setupAP();// Setup accesspoint or HotSpot

  // }



  Serial.println();

  // Serial.println("Waiting.");



  // while ((WiFi.status() != WL_CONNECTED))

  // {

  //   Serial.print(".");

  //   delay(100);

  //   server.handleClient();

  // }
}

void loop() {

  server.handleClient();

  if ((WiFi.status() == WL_CONNECTED))

  {

    if (!mqtt.connected()) {
      server.handleClient();

      Serial.println("MQTT not connected...");
      delay(60000);
      server.handleClient();
      if (WiFi.status() == WL_CONNECTED)
        ESP.reset();

      //while(mqttConnect()==false)

      //continue;

      //Serial.println("MQTT is connected");
    }
    //
    while (mqtt.loop() && i < 2)

    {
      server.handleClient();

      i++;

      delay(1);
    }



  }

  else

  {
    server.handleClient();
    WiFi.begin(esid.c_str(), epass.c_str());
    while (WiFi.status() != WL_CONNECTED) {
      server.handleClient();
      delay(1000);
      server.handleClient();
      Serial.println("Connecting to WiFi..");
    }

    Serial.println("Succesfully Connected in loop");
    ESP.reset();
    //      delay(2000);
    //
    //      Serial.println("Connecting to MQTT Broker...");
    //
    //      while(mqttConnect()==false)
    //
    //      continue;
  }
}



//Functions used for saving WiFi credentials and to connect to it which you do not need to change

bool testWifi(void)

{
  server.handleClient();
  int c = 0;

  Serial.println("Waiting for WiFi to connect");

  while (c < 20) {

    if (WiFi.status() == WL_CONNECTED)

    {
      server.handleClient();
      return true;
    }

    delay(1000);

    server.handleClient();

    Serial.print("*");

    c++;
  }

  Serial.println("");

  // Serial.println("Connection timed out, opening AP or Hotspot");

  return false;
}



void launchWeb()

{

  Serial.println("");

  if (WiFi.status() == WL_CONNECTED)

    Serial.println("WiFi connected");

  Serial.print("Local IP: ");

  Serial.println(WiFi.localIP());

  Serial.print("SoftAP IP: ");

  Serial.println(WiFi.softAPIP());

  createWebServer();

  // Start the server

  server.begin();

  Serial.println("Server started");
}



void setupAP(void)

{

  // WiFi.mode(WIFI_STA);

  // WiFi.disconnect();

  delay(100);

  String lastFiveDigits = MAC_ID.substring(12);

  int n = WiFi.scanNetworks();

  Serial.println("scan completed");

  if (n == 0)

    Serial.println("No WiFi Networks found");

  else

  {

    Serial.print(n);

    Serial.println(" Networks found");

    for (int i = 0; i < n; ++i)

    {

      // Print SSID and RSSI for each network found

      Serial.print(i + 1);

      Serial.print(": ");

      Serial.print(WiFi.SSID(i));

      Serial.print(" (");

      Serial.print(WiFi.RSSI(i));

      Serial.print(")");

      Serial.println((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*");

      delay(10);
    }
  }

  Serial.println("");

  st = "<ol>";

  for (int i = 0; i < n; ++i)

  {

    // Print SSID and RSSI for each network found

    st += "<li>";

    st += WiFi.SSID(i);

    st += " (";

    st += WiFi.RSSI(i);



    st += ")";

    st += (WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*";

    st += "</li>";
  }

  st += "</ol>";

  delay(100);

  WiFi.softAP("RemSmart " + lastFiveDigits);

  Serial.println("Initializing_Wifi_accesspoint");

  launchWeb();

  Serial.println("over");
  server.handleClient();
}



void createWebServer()

{

  {

    server.on("/", []() {
      IPAddress ip = WiFi.softAPIP();

      String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);

      // content = "<!DOCTYPE HTML>\r\n<html>Hello from Remsmart at ";

      // content += "<form action=\"/scan\" method=\"POST\"><input type=\"submit\" value=\"scan\"></form>";

      // content += ipStr;

      // content += "<p>";

      // content += st;

      // content += "</p><form method='get' action='setting'><label>SSID: </label><input name='ssid' placeholder='ssid' length=32><input name='pass' placeholder='password' length=32><input name='broker' placeholder='server' length=64><input name='port' placeholder='port' length=32><input type='submit'></form>";

      // content += "</html>";

      // server.send(200, "text/html", content);

      // });
      content = "<!DOCTYPE HTML><html>";
      content += "<head>";
      content += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
      content += "<title>Remsmart Device Setup</title>";

      content += "<style>";

      content += "body{";
      content += "margin:0;";
      content += "font-family:Arial,sans-serif;";
      content += "background:#f4f7f9;";
      content += "color:#222;";
      content += "}";

      content += ".header{";
      content += "background:#1565c0;";
      content += "color:white;";
      content += "padding:22px 20px;";
      content += "text-align:center;";
      content += "}";

      content += ".header h2{";
      content += "margin:0;";
      content += "font-size:24px;";
      content += "}";

      content += ".header p{";
      content += "margin:6px 0 0;";
      content += "font-size:14px;";
      content += "opacity:.9;";
      content += "}";

      content += ".container{";
      content += "max-width:480px;";
      content += "margin:20px auto;";
      content += "padding:0 15px;";
      content += "}";

      content += ".card{";
      content += "background:white;";
      content += "border-radius:14px;";
      content += "padding:20px;";
      content += "margin-bottom:16px;";
      content += "box-shadow:0 3px 12px rgba(0,0,0,.08);";
      content += "}";

      content += ".card h3{";
      content += "margin-top:0;";
      content += "font-size:18px;";
      content += "}";

      content += ".status{";
      content += "background:#eef5ff;";
      content += "border-left:4px solid #1565c0;";
      content += "padding:12px;";
      content += "border-radius:6px;";
      content += "font-size:14px;";
      content += "}";

      content += ".scan{";
      content += "width:100%;";
      content += "padding:12px;";
      content += "border:0;";
      content += "border-radius:8px;";
      content += "background:#1565c0;";
      content += "color:white;";
      content += "font-size:16px;";
      content += "cursor:pointer;";
      content += "}";

      content += ".network{";
      content += "background:#f8f9fa;";
      content += "border:1px solid #ddd;";
      content += "border-radius:8px;";
      content += "padding:10px;";
      content += "margin-top:10px;";
      content += "font-size:14px;";
      content += "}";

      content += "label{";
      content += "display:block;";
      content += "font-size:14px;";
      content += "font-weight:bold;";
      content += "margin-bottom:6px;";
      content += "}";

      content += "input{";
      content += "width:100%;";
      content += "box-sizing:border-box;";
      content += "padding:12px;";
      content += "border:1px solid #ccc;";
      content += "border-radius:8px;";
      content += "font-size:15px;";
      content += "margin-bottom:15px;";
      content += "}";

      content += "input:focus{";
      content += "outline:none;";
      content += "border-color:#1565c0;";
      content += "}";

      content += ".save{";
      content += "width:100%;";
      content += "padding:13px;";
      content += "border:0;";
      content += "border-radius:8px;";
      content += "background:#2e7d32;";
      content += "color:white;";
      content += "font-size:16px;";
      content += "font-weight:bold;";
      content += "}";

      content += ".footer{";
      content += "text-align:center;";
      content += "font-size:12px;";
      content += "color:#777;";
      content += "padding:15px;";
      content += "}";

      content += "</style>";
      content += "</head>";

      content += "<body>";

      content += "<div class='header'>";
      content += "<h2>Remsmart</h2>";
      content += "<p>Device Configuration</p>";
      content += "</div>";

      content += "<div class='container'>";

      content += "<div class='card'>";
      content += "<h3>Device Status</h3>";

      content += "<div class='status'>";
      content += "Device IP: ";
      content += ipStr;
      content += "<br><br>";
      content += "Wi-Fi Status: Connected";
      content += "</div>";

      content += "</div>";

      content += "<div class='card'>";
      content += "<h3>Wi-Fi Networks</h3>";

      content += "<form action='/scan' method='POST'>";
      content += "<input class='scan' type='submit' value='Scan Wi-Fi Networks'>";
      content += "</form>";

      content += "<div class='network'>";
      content += st;
      content += "</div>";

      content += "</div>";

      content += "<div class='card'>";
      content += "<h3>Network Settings</h3>";

      content += "<form method='get' action='/setting'>";

      content += "<label>Wi-Fi SSID</label>";
      content += "<input name='ssid' placeholder='Enter Wi-Fi name' maxlength='32' required>";

      content += "<label>Wi-Fi Password</label>";
      content += "<input type='password' name='pass' placeholder='Enter Wi-Fi password' maxlength='32'required>";

      content += "<label>MQTT Broker</label>";
      content += "<input name='broker' placeholder='Enter MQTT server' maxlength='64'required>";

      content += "<label>MQTT Port</label>";
      content += "<input name='port' placeholder='Example: 1883' maxlength='32' inputmode='numeric'required>";

      content += "<input class='save' type='submit' value='Save Settings'>";

      content += "</form>";

      content += "</div>";

      content += "<div class='footer'>";
      content += "Remsmart Device Configuration";
      content += "</div>";

      content += "</div>";

      content += "</body>";
      content += "</html>";

      server.send(200, "text/html", content);
    });

    server.on("/scan", HTTP_POST, []() {
      Serial.println("Scanning WiFi networks...");

      int n = WiFi.scanNetworks();

      st = "";

      if (n == 0) {
        st = "No WiFi networks found.";
      } else {

        st += "<b>Available WiFi Networks:</b><br><br>";

        for (int i = 0; i < n; i++) {

          st += String(i + 1);
          st += ". ";
          st += WiFi.SSID(i);
          st += " (";
          st += String(WiFi.RSSI(i));
          st += " dBm)";

          if (WiFi.encryptionType(i) != ENC_TYPE_NONE) {
            st += " 🔒";
          }

          st += "<br>";
        }
      }

      WiFi.scanDelete();

      String page = "<!DOCTYPE HTML><html>";
      page += "<head>";
      page += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
      page += "<title>Remsmart WiFi Setup</title>";
      page += "<style>";
      page += "body{margin:0;font-family:Arial;background:#f4f7f9;color:#222;}";
      page += ".header{background:#1565c0;color:white;padding:22px;text-align:center;}";
      page += ".container{max-width:480px;margin:20px auto;padding:15px;}";
      page += ".card{background:white;border-radius:14px;padding:20px;margin-bottom:16px;box-shadow:0 3px 12px rgba(0,0,0,.08);}";
      page += ".network{background:#eef5ff;border-left:4px solid #1565c0;padding:15px;border-radius:8px;line-height:1.8;}";
      page += ".back{width:100%;padding:12px;border:0;border-radius:8px;background:#1565c0;color:white;font-size:16px;}";
      page += "</style>";
      page += "</head><body>";

      page += "<div class='header'>";
      page += "<h2>Remsmart</h2>";
      page += "<p>WiFi Network Scan</p>";
      page += "</div>";

      page += "<div class='container'>";
      page += "<div class='card'>";
      page += "<h3>Available WiFi Networks</h3>";
      page += "<div class='network'>";
      page += st;
      page += "</div>";
      page += "</div>";

      page += "<div class='card'>";
      page += "<form action='/' method='GET'>";
      page += "<input class='back' type='submit' value='Back to Settings'>";
      page += "</form>";
      page += "</div>";

      page += "</div>";
      page += "</body></html>";

      server.send(200, "text/html", page);
    });



   server.on("/setting", []() {

  String qsid = server.arg("ssid");
  qsid.trim();

  String qpass = server.arg("pass");
  qpass.trim();

  String qport = server.arg("port");
  qport.trim();

  String qbroker = server.arg("broker");
  qbroker.trim();

  if (qsid.length() == 0 ||
      qpass.length() == 0 ||
      qport.length() == 0 ||
      qbroker.length() == 0) {

    content = "<!DOCTYPE HTML><html>";
    content += "<head>";
    content += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    content += "<title>Remsmart</title>";
    content += "</head>";
    content += "<body style='font-family:Arial;text-align:center;padding:30px;'>";
    content += "<h2 style='color:#c62828;'>Missing Information</h2>";
    content += "<p>Please fill in all fields.</p>";
    content += "<a href='/'>Go Back</a>";
    content += "</body></html>";

    server.send(400, "text/html", content);
    return;
  }

  Serial.println();
  Serial.println("Testing new WiFi credentials...");
  Serial.println("SSID: " + qsid);

  WiFi.disconnect();
  delay(1000);

  WiFi.mode(WIFI_AP_STA);

  WiFi.begin(qsid.c_str(), qpass.c_str());

  int attempts = 0;

  while (WiFi.status() != WL_CONNECTED && attempts < 20) {

    delay(500);
    Serial.print(".");

    attempts++;
  }

  Serial.println();

  if (WiFi.status() != WL_CONNECTED) {

    Serial.println("WiFi connection failed");
    Serial.println("Wrong SSID or password");

    content = "<!DOCTYPE HTML><html>";
    content += "<head>";
    content += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    content += "<title>Remsmart</title>";
    content += "</head>";

    content += "<body style='font-family:Arial;background:#f4f7f9;text-align:center;padding:30px;'>";

    content += "<div style='background:white;padding:30px;border-radius:14px;max-width:400px;margin:auto;box-shadow:0 3px 12px rgba(0,0,0,.1);'>";

    content += "<h2 style='color:#c62828;'>WiFi Connection Failed</h2>";

    content += "<p>SSID or password is incorrect.</p>";
    content += "<p><b>Settings were NOT saved.</b></p>";

    content += "<a href='/' style='display:block;background:#1565c0;color:white;padding:12px;border-radius:8px;text-decoration:none;'>Try Again</a>";

    content += "</div>";
    content += "</body></html>";

    server.send(200, "text/html", content);

    return;
  }

  Serial.println("WiFi connected successfully");
  Serial.print("New WiFi IP: ");
  Serial.println(WiFi.localIP());

  Serial.println("Clearing EEPROM");

  for (int i = 0; i < 128; ++i) {
    EEPROM.write(i, 0);
  }

  Serial.println("Writing SSID");

  for (int i = 0; i < qsid.length(); ++i) {
    EEPROM.write(i, qsid[i]);
  }

  Serial.println("Writing password");

  for (int i = 0; i < qpass.length(); ++i) {
    EEPROM.write(32 + i, qpass[i]);
  }

  Serial.println("Writing port");

  for (int i = 0; i < qport.length(); ++i) {
    EEPROM.write(64 + i, qport[i]);
  }

  Serial.println("Writing broker");

  for (int i = 0; i < qbroker.length(); ++i) {
    EEPROM.write(96 + i, qbroker[i]);
  }

  EEPROM.commit();

  Serial.println("Settings saved successfully");

  content = "<!DOCTYPE HTML><html>";
  content += "<head>";
  content += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  content += "<title>Remsmart</title>";
  content += "</head>";

  content += "<body style='font-family:Arial;background:#f4f7f9;text-align:center;padding:30px;'>";

  content += "<div style='background:white;padding:30px;border-radius:14px;max-width:400px;margin:auto;box-shadow:0 3px 12px rgba(0,0,0,.1);'>";

  content += "<h2 style='color:#2e7d32;'>WiFi Connected Successfully</h2>";

  content += "<p>Settings saved successfully.</p>";
  content += "<p>The device will restart and connect to the new WiFi.</p>";

  content += "</div>";
  content += "</body></html>";

  server.send(200, "text/html", content);

  delay(3000);

  ESP.reset();
});
  }
}



boolean mqttConnect()

{

  server.handleClient();

  if (mqtt.connect(ClientID))

  {

    Serial.println("Connected to broker.");

    //  disableCore0WDT();

    mqtt.subscribe(topic1);

    mqtt.setCallback(mqttCallback);

    server.handleClient();

    return mqtt.connected();
  }

  Serial.print(",.,");

  server.handleClient();

  return false;
}



void mqttCallback(char* topic, byte* payload, unsigned int len)  // topic  = STATUS  message/payload = MacAddress$xyzx

{

  Serial.println("In mqtt callback");

  String message;

  String macidVal = WiFi.macAddress();  //Here is the MAC_id of ESP32

  Serial.println("macidVal");

  for (int i = 0; i < len; i++)

  {

    message += (char)payload[i];
  }

  Serial.println("code is working: " + message);
  Serial.println("mac id: " + macidVal);

  //  String Statusmessage = "MAC ID: " + macidVal + " | Received message: " + message;
  String Statusmessage = "MAC ID: " + macidVal + " | Status: Active ";
  Serial.println("Statusmessage: " + Statusmessage);

  //  getting MacId
  //  int index = message.indexOf('$');
  //  Serial.println(message.substring(0,index));

  //  getting action
  int index1 = message.lastIndexOf('$');
  int length = message.length();
  String action = message.substring(index1, length);
  Serial.println(action);
  Serial.println("****************************");
  Serial.println(message.indexOf(macidVal));
  Serial.println("****************************");

  //  if(message.indexOf(macidVal)>=0){

  if (action == ("$Stop"))

  {

    Serial.println("Stop");

    digitalWrite(PinStopNC, HIGH);

    delay(500);

    digitalWrite(PinStopNC, LOW);

    delay(500);

  }

  else if (action == ("$Close"))

  {
    Serial.println("Close");

    digitalWrite(PinCloseNO, HIGH);

    delay(500);

    digitalWrite(PinCloseNO, LOW);

    delay(500);

  }

  else if (action == ("$Open"))

  {

    Serial.println("Open");

    digitalWrite(PinOpenNO, HIGH);

    delay(500);

    digitalWrite(PinOpenNO, LOW);

    delay(500);

  }

  else if (action == ("$toggle1"))

  {

    Serial.println("toggle1");

    digitalWrite(PinStopNC, HIGH);

    delay(500);

    digitalWrite(PinStopNC, LOW);

    delay(500);

  }

  else if (action == ("$toggle2"))

  {
    Serial.println("toggle2");

    digitalWrite(PinCloseNO, HIGH);

    delay(500);

    digitalWrite(PinCloseNO, LOW);

    delay(500);

  }

  else if (action == ("$toggle3"))

  {

    Serial.println("toggle3");

    digitalWrite(PinOpenNO, HIGH);

    delay(500);

    digitalWrite(PinOpenNO, LOW);

    delay(500);

  }

  else if (action == ("$Hello"))

  {

    Serial.println("Hello from ESP8266.....");

    mqtt.publish("Status", Statusmessage.c_str());  //"Status" is topic name,"Active" is message

    delay(1000);
  }


  //  }
}