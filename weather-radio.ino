#include "Radio.h"
#include "pixel.h"

// IMPORTANT: Set pixel COUNT, PIN and TYPE
#define PIXEL_COUNT 12
#define PIXEL_PIN A1
#define PIXEL_TYPE WS2812B

// Parameter 1 = number of pixels in strip
// Parameter 2 = pin number (most are valid)
//               note: if not specified, D2 is selected for you.
// Parameter 3 = pixel type [ WS2812, WS2812B, WS2811, TM1803 ]
//               note: if not specified, WS2812B is selected for you.
//               note: RGB order is automatically applied to WS2811,
//                     WS2812/WS2812B/TM1803 is GRB order.
//
// 800 KHz bitstream 800 KHz bitstream (most NeoPixel products ...
//                         ... WS2812 (6-pin part)/WS2812B (4-pin part) )
//
// 400 KHz bitstream (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//                   (Radio Shack Tri-Color LED Strip - TM1803 driver
//                    NOTE: RS Tri-Color LED's are grouped in sets of 3)

Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, PIXEL_TYPE);

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

  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
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

  rainbow(200);

}

void rainbow(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i+j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  if(WheelPos < 85) {
   return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if(WheelPos < 170) {
   WheelPos -= 85;
   return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
   WheelPos -= 170;
   return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}
