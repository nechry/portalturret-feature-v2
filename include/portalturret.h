

void UpdateLEDPreloader();

void preloader(uint8_t led);

void blinkGUN_LEDS(int times);

bool isOpen();

bool isDetectingMotion();

bool isPlayingAudio();

void setupAccelerometer();

void updateAccelerometer();

void startWebServer();

void startWebSocket();

void checkForUpdates();

void stateBehaviour();

void performDiagnose(int action);