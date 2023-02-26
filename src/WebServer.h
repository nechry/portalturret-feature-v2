
// Network

#include <ESPAsyncWebServer.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>
#include <ESPAsyncWiFiManager.h>
#include <ESP8266mDNS.h> // Include the mDNS library

// #include <ESP8266httpUpdate.h>

// Storage
#include "LittleFS.h"

AsyncWebServer server = AsyncWebServer(80);
DNSServer dns;
AsyncWiFiManager wifiManager(&server, &dns);

bool diagnoseMode = false;
int diagnoseAction = -1;
int currentMoveSpeed = 0;

void requestReboot()
{
  debugln("Rebooting...");
  while (true)
  {
    int i = 0;
  }
}

String processor(const String &var)
{
  if (var == "IP")
    return WiFi.localIP().toString();
  return String();
}

void startWebServer()
{
  server.serveStatic("/", LittleFS, "/");
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(LittleFS, "index.html", String(), false, processor); });

  server.onNotFound([](AsyncWebServerRequest *request)
                    {
    debug("WebServer Not Found request ");
    debugln(request->url());

    if (request->method() == HTTP_OPTIONS) {
      request->send(200);
    } else {
      request->send(404);
    } });

  server.on("/setup", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    debugln("Do Setup");
    request->send(LittleFS, "/setup.html", String(), false, processor); });

  server.on("/setup", HTTP_POST, [](AsyncWebServerRequest *request)
            {
              debugln("Setup WiFi");
              request->send(200, "text/html", "Set Portal Turret as AP"); 
              delay(1000);
              wifiManager.startConfigPortal("Portal Turret"); });

  server.on("/set_mode", HTTP_POST, [](AsyncWebServerRequest *request)
            {
    if (request->hasParam("mode", true))
    {      
      AsyncWebParameter *modeParam = request->getParam("mode", true);
      currentTurretMode = (TurretMode) modeParam->value().toInt();
      currentRotateAngle = CENTER_ANGLE;
      debug("Set TurretMode ");
      switch (currentTurretMode)
      {
      case TurretMode::Automatic:
        debugln("Automatic");
        break;
      case TurretMode::Manual:
        debugln("Manual");
        break;      
      default:
        debugln("Unknown");
        break;
      }      
      request->send(200, "text/html", "TurretMode set to "+ (currentTurretMode==TurretMode::Automatic)?"Automatic":"Manual" );
    } else {
      request->send(200, "text/html", "Failed to set mode");
    } });

  server.on("/set_state", HTTP_POST, [](AsyncWebServerRequest *request)
            {
    if (request->hasParam("state", true))
    {
      AsyncWebParameter *stateParam = request->getParam("state", true);
      int state = stateParam->value().toInt();
      debug("Set TurretState ");
      debugln(state);
      setState((TurretState) state);
      request->send(200, "text/html", "State set " ); //+ currentState
    }
    else
    {
      request->send(200, "text/html", "No state sent");
    } });

  server.on("/diagnose", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    diagnoseMode = true;
    debugln("Diagnose page");
    request->send(LittleFS, "/diagnose.html", String(), false, processor); });

  server.on("/diagnose", HTTP_POST, [](AsyncWebServerRequest *request)
            {
    if (request->hasParam("action", true))
    {
      AsyncWebParameter *stateParam = request->getParam("action", true);
      diagnoseAction = stateParam->value().toInt();
      request->send(200, "text/html", "Diagnose");
    }
    else
    {
      request->send(200, "text/html", "No Action Sent");
    } });

  server.on("/set_open", HTTP_POST, [](AsyncWebServerRequest *request)
            {
    if (currentTurretMode == TurretMode::Manual) {
      if (request->hasParam("open", true))
      {
        AsyncWebParameter *openParam = request->getParam("open", true);
        if (openParam->value().toInt() == 1) {
            debugln("Set ManualState Opening");
          setManualState(ManualState::Opening);
          request->send(200, "text/html", "Opening");
        } else {
            debugln("Set ManualState Closing");
          setManualState(ManualState::Closing);
          request->send(200, "text/html", "Closing");
        }
      } else {
        debugln("No open sent");
        request->send(200, "text/html", "No open parameter sent");
      }
    } else {
      debugln("Not in Manual mode");
      request->send(200, "text/html", "Not in Manual mode");
    } });

  server.on("/set_angle", HTTP_POST, [](AsyncWebServerRequest *request)
            {
    //Serial.println("Set angle request");
    if (request->hasParam("angle", true))
    {
      AsyncWebParameter *angleParam = request->getParam("angle", true);
      AsyncWebParameter *servoParam = request->getParam("servo", true);
      int angle = angleParam->value().toInt();
      int servo = servoParam->value().toInt();
      currentMoveSpeed = angle;
      if (servo == 0) {
        debug("Set wing angle");
        debugln(angle);
        wingServo.write(STATIONARY_ANGLE + angle);
      } else {
        debug("Set rotate angle");
        debugln(angle);
        rotateServo.write(CENTER_ANGLE + angle);
      }
      request->send(200, "text/html", "Angle set");
    }
    else
    {
      debugln("No angle sent");
      request->send(200, "text/html", "No angle sent");
    } });

  server.on("/reset", HTTP_POST, [](AsyncWebServerRequest *request)
            {    
    request->send(200, "text/html", "Reset the turret"); 
    debugln("Reset the turret");
    delay(1000);
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(2000); });
  // DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
  server.begin();
}
