#include "Arduino.h"
#include "ESP8266WiFi.h"
#include "ESP8266WiFiAP.h"
#include "ESPAsyncWebServer.h"
#include "LittleFS.h"

#define MOTOR_R1 D5
#define MOTOR_R2 D6
#define MOTOR_L1 D7
#define MOTOR_L2 D8

const char *ssid = "robo123456";     // Enter SSID here
const char *password = "robo123456"; // Enter Password here

IPAddress local_ip(10, 0, 1, 1);
IPAddress gateway(10, 0, 1, 1);
IPAddress subnet(255, 255, 255, 0);

AsyncWebServer server(80);

void setMotor(char side, char direction)
{
  uint8_t M1, M2;
  if (side == 'L')
  {
    M1 = MOTOR_L1;
    M2 = MOTOR_L2;
  }
  else if (side == 'R')
  {
    M1 = MOTOR_R1;
    M2 = MOTOR_R2;
  }

  switch (direction)
  {
  case 'H':
    digitalWrite(M1, LOW);
    digitalWrite(M2, LOW);
    break;

  case 'F':
    digitalWrite(M1, LOW);
    digitalWrite(M2, HIGH);
    break;

  case 'R':
    digitalWrite(M1, HIGH);
    digitalWrite(M2, LOW);
    break;
  }
}

void setup()
{
  Serial.begin(9600);
  Serial.println("Hello");

  pinMode(LED_BUILTIN, OUTPUT);

  // put your setup code here, to run once:
  pinMode(MOTOR_R1, OUTPUT);
  pinMode(MOTOR_R2, OUTPUT);
  pinMode(MOTOR_L1, OUTPUT);
  pinMode(MOTOR_L2, OUTPUT);

  setMotor('L', 'H');
  setMotor('R', 'H');

  if (!LittleFS.begin())
  {
    Serial.println("An Error has occurred while mounting LittleFS");
    return;
  }

  WiFi.softAPConfig(local_ip, gateway, subnet);
  WiFi.softAP(ssid, password);

  server.on(
      "/halt",
      HTTP_GET,
      [](AsyncWebServerRequest *request)
      {
        request->send(200);
        setMotor('L', 'H');
        setMotor('R', 'H');
      },
      NULL, NULL);
  server.on(
      "/forward",
      HTTP_GET,
      [](AsyncWebServerRequest *request)
      {
        request->send(200);
        setMotor('L', 'F');
        setMotor('R', 'F');
      },
      NULL, NULL);
  server.on(
      "/reverse",
      HTTP_GET,
      [](AsyncWebServerRequest *request)
      {
        request->send(200);
        setMotor('L', 'R');
        setMotor('R', 'R');
      },
      NULL, NULL);

  server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");

  server.begin();

  digitalWrite(LED_BUILTIN, LOW);
}

void loop()
{
  // put your main code here, to run repeatedly:
}