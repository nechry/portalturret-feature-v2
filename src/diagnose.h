#include <Arduino.h>

void blinkGUN_LEDS(int times)
{
  debugln("Blinking gun leds");
  for (int i = 0; i < times; i++)
  {
    analogWrite(GUN_LEDS, 255);
    delay(1);
    analogWrite(GUN_LEDS, 0);
    delay(100);
  }
}

void blinkCENTER_LED(int times, int delayTime)
{
  debugln("Blinking center led");
  for (int i = 0; i < times; i++)
  {
    static unsigned long fromTime;
    static unsigned long toTime;
    static unsigned long t;

    fromTime = t = millis();
    toTime = fromTime + delayTime;
    while (t < toTime)
    {
      t = millis();
      if (t > toTime)
        t = toTime;
      uint8_t red = map(t, fromTime, toTime, 0, 255);
      analogWrite(CENTER_LED, red);
      delay(10);
    }
    fromTime = t = millis();
    toTime = fromTime + delayTime;
    while (t < toTime)
    {
      t = millis();
      if (t > toTime)
        t = toTime;
      uint8_t red = map(t, fromTime, toTime, 255, 0);
      analogWrite(CENTER_LED, red);
      delay(10);
    }
  }
}

void fadeall()
{
  for (int i = 0; i < NUM_LEDS; i++)
  {
    leds[i].nscale8(250);
  }
}

void cylon()
{
  static uint8_t hue = 0;
  for (int i = 0; i < NUM_LEDS; i++)
  {
    // Set the i'th led to red
    leds[i] = CHSV(hue++, 255, 255);
    // Show the leds
    FastLED.show();
    // now that we've shown the leds, reset the i'th led to black
    // leds[i] = CRGB::Black;
    fadeall();
    // Wait a little bit before we loop around and do it again
    delay(50);
  }  
  for (int i = (NUM_LEDS)-1; i >= 0; i--)
  {
    // Set the i'th led to red
    leds[i] = CHSV(hue++, 255, 255);
    // Show the leds
    FastLED.show();
    // now that we've shown the leds, reset the i'th led to black
    // leds[i] = CRGB::Black;
    fadeall();
    // Wait a little bit before we loop around and do it again
    delay(50);
  }
}

void loopRing(CRGB color)
{
  for (int dot = 0; dot < NUM_LEDS; dot++)
  {
    leds[dot] = color;
    FastLED.show();
    // clear this led for the next time around the loop
    leds[dot] = CRGB::Black;
    delay(10);
  }
}

void performDiagnose(int action)
{
  switch (action)
  {
  case 0:
    debugln("Diagnose action 0");
    wingServo.write(STATIONARY_ANGLE - 90);
    delay(250);
    wingServo.write(STATIONARY_ANGLE);
    break;
  case 1:
    debugln("Diagnose action 1");
    wingServo.write(STATIONARY_ANGLE + 90);
    delay(250);
    wingServo.write(STATIONARY_ANGLE);
    break;
  case 2:
    debugln("Diagnose action 2");
    rotateServo.write(CENTER_ANGLE - MAX_ROTATION);
    delay(2000);
    rotateServo.write(CENTER_ANGLE);
    break;
  case 3:
    debugln("Diagnose action 3");
    rotateServo.write(CENTER_ANGLE + MAX_ROTATION);
    delay(2000);
    rotateServo.write(CENTER_ANGLE);
    break;
  case 4:
    debugln("Diagnose action 4");
    blinkGUN_LEDS(5);
    break;
  case 5:
    debugln("Diagnose action 5");
    FastLED.clear();
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    FastLED.show();
    delay(500);

    FastLED.clear();
    loopRing(CRGB::Red);
    delay(1000);
    FastLED.clear();
    loopRing(CRGB::Green);
    delay(1000);
    FastLED.clear();
    loopRing(CRGB::Blue);
    delay(1000);

    FastLED.clear();
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    FastLED.show();
    delay(500);

    FastLED.clear();
    cylon();
    FastLED.clear();
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    FastLED.show();
    FastLED.clear();
    break;
  case 6:
#ifdef USE_AUDIO
    myDFPlayer.playFolder(1, random(1, 9));
#else
    Serial.println("Audio is disabled...");
#endif
    break;
  case 7:
    blinkCENTER_LED(3, 1000);
    break;
  }
}