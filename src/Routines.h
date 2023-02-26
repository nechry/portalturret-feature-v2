// Routines
#include <AceRoutine.h>

using namespace ace_routine;

COROUTINE(openWingsRoutine)
{
  COROUTINE_BEGIN();
  if (!isOpen())
  {
    fullyOpened = false;
    wingServo.write(STATIONARY_ANGLE - 90);
    COROUTINE_DELAY(OPEN_DURATION);
    wingServo.write(STATIONARY_ANGLE);
    fullyOpened = true;
  }
  COROUTINE_END();
}

COROUTINE(closeWingsRoutine)
{
  COROUTINE_BEGIN();
  debugln("Routine closeWings");
  rotateServo.write(CENTER_ANGLE);
  COROUTINE_DELAY(250);
  fullyOpened = false;
  wingServo.write(STATIONARY_ANGLE + 90);
  COROUTINE_AWAIT(!isOpen());
  COROUTINE_DELAY(CLOSE_STOP_DELAY);
  wingServo.write(STATIONARY_ANGLE);
  COROUTINE_END();
}

COROUTINE(activatedRoutine)
{
  COROUTINE_BEGIN();

#ifdef USE_AUDIO
  if (isPlayingAudio())
  {
#ifdef STOP_MP3_ON_ROUTINE
    myDFPlayer.stop();
#endif
    COROUTINE_AWAIT(!isPlayingAudio());
  }
#endif

  // static unsigned long fromTime;
  // static unsigned long toTime;
  // static int fromAngle;
  // static int toAngle;
  static bool closedAtStart;
  closedAtStart = !isOpen();

#ifdef USE_AUDIO
  myDFPlayer.playFolder(1, random(1, 9));
  COROUTINE_AWAIT(isPlayingAudio());
  COROUTINE_AWAIT(!isPlayingAudio());
#endif

  if (closedAtStart)
  {
    if (!isOpen())
    {
      debugln("Routine activated: Opening wings");
      fullyOpened = false;
      wingServo.write(STATIONARY_ANGLE - 90);
      COROUTINE_DELAY(OPEN_DURATION - 100);
      wingServo.write(STATIONARY_ANGLE);
      fullyOpened = true;
    }
  }

  COROUTINE_END();
}

COROUTINE(searchingRoutine)
{
  COROUTINE_BEGIN();

#ifdef USE_AUDIO
  if (isPlayingAudio())
  {
#ifdef STOP_MP3_ON_ROUTINE
    myDFPlayer.stop();
#endif
    COROUTINE_AWAIT(!isPlayingAudio());
  }
  static unsigned long nextAudioClipTime = 0;
#endif

  while (true)
  {
#ifdef USE_AUDIO
    if (millis() > nextAudioClipTime)
    {
      nextAudioClipTime = millis() + 5000;
      myDFPlayer.playFolder(7, random(1, 11));
    }
#endif
    float t = millis() / 1000.0;
    uint16_t s = t * 255;
    if (fullyOpened)
    {
      rotateServo.write(map(constrain(inoise8_raw(s) * 2, -100, 100), -100, 100, CENTER_ANGLE - MAX_ROTATION, CENTER_ANGLE + MAX_ROTATION));
    }

    for (int i = 0; i < NUM_LEDS; i++)
    {
      leds[i] = CRGB(0, 0, 0);
      if (i == NUM_LEDS - 1)
      {
        leds[0] = CRGB(58, 0, 0);
      }
      else
      {
        leds[i + 1] = CRGB(58, 0, 0);
      }
      FastLED.show();
    }
    FastLED.clear();

    COROUTINE_YIELD();
  }

  COROUTINE_END();
}

