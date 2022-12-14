#include <Arduino.h>
#include <ArduinoOTA.h>
#include <FS.h>
#include <SPIFFS.h>
#include <ESPmDNS.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFSEditor.h>

#include <Adafruit_MCP23X17.h>

#include "simona.h"

// SKETCH BEGIN
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
AsyncEventSource events("/events");

TaskHandle_t Task1;
TaskHandle_t Task2;

Adafruit_MCP23X17 mcp_a;
Adafruit_MCP23X17 mcp_b;
Adafruit_MCP23X17 mcp_c;
Adafruit_MCP23X17 mcp_d;

void Task1code(void *parameter)
{
  Serial.print("Task1 is running on core ");
  Serial.println(xPortGetCoreID());
  for (;;)
  {

    uint8_t i;

    Serial.println("mcp_a");
    
    for (i = 0; i <= 15; ++i)
    {
      mcp_a.digitalWrite(i, HIGH);
      mcp_a.writeGPIOAB(0b0000000000000001);
      delay(100);
      mcp_a.digitalWrite(i, LOW);
      delay(100);
    }
    
    /*
        for (i = 0; i <= 1; ++i)
        {
          mcp_a.writeGPIOAB(0b0000000000000001);
          delay(100);
          mcp_a.writeGPIOAB(0b0000000000000000);
          delay(100);

          mcp_a.writeGPIOAB(0b0000000000000010);
          delay(100);
          mcp_a.writeGPIOAB(0b0000000000000000);
          delay(100);

          mcp_a.writeGPIOAB(0b0000000000000100);
          delay(100);
          mcp_a.writeGPIOAB(0b0000000000000000);
          delay(100);

          mcp_a.writeGPIOAB(0b0000000000001000);
          delay(100);
          mcp_a.writeGPIOAB(0b0000000000000000);
          delay(100);

          mcp_a.writeGPIOAB(0b0000000000010000);
          delay(100);
          mcp_a.writeGPIOAB(0b0000000000000000);
          delay(100);

          mcp_a.writeGPIOAB(0b0000000000100000);
          delay(100);
          mcp_a.writeGPIOAB(0b0000000000000000);
          delay(100);
        }
    */

    Serial.println("mcp_b");
    for (i = 0; i <= 15; ++i)
    {
      mcp_b.digitalWrite(i, HIGH);
      delay(100);
      mcp_b.digitalWrite(i, LOW);
      delay(100);
    }

    Serial.println("mcp_c");
    for (i = 0; i <= 15; ++i)
    {
      mcp_c.digitalWrite(i, HIGH);
      delay(100);
      mcp_c.digitalWrite(i, LOW);
      delay(100);
    }

    Serial.println("mcp_d");
    for (i = 0; i <= 15; ++i)
    {
      mcp_d.digitalWrite(i, HIGH);
      delay(100);
      mcp_d.digitalWrite(i, LOW);
      delay(100);
    }
  }
}

void Task2code(void *parameter)
{
  Serial.print("Task2 is running on core ");
  Serial.println(xPortGetCoreID());
  for (;;)
  {
    /*
    digitalWrite(led_2, HIGH);
    delay(1000);
    digitalWrite(led_2, LOW);
    delay(1000);
    */
  }
}

