#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>

// =====================================================
// QUARK
// ESP32-C3 AI Usage + System Monitor
// =====================================================

// =====================================================
// PINS
// =====================================================

#define OLED_SDA    0
#define OLED_SCL    1

#define BUTTON_PIN  3

#define LED_R       4
#define BUZZER_PIN  5
#define SWITCH_PIN  6
#define LED_G       7
#define LED_B       10

#define LED_BRIGHTNESS 80

// =====================================================
// OLED
// =====================================================

U8G2_SH1106_128X64_NONAME_F_HW_I2C oled(
  U8G2_R0,
  U8X8_PIN_NONE
);

// =====================================================
// QUARK LOGO - 64 x 56 MONOCHROME
// =====================================================

#define QUARK_LOGO_W 64
#define QUARK_LOGO_H 56

static const unsigned char quarkLogo[] U8X8_PROGMEM = {

0x00, 0x00, 0x00, 0xFC, 0x07, 0x00, 0x00, 0x00,
0x00, 0x00, 0xC0, 0xFF, 0x7F, 0x00, 0x00, 0x00,
0x00, 0x00, 0xF0, 0x03, 0xFC, 0x01, 0x00, 0x00,
0x00, 0x00, 0x3C, 0x00, 0xC0, 0x07, 0x00, 0x00,
0x00, 0x00, 0x0E, 0x00, 0x00, 0x1E, 0x00, 0x00,
0x00, 0x80, 0x03, 0x00, 0x00, 0x3C, 0xE0, 0x01,
0x00, 0x80, 0x18, 0xC0, 0xFF, 0x70, 0xFE, 0x0F,
0x00, 0x00, 0x12, 0xE0, 0xFF, 0xE1, 0x0F, 0x0E,
0x00, 0x18, 0x06, 0x00, 0x00, 0xC3, 0x01, 0x0C,
0x00, 0xDC, 0x80, 0xFF, 0x07, 0x86, 0x03, 0x0C,
0x00, 0xCC, 0xC0, 0xFF, 0x0F, 0x0C, 0x07, 0x0E,
0x00, 0x0E, 0xE0, 0x07, 0x3E, 0x18, 0x06, 0x0F,
0x00, 0x07, 0xF0, 0x00, 0x70, 0x30, 0x0E, 0x0F,
0x00, 0x03, 0x38, 0x00, 0xE0, 0xE0, 0x0D, 0x0F,
0x80, 0x03, 0x1C, 0x00, 0x80, 0xC1, 0x1D, 0x07,
0x80, 0x01, 0x0E, 0x00, 0x00, 0x01, 0x9D, 0x01,
0xC0, 0x01, 0x07, 0x00, 0x00, 0x03, 0xCD, 0x00,
0xC0, 0x00, 0x07, 0x00, 0x00, 0x06, 0x65, 0x00,
0xC0, 0x80, 0x03, 0x00, 0x00, 0x04, 0x39, 0x00,
0xE0, 0xF0, 0x03, 0x00, 0x00, 0x0C, 0x0E, 0x00,
0x40, 0x98, 0x01, 0x00, 0x00, 0x08, 0x37, 0x00,
0x40, 0x88, 0x01, 0x00, 0x00, 0xD8, 0x31, 0x00,
0x40, 0x8C, 0x01, 0x00, 0x00, 0x78, 0x70, 0x00,
0x40, 0x8C, 0x01, 0x00, 0x00, 0x1C, 0x70, 0x00,
0x40, 0x8C, 0x01, 0x00, 0x00, 0x1F, 0x70, 0x00,
0x40, 0x8C, 0x01, 0x00, 0xC0, 0x19, 0x70, 0x00,
0x40, 0x8C, 0x05, 0x00, 0x70, 0x18, 0x71, 0x00,
0x40, 0x8C, 0x01, 0x00, 0x1C, 0x18, 0x30, 0x00,
0x40, 0x8C, 0x01, 0x00, 0x07, 0x18, 0x38, 0x00,
0x40, 0x8C, 0x03, 0xC0, 0x00, 0x18, 0x38, 0x00,
0xD8, 0x0C, 0x0B, 0x30, 0x00, 0x0C, 0x3C, 0x00,
0xC8, 0x2C, 0x03, 0x0C, 0x00, 0x0C, 0x1E, 0x00,
0xC6, 0x20, 0x87, 0xFF, 0x7F, 0x04, 0x1F, 0x00,
0xC7, 0x01, 0x76, 0xF8, 0xFF, 0x80, 0x1F, 0x00,
0x83, 0xC1, 0x0F, 0xF0, 0xFF, 0xC1, 0x0F, 0x00,
0x83, 0xFF, 0x19, 0xE0, 0xC0, 0xC3, 0x0F, 0x00,
0xFF, 0x3F, 0x38, 0xC0, 0x81, 0xE7, 0x07, 0x00,
0x7E, 0x06, 0xF0, 0xC0, 0x03, 0xCF, 0x03, 0x00,
0x00, 0x06, 0xE0, 0x83, 0x07, 0x9E, 0x01, 0x00,
0x00, 0x0E, 0x83, 0x7F, 0x0F, 0x3C, 0x00, 0x00,
0x00, 0x1C, 0x03, 0xFC, 0x1E, 0x78, 0x00, 0x00,
0x00, 0x38, 0x00, 0x00, 0xFC, 0xF3, 0x00, 0x00,
0x00, 0x70, 0x08, 0x00, 0x78, 0xE6, 0x07, 0x00,
0x00, 0xE0, 0x00, 0x00, 0xF0, 0xCC, 0x0F, 0x00,
0x00, 0xC0, 0x01, 0x00, 0xE0, 0x98, 0x1F, 0x00,
0x00, 0x80, 0xFF, 0xFF, 0xC1, 0x31, 0x3F, 0x00,
0x00, 0x00, 0x1F, 0x00, 0x83, 0x63, 0x7E, 0x00,
0x00, 0x00, 0x7C, 0x00, 0x06, 0x47, 0xFE, 0x00,
0x00, 0x00, 0xF8, 0xFF, 0x7E, 0x8E, 0xFC, 0x01,
0x00, 0x00, 0xC0, 0xFF, 0x3F, 0x1E, 0xF9, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x06, 0x04,
0x00, 0x00, 0x00, 0x00, 0x00, 0x78, 0xBC, 0x04,
0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0x80, 0x19,
0x00, 0x00, 0x00, 0x00, 0x00, 0xE0, 0x01, 0x38,
0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0xFF, 0x7F,
0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xFF, 0xFF
};