COROUTINE(engagingRoutine)
{
  COROUTINE_BEGIN();
  for (int i = 0; i < NUM_LEDS; i++)
  {
    leds[i] = CRGB(255, 0, 0);
    FastLED.show();
  }
  if (fullyOpened)
  {
    static unsigned long fromTime;
    static unsigned long toTime;
    static int fromAngle;
    static int toAngle;

    fromTime = millis();
#ifdef USE_AUDIO
    if (isPlayingAudio())
    {
#ifdef STOP_MP3_ON_ROUTINE
      myDFPlayer.stop();
#endif
      COROUTINE_AWAIT(!isPlayingAudio());
    }
    myDFPlayer.playFolder(9, 13);
#endif
    alarm = true;
    fromTime = millis();
#ifdef USE_AUDIO
    COROUTINE_AWAIT(isPlayingAudio() || (!isPlayingAudio() && millis() > fromTime + 1000));
    COROUTINE_AWAIT(!isPlayingAudio());
    myDFPlayer.playFolder(9, 8);
#endif
    alarm = false;

    int whatSide = random(0, 2);
    fromAngle = whatSide == 0 ? CENTER_ANGLE - MAX_ROTATION : CENTER_ANGLE + MAX_ROTATION;
    toAngle = whatSide == 0 ? CENTER_ANGLE + MAX_ROTATION : CENTER_ANGLE - MAX_ROTATION;

    if (fullyOpened)
    {
      rotateServo.write(fromAngle);
    }

    COROUTINE_DELAY(200);
    fromTime = millis();
    toTime = fromTime + 1200;
    while (toTime > millis())
    {
      if (fullyOpened)
      {
        rotateServo.write(map(millis(), fromTime, toTime, fromAngle, toAngle));
      }
      analogWrite(GUN_LEDS, 255);
      delay(2);
      // COROUTINE_DELAY(0);
      analogWrite(GUN_LEDS, 0);
      COROUTINE_DELAY(1);
    }
  }
  COROUTINE_END();
}

COROUTINE(targetLostRoutine)
{
  COROUTINE_BEGIN();

#ifdef USE_AUDIO
  if (isPlayingAudio())
  {
#ifdef STOP_MP3_ON_ROUTINE
    myDFPlayer.stop();
#endif
    COROUTINE_AWAIT(!isPlayingAudio());
  }
  myDFPlayer.playFolder(6, random(1, 8));

  COROUTINE_AWAIT(isPlayingAudio());
  COROUTINE_AWAIT(!isPlayingAudio());
#endif

  for (int i = 0; i < NUM_LEDS; i++)
  {
    leds[i] = CRGB(0, 0, 28);
    FastLED.show();
  }

  rotateServo.write(CENTER_ANGLE);
  COROUTINE_DELAY(250);
  fullyOpened = false;
  if (isOpen())
  {
    debugln("Routine TargetLost need to close win");
    wingServo.write(STATIONARY_ANGLE + 90);
    COROUTINE_AWAIT(!isOpen());
    COROUTINE_DELAY(CLOSE_STOP_DELAY);
    wingServo.write(STATIONARY_ANGLE);
  }
  COROUTINE_END();
}

COROUTINE(pickedUpRoutine)
{
  COROUTINE_BEGIN();
#ifdef USE_AUDIO
  if (isPlayingAudio())
  {
#ifdef STOP_MP3_ON_ROUTINE
    myDFPlayer.stop();
#endif
    COROUTINE_AWAIT(!isPlayingAudio());
  }

  static unsigned long nextAudioClipTime = 0;
#endif

  while (true)
  {
    if (fullyOpened)
    {
      float t = millis() / 1000.0 * 5.0;
      uint16_t s = t * 255;
      rotateServo.write(map(constrain(inoise8_raw(s) * 2, -100, 100), -100, 100, CENTER_ANGLE - MAX_ROTATION, CENTER_ANGLE + MAX_ROTATION));
    }
#ifdef USE_AUDIO
    if (millis() > nextAudioClipTime)
    {
      nextAudioClipTime = millis() + 2500;
      myDFPlayer.playFolder(5, random(1, 11));
    }
#endif
    COROUTINE_YIELD();
  }

  COROUTINE_END();
}

