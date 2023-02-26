// Tweak these according to servo speed
#define OPEN_DURATION 600
#define CLOSE_STOP_DELAY 50
#define MAX_ROTATION 50

#define CENTER_ANGLE 55
#define STATIONARY_ANGLE 91

#define GFORCE_PICKED_UP_MIN 7.8
#define GFORCE_PICKED_UP_MAX 12
#define GFORCE_STEADY_MIN 9.5
#define GFORCE_STEADY_MAX 10.5
#define TIPPED_OVER_Z_TRESHOLD 5

#define MOTION_SENSITIVITY 1000

#define RING_BRIGHTNESS 84

#ifdef USE_MQTT
const char *const mqtt_server = "";
const int mqtt_server_port = 1883;
const char *const mqtt_user = "";
const char *const mqtt_pass = "";
const char *const mqtt_clientName = "esp_turret";
/*
For understanding when "cmnd", "stat" and "tele" is used, have a look at how Tasmota is doing it.
https://tasmota.github.io/docs/MQTT
https://tasmota.github.io/docs/openHAB/
https://www.openhab.org/addons/bindings/mqtt.generic/
https://www.openhab.org/addons/bindings/mqtt/
https://community.openhab.org/t/itead-sonoff-switches-and-sockets-cheap-esp8266-wifi-mqtt-hardware/15024
for debugging:
mosquitto_sub -h localhost -t "esp32_fan_controller/#" -v
*/
const char *const mqttCmndTargetMode = "portal_sentry/cmnd/TARGETMODE";
const char *const mqttStatTargetMode = "portal_sentry/stat/TARGETMODE";
const char *const mqttCmndActualMode = "portal_sentry/cmnd/ACTUALMODE";
const char *const mqttStatActualMode = "portal_sentry/stat/ACTUALMODE";
const char *const mqttCmndWing = "portal_sentry/cmnd/WING";
const char *const mqttStatWing = "portal_sentry/stat/WING";

const char *const mqttTeleState2 = "portal_sentry/tele/STATE2";
const char *const mqttTeleState3 = "portal_sentry/tele/STATE3";
const char *const mqttTeleState4 = "portal_sentry/tele/STATE4";

#endif
