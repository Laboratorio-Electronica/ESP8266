#include "ESP8266HTTPClient.h"
#include "ESP8266WiFi.h"

const char* ssid = "Krloz Medina";
const char* password =  "F@mili@571112";

void setup() {
  Serial.begin(9600);
  WiFi.begin(ssid, password); 
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to the WiFi network");
}

void loop() {
  if(WiFi.status()== WL_CONNECTED){
    WiFiClientSecure client;
    // client.connect("https://60fqd2g261.execute-api.us-east-1.amazonaws.com/records", 80);
    client.setInsecure();

    HTTPClient http;   
    // http.begin(client, "http://jsonplaceholder.typicode.com/posts/1");
    http.begin(client, "https://60fqd2g261.execute-api.us-east-1.amazonaws.com/records");

    http.addHeader("Content-Type", "text/plain");            
    // int httpResponseCode = http.PUT("PUT sent from ESP32");   
    int httpResponseCode = http.PUT("{'date': '15-January-2023 20:20','humidity': 52,'time': 'W015232020','id': 'Ene23','ubiety': 'warehouse','temperature': 22}");
    if(httpResponseCode>0){
      String response = http.getString();
      Serial.println("La respuesta del servidor es:");
      Serial.println(httpResponseCode);
      Serial.println(response);
    }else{
      Serial.print("Error on sending PUT Request: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  }else{
  Serial.println("Error in WiFi connection");
  }
  delay(10000);
}