COROUTINE(shutdownRoutine)
{
  COROUTINE_BEGIN();

  static unsigned long fromTime;
  static unsigned long toTime;
  ;
  static unsigned long t;
#ifdef USE_AUDIO
  if (isPlayingAudio())
  {
#ifdef STOP_MP3_ON_ROUTINE
    myDFPlayer.stop();
#endif
    COROUTINE_AWAIT(!isPlayingAudio());
  }
  myDFPlayer.playFolder(4, random(1, 9));
#endif

  rotateServo.write(CENTER_ANGLE);
  COROUTINE_DELAY(250);
  fullyOpened = false;
  debugln("Routine shutdown: Closing wings");
  wingServo.write(STATIONARY_ANGLE + 90);
  COROUTINE_AWAIT(!isOpen());
  COROUTINE_DELAY(CLOSE_STOP_DELAY);
  wingServo.write(STATIONARY_ANGLE);

  // fadeToBlackBy( leds, NUM_LEDS, 10);

  fromTime = t = millis();
  toTime = fromTime + 2000;
  while (t < toTime)
  {
    t = millis();
    if (t > toTime)
      t = toTime;
    uint16_t s = (t / 1000.0 * 15.0) * 255;
    uint8_t red = map(t, fromTime, toTime, inoise8(s), 0);

    analogWrite(CENTER_LED, red);

    for (int i = 0; i < NUM_LEDS; i++)
    {
      leds[i] = CRGB(red, 0, 0);
      FastLED.show();
    }
    COROUTINE_YIELD();
  }

  FastLED.clear();
  FastLED.show();

  COROUTINE_DELAY(5000);
  COROUTINE_END();
}

COROUTINE(rebootRoutine)
{
  COROUTINE_BEGIN();

  static unsigned long fromTime;
  static unsigned long toTime;
  static unsigned long t;

  COROUTINE_DELAY(1000);

  fromTime = t = millis();
  toTime = fromTime + 500;
  while (t < toTime)
  {
    t = millis();
    if (t > toTime)
      t = toTime;
    uint8_t red = map(t, fromTime, toTime, 0, 255);
    analogWrite(CENTER_LED, red);
    for (int i = 0; i < NUM_LEDS; i++)
    {
      leds[i] = CRGB(red, 0, 0);
      FastLED.show();
    }
    COROUTINE_YIELD();
  }
  COROUTINE_DELAY(1000);
  COROUTINE_END();
}

COROUTINE(manualEngagingRoutine)
{
  COROUTINE_BEGIN();
  if (fullyOpened)
  {
#ifdef USE_AUDIO
    if (isPlayingAudio())
    {
#ifdef STOP_MP3_ON_ROUTINE
      myDFPlayer.stop();
#endif
      COROUTINE_AWAIT(!isPlayingAudio());
    }
    myDFPlayer.playFolder(9, 8);
#endif
    static unsigned long fromTime;
    static unsigned long toTime;
    // static int fromAngle;
    // static int toAngle;
    fromTime = millis();
    toTime = fromTime + 1200;

    COROUTINE_DELAY(200);

    while (toTime > millis())
    {
      analogWrite(GUN_LEDS, 255);
      COROUTINE_DELAY(10);
      analogWrite(GUN_LEDS, 0);
      COROUTINE_DELAY(15);
    }
  }
  COROUTINE_END();
}

int8_t currentRotateDirection = 0;
int currentRotateAngle = CENTER_ANGLE;

COROUTINE(manualMovementRoutine)
{
  COROUTINE_LOOP()
  {
    if (currentRotateDirection != 0)
    {
      if (fullyOpened)
      {
        currentRotateAngle += currentRotateDirection;
        currentRotateAngle = constrain(currentRotateAngle, CENTER_ANGLE - MAX_ROTATION, CENTER_ANGLE + MAX_ROTATION);
        rotateServo.write(currentRotateAngle);
      }
    }
    COROUTINE_DELAY(5);
  }
}
