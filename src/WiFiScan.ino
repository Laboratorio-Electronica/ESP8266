/*
 Gracias a   Rui Santos  https://RandomNerdTutorials.com/esp8266-nodemcu-date-time-ntp-client-server-arduino/
 */
// entramos las libretrÃ­as necesarias para nuestro NodeMcu
#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>
#include <NTPClient.h>
#include <WiFiUdp.h>


AsyncWebServer server(80);

// Replace with your network credentials
const char* ssid = "IngenicoLAB";
const char* password = "Lab@ingenico1";


//const char* PARAM_HORA = "hora"; // valor 1
const char* PARAM_STRING = "inputString"; // valor 1
const char* PARAM_STRING2 = "inputString2"; // valor 1
const char* PARAM_STRING3 = "inputString3"; // valor 1

// Define NTP Client to get time

WiFiUDP ntpUDP;
// NTPClient timeClient(ntpUDP, "pool.ntp.org");
NTPClient timeClient(ntpUDP, "co.pool.ntp.org", -18000, 60000);


//Week Days
String weekDays[7]={"Domingo", "Lunes", "Martes", "Miercoles", "Jueves", "Viernes", "SÃ¡bado"};

//Month names
String months[12]={"Enero", "Febrero", "Marzo", "Abril", "Mayo", "Junio", "Julio", "Agosto", "Septiembre", "Octubre", "Noviembre", "Diciembre"};

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html lang="en">

<head>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css"
        integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">
    <style>
        html {
            font-family: Arial;
            display: inline-block;
            margin: 0px auto;
            text-align: center;
        }
        h2 { font-size: 3.0rem; }
        p { font-size: 3.0rem; }
        .units { font-size: 1.2rem; }
        .ds-labels {
            font-size: 1.5rem;
            vertical-align: middle;
            padding-bottom: 15px;
        }
    </style>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>ESP Input Form</title>
    <script>
        function submitMessage() {
            alert("Guardamos los datos en la placa Nodemcu por  ESP SPIFFS");
            setTimeout(function () { document.location.reload(false); }, 500);
        }
    </script>
</head>

<body>
    <h2>Ejemplo de Network Time Protocol (NTP)</h2>
    <h3>Adem&aacute;s se muestra estado de salida en base a variables </h3>
    <p>
        <i class="fas fa-thermometer-half" style="color:#04463d;"></i>
        <span class="ds-labels">Hoy es</span>
        <span id="fecha">%inputString%</span>
    </p>
    <p>
        <i class="fas fa-thermometer-half" style="color:#059e8a;"></i>
        <span class="ds-labels">Hora</span>
        <span id="hora">%inputString2%</span>
    </p>
    <p>Estado del Termo<span id="temperaturec">%inputString3%</span></p>
</body>

<script type="text/javascript">
    function actualizar() { location.reload(true); }
    //FunciÃ³n para actualizar cada 4 segundos(4000 milisegundos)
    setInterval("actualizar()", 4000);
</script>
</html>
)rawliteral";

  void notFound(AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Not found");
  }

  String processor5(const String& var){
  //Serial.println(var);
  if(var == "inputString"){
    String CUCU= weekDays[timeClient.getDay()];

    //Get a time structure
    time_t epochTime = timeClient.getEpochTime();
    struct tm *ptm = gmtime ((time_t *)&epochTime); 

    int monthDay = ptm->tm_mday;
    int currentYear = ptm->tm_year+1900;
    int currentMonth = ptm->tm_mon+1;
    String currentMonthName = months[currentMonth-1];
    Serial.print("Month name: ");

    CUCU = CUCU + " "  +  monthDay +  " de " + currentMonthName +  " de " + currentYear  ;

    return CUCU;
   
  }
  else if(var == "inputString2"){
  String  hora = timeClient.getFormattedTime();
    return hora;
  }
  time_t epochTime = timeClient.getEpochTime();
  struct tm *ptm = gmtime ((time_t *)&epochTime);
  int currentYear = ptm->tm_year+1900;
  Serial.print(currentYear);
  if (var == "inputString3" and currentYear==2023){
    String  fuego = "<img src= https://openlab.bmcc.cuny.edu/jasmine-bush-portfolio/wp-content/uploads/sites/302/2020/02/flame-1.gif width= 250 height=166  >";
    return fuego;
  }
  
  return String();
}



void setup() {
  // Initialize Serial Monitor
  Serial.begin(9600);
  
  // Connect to Wi-Fi
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }



    Serial.println();
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
   delay(1000);

 // Send web page with input fields to client
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor5);
  });
   server.onNotFound(notFound);
  server.begin();
 

// Initialize a NTPClient to get time
  timeClient.begin();
  // Set offset time in seconds to adjust for your timezone, for example:
  // GMT +1 = 3600
  // GMT +8 = 28800
  // GMT -1 = -3600
  // GMT 0 = 0
  // timeClient.setTimeOffset(-18000);
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
  String CUCU= weekDays[timeClient.getDay()];
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
  Serial.print("Month name: ");
  Serial.println(currentMonthName);

  int currentYear = ptm->tm_year+1900;
  Serial.print("Year: ");
  Serial.println(currentYear);

  //Print complete date:
  String currentDate = String(currentYear) + "-" + String(currentMonth) + "-" + String(monthDay);
  Serial.print("Current date: ");
  Serial.println(currentDate);
  Serial.println("");
  delay(60000);
  
    
}