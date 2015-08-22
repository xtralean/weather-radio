#include "Radio.h"

int ledOutput = D7;

void setup()
{
  pinMode(ledOutput, OUTPUT);
}

int lastSecond = -1;
int lastMinute = -1;

void loop()
{
  // process tasks once a second
  if (lastSecond != Time.second()) {
    lastSecond = Time.second();

    if (lastSecond & 1) {
      digitalWrite(ledOutput, LOW);
    } else {
      digitalWrite(ledOutput, HIGH);
    }
  }
}
