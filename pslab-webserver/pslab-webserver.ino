 /*
  * This is a HTTP web server running on esp8622
  * This web server will interface the functionalities of the PSLab to the user who access it through web
 
  by Nuwan Tharaka 
  3/4/2018
*/

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

const char* ssid = "testssid";
const char* password = "testpassword";

ESP8266WebServer server(80);

void handleRoot() {
  server.send(200, "text/plain", "Welcome to the PSLab!");
}

void handleNotFound(){
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void setup(void){
  //init UART communication
  Serial.begin(115200);

  //iniit WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  
  //UART communication initialization with PSLab hardware goes here

  //mDNS init - sets the domain to "pslab.local"
  if (MDNS.begin("pslab")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);

  server.onNotFound(handleNotFound);

  server.begin();
  
  // Add service to MDNS-SD
  MDNS.addService("http", "tcp", 80);
}

void loop(void){
  server.handleClient();
}
