// Librerías
#include <Arduino.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266WebServer.h>
#include <Adafruit_Sensor.h>
#include "DHT.h"
#include <DHT_U.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include "ESP8266HTTPClient.h"

// Configuración del hardware
#define PIN_DATA_DHT D2
#define SENSOR_TYPE DHT11
#define LED_STATE 2

// Inicializando librerías
ESP8266WiFiMulti wifiMulti;
ESP8266WebServer server(80);
DHT sensor(PIN_DATA_DHT, SENSOR_TYPE);
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "co.pool.ntp.org", -18000, 60000);

// Variables
int lastUpdate = 60000, ubiety = 0;
long intervalUpdate = 30000;   // Debería ser mayor que 2000
unsigned long lastUpdateSuccess = 0;
float humidity = 0, temperature = 0;
String weekDays[7]={"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
String months[12]={"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};
String monthsShort[12]={"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
String ubieties[2] = {"warehouse", "laboratory"};
String ubietiesLetter[2] = {"W", "L"};

// Prototipos de funciones
void homePage();
void notFoundPage();
void apiPage();
void errorDHT();
void successDHT();
char* dataDHT();


void setup(void) {
  Serial.begin(9600);

  pinMode(LED_STATE, OUTPUT);
  digitalWrite(LED_STATE, LOW);

  sensor.begin();

  // Aquí puedes agregar varias redes. La tarjeta se conectará a la más cercana
  // wifiMulti.addAP("Otra red", "Contraseña");
  wifiMulti.addAP("IngenicoLAB", "Lab@ingenico1");
  wifiMulti.addAP("Krloz Medina", "F@mili@571112");

  // Esperar conexión WiFi
  Serial.println("Searching AP WiFI");
  wifiMulti.run();

  if(wifiMulti.run() == WL_CONNECTED) {
    Serial.print("WiFi connected, IP address: ");
    Serial.println(WiFi.localIP());
  }

  //  Configurar rutas
  server.on("/", homePage);
  server.on("/api", apiPage);
  server.onNotFound(notFoundPage);

  // Iniciar server
  server.begin();
  digitalWrite(LED_STATE, HIGH);

  // Initialize a NTPClient to get time
  timeClient.begin();
}


void loop(void) {
  if (lastUpdate > intervalUpdate) {
    // Captura de fecha y hora con NTP
    timeClient.update();

    time_t epochTime = timeClient.getEpochTime();
    String formattedTime = timeClient.getFormattedTime();
    int currentHour = timeClient.getHours();
    int currentMinute = timeClient.getMinutes();
    String weekDay = weekDays[timeClient.getDay()];

    //Get a time structure
    struct tm *ptm = gmtime ((time_t *)&epochTime); 

    int monthDay = ptm->tm_mday;
    int currentMonth = ptm->tm_mon+1;
    int currentYear = ptm->tm_year+1900;

    String currentMonthName = months[currentMonth-1];
    String currentMonthString = String(currentMonth);

    if (currentMonth < 10) {
      currentMonthString = String(0) + String(currentMonth);
    }
    
    // Crear variables con datos del NTP
    String id = String(monthsShort[currentMonth - 1] + String(currentYear - 2000));
    String time = String(ubietiesLetter[ubiety]) + String(currentYear - 2000) + String(currentMonthString) + String(monthDay) +  String(currentHour);
    
    // Captura de datos del DHT
    float newHumidity = sensor.readHumidity();
    float newTemperature = sensor.readTemperature();

    if (isnan(newTemperature) || isnan(newHumidity))
    {
      errorDHT();
      lastUpdate = 0;
      return;
    }

    lastUpdateSuccess = millis();
    humidity = newHumidity;
    temperature = newTemperature;
    lastUpdate = 0;
    successDHT();

        // Hora de envió de datos
    // if ((currentHour==14 && currentMinute==40) || (currentHour==14 && currentMinute==43)) {
    if (currentMinute==10) {
      // Buffer para escribir datos en JSON
      char buffer[150];
      sprintf(buffer, "{\"id\":\"%s\",\"time\":\"%s\",\"date\":\"%i-%s-%i %s\",\"humidity\":\"%.2f\",\"temperature\":\"%.2f\",\"ubiety\":\"%s\"}", id, time, monthDay, currentMonthName, currentYear, formattedTime, humidity, temperature, ubieties[ubiety]);
      Serial.println(buffer);

      // Envió de los datos a AWS
      WiFiClientSecure client;
      client.setInsecure();
      HTTPClient http;
      http.begin(client, "https://60fqd2g261.execute-api.us-east-1.amazonaws.com/records");
      http.addHeader("Content-Type", "text/plain");            
      int httpResponseCode = http.PUT(buffer);
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
    }

  }

  delay(1);
  lastUpdate += 1;
  
  server.handleClient();    // Responder las solicitudes entrantes en caso de que haya
}


void homePage()
{
  server.send(200, "text/html", "<!DOCTYPE html><html lang='es'><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width,initial-scale=1'><title>Termohigrómetro</title><link rel='stylesheet' href='https://stackpath.bootstrapcdn.com/font-awesome/4.7.0/css/font-awesome.min.css'></head><body><style>body{background-color:#395b64;color:#c4c3c3;text-align:center;font-family:BlinkMacSystemFont,-apple-system,'Segoe UI',Roboto,Oxygen,Ubuntu,Cantarell,'Fira Sans','Droid Sans','Helvetica Neue',Helvetica,Arial,sans-serif}.container{height:90vh;display:flex;flex-direction:column;justify-content:space-evenly}.columns{display:flex;justify-content:space-evenly;flex-wrap:wrap}.is-size-4{font-size:32px;color:#ffdd57;margin:0}.is-size-1{font-size:40px;margin:0}</style><section id='app' class='hero is-link is-fullheight'><div class='hero-body'><div class='container'><div class='columns has-text-centered'><div class='column'><h1 style='font-size:2.5rem'>Termohigrómetro Bodega</h1><i class='fa' :class='claseTermometro' style='font-size:4rem'></i></div></div><div class='columns'><div class='column has-text-centered'><h2 class='is-size-4 has-text-warning'>temperature</h2><h2 class='is-size-1'>{{temperature}}°C</h2></div><div class='column has-text-centered'><h2 class='is-size-4 has-text-warning'>humidity</h2><h2 class='is-size-1'>{{humidity}}%</h2></div></div><div class='columns'><div class='column'><p>Última lectura: Hace <strong class='has-text-white'>{{ultimaLectura}}</strong> segundo(s)</p></div></div></div></div></section><footer><em>By KrlozMedina, visited <a style='text-decoration:none;color:#ffdd57' href='http://krlozmedina.com'>krlozmedina.com</a></em></footer><script src='https://unpkg.com/vue@2.6.12/dist/vue.min.js'></script><script>const INTERVALO_REFRESCO = 10000; new Vue({el: '#app', data: () => ({ ultimaLectura: 0, temperature: 0, humidity: 0, }), mounted() { this.refrescarDatos(); }, methods: { async refrescarDatos() { try { const respuestaRaw = await fetch('./api'); const datos = await respuestaRaw.json(); this.ultimaLectura = datos.u; this.temperature = datos.t; this.humidity = datos.h; setTimeout(() => { this.refrescarDatos(); }, INTERVALO_REFRESCO); } catch (e) { setTimeout(() => { this.refrescarDatos(); }, INTERVALO_REFRESCO); } } }, computed: { claseTermometro() { if (this.temperature <= 5) { return 'fa-thermometer-empty'; } else if (this.temperature > 5 && this.temperature <= 13) { return 'fa-thermometer-quarter'; } else if (this.temperature > 13 && this.temperature <= 21) { return 'fa-thermometer-half'; } else if (this.temperature > 21 && this.temperature <= 30) { return 'fa-thermometer-three-quarters'; } else { return 'fa-thermometer-full'; } } } });</script></body></html>");
}


void notFoundPage()
{
  server.send(404, "text/plain", "Page no found");
}


void apiPage()
{
  // Calcular última lectura exitosa en segundos
  unsigned long timeElapsedMS = millis() - lastUpdateSuccess;
  int tiempoTranscurrido = timeElapsedMS / 1000;

  // Buffer para escribir datos en JSON
  char buffer[50];
  sprintf(buffer, "{\"t\":%.2f,\"h\":%.2f,\"u\":%d}", temperature, humidity, tiempoTranscurrido);

  server.send(200, "application/json", buffer);
}


void errorDHT()
{
  int x = 0;
  for (x = 0; x < 5; x++)
  {
    digitalWrite(LED_STATE, LOW);
    delay(50);
    digitalWrite(LED_STATE, HIGH);
    delay(50);
  }
}

void successDHT()
{
  digitalWrite(LED_STATE, LOW);
  delay(50);
  digitalWrite(LED_STATE, HIGH);
}