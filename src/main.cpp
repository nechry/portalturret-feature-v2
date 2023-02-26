// General
#include "Arduino.h"
#include <string.h>
#include "portalturret.h"

// Devices
#include <FastLED.h>
#include <Servo.h>
#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"

#define STRINGIFY(s) STRINGIFY1(s)
#define STRINGIFY1(s) #s

#include "config.h"



#define BAUD monitor_speed

#ifdef HOSTNAME
const char *hostname = STRINGIFY(HOSTNAME);
#else
const char *hostname = "turret";
#endif

#define BUSY D0
#define CENTER_LED D3
#define GUN_LEDS D4
#define RING_LEDS D8
#define SERVO_A D6
#define SERVO_B D7
#define WING_SWITCH D5
#define PID A0


#define NUM_LEDS 8
CRGB leds[NUM_LEDS];

Servo wingServo;
Servo rotateServo;

#ifdef USE_AUDIO
SoftwareSerial mySoftwareSerial(RX, TX); // RX, TX
DFRobotDFPlayerMini myDFPlayer;

#define FREQ 50          // one clock is 20 ms
#define FREQ_MINIMUM 205 // 1ms is 1/20, of 4096
#define FREQ_MAXIMUM 410 // 2ms is 2/20, of 4096

#ifndef VOLUME
#define VOLUME 10 ////Set volume value. From 0 to 30
#endif       
#endif

bool websocketStarted;
unsigned long nextWebSocketUpdateTime = 0;

bool wingsOpen;
bool wasOpen;
bool fullyOpened;
bool alarm;
bool needsSetup;
bool myDFPlayerSetup = false;
bool shouldUpdate = false;
bool isConnected;

// For some reason we need to cache this value, as checking it every loop causes the webserver to freeze.
// https://github.com/me-no-dev/ESPAsyncWebServer/issues/944
bool isDetectingMotionCached = false;
unsigned long lastMotionCheck = 0;

#include "serialDebug.h"
#include "Accelerometer.h"
#include "Routines.h"
#include "StateBehaviour.h"
#include "WebSocket.h"
#include "WebServer.h"
#include "OTA.h"

#ifdef USE_MQTT
#include "mqtt.h"

#endif
#include "diagnose.h"

bool isOpen()
{
  return digitalRead(WING_SWITCH) == HIGH;
}

bool isDetectingMotion()
{
  unsigned long curMillis = millis();
  if (curMillis > lastMotionCheck + 50)
  {
    isDetectingMotionCached = analogRead(PID) > MOTION_SENSITIVITY;
    lastMotionCheck = curMillis;
  }
  return isDetectingMotionCached;
}

bool isPlayingAudio()
{
  #ifdef USE_AUDIO
  return digitalRead(BUSY) == LOW;
  #else
  return false;
  #endif
}

void UpdateLEDPreloader()
{
  int t = floor(millis() / 10);
  for (int dot = 0; dot < NUM_LEDS; dot++)
  {
    leds[dot] = CRGB(0,(dot + t) % 8 == 0 ? 255 : 0, 0);
    FastLED.show();
    leds[dot] = CRGB::Black;
    delay(10);
  }
}



// void preloader(uint8_t led)
// {
//   FastLED.clear();
//   leds[led] = CRGB(0, 255, 0);
//   FastLED.show();
// }

void configModeCallback(AsyncWiFiManager *myWiFiManager)
{
  // FastLED's built-in rainbow generator
  fill_rainbow( leds, NUM_LEDS, 0, 7);

  debugln("Entered config mode");
  debugln(WiFi.softAPIP().toString());
  debugln(myWiFiManager->getConfigPortalSSID());
}


void setup()
{
  Serial.begin(BAUD);
#ifdef USE_AUDIO
  pinMode(BUSY, INPUT);
#endif

  pinMode(WING_SWITCH, INPUT_PULLUP);
  pinMode(GUN_LEDS, OUTPUT);
  pinMode(CENTER_LED, OUTPUT);

  // blink guns leds
  performDiagnose(4);
  // center led fade in / out
  performDiagnose(7);

  Serial.println("Initializing LittleFS...");
  if (!LittleFS.begin())
  {
    Serial.println("LittleFS Initialization failed!");
    Serial.println("Stop!");
    while (true)
    {
      digitalWrite(GUN_LEDS, HIGH);
      delay(2000);
      digitalWrite(GUN_LEDS, LOW);
      delay(500);
    }
    return;
  }
  Serial.println("LittleFS Initialized");

  Serial.println("Initializing FastLED...");
  FastLED.addLeds<WS2812, RING_LEDS, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(RING_BRIGHTNESS);
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB(0, 0, 0);
    FastLED.show();
  }
  fill_rainbow(leds, NUM_LEDS, 0, 7);
  FastLED.show();
  delay(500);
  FastLED.clear();
  cylon();  
  UpdateLEDPreloader();
  Serial.println("FastLED Initialized");

  wingServo.attach(SERVO_A);
  rotateServo.attach(SERVO_B);
  rotateServo.write(CENTER_ANGLE);

  delay(250);
  UpdateLEDPreloader();
  // assume wings aren't fully open
  fullyOpened = false;

