#include "Radio.h"
Radio radio = Radio();

int ledOutput = D7;

int radioControl(String command)
{
  if (command == "mute")
  {
    radio.setMuteVolume(1);
    return 1;
  }
  if (command == "play")
  {
    radio.setMuteVolume(0);
    return 1;
  }
  return -1;
}

void setup()
{
  pinMode(ledOutput, OUTPUT);

  if (radio.begin())
  {
    radio.setMuteVolume(1);
    radio.setWBFrequency(162550);
    radio.setVolume(63);
    Spark.function("radio", radioControl);
  }

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
