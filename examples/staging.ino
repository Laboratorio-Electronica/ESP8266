#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

AsyncWebServer server(80);

// const char* SSID = "IngenicoLAB";
// const char* password = "Lab@ingenico1";
const char* SSID = "Krloz Medina";
const char* password = "F@mili@571112";

//const char* PARAM_HORA = "hora"; // valor 1
const char* PARAM_STRING = "stringDate";
const char* PARAM_STRING2 = "stringHour";
const char* PARAM_STRING3 = "stringTemperature";
const char* PARAM_STRING4 = "stringHumidity";

#define DHTPIN 0    // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11   // DHT 11
DHT_Unified dht(DHTPIN, DHTTYPE);
uint32_t delayMS;

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "co.pool.ntp.org", -18000, 60000);

//Week Days
String weekDays[7]={"Domingo", "Lunes", "Martes", "Miércoles", "Jueves", "Viernes", "Sábado"};

//Month names
String months[12]={"Enero", "Febrero", "Marzo", "Abril", "Mayo", "Junio", "Julio", "Agosto", "Septiembre", "Octubre", "Noviembre", "Diciembre"};

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html lang="en">

<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <script src="https://kit.fontawesome.com/e00844d238.js" crossorigin="anonymous"></script>
    <!-- <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css"
        integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous"> -->
    <style>
        html {
            font-family: Arial;
            display: inline-block;
            margin: 0px auto;
            text-align: center;
        }
        h1 { font-size: 3.0rem; }
        p { font-size: 3.0rem; }
        .units { font-size: 1.2rem; }
        .ds-labels {
            font-size: 1.5rem;
            vertical-align: middle;
            padding-bottom: 15px;
        }
        .container{
            display: flex;
            justify-content: space-evenly;
        }
    </style>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>ESP Input Form</title>
    <script>
        function submitMessage() {
            alert("Guardamos los datos en la placa NodeMCU por  ESP SPIFFS");
            setTimeout(function () { document.location.reload(false); }, 500);
        }
    </script>
</head>

<body>
    <h1>Ingenico Colombia</h1>
    <p>
        <span class="ds-labels">Hoy es</span>
        <span id="fecha">%stringDate%</span>
    </p>
    <p>
        <span class="ds-labels">Hora</span>
        <span id="hora">%stringHour%</span>
    </p>
    <h2>Termohigrómetro</h2>
    <div class="container">
        <p>
            <i class="fas fa-thermometer-half" style="color:#059e8a;"></i>
            <span class="ds-labels">Temperatura</span>
            <span id="temperatureC">%stringTemperature%</span>
        </p>
        <p>
            <i class="fa-solid fa-droplet" style="color:#059e8a;"></i>
            <span class="ds-labels">Humedad</span>
            <span id="humidity">%stringHumidity%</span>
        </p>
    </div>
    <footer>
        <em>By KrlozMedina, visited <a href="http://krlozmedina.com">krlozmedina.com</a></em>
    </footer>
</body>

<script type="text/javascript">
    function actualizar() { location.reload(true); }
    //Funciona para actualizar cada 4 segundos(4000 miliseconds)
    setInterval("actualizar()", 10000);
</script>
</html>
)rawliteral";

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

float humidity() {
  // Delay between measurements.
  delay(delayMS);
  sensors_event_t event;
  // Get humidity event and print its value.
  dht.humidity().getEvent(&event);
  if (isnan(event.relative_humidity)) {
    // Serial.println(F("Error reading humidity!"));
    return 0;
  }
  else {
    return event.relative_humidity;
  }
}

String processor5(const String& var){

  // ---------------------------------------------------------------
  // Delay between measurements.
  // delay(delayMS);
  // Get temperature event and print its value.
  sensors_event_t event;
  DHT_Unified dht(DHTPIN, DHTTYPE);
  // dht.temperature().getEvent(&event);
  // ---------------------------------------------------------------


  if(var == "stringDate"){
    String CUCU = weekDays[timeClient.getDay()];

    //Get a time structure
    time_t epochTime = timeClient.getEpochTime();
    struct tm *ptm = gmtime ((time_t *)&epochTime); 

    int monthDay = ptm->tm_mday;
    int currentYear = ptm->tm_year+1900;
    int currentMonth = ptm->tm_mon+1;
    String currentMonthName = months[currentMonth-1];

    CUCU = CUCU + " "  +  monthDay +  " de " + currentMonthName +  " del " + currentYear;

    return CUCU;
  }
  else if(var == "stringHour"){
    String hora = timeClient.getFormattedTime();
    return hora;
  }

  time_t epochTime = timeClient.getEpochTime();
  struct tm *ptm = gmtime ((time_t *)&epochTime);
  int currentYear = ptm->tm_year+1900;
  
  if (var == "stringTemperature" and currentYear==2023){
    // String  fuego = "<img src= https://openlab.bmcc.cuny.edu/jasmine-bush-portfolio/wp-content/uploads/sites/302/2020/02/flame-1.gif width= 250 height=166  >";
    String fuego = String(event.temperature);
    return fuego;
  }

  if (var == "stringHumidity"){
    // String fuego = String(humidity());
    // return fuego;
  }
  
  return String();
}

