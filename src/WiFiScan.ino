/*
  ____          _____               _ _           _       
 |  _ \        |  __ \             (_) |         | |      
 | |_) |_   _  | |__) |_ _ _ __ _____| |__  _   _| |_ ___ 
 |  _ <| | | | |  ___/ _` | '__|_  / | '_ \| | | | __/ _ \
 | |_) | |_| | | |  | (_| | |   / /| | |_) | |_| | ||  __/
 |____/ \__, | |_|   \__,_|_|  /___|_|_.__/ \__, |\__\___|
         __/ |                               __/ |        
        |___/                               |___/         
    
____________________________________
/ Si necesitas ayuda, contáctame en \
\ https://parzibyte.me               /
 ------------------------------------
        \   ^__^
         \  (oo)\_______
            (__)\       )\/\
                ||----w |
                ||     ||
Creado por Parzibyte (https://parzibyte.me).
------------------------------------------------------------------------------------------------
Si el código es útil para ti, puedes agradecerme siguiéndome: https://parzibyte.me/blog/sigueme/
Y compartiendo mi blog con tus amigos
También tengo canal de YouTube: https://www.youtube.com/channel/UCroP4BTWjfM0CkGB6AFUoBg?sub_confirmation=1
------------------------------------------------------------------------------------------------
*/
#include <ESP8266WiFiMulti.h>
#include <ESP8266WebServer.h>
#include <Adafruit_Sensor.h>
#include "DHT.h"
#include <DHT_U.h>
#include <Arduino.h>

#define PIN_CONEXION_DHT D2
#define TIPO_SENSOR DHT11
#define LED_DE_ESTADO 2

ESP8266WiFiMulti wifiMulti;
ESP8266WebServer servidor(80);
DHT sensor(PIN_CONEXION_DHT, TIPO_SENSOR);

int ultimaVezLeido = 0;
long intervaloLectura = 5000; // Debería ser mayor que 2000
unsigned long ultimaLecturaExitosa = 0;

float humedad, temperatura = 0;

// Prototipos de funciones
void rutaRaiz();
void rutaNoEncontrada();
void rutaJson();
void indicarErrorDht();
void indicarExitoDht();

void setup(void)
{
  pinMode(LED_DE_ESTADO, OUTPUT);
  digitalWrite(LED_DE_ESTADO, LOW);

  sensor.begin();
  // Aquí puedes agregar varias redes. La tarjeta se conectará a la más cercana
  wifiMulti.addAP("IngenicoLAB", "Lab@ingenico1");
  wifiMulti.addAP("Krloz Medina", "F@mili@571112");
  // wifiMulti.addAP("Otra red", "Contraseña");

  // Esperar conexión WiFi
  while (wifiMulti.run() != WL_CONNECTED)
  {
    delay(250);
  }

  //  Configurar rutas
  servidor.on("/", rutaRaiz);
  servidor.on("/api", rutaJson);
  servidor.onNotFound(rutaNoEncontrada);

  // Iniciar servidor
  servidor.begin();
  digitalWrite(LED_DE_ESTADO, HIGH);
}

void loop(void)
{
  // Si el intervalo se ha alcanzado, leer la temperatura
  if (ultimaVezLeido > intervaloLectura)
  {
    float nuevaHumedad = sensor.readHumidity();
    float nuevaTemperatura = sensor.readTemperature();
    // Si los datos son correctos, actualizamos las globales
    if (isnan(nuevaTemperatura) || isnan(nuevaHumedad))
    {
      indicarErrorDht();
      ultimaVezLeido = 0;
      return;
    }
    ultimaLecturaExitosa = millis();
    humedad = nuevaHumedad;
    temperatura = nuevaTemperatura;
    ultimaVezLeido = 0;
    indicarExitoDht();
  }
  delay(1);
  ultimaVezLeido += 1;
  // Responder las solicitudes entrantes en caso de que haya
  servidor.handleClient();
}

// Servir toda la página web, desde la misma se consultará a la API que está aquí mismo
void rutaRaiz()
{
  servidor.send(200, "text/html", "");
}

// Manejador de 404
void rutaNoEncontrada()
{
  servidor.send(404, "text/plain", "No encontrado");
}

// Nuestra pequeña API
void rutaJson()
{
  // Calcular última lectura exitosa en segundos
  unsigned long tiempoTranscurridoEnMilisegundos = millis() - ultimaLecturaExitosa;
  int tiempoTranscurrido = tiempoTranscurridoEnMilisegundos / 1000;
  // Búfer para escribir datos en JSON
  char bufer[50];
  // Crear la respuesta pasando las variables globales
  // La salida será algo como:
  // {"t":14.20,"h":79.20,"l":5.00}
  sprintf(bufer, "{\"t\":%.2f,\"h\":%.2f,\"u\":%d}", temperatura, humedad, tiempoTranscurrido);
  // Responder con ese JSON
  servidor.send(200, "application/json", bufer);
}

/*
 Patrones para parpadear LED
*/
void indicarErrorDht()
{
  int x = 0;
  for (x = 0; x < 5; x++)
  {
    digitalWrite(LED_DE_ESTADO, LOW);
    delay(50);
    digitalWrite(LED_DE_ESTADO, HIGH);
    delay(50);
  }
}

void indicarExitoDht()
{
  digitalWrite(LED_DE_ESTADO, LOW);
  delay(50);
  digitalWrite(LED_DE_ESTADO, HIGH);
}