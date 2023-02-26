#include <Arduino.h>

void debugln(String message)
{
#ifndef USE_AUDIO
  Serial.println(message);
#endif
}

void debug(String message)
{
#ifndef USE_AUDIO
  Serial.print(message);
#endif
}

void debug(char message)
{
#ifndef USE_AUDIO
  Serial.print(message);
#endif
}

void debugln(const char *message)
{
#ifndef USE_AUDIO
  Serial.println(message);
#endif
}

void debugln(int message)
{
#ifndef USE_AUDIO
  Serial.println(message);
#endif
}

void debug(int message)
{
#ifndef USE_AUDIO
  Serial.print(message);
#endif
}

void debug(float message)
{
#ifndef USE_AUDIO
  Serial.print(message);
#endif
}