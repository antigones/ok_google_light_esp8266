#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <WiFiUdp.h>
#include "ArduinoJson.h"

#include <ESP8266HTTPClient.h>

const char *wifi_ssid = "";
const char *wifi_password = "";
 
ESP8266WebServer server(3388);
WiFiUDP Udp;

   
unsigned int localPort = 3311;
// buffers for receiving and sending data
char packetBuffer[UDP_TX_PACKET_MAX_SIZE + 1]; //buffer to hold incoming packet,
char ReplyBuffer[] = "deviceid123";       // a string to send back

#define LED_BUILTIN 2

void setup() {
  Serial.begin(115200);
  Serial.println();
  setupWifi(); 
  startWebServer();
  startUdpServer();
  startPin();
}

void startUdpServer() {
  Serial.printf("UDP server on port %d\n", localPort);
  Udp.begin(localPort);
}

void startPin() {
  pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
  digitalWrite(LED_BUILTIN, HIGH); 
}


void setupWifi() { 
  delay(10);
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);
  WiFi.begin(wifi_ssid, wifi_password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  randomSeed(micros());
  Serial.println("");
  Serial.print("WiFi connected at IP address: ");
  Serial.println(WiFi.localIP());
}

void startWebServer() {
  server.on("/", HTTP_ANY, handleRoot);
  server.begin();
  Serial.println("SERVER BEGIN!!");
}
 
void handleRoot() {  
  Serial.println("handleRoot!"); 
  Serial.println(server.arg("plain"));
  StaticJsonDocument<200> doc;
  deserializeJson(doc ,server.arg("plain"));
  // {"on":false}
  bool led_on = doc["on"];
  String led_on_s = doc["on"];
  if (led_on) {
     digitalWrite(LED_BUILTIN, LOW);
  }
  else
  {
    digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED off by making the voltage HIGH
  }
  
  WiFiClient client;
  HTTPClient http;
  Serial.println("HTTP Client Started...\n");
  http.begin("https://<PROJECT_ID>.firebaseapp.com/updatestate", "46 F2 E8 99 89 6D 93 C2 44 E0 44 22 D0 86 9B F2 56 A7 7C 95"); //HTTP
  http.addHeader("Content-Type", "application/json");
  int httpCode = http.POST(server.arg("plain"));
  Serial.printf("[HTTP] POST... code: %s\n", http.errorToString(httpCode).c_str());
  http.end();
  server.send(200, "text/html", "OK");
}

void loop() { 
  server.handleClient(); 
  
   // if there's data available, read a packet
  int packetSize = Udp.parsePacket();
  if (packetSize) {
    Serial.printf("Received packet of size %d from %s:%d\n    (to %s:%d, free heap = %d B)\n",
                  packetSize,
                  Udp.remoteIP().toString().c_str(), Udp.remotePort(),
                  Udp.destinationIP().toString().c_str(), Udp.localPort(),
                  ESP.getFreeHeap());

    // read the packet into packetBufffer
    int n = Udp.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);
    packetBuffer[n] = 0;
    Serial.println("Contents:");
    Serial.println(packetBuffer);

    // send a reply, to the IP address and port that sent us the packet we received
    Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
    Udp.write(ReplyBuffer);
    Serial.println(ReplyBuffer);
    Serial.println(Udp.remoteIP());
    Serial.println(Udp.remotePort());
    Udp.endPacket();
  }
}
 
