#include <ArduinoOTA.h>

void checkForUpdates() {
  ArduinoOTA.onStart([]()
                     {
                       String type;
                       if (ArduinoOTA.getCommand() == U_FLASH)
                       {
                         type = "sketch";
                       }
                       else
                       { // U_FS
                         type = "filesystem";
                       }

                       // NOTE: if updating FS this would be the place to unmount FS using FS.end()
                       debugln("Start updating " + type);
                     });
  ArduinoOTA.onEnd([]()
                   {
                     debugln("\nEnd");
                   });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
                        {
                          int percent = progress / (total / 100);
                          debugln(String(percent));
                          
                        });
  ArduinoOTA.onError([](ota_error_t error)
                     {
    //debugln("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      debugln("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      debugln("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      debugln("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      debugln("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      debugln("End Failed");
    } });
  debug("OTA set hostname to: ");
  debugln(hostname);
  ArduinoOTA.setHostname(hostname);
  ArduinoOTA.begin();
  debugln("OTA setup completed");
}