// =====================================================
// DATA STRUCTURES
// =====================================================

struct AIData {
  String today = "--";
  String lifetime = "--";

  String fiveHour = "--";
  String weekly = "--";

  String fiveReset = "--";
  String weekReset = "--";

  unsigned long lastRx = 0;

  bool error = false;
};

struct SystemData {
  String cpu = "--";
  String gpu = "--";
  String ram = "--";
  String gpuTemp = "--";

  unsigned long lastRx = 0;

  bool error = false;
};

AIData ai;
SystemData sys;

// =====================================================
// STATE
// =====================================================

int page = 0;
const int totalPages = 4;

bool aiMode = true;

bool lastMode = true;
bool lastModeReading = true;

bool lastButtonState = HIGH;
bool buttonState = HIGH;

bool waitingRefresh = false;

unsigned long refreshStarted = 0;

unsigned long lastDebounce = 0;
unsigned long modeDebounceStarted = 0;
unsigned long pressStart = 0;

unsigned long lastDisplayUpdate = 0;

const unsigned long DEBOUNCE_DELAY = 40;
const unsigned long REFRESH_TIMEOUT = 5000;
const size_t SERIAL_BUFFER_LIMIT = 300;
const int LOW_QUOTA_THRESHOLD = 15;

// AI is only updated about once/minute.
// System telemetry updates every ~2 seconds.

const unsigned long AI_TIMEOUT  = 90000;
const unsigned long SYS_TIMEOUT = 6000;

String serialBuffer;
bool serialLineOverflow = false;

// =====================================================
// RGB
// =====================================================

void setRGB(
  uint8_t r,
  uint8_t g,
  uint8_t b
) {

  ledcWrite(LED_R, r);
  ledcWrite(LED_G, g);
  ledcWrite(LED_B, b);
}

void ledOff() {

  setRGB(0, 0, 0);
}

// =====================================================
// AI RGB LANGUAGE
//
// GREEN = connected
// BLUE  = busy
// RED   = error
// =====================================================