void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
{
  if (type == WS_EVT_CONNECT)
  {
    Serial.printf("ws[%s][%u] connect\n", server->url(), client->id());
    client->printf("Hello Client %u :)", client->id());
    client->ping();
  }
  else if (type == WS_EVT_DISCONNECT)
  {
    Serial.printf("ws[%s][%u] disconnect\n", server->url(), client->id());
  }
  else if (type == WS_EVT_ERROR)
  {
    Serial.printf("ws[%s][%u] error(%u): %s\n", server->url(), client->id(), *((uint16_t *)arg), (char *)data);
  }
  else if (type == WS_EVT_PONG)
  {
    Serial.printf("ws[%s][%u] pong[%u]: %s\n", server->url(), client->id(), len, (len) ? (char *)data : "");
  }
  else if (type == WS_EVT_DATA)
  {
    AwsFrameInfo *info = (AwsFrameInfo *)arg;
    String msg = "";
    if (info->final && info->index == 0 && info->len == len)
    {
      // the whole message is in a single frame and we got all of it's data
      Serial.printf("ws[%s][%u] %s-message[%llu]: ", server->url(), client->id(), (info->opcode == WS_TEXT) ? "text" : "binary", info->len);

      if (info->opcode == WS_TEXT)
      {
        for (size_t i = 0; i < info->len; i++)
        {
          msg += (char)data[i];
        }
      }
      else
      {
        char buff[3];
        for (size_t i = 0; i < info->len; i++)
        {
          sprintf(buff, "%02x ", (uint8_t)data[i]);
          msg += buff;
        }
      }
      Serial.printf("%s\n", msg.c_str());

      if (info->opcode == WS_TEXT)
        client->text("I got your text message");
      else
        client->binary("I got your binary message");
    }
    else
    {
      // message is comprised of multiple frames or the frame is split into multiple packets
      if (info->index == 0)
      {
        if (info->num == 0)
          Serial.printf("ws[%s][%u] %s-message start\n", server->url(), client->id(), (info->message_opcode == WS_TEXT) ? "text" : "binary");
        Serial.printf("ws[%s][%u] frame[%u] start[%llu]\n", server->url(), client->id(), info->num, info->len);
      }

      Serial.printf("ws[%s][%u] frame[%u] %s[%llu - %llu]: ", server->url(), client->id(), info->num, (info->message_opcode == WS_TEXT) ? "text" : "binary", info->index, info->index + len);

      if (info->opcode == WS_TEXT)
      {
        for (size_t i = 0; i < len; i++)
        {
          msg += (char)data[i];
        }
      }
      else
      {
        char buff[3];
        for (size_t i = 0; i < len; i++)
        {
          sprintf(buff, "%02x ", (uint8_t)data[i]);
          msg += buff;
        }
      }
      Serial.printf("%s\n", msg.c_str());

      if ((info->index + len) == info->len)
      {
        Serial.printf("ws[%s][%u] frame[%u] end[%llu]\n", server->url(), client->id(), info->num, info->len);
        if (info->final)
        {
          Serial.printf("ws[%s][%u] %s-message end\n", server->url(), client->id(), (info->message_opcode == WS_TEXT) ? "text" : "binary");
          if (info->message_opcode == WS_TEXT)
            client->text("I got your text message");
          else
            client->binary("I got your binary message");
        }
      }
    }
  }
}

const char *ssid = "*******";
const char *password = "*******";
const char *hostName = "esp-async";
const char *http_username = "admin";
const char *http_password = "admin";

