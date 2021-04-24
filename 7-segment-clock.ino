#include <NTPEventTypes.h>
#include <ESPNtpClient.h>
#include <TZdef.h>
#include <WiFi.h>
#include <TM1637Display.h>
#include <AceTime.h>

// FILL THESE IN with the values for your wi-fi network and timezone, then compile and upload.
#define WIFI_SSID ""
#define WIFI_PASSWORD ""
auto timezoneInfo = &ace_time::zonedb::kZoneAmerica_Denver;

#define DISPLAY_CLOCK_PIN 32
#define DISPLAY_DATA_PIN 33

// Display brightness, from 0 to 7
#define DISPLAY_BRIGHTNESS 2

ace_time::BasicZoneProcessor timezoneProcessor;
auto timezone = ace_time::TimeZone::forZoneInfo(timezoneInfo, &timezoneProcessor);

TM1637Display display(DISPLAY_CLOCK_PIN, DISPLAY_DATA_PIN);

int64_t lastDisplayMilliseconds = 0;

void setup() {
  Serial.begin(115200); // for future debugging

  display.setBrightness(DISPLAY_BRIGHTNESS);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  NTP.begin();
}

void loop() {
  // Get the current time, in milliseconds since January 1, 1970. The "/ 500 * 500" bit rounds down to the nearest 500 milliseconds.
  int64_t currentMilliseconds = NTP.millis() / 500 * 500;

  // Don't do anything if the time hasn't changed since we last told the 7 segment display to show this time
  if (currentMilliseconds == lastDisplayMilliseconds) {
    return;
  }

  lastDisplayMilliseconds = currentMilliseconds;

  // Figure out if it's the first half of the current second (in which case we want to turn on the colon sign on the display; if it isn't, we'll turn it off instead)
  bool isFirstHalfOfSecond = currentMilliseconds % 1000 == 0;

  // See if the NTP client has figured out what time it is yet
  if (currentMilliseconds < 1000000000L) {
    // It hasn't, so show a bunch of underscores
    uint8_t segments[] = {SEG_D, SEG_D | (isFirstHalfOfSecond ? 0b10000000 : 0), SEG_D, SEG_D};
    display.setSegments(segments);
  } else {
    // It has, so show the time. (We show it in 12 hour format; if you want to show it in 24 hour format, remove the "% 12" two lines below.)
    auto currentTime = ace_time::ZonedDateTime::forUnixSeconds(currentMilliseconds / 1000L, timezone);
    display.showNumberDecEx((currentTime.hour() % 12) * 100 + currentTime.minute(), isFirstHalfOfSecond ? 0b01000000 : 0, false, 4, 0);
  }
}