void aiGreen() {

  setRGB(
    0,
    LED_BRIGHTNESS,
    0
  );
}

void aiBlue() {

  setRGB(
    0,
    0,
    LED_BRIGHTNESS
  );
}

void aiRed() {

  setRGB(
    LED_BRIGHTNESS,
    0,
    0
  );
}

// =====================================================
// SYSTEM RGB LANGUAGE
//
// GB = connected = cyan
// BR = busy      = magenta
// RG = error     = yellow
// =====================================================

void systemGB() {

  setRGB(
    0,
    LED_BRIGHTNESS,
    LED_BRIGHTNESS
  );
}

void systemBR() {

  setRGB(
    LED_BRIGHTNESS,
    0,
    LED_BRIGHTNESS
  );
}

void systemRG() {

  setRGB(
    LED_BRIGHTNESS,
    LED_BRIGHTNESS,
    0
  );
}

// =====================================================
// BUZZER
// =====================================================

void toneFor(
  uint16_t hz,
  uint16_t duration,
  uint16_t gap = 30
) {

  ledcWriteTone(
    BUZZER_PIN,
    hz
  );

  delay(duration);

  ledcWriteTone(
    BUZZER_PIN,
    0
  );

  delay(gap);
}

// =====================================================
// ORIGINAL QUARK STARTUP SOUND
// =====================================================

void quarkStartupSound() {

  // Original electronic startup motif.
  // Passive piezo = monophonic, so notes play sequentially.

  toneFor(440,  90,  35);   // A4
  toneFor(659,  90,  30);   // E5
  toneFor(831, 110,  30);   // G#5
  toneFor(988, 130,  45);   // B5

  toneFor(1319, 190, 70);   // E6

  toneFor(988,  80,  25);   // B5
  toneFor(1245, 110, 30);   // D#6

  toneFor(1319, 320, 0);    // E6 resolve
}

// =====================================================
// SPLASH SCREEN
// =====================================================

void showQuarkSplash() {

  unsigned long splashStart = millis();

  ledOff();

  oled.clearBuffer();

  // Center 64px wide image:
  // (128 - 64) / 2 = 32

  oled.drawXBMP(
    32,
    4,
    QUARK_LOGO_W,
    QUARK_LOGO_H,
    quarkLogo
  );

  oled.sendBuffer();

  quarkStartupSound();

  // Total logo time = 3 seconds
  while (
    millis() - splashStart < 3000
  ) {
    delay(10);
  }
}

// =====================================================
// CONNECTION STATUS
// =====================================================

bool aiConnected() {

  if (ai.error)
    return false;

  if (ai.lastRx == 0)
    return false;

  return (
    millis() - ai.lastRx
    <
    AI_TIMEOUT
  );
}

bool systemConnected() {

  if (sys.error)
    return false;

  if (sys.lastRx == 0)
    return false;

  return (
    millis() - sys.lastRx
    <
    SYS_TIMEOUT
  );
}

// =====================================================
// DEVICE STATUS
// =====================================================

enum DeviceStatus {
  STATUS_NORMAL,
  STATUS_BUSY,
  STATUS_ERROR
};

// Use an int return type so Arduino's generated prototype does not need
// DeviceStatus before the enum declaration.
int currentStatus() {

  if (waitingRefresh)
    return STATUS_BUSY;

  if (aiMode) {

    if (aiConnected())
      return STATUS_NORMAL;

    return STATUS_ERROR;

  } else {

    if (systemConnected())
      return STATUS_NORMAL;

    return STATUS_ERROR;
  }
}

// =====================================================
// STATUS LED
// =====================================================

void updateStatusLED() {

  int status =
    currentStatus();

if (aiMode) {

    if (status == STATUS_NORMAL)
      aiGreen();

    else if (status == STATUS_BUSY)
      aiBlue();

    else
      aiRed();

  }

  else {

    if (status == STATUS_NORMAL)
      systemGB();

    else if (status == STATUS_BUSY)
      systemBR();

    else
      systemRG();
  }
}

// =====================================================
// SERIAL PARSER
// =====================================================

String getField(
  const String &line,
  const String &key
) {

  String pattern =
    "|" + key + "=";

  int start =
    line.indexOf(pattern);

  if (start < 0)
    return "";

  start += pattern.length();

  int end =
    line.indexOf('|', start);

  if (end < 0)
    end = line.length();

  return line.substring(
    start,
    end
  );
}

// =====================================================
// RECEIVE DATA FROM PC
//
// AI|TODAY=18.4K|LIFE=1.42M|H5=74|WEEK=61|H5R=2h14m|WR=3d4h
//
// SYS|CPU=42|GPU=68|RAM=57|TEMP=71
// =====================================================

void handleSerialLine(
  String line
) {

  line.trim();

  if (!line.length())
    return;

// -----------------------------------------
  // Connectivity
  // -----------------------------------------

  if (line == "PING") {

    Serial.println("PONG");

    return;
  }

// -----------------------------------------
  // AI DATA
  // -----------------------------------------

  if (
    line.startsWith("AI|")
  ) {

    String value;

value =
      getField(line, "TODAY");

    if (value.length())
      ai.today = value;

value =
      getField(line, "LIFE");

    if (value.length())
      ai.lifetime = value;

value =
      getField(line, "H5");

    if (value.length())
      ai.fiveHour = value;

value =
      getField(line, "WEEK");

    if (value.length())
      ai.weekly = value;

value =
      getField(line, "H5R");

    if (value.length())
      ai.fiveReset = value;

value =
      getField(line, "WR");

    if (value.length())
      ai.weekReset = value;

ai.lastRx = millis();

    ai.error = false;

if (aiMode)
      waitingRefresh = false;

updateStatusLED();

    return;
  }

// -----------------------------------------
  // SYSTEM DATA
  // -----------------------------------------

  if (
    line.startsWith("SYS|")
  ) {

    String value;

value =
      getField(line, "CPU");

    if (value.length())
      sys.cpu = value;

value =
      getField(line, "GPU");

    if (value.length())
      sys.gpu = value;

value =
      getField(line, "RAM");

    if (value.length())
      sys.ram = value;

value =
      getField(line, "TEMP");

    if (value.length())
      sys.gpuTemp = value;

sys.lastRx = millis();

    sys.error = false;

if (!aiMode)
      waitingRefresh = false;

updateStatusLED();

    return;
  }

// -----------------------------------------
  // AI ERROR
  // -----------------------------------------

  if (
    line == "ERR|AI"
  ) {

    ai.error = true;

    if (aiMode)
      waitingRefresh = false;

    updateStatusLED();

    return;
  }

// -----------------------------------------
  // SYSTEM ERROR
  // -----------------------------------------

  if (
    line == "ERR|SYS"
  ) {

    sys.error = true;

    if (!aiMode)
      waitingRefresh = false;

    updateStatusLED();

    return;
  }
}

// =====================================================
// READ USB SERIAL
// =====================================================

void updateSerial() {
  while (Serial.available()) {
    const char c = Serial.read();

    if (c == '\n') {
      if (!serialLineOverflow)
        handleSerialLine(serialBuffer);

      serialBuffer = "";
      serialLineOverflow = false;
      continue;
    }

    if (c == '\r' || serialLineOverflow)
      continue;

    if (serialBuffer.length() < SERIAL_BUFFER_LIMIT) {
      serialBuffer += c;
    } else {
      // Never parse a truncated packet as if it were complete.
      serialBuffer = "";
      serialLineOverflow = true;
    }
  }
}

// =====================================================
// OLED UI
// =====================================================

int percentValue(const String &value) {
  if (value.length() == 0 || value == "--")
    return -1;

  const int parsed = value.toInt();
  if (parsed < 0 || parsed > 100)
    return -1;

  // Reject non-numeric values that String::toInt() would turn into zero.
  if (parsed == 0 && value[0] != '0')
    return -1;

  return parsed;
}

void drawProgressBar(int x, int y, int width, int height, int percent) {
  oled.drawFrame(x, y, width, height);

  if (percent < 0) {
    oled.drawHLine(x + 3, y + (height / 2), width - 6);
    return;
  }

  const int fillWidth = ((width - 4) * percent) / 100;
  if (fillWidth > 0)
    oled.drawBox(x + 2, y + 2, fillWidth, height - 4);
}

void drawBigValue(const String &value, int baseline) {
  oled.setFont(u8g2_font_logisoso20_tf);

  const int textWidth = oled.getStrWidth(value.c_str());
  const int x = (128 - textWidth) / 2;
  oled.drawStr(x < 0 ? 0 : x, baseline, value.c_str());

  oled.setFont(u8g2_font_6x10_tf);
}

void drawHeader() {
  const bool connected = aiMode ? aiConnected() : systemConnected();

  oled.setFont(u8g2_font_6x10_tf);
  oled.drawStr(0, 10, "QUARK");

  oled.drawBox(46, 1, 24, 13);
  oled.setDrawColor(0);
  oled.drawStr(49, 10, aiMode ? "AI" : "SYS");
  oled.setDrawColor(1);

  if (connected)
    oled.drawDisc(80, 6, 2);
  else
    oled.drawCircle(80, 6, 2);

  oled.drawStr(86, 10, connected ? "LIVE" : "WAIT");

  char pageLabel[6];
  snprintf(pageLabel, sizeof(pageLabel), "%d/%d", page + 1, totalPages);
  oled.drawStr(110, 10, pageLabel);
  oled.drawHLine(0, 14, 128);
}

void drawMiniMetric(const char *label, const String &value, int y) {
  const int percent = percentValue(value);
  String displayValue = value;
  if (percent < 0)
    displayValue = "--";
  else
    displayValue += "%";

  oled.setFont(u8g2_font_6x10_tf);
  oled.drawStr(0, y + 7, label);
  drawProgressBar(29, y, 65, 8, percent);
  oled.drawStr(98, y + 7, displayValue.c_str());
}

void drawPercentPage(
  const char *title,
  const String &value,
  const String &footer
) {
  const int percent = percentValue(value);
  String displayValue = value;
  if (percent < 0)
    displayValue = "--";
  else
    displayValue += "%";

  oled.setFont(u8g2_font_6x12_tf);
  oled.drawStr(0, 27, title);

  drawBigValue(displayValue, 47);
  drawProgressBar(2, 50, 124, 8, percent);

  if (footer.length()) {
    oled.setFont(u8g2_font_6x10_tf);
    oled.drawStr(0, 63, footer.c_str());
  }
}

void drawQuotaPage(
  const char *title,
  const String &value,
  const String &reset
) {
  const int percent = percentValue(value);
  String footer = "RESET  " + reset;

  if (percent >= 0 && percent <= LOW_QUOTA_THRESHOLD)
    footer = "LOW  " + footer;

  drawPercentPage(title, value, footer);
}

void drawTokenPage(
  const char *title,
  const String &value,
  const char *footerLabel,
  const String &footerValue
) {
  oled.setFont(u8g2_font_6x12_tf);
  oled.drawStr(0, 27, title);

  drawBigValue(value, 49);

  oled.setFont(u8g2_font_6x10_tf);
  oled.drawStr(0, 63, footerLabel);
  oled.drawStr(54, 63, footerValue.c_str());
}

void drawDisplay() {
  oled.clearBuffer();
  drawHeader();

  if (aiMode) {
    switch (page) {
      case 0:
        drawTokenPage("TOKENS TODAY", ai.today, "LIFETIME", ai.lifetime);
        break;

      case 1:
        drawTokenPage("LIFETIME TOKENS", ai.lifetime, "TODAY", ai.today);
        break;

      case 2: {
        drawQuotaPage("5-HOUR LEFT", ai.fiveHour, ai.fiveReset);
        break;
      }

      case 3: {
        drawQuotaPage("WEEKLY LEFT", ai.weekly, ai.weekReset);
        break;
      }
    }
  } else {
    switch (page) {
      case 0: {
        oled.setFont(u8g2_font_6x12_tf);
        oled.drawStr(0, 26, "SYSTEM LOAD");

        drawMiniMetric("CPU", sys.cpu, 29);
        drawMiniMetric("GPU", sys.gpu, 40);
        drawMiniMetric("RAM", sys.ram, 51);

        oled.setFont(u8g2_font_6x10_tf);
        const String temperature = "GPU TEMP  " + sys.gpuTemp + " C";
        oled.drawStr(0, 63, temperature.c_str());
        break;
      }

      case 1:
        drawPercentPage("CPU LOAD", sys.cpu, "PROCESSOR");
        break;

      case 2: {
        const String temperature = "TEMP  " + sys.gpuTemp + " C";
        drawPercentPage("GPU LOAD", sys.gpu, temperature);
        break;
      }

      case 3:
        drawPercentPage("RAM USED", sys.ram, "MEMORY");
        break;
    }
  }

  oled.sendBuffer();
}
// =====================================================
// MODE SWITCH
// =====================================================

void updateMode() {
  const bool reading = digitalRead(SWITCH_PIN) == LOW;

  if (reading != lastModeReading) {
    lastModeReading = reading;
    modeDebounceStarted = millis();
    return;
  }

  if (millis() - modeDebounceStarted < DEBOUNCE_DELAY || reading == lastMode)
    return;

  aiMode = reading;
  lastMode = reading;
  page = 0;
  waitingRefresh = false;

  if (aiMode) {
    Serial.println("MODE|AI");
    toneFor(1800, 45, 0);
  } else {
    Serial.println("MODE|SYS");
    toneFor(2300, 45, 0);
  }

  updateStatusLED();
  drawDisplay();
}

// =====================================================
// BUTTON
// =====================================================

void updateButton() {

  bool reading =
    digitalRead(BUTTON_PIN);

if (
    reading != lastButtonState
  ) {

    lastDebounce =
      millis();
  }

if (
    millis() - lastDebounce
    >
    DEBOUNCE_DELAY
  ) {

    if (
      reading != buttonState
    ) {

      buttonState =
        reading;

// BUTTON DOWN
      if (
        buttonState == LOW
      ) {

        pressStart =
          millis();

      }

// BUTTON RELEASE
      else {

        unsigned long held =
          millis()
          -
          pressStart;

// ---------------------------------------------
        // LONG PRESS = REFRESH
        // ---------------------------------------------

        if (
          held >= 800
        ) {

          waitingRefresh = true;

          refreshStarted =
            millis();

updateStatusLED();

toneFor(
            2600,
            70,
            0
          );

if (aiMode) {

            Serial.println(
              "REFRESH|AI"
            );

          }

          else {

            Serial.println(
              "REFRESH|SYS"
            );
          }
        }

// ---------------------------------------------
        // SHORT PRESS = NEXT SCREEN
        // ---------------------------------------------

        else {

          page =
            (page + 1)
            %
            totalPages;

toneFor(
            2100,
            25,
            0
          );

drawDisplay();
        }
      }
    }
  }

lastButtonState =
    reading;
}

// =====================================================
// SETUP
// =====================================================

void setup() {

  Serial.begin(115200);

  serialBuffer.reserve(SERIAL_BUFFER_LIMIT);

pinMode(
    BUTTON_PIN,
    INPUT_PULLUP
  );

pinMode(
    SWITCH_PIN,
    INPUT_PULLUP
  );

// ---------------- RGB PWM ----------------

  ledcAttach(
    LED_R,
    5000,
    8
  );

ledcAttach(
    LED_G,
    5000,
    8
  );

ledcAttach(
    LED_B,
    5000,
    8
  );

// ---------------- BUZZER PWM ----------------

  ledcAttach(
    BUZZER_PIN,
    2000,
    8
  );

ledOff();

// ---------------- OLED ----------------

  Wire.begin(
    OLED_SDA,
    OLED_SCL
  );

oled.begin();

// ---------------- STARTUP ----------------

  showQuarkSplash();

// ---------------- DETECT CURRENT MODE ----------------

  aiMode =
    digitalRead(SWITCH_PIN)
    == LOW;

lastMode =
    aiMode;
  lastModeReading = aiMode;
  modeDebounceStarted = millis();

updateStatusLED();

  drawDisplay();

Serial.println(
    "QUARK|READY"
  );
}

// =====================================================
// LOOP
// =====================================================

void loop() {

  updateSerial();

  updateButton();

  updateMode();

// If laptop never replies to a manual refresh,
  // leave BUSY mode after five seconds.

  if (
    waitingRefresh
    &&
    millis() - refreshStarted > REFRESH_TIMEOUT
  ) {

    waitingRefresh = false;

    updateStatusLED();
  }

// Refresh screen + connection state twice per second.

  if (
    millis() - lastDisplayUpdate >= 500
  ) {

    lastDisplayUpdate =
      millis();

updateStatusLED();

    drawDisplay();
  }
}