void setup()
{
  Serial.begin(921600);
  Serial.print("setup() is running on core ");
  Serial.println(xPortGetCoreID());

  Serial.setDebugOutput(true);
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(hostName);
  WiFi.begin(ssid, password);
  if (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    Serial.printf("STA: Failed!\n");
    WiFi.disconnect(false);
    delay(1000);
    WiFi.begin(ssid, password);
  }

  // Send OTA events to the browser
  ArduinoOTA.onStart([]()
                     { events.send("Update Start", "ota"); });
  ArduinoOTA.onEnd([]()
                   { events.send("Update End", "ota"); });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
                        {
    char p[32];
    sprintf(p, "Progress: %u%%\n", (progress/(total/100)));
    events.send(p, "ota"); });
  ArduinoOTA.onError([](ota_error_t error)
                     {
    if(error == OTA_AUTH_ERROR) events.send("Auth Failed", "ota");
    else if(error == OTA_BEGIN_ERROR) events.send("Begin Failed", "ota");
    else if(error == OTA_CONNECT_ERROR) events.send("Connect Failed", "ota");
    else if(error == OTA_RECEIVE_ERROR) events.send("Recieve Failed", "ota");
    else if(error == OTA_END_ERROR) events.send("End Failed", "ota"); });
  ArduinoOTA.setHostname(hostName);
  ArduinoOTA.begin();

  MDNS.addService("http", "tcp", 80);

  SPIFFS.begin();

  ws.onEvent(onWsEvent);
  server.addHandler(&ws);

  events.onConnect([](AsyncEventSourceClient *client)
                   { client->send("hello!", NULL, millis(), 1000); });
  server.addHandler(&events);

  server.addHandler(new SPIFFSEditor(SPIFFS, http_username, http_password));

  server.on("/heap", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(200, "text/plain", String(ESP.getFreeHeap())); });

  server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.htm");

  server.onNotFound([](AsyncWebServerRequest *request)
                    {
    Serial.printf("NOT_FOUND: ");
    if(request->method() == HTTP_GET)
      Serial.printf("GET");
    else if(request->method() == HTTP_POST)
      Serial.printf("POST");
    else if(request->method() == HTTP_DELETE)
      Serial.printf("DELETE");
    else if(request->method() == HTTP_PUT)
      Serial.printf("PUT");
    else if(request->method() == HTTP_PATCH)
      Serial.printf("PATCH");
    else if(request->method() == HTTP_HEAD)
      Serial.printf("HEAD");
    else if(request->method() == HTTP_OPTIONS)
      Serial.printf("OPTIONS");
    else
      Serial.printf("UNKNOWN");
    Serial.printf(" http://%s%s\n", request->host().c_str(), request->url().c_str());

    if(request->contentLength()){
      Serial.printf("_CONTENT_TYPE: %s\n", request->contentType().c_str());
      Serial.printf("_CONTENT_LENGTH: %u\n", request->contentLength());
    }

    int headers = request->headers();
    int i;
    for(i=0;i<headers;i++){
      AsyncWebHeader* h = request->getHeader(i);
      Serial.printf("_HEADER[%s]: %s\n", h->name().c_str(), h->value().c_str());
    }

    int params = request->params();
    for(i=0;i<params;i++){
      AsyncWebParameter* p = request->getParam(i);
      if(p->isFile()){
        Serial.printf("_FILE[%s]: %s, size: %u\n", p->name().c_str(), p->value().c_str(), p->size());
      } else if(p->isPost()){
        Serial.printf("_POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
      } else {
        Serial.printf("_GET[%s]: %s\n", p->name().c_str(), p->value().c_str());
      }
    }

    request->send(404); });
  server.onFileUpload([](AsyncWebServerRequest *request, const String &filename, size_t index, uint8_t *data, size_t len, bool final)
                      {
    if(!index)
      Serial.printf("UploadStart: %s\n", filename.c_str());
    Serial.printf("%s", (const char*)data);
    if(final)
      Serial.printf("UploadEnd: %s (%u)\n", filename.c_str(), index+len); });
  server.onRequestBody([](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
                       {
    if(!index)
      Serial.printf("BodyStart: %u\n", total);
    Serial.printf("%s", (const char*)data);
    if(index + len == total)
      Serial.printf("BodyEnd: %u\n", total); });
  server.begin();

  Serial.println("MCP23xxx Blink Test!");

  if (!mcp_a.begin_I2C(0x20))
  {
    Serial.println("Error - mcp_a");
    while (1)
      ;
  }
  if (!mcp_b.begin_I2C(0x21))
  {
    Serial.println("Error - mcp_b");
    while (1)
      ;
  }
  if (!mcp_c.begin_I2C(0x22))
  {
    Serial.println("Error - mcp_c");
    while (1)
      ;
  }
  if (!mcp_d.begin_I2C(0x23))
  {
    Serial.println("Error - mcp_d");
    while (1)
      ;
  }

  // configure pin for output
  uint8_t i = 0;
  for (i = 0; i <= 15; ++i)
  {
    mcp_a.pinMode(i, OUTPUT);
    mcp_b.pinMode(i, OUTPUT);
    mcp_c.pinMode(i, OUTPUT);
    mcp_d.pinMode(i, OUTPUT);
  }
  /*
      core1 - main program code
      core0 - esp32 network stack
  */
  xTaskCreatePinnedToCore(Task1code, "Task1", 10000, NULL, 1, &Task1, 0);
  delay(500);
  xTaskCreatePinnedToCore(Task2code, "Task2", 10000, NULL, 1, &Task2, 1);
  delay(500);
}

void loop()
{
  ArduinoOTA.handle();
  ws.cleanupClients();
  // Serial.print("loop() is running on core ");
  // Serial.println(xPortGetCoreID());
}