#ifdef CLOSE_WINGS_ON_STARTUP
  // first check if wings are open, if so close them
  if (isOpen())
  {
    Serial.println("wings are somehow open, closing them");
    wingServo.write(STATIONARY_ANGLE + 90);
    // wait for wings are closed
    while (isOpen())
    {
      delay(10);
    }
    // wait for a bit
    delay(CLOSE_STOP_DELAY);
    // set wings to stationary
    wingServo.write(STATIONARY_ANGLE);
  }

#endif

  UpdateLEDPreloader();
  Serial.println("trying to connect to a known wifi...");
  wifiManager.setDebugOutput(false);
  wifiManager.setAPCallback(configModeCallback);
  isConnected = wifiManager.autoConnect(hostname);
  if (!isConnected)
  {
    Serial.println("failed to connect and hit timeout");
    blinkGUN_LEDS(10);
    delay(1000);
    // reset and try again, or maybe put it to deep sleep
    Serial.println("Reset and try again");
    ESP.reset();
    delay(5000);
  }
  else
  {
    Serial.print("Connected, with IP address: ");
    Serial.println(WiFi.localIP());
  }

  UpdateLEDPreloader();

  currentState = TurretState::Idle;
  currentManualState = ManualState::Idle;
  currentTurretMode = TurretMode::Automatic;

  wasOpen = isOpen();

  AsyncElegantOTA.begin(&server);

  UpdateLEDPreloader();

  Serial.println("Starting webserver...");
  startWebServer();
  Serial.println("Webserver started");
  UpdateLEDPreloader();

  Serial.println("Starting webSocket...");
  startWebSocket();
  Serial.println("Websocket started");
  UpdateLEDPreloader();

  Serial.println("Starting accelerometer...");
  setupAccelerometer();
  Serial.println("Accelerometer started");
  UpdateLEDPreloader();

  Serial.println("Begin MDNS...");
  if (MDNS.begin(hostname, WiFi.localIP()))
  {
    Serial.println("MDSN setup");
    MDNS.addService("http", "tcp", 80);
    MDNS.addService("http", "tcp", 81);
  }
  else
  {
    Serial.println("MDSN failed");
  }
  Serial.println("MDNS completed");
  delay(200);

  UpdateLEDPreloader();
  checkForUpdates();

#ifdef USE_AUDIO
  Serial.end();
  delay(100);
  mySoftwareSerial.begin(9600);
  delay(200);
  myDFPlayer.begin(mySoftwareSerial, true, true);
  delay(100);
  myDFPlayer.volume(VOLUME);

#else
  Serial.println("Audio disabled, serial enabled...");
#endif

#ifdef USE_MQTT
  debugln("Starting MQTT...");
  setupMqtt();
  debugln("MQTT started");
#endif

  for (int i = 0; i < NUM_LEDS; i++)
  {
    leds[i] = CRGB(0, 0, 28);
    FastLED.show();
    delay(100);
  }

  previousTime = millis();
  debugln("End setup");
}

void loop()
{
  MDNS.update();

  wingsOpen = isOpen();

  ArduinoOTA.handle();
  webSocket.loop();
  if (!diagnoseMode)
  {
    stateBehaviour();
  }
  else
  {
    performDiagnose(diagnoseAction);
    diagnoseAction = -1;
  }

  if (currentMoveSpeed > 0 && wasOpen && !wingsOpen)
  {
    debugln("Test Wings are closed, stopping");
    currentMoveSpeed = 0;
    delay(CLOSE_STOP_DELAY);
    wingServo.write(STATIONARY_ANGLE);
  }

  wasOpen = wingsOpen;

  if (websocketStarted && millis() > nextWebSocketUpdateTime)
  {
    nextWebSocketUpdateTime = millis() + 30;
    int a = analogRead(PID);
    updateAccelerometer();
    int16_t x = smoothX / measurements;
    int16_t y = smoothY / measurements;
    int16_t z = smoothZ / measurements;

    const uint8_t values[] = {
        (uint8_t)(x >> 8),
        (uint8_t)(x & 0xFF),
        (uint8_t)(y >> 8),
        (uint8_t)(y & 0xFF),
        (uint8_t)(z >> 8),
        (uint8_t)(z & 0xFF),
        (uint8_t)(!isOpen() ? 1 : 0),
        (uint8_t)(isDetectingMotion() ? 1 : 0),
        (uint8_t)((a >> 8) & 0xFF),
        (uint8_t)(a & 0xFF),
        (uint8_t)currentState,
        (uint8_t)(isPlayingAudio() ? 1 : 0),
        (uint8_t)(myDFPlayer.readVolume())};

    webSocket.broadcastBIN(values, 13);
  }
  #ifdef USE_MQTT
  updateMQTTstatus();
  #endif
}
