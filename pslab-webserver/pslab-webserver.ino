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
#include <WebSocketsServer.h>

const char* ssid = "testssid";
const char* password = "testpassword";

String incomingStr = "";

ESP8266WebServer server(80);
WebSocketsServer ws = WebSocketsServer(81);

//webpages will be developed externally and integrated later
void handleRoot() {
  server.on("/version.txt", [](){
    server.send(200, "text/html", incomingStr);
  });
  
  server.on("/", [](){
   String page = "<h1>Version Number</h1><p id=\"data\">""</p>\r\n";
   page += "<script>\r\n";
   page += "var x = setInterval(function() {loadData(\"data.txt\",updateData)}, 1000);\r\n";
   page += "function loadData(url, callback){\r\n";
   page += "var xhttp = new XMLHttpRequest();\r\n";
   page += "xhttp.onreadystatechange = function(){\r\n";
   page += " if(this.readyState == 4 && this.status == 200){\r\n";
   page += " callback.apply(xhttp);\r\n";
   page += " }\r\n";
   page += "};\r\n";
   page += "xhttp.open(\"GET\", url, true);\r\n";
   page += "xhttp.send();\r\n";
   page += "}\r\n";
   page += "function updateData(){\r\n";
   page += " document.getElementById(\"data\").innerHTML = this.responseText;\r\n";
   page += "}\r\n";
   page += "</script>\r\n";
   server.send(200, "text/html", page);
});
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

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {

    switch(type) {
        case WStype_DISCONNECTED:
            break;
        case WStype_CONNECTED:
            {
            IPAddress ip = ws.remoteIP(num);
        
            // send message to client
            
            //UART to retrieve version info
            Serial.write(11);
            Serial.write(5);
          
            incomingStr.reserve(50);
          
            while(!Serial.available());
          
            if (Serial.available()) {
              // read the incoming String
              incomingStr = Serial.readStringUntil('\n');
            }
            ws.sendTXT(num, incomingStr);
            }
            break;
        case WStype_TEXT:
            // send message to client
            // webSocket.sendTXT(num, "message here");

            // send data to all connected clients
            // webSocket.broadcastTXT("message here");
            break;
        case WStype_BIN:
            // send message to client
            //ws.sendBIN(num, payload, length);
            break;
    }
}

void setup(void){
  //init UART communication
  Serial.begin(1000000); //have to check and change accrodingly

  //iniit WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  //start webSocket server
  ws.begin();
  ws.onEvent(webSocketEvent);
  
  //mDNS init - sets the domain to "pslab.local"
  MDNS.begin("pslab");
  
  server.on("/", handleRoot);

  server.onNotFound(handleNotFound);

  server.begin();
    
  // Add service to MDNS-SD
  MDNS.addService("http", "tcp", 80);
  MDNS.addService("ws", "tcp", 81);


}

void loop(void){
  ws.loop();
  server.handleClient();
}
