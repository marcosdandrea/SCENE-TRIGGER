#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiUdp.h>

#define PIR D2
#define LED_CON D5
#define LED_ACT D6

HTTPClient http;
WiFiClient client;
WiFiUDP Udp;

// variables de configuracion
String triggerID = "s01_00";
const char *ssid = "museoctrol";
const char *password = "Pesp1102";
String url = "http://192.168.10.24/SceneTriggerConfig/config.api";
unsigned long timeBetweenRequests = 20000; // tiempo en milisegundos para consultar configuracion al servidor.
unsigned long retrigger = 60000;           // valor por defecto de trigger entre disparos en milisegundos.
unsigned long triggerDelay = 60000;           // valor por defecto de trigger demorado en milisegundos.

unsigned int remotePort = 5000;
int defaulValue = 1000;

// Variables de trabajo (No tocar)
String stringCommand = "trigger_" + triggerID;
unsigned long lastRequestToServer = 10000;
unsigned long delayToTrigger = 10000;
String payload = "";
unsigned int localPort = 8888;
IPAddress broadcast(192, 168, 10, 255);
float global_i = 0.0f;
unsigned long lastIteration = 0;
unsigned long lastTrigger = 0;
bool triggerDelayed = false;

// hoising
void serverRequest();
void parseData(String data);
void detectionPIR();
void fadeLED(int LED);
void sendUdp(String msg);

// inicio
void setup()
{
  Serial.begin(115200);
  delay(10);

  pinMode(PIR, INPUT);
  pinMode(LED_CON, OUTPUT);
  pinMode(LED_ACT, OUTPUT);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    fadeLED(LED_CON);
  }
  digitalWrite(LED_CON, 1);
  Serial.print("Connected! IP address: ");
  Serial.println(WiFi.localIP());
  Serial.printf("UDP server on port %d\n", localPort);
  Udp.begin(localPort);
}

void loop()
{
  if (lastRequestToServer < millis())
    serverRequest();
  if (payload.length() > 0)
    retrigger = parseData(payload, "retrigger");
    triggerDelay = parseData(payload, "delay");
    payload = "";
  detectionPIR();
  if (delayToTrigger < millis() && triggerDelayed) triggerBrightsignWithDelay ();
  delay(10);
}


void detectionPIR()
{
  int _PIR = digitalRead(PIR);
  if (_PIR && lastTrigger < millis())
  {    
    triggerDelayed = true;  
    delayToTrigger = millis() + triggerDelay;  
  }
  else
  {
    digitalWrite(LED_ACT, 0);
  }
}

void triggerBrightsignWithDelay (){
  sendUdp(stringCommand);
  lastTrigger = millis() + retrigger;
  triggerDelayed = false;
  digitalWrite(LED_ACT, 1);
}

void sendUdp(String msg)
{
  Udp.beginPacket(broadcast, remotePort);
  Udp.print(msg);
  Udp.endPacket();

  digitalWrite(LED_CON, 0);
  delay(100);
  digitalWrite(LED_CON, 1);
}

unsigned long parseData(String data, String commandToParse)
{
  Serial.println("Trying to parse data from server...");
  int start = data.indexOf("<" + triggerID + ">");
  int _start = start + triggerID.length() + 2;
  int end = data.indexOf("<" + triggerID + "/>", _start) - 1;
  String extractedData = data.substring(_start, end);
  if (start == -1)
    return;

  start = extractedData.indexOf(commandToParse+":") + commandToParse.length() + 1;
  end = extractedData.indexOf(";", start);
  int commnadValue = extractedData.substring(start, end).toInt();

  if (start - 10 == -1)
    return defaulValue;
  
  Serial.println (commandToParse + ": " + commnadValue);
  return commnadValue;
}

void serverRequest()
{
  lastRequestToServer = millis() + timeBetweenRequests;
  if (http.begin(client, url)) // Iniciar conexión
  {
    int httpCode = http.GET(); // Realizar petición

    if (httpCode > 0)
    {
      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY)
      {
        payload = http.getString(); // Obtener respuesta
      }
    }
    else
    {
  }
    http.end();
  }
}

void fadeLED(int LED)
{
  if (lastIteration < millis())
  {
    float sin_result = ((sin(global_i) + 1) / 2) + 0.01f;

    int toPrint = 0;
    if (int(sin_result * 255) > 255)
    {
      toPrint = 255;
    }
    else
    {
      toPrint = int(sin_result * 255);
    }
    analogWrite(LED, toPrint);
    global_i += 0.05;
    lastIteration = millis() + 5;
  }
}