void setup() {
  // Initialize Serial Monitor
  Serial.begin(9600);
  
  // Connect to Wi-Fi
  Serial.print("Connecting to ");
  Serial.println(SSID);
  WiFi.begin(SSID, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  delay(1000);

  dht.begin();
  Serial.println(F("DHTxx Unified Sensor Example"));
  // Print temperature sensor details.
  sensor_t sensor;
  dht.temperature().getSensor(&sensor);
  Serial.println(F("------------------------------------"));
  Serial.println(F("Temperature Sensor"));
  Serial.print  (F("Sensor Type: ")); Serial.println(sensor.name);
  Serial.print  (F("Driver Ver:  ")); Serial.println(sensor.version);
  Serial.print  (F("Unique ID:   ")); Serial.println(sensor.sensor_id);
  Serial.print  (F("Max Value:   ")); Serial.print(sensor.max_value); Serial.println(F("°C"));
  Serial.print  (F("Min Value:   ")); Serial.print(sensor.min_value); Serial.println(F("°C"));
  Serial.print  (F("Resolution:  ")); Serial.print(sensor.resolution); Serial.println(F("°C"));
  Serial.println(F("------------------------------------"));
  // Print humidity sensor details.
  dht.humidity().getSensor(&sensor);
  Serial.println(F("Humidity Sensor"));
  Serial.print  (F("Sensor Type: ")); Serial.println(sensor.name);
  Serial.print  (F("Driver Ver:  ")); Serial.println(sensor.version);
  Serial.print  (F("Unique ID:   ")); Serial.println(sensor.sensor_id);
  Serial.print  (F("Max Value:   ")); Serial.print(sensor.max_value); Serial.println(F("%"));
  Serial.print  (F("Min Value:   ")); Serial.print(sensor.min_value); Serial.println(F("%"));
  Serial.print  (F("Resolution:  ")); Serial.print(sensor.resolution); Serial.println(F("%"));
  Serial.println(F("------------------------------------"));
  // Set delay between sensor readings based on sensor details.
  delayMS = sensor.min_delay / 1000;


  // Send web page with input fields to client
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor5);
  });
  
  server.onNotFound(notFound);
  server.begin();

  // Initialize a NTPClient to get time
  timeClient.begin();
}

void loop() {
  timeClient.update();

  time_t epochTime = timeClient.getEpochTime();
  Serial.print("Epoch Time: ");
  Serial.println(epochTime);
  
  String formattedTime = timeClient.getFormattedTime();
  Serial.print("Formatted Time: ");
  Serial.println(formattedTime);  

  int currentHour = timeClient.getHours();
  Serial.print("Hour: ");
  Serial.println(currentHour);  

  int currentMinute = timeClient.getMinutes();
  Serial.print("Minutes: ");
  Serial.println(currentMinute); 

  int currentSecond = timeClient.getSeconds();
  Serial.print("Seconds: ");
  Serial.println(currentSecond);  

  String weekDay = weekDays[timeClient.getDay()];
  String CUCU = weekDays[timeClient.getDay()];
  Serial.print("Week Day: ");
  Serial.println(weekDay);    

  //Get a time structure
  struct tm *ptm = gmtime ((time_t *)&epochTime); 

  int monthDay = ptm->tm_mday;
  Serial.print("Month day: ");
  Serial.println(monthDay);

  int currentMonth = ptm->tm_mon+1;
  Serial.print("Month: ");
  Serial.println(currentMonth);

  String currentMonthName = months[currentMonth-1];
  Serial.println(currentMonthName);

  int currentYear = ptm->tm_year+1900;
  Serial.print("Year: ");
  Serial.println(currentYear);

  //Print complete date:
  String currentDate = String(monthDay) + "-" + String(currentMonth) + "-" + String(currentYear);
  Serial.print("Current date: ");
  Serial.println(currentDate);
  Serial.println("");

  // Delay between measurements.
  delay(delayMS);
  // Get temperature event and print its value.
  sensors_event_t event;
  dht.temperature().getEvent(&event);
  if (isnan(event.temperature)) {
    Serial.println(F("Error reading temperature!"));
  }
  else {
    Serial.print(F("Temperature: "));
    Serial.print(event.temperature);
    Serial.println(F("°C"));
  }
  // Get humidity event and print its value.
  Serial.print("Humedad: ");
  Serial.print(humidity());
  Serial.println("%");

  delay(10000);
}