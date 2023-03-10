#include "Arduino.h"
#include "ESP8266WiFi.h"
#include "ESP8266WiFiAP.h"
#include "ESPAsyncWebServer.h"
#include "LittleFS.h"

#define MOTOR_R1 D5
#define MOTOR_R2 D6
#define MOTOR_L1 D7
#define MOTOR_L2 D8

#define NOCHANGE 1000

const char *ssid = "robofc0004";     // Enter SSID here
const char *password = "robofc0004"; // Enter Password here

IPAddress local_ip(10, 0, 1, 1);
IPAddress gateway(10, 0, 1, 1);
IPAddress subnet(255, 255, 255, 0);

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

void setMotor(char side, int pwm)
{
  if (pwm == NOCHANGE)
    return;

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
  else
  {
    return;
  }

  if (pwm > 0)
  {
    analogWrite(M1, 0);
    analogWrite(M2, pwm);
  }
  else
  {
    analogWrite(M1, -pwm);
    analogWrite(M2, 0);
  }
}
void handleWebSocketMessage(void *arg, uint8_t *data, size_t len)
{
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len) {
    data[len] = 0;
    uint8_t motor_sidetype = data[0];
    uint8_t pwm = data[1];

    bool isRightMotor = (motor_sidetype & 0b01) > 0;
    bool isReversed = (motor_sidetype & 0b10) > 0;

    Serial.print(isRightMotor ? "R" : "L");
    Serial.print(isReversed ? "-" : "+");
    Serial.println(pwm);

    setMotor(isRightMotor ? 'R' : 'L', pwm * (isReversed ? -1: 1));
  }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len)
{
  switch (type)
  {
  case WS_EVT_CONNECT:
    Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
    break;
  case WS_EVT_DISCONNECT:
    Serial.printf("WebSocket client #%u disconnected\n", client->id());
    break;
  case WS_EVT_DATA:
    handleWebSocketMessage(arg, data, len);
    break;
  case WS_EVT_PONG:
  case WS_EVT_ERROR:
    break;
  }
}

void initWebSocket() {
  ws.onEvent(onEvent);
  server.addHandler(&ws);
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

  setMotor('L', 0);
  setMotor('R', 0);

  if (!LittleFS.begin())
  {
    Serial.println("An Error has occurred while mounting LittleFS");
    return;
  }

  WiFi.softAPConfig(local_ip, gateway, subnet);
  WiFi.softAP(ssid, password);

  initWebSocket();

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
        setMotor('L', 255);
        setMotor('R', 255);
      },
      NULL, NULL);
  server.on(
      "/reverse",
      HTTP_GET,
      [](AsyncWebServerRequest *request)
      {
        request->send(200);
        setMotor('L', -255);
        setMotor('R', -255);
      },
      NULL, NULL);

  server.on(
      "/set",
      HTTP_GET,
      [](AsyncWebServerRequest *request)
      {
        request->send(200);

        char motor = 'X';
        int pwm = NOCHANGE;
        size_t paramCount = request->params();
        for (size_t p = 0; p < paramCount; p++)
        {
          AsyncWebParameter *param = request->getParam(p);
          if (param->name() == "motor")
          {
            motor = param->value()[0];
          }
          else if (param->name() == "pwm")
          {
            pwm = param->value().toInt();
          }
        }
        setMotor(motor, pwm);
      },
      NULL, NULL);

  server.serveStatic("/", LittleFS, "/")
      .setDefaultFile("index.html");

  server.begin();

  digitalWrite(LED_BUILTIN, LOW);
}

void loop()
{
  ws.cleanupClients();
}