#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#ifndef STASSID
#define STASSID "museoctrol"
#define STAPSK "Pesp1102"
#endif

#define PIR D2
#define LED_CON D5
#define LED_ACT D6

unsigned int localPort = 8888;

IPAddress broadcast(192, 168, 10, 255);
String triggerID = "s01_00";
String stringCommand = "trigger_" + triggerID;
unsigned int remotePort = 5000;
unsigned long timeBetweenPackets = 15000; //tiempo en milisgundos entre paquetes

unsigned long lastPacket = 0;

WiFiUDP Udp;
void sendUdp(String msg);
void fadeLED(int LED);

void setup()
{
  Serial.begin(115200);
  pinMode(PIR, INPUT);
  pinMode(LED_CON, OUTPUT);
  pinMode(LED_ACT, OUTPUT);

  WiFi.mode(WIFI_STA);
  WiFi.begin(STASSID, STAPSK);
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
  delay(5);
  int _PIR = digitalRead(PIR);
  if (_PIR)
  {
    Serial.println("PIR DETECTION");
    digitalWrite(LED_ACT, 1);
    sendUdp(stringCommand);
  }
  else
  {
    digitalWrite(LED_ACT, 0);
  }
}

void sendUdp(String msg)
{
  if (lastPacket > millis())
    return;
  // send a reply, to the IP address and port that sent us the packet we received
  Udp.beginPacket(broadcast, remotePort);
  Udp.print(msg);
  Udp.endPacket();
  lastPacket = millis() + timeBetweenPackets;
  digitalWrite(LED_CON, 0);
  delay(100);
  digitalWrite(LED_CON, 1);
}


float global_i = 0.0f;
unsigned long lastIteration = 0;
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
    Serial.println(toPrint);
    analogWrite(LED, toPrint);
    global_i += 0.05;
    lastIteration = millis() + 5;
  }
}