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
#define LED_B       10
// GPIO7's green channel is intentionally unused because the LED is faulty.

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

const uint8_t GRAPH_POINTS = 24;

struct GraphHistory {
  int8_t values[GRAPH_POINTS] = {};
  uint8_t count = 0;
  uint8_t next = 0;
};

AIData ai;
SystemData sys;
GraphHistory cpuHistory;
GraphHistory ramHistory;
GraphHistory gpuHistory;
GraphHistory fiveHourHistory;
GraphHistory weeklyHistory;

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
const size_t SERIAL_BUFFER_LIMIT = 160;
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

void setRedBlue(uint8_t red, uint8_t blue) {
  ledcWrite(LED_R, red);
  ledcWrite(LED_B, blue);
}

void ledOff() {
  setRedBlue(0, 0);
}

// Available RGB outputs: blue = normal, red+blue = busy, red = error.
void statusNormal() {
  setRedBlue(0, LED_BRIGHTNESS);
}

void statusBusy() {
  setRedBlue(LED_BRIGHTNESS, LED_BRIGHTNESS);
}

void statusError() {
  setRedBlue(LED_BRIGHTNESS, 0);
}

// =====================================================
// BUZZER
// =====================================================

bool buzzerPwmAttached = false;

void buzzerOff() {
  if (buzzerPwmAttached) {
    ledcWriteTone(BUZZER_PIN, 0);
    ledcDetach(BUZZER_PIN);
    buzzerPwmAttached = false;
  }

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
}

void toneFor(
  uint16_t hz,
  uint16_t duration,
  uint16_t gap = 30
) {

  if (hz == 0 || !ledcAttach(BUZZER_PIN, hz, 10)) {
    buzzerOff();
    delay(gap);
    return;
  }

  buzzerPwmAttached = true;
  ledcWriteTone(BUZZER_PIN, hz);

  delay(duration);

  buzzerOff();

  delay(gap);
}

// A short rising two-note chime acknowledges a newly live companion feed.
void connectionAcknowledgeSound() {
  toneFor(1320, 75, 25);
  toneFor(1760, 115, 0);
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
  const int status = currentStatus();
  if (status == STATUS_NORMAL)
    statusNormal();
  else if (status == STATUS_BUSY)
    statusBusy();
  else
    statusError();
}

// =====================================================
// SERIAL PARSER
// =====================================================

bool hasField(const String &line, const String &key) {
  return line.indexOf("|" + key + "=") >= 0;
}

bool readField(const String &line, const String &key, String &value) {
  const String pattern = "|" + key + "=";
  const int patternStart = line.indexOf(pattern);
  if (patternStart < 0)
    return false;

  // Duplicate fields are ambiguous; discard the whole packet instead.
  if (line.indexOf(pattern, patternStart + pattern.length()) >= 0)
    return false;

  const int valueStart = patternStart + pattern.length();
  int valueEnd = line.indexOf('|', valueStart);
  if (valueEnd < 0)
    valueEnd = line.length();

  const int valueLength = valueEnd - valueStart;
  if (valueLength < 1 || valueLength > 20)
    return false;

  for (int i = valueStart; i < valueEnd; ++i) {
    const uint8_t character = static_cast<uint8_t>(line[i]);
    if (character < 32 || character > 126)
      return false;
  }

  value = line.substring(valueStart, valueEnd);
  return true;
}

bool validPercent(const String &value) {
  if (value == "--")
    return true;
  if (!value.length() || value.length() > 3)
    return false;

  for (size_t i = 0; i < value.length(); ++i) {
    if (value[i] < '0' || value[i] > '9')
      return false;
  }

  const int parsed = value.toInt();
  return parsed >= 0 && parsed <= 100;
}

void appendSample(GraphHistory &history, const String &value) {
  const int sample = value == "--" ? -1 : value.toInt();
  history.values[history.next] = static_cast<int8_t>(sample);
  history.next = (history.next + 1) % GRAPH_POINTS;
  if (history.count < GRAPH_POINTS)
    ++history.count;
}

bool validTemperature(const String &value) {
  if (value == "--")
    return true;
  if (!value.length() || value.length() > 3)
    return false;

  for (size_t i = 0; i < value.length(); ++i) {
    if (value[i] < '0' || value[i] > '9')
      return false;
  }

  const int parsed = value.toInt();
  return parsed >= 0 && parsed <= 150;
}

bool validTokenLabel(const String &value) {
  if (value == "--")
    return true;
  if (!value.length() || value.length() > 12)
    return false;

  bool hasDigit = false;
  bool hasDecimal = false;
  bool hasSuffix = false;
  for (size_t i = 0; i < value.length(); ++i) {
    const char character = value[i];
    if (character >= '0' && character <= '9' && !hasSuffix) {
      hasDigit = true;
    } else if (character == '.' && !hasDecimal && !hasSuffix) {
      hasDecimal = true;
    } else if ((character == 'K' || character == 'M' || character == 'B')
               && !hasSuffix && i == value.length() - 1) {
      hasSuffix = true;
    } else {
      return false;
    }
  }
  return hasDigit && (!hasDecimal || hasSuffix);
}

bool validResetLabel(const String &value) {
  if (value == "--")
    return true;
  if (!value.length() || value.length() > 12)
    return false;

  bool hasDigit = false;
  for (size_t i = 0; i < value.length(); ++i) {
    const char character = value[i];
    if (character >= '0' && character <= '9')
      hasDigit = true;
    else if (character != 'd' && character != 'h' && character != 'm')
      return false;
  }
  return hasDigit;
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
  if (!line.length() || line.length() > SERIAL_BUFFER_LIMIT)
    return;

  for (size_t i = 0; i < line.length(); ++i) {
    const uint8_t character = static_cast<uint8_t>(line[i]);
    if (character < 32 || character > 126)
      return;
  }

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

  if (line.startsWith("AI|")) {
    const bool wasConnected = aiConnected();
    String today, lifetime, fiveHour, weekly, fiveReset, weekReset;
    const bool hasToday = hasField(line, "TODAY");
    const bool hasLifetime = hasField(line, "LIFE");
    const bool hasFiveHour = hasField(line, "H5");
    const bool hasWeekly = hasField(line, "WEEK");
    const bool hasFiveReset = hasField(line, "H5R");
    const bool hasWeekReset = hasField(line, "WR");

    // Validate the complete packet before changing any live values.
    if (hasToday && (!readField(line, "TODAY", today) || !validTokenLabel(today))) return;
    if (hasLifetime && (!readField(line, "LIFE", lifetime) || !validTokenLabel(lifetime))) return;
    if (hasFiveHour && (!readField(line, "H5", fiveHour) || !validPercent(fiveHour))) return;
    if (hasWeekly && (!readField(line, "WEEK", weekly) || !validPercent(weekly))) return;
    if (hasFiveReset && (!readField(line, "H5R", fiveReset) || !validResetLabel(fiveReset))) return;
    if (hasWeekReset && (!readField(line, "WR", weekReset) || !validResetLabel(weekReset))) return;

    const bool updated = hasToday || hasLifetime || hasFiveHour || hasWeekly || hasFiveReset || hasWeekReset;
    if (!updated)
      return;

    if (hasToday) ai.today = today;
    if (hasLifetime) ai.lifetime = lifetime;
    if (hasFiveHour) {
      ai.fiveHour = fiveHour;
      appendSample(fiveHourHistory, fiveHour);
    }
    if (hasWeekly) {
      ai.weekly = weekly;
      appendSample(weeklyHistory, weekly);
    }
    if (hasFiveReset) ai.fiveReset = fiveReset;
    if (hasWeekReset) ai.weekReset = weekReset;

    ai.lastRx = millis();
    ai.error = false;
    if (aiMode)
      waitingRefresh = false;
    updateStatusLED();
    if (aiMode && !wasConnected)
      connectionAcknowledgeSound();
    return;
  }

// -----------------------------------------
  // SYSTEM DATA
  // -----------------------------------------

  if (line.startsWith("SYS|")) {
    const bool wasConnected = systemConnected();
    String cpu, gpu, ram, temperature;
    const bool hasCpu = hasField(line, "CPU");
    const bool hasGpu = hasField(line, "GPU");
    const bool hasRam = hasField(line, "RAM");
    const bool hasTemperature = hasField(line, "TEMP");

    // Keep updates atomic so a malformed trailing value cannot partly apply.
    if (hasCpu && (!readField(line, "CPU", cpu) || !validPercent(cpu))) return;
    if (hasGpu && (!readField(line, "GPU", gpu) || !validPercent(gpu))) return;
    if (hasRam && (!readField(line, "RAM", ram) || !validPercent(ram))) return;
    if (hasTemperature && (!readField(line, "TEMP", temperature) || !validTemperature(temperature))) return;

    const bool updated = hasCpu || hasGpu || hasRam || hasTemperature;
    if (!updated)
      return;

    if (hasCpu) {
      sys.cpu = cpu;
      appendSample(cpuHistory, cpu);
    }
    if (hasGpu) {
      sys.gpu = gpu;
      appendSample(gpuHistory, gpu);
    }
    if (hasRam) {
      sys.ram = ram;
      appendSample(ramHistory, ram);
    }
    if (hasTemperature) sys.gpuTemp = temperature;

    sys.lastRx = millis();
    sys.error = false;
    if (!aiMode)
      waitingRefresh = false;
    updateStatusLED();
    if (!aiMode && !wasConnected)
      connectionAcknowledgeSound();
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

    if (c < 32 || c > 126) {
      // Protocol is ASCII; discard malformed input through its newline.
      serialBuffer = "";
      serialLineOverflow = true;
      continue;
    }

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

  if (value.length() > 3)
    return -1;

  for (size_t i = 0; i < value.length(); ++i) {
    if (value[i] < '0' || value[i] > '9')
      return -1;
  }

  const int parsed = value.toInt();
  if (parsed < 0 || parsed > 100)
    return -1;
  return parsed;
}

void drawLineChart(const GraphHistory &history, int x, int y, int width, int height) {
  oled.drawFrame(x, y, width, height);

  if (width < 8 || height < 6)
    return;

  const int plotLeft = x + 2;
  const int plotRight = x + width - 3;
  const int plotTop = y + 2;
  const int plotBottom = y + height - 3;
  const int plotHeight = plotBottom - plotTop;

  // Light dotted guides make the trend legible without turning the chart into a bar.
  for (int guide = 1; guide <= 2; ++guide) {
    const int guideY = plotTop + (plotHeight * guide) / 3;
    for (int guideX = plotLeft + 2; guideX < plotRight; guideX += 5)
      oled.drawPixel(guideX, guideY);
  }

  if (!history.count) {
    oled.drawHLine(plotLeft + 2, plotTop + plotHeight / 2, plotRight - plotLeft - 3);
    return;
  }

  int previousX = -1;
  int previousY = -1;
  for (uint8_t point = 0; point < history.count; ++point) {
    const uint8_t index = (history.next + GRAPH_POINTS - history.count + point) % GRAPH_POINTS;
    const int value = history.values[index];
    if (value < 0 || value > 100) {
      previousX = -1;
      previousY = -1;
      continue;
    }

    const int span = history.count > 1 ? history.count - 1 : 1;
    const int px = plotLeft + ((plotRight - plotLeft) * point) / span;
    const int py = plotBottom - (plotHeight * value) / 100;
    if (previousX >= 0)
      oled.drawLine(previousX, previousY, px, py);
    else
      oled.drawDisc(px, py, 1);

    if (point == history.count - 1)
      oled.drawDisc(px, py, 1);
    previousX = px;
    previousY = py;
  }
}

void drawBigValue(const String &value, int baseline) {
  oled.setFont(u8g2_font_logisoso20_tf);

  const int textWidth = oled.getStrWidth(value.c_str());
  const int x = (128 - textWidth) / 2;
  oled.drawStr(x < 0 ? 0 : x, baseline, value.c_str());

  oled.setFont(u8g2_font_6x10_tf);
}

void drawMetricValue(const String &value, int baseline) {
  oled.setFont(u8g2_font_logisoso16_tf);

  const int textWidth = oled.getStrWidth(value.c_str());
  const int x = (128 - textWidth) / 2;
  oled.drawStr(x < 0 ? 0 : x, baseline, value.c_str());

  oled.setFont(u8g2_font_6x10_tf);
}

void drawCentered(const char *text, int baseline) {
  const int width = oled.getStrWidth(text);
  oled.drawStr((128 - width) / 2, baseline, text);
}

void drawHeader() {
  const bool connected = aiMode ? aiConnected() : systemConnected();
  const char *statusText = waitingRefresh ? "BUSY" : (connected ? "LIVE" : "WAIT");

  oled.setFont(u8g2_font_6x10_tf);
  // Tiny header-safe Q mark: ring plus a short tail.
  oled.drawCircle(6, 6, 4);
  oled.drawLine(8, 8, 12, 12);
  oled.drawStr(14, 10, "QUARK");

  oled.drawBox(51, 1, 23, 12);
  oled.setDrawColor(0);
  oled.drawStr(aiMode ? 58 : 54, 10, aiMode ? "AI" : "SYS");
  oled.setDrawColor(1);

  if (connected)
    oled.drawDisc(80, 6, 2);
  else
    oled.drawCircle(80, 6, 2);

  oled.drawStr(85, 10, statusText);
  for (int i = 0; i < totalPages; ++i) {
    const int x = 110 + i * 4;
    if (i == page)
      oled.drawBox(x, 4, 3, 3);
    else
      oled.drawFrame(x, 4, 3, 3);
  }
  oled.drawHLine(0, 14, 128);
}

void drawMiniChart(const char *label, const String &value, const GraphHistory &history, int x) {
  const int percent = percentValue(value);
  const String displayValue = percent < 0 ? String("--") : String(value + "%");
  oled.drawFrame(x, 19, 62, 43);
  oled.setFont(u8g2_font_6x10_tf);
  oled.drawStr(x + 5, 29, label);
  const int valueWidth = oled.getStrWidth(displayValue.c_str());
  oled.drawStr(x + 57 - valueWidth, 29, displayValue.c_str());
  drawLineChart(history, x + 4, 33, 54, 24);
}

void drawMetricPage(
  const char *title,
  const String &value,
  const GraphHistory &history,
  const char *sideLabel,
  const String &sideValue,
  bool lowWarning
) {
  const int percent = percentValue(value);
  String displayValue = value;
  if (percent < 0)
    displayValue = "--";
  else
    displayValue += "%";

  if (lowWarning) {
    oled.drawBox(0, 17, 128, 12);
    oled.setDrawColor(0);
  }
  oled.setFont(u8g2_font_6x12_tf);
  oled.drawStr(2, 26, title);
  oled.setDrawColor(1);

  drawMetricValue(displayValue, 45);
  if (sideLabel[0]) {
    drawLineChart(history, 2, 47, 77, 14);
    oled.setFont(u8g2_font_6x10_tf);
    oled.drawStr(86, 51, sideLabel);
    oled.drawStr(86, 62, sideValue.c_str());
  } else {
    drawLineChart(history, 2, 47, 124, 15);
  }
}

void drawQuotaPage(
  const char *title,
  const String &value,
  const String &reset,
  const GraphHistory &history
) {
  const int percent = percentValue(value);
  const bool lowWarning = percent >= 0 && percent <= LOW_QUOTA_THRESHOLD;
  drawMetricPage(title, value, history, "RESET", reset.length() ? reset : String("--"), lowWarning);
}

void drawTokenPage(
  const char *title,
  const String &value,
  const char *footerLabel,
  const String &footerValue
) {
  oled.setFont(u8g2_font_6x12_tf);
  oled.drawStr(0, 25, title);

  drawMetricValue(value, 48);

  oled.setFont(u8g2_font_6x10_tf);
  oled.drawStr(0, 63, footerLabel);
  oled.drawStr(54, 63, footerValue.c_str());
}

void drawAIOverview() {
  oled.setFont(u8g2_font_6x10_tf);
  oled.drawStr(0, 24, "TOKENS TODAY");
  drawMetricValue(ai.today, 44);

  const int fiveHour = percentValue(ai.fiveHour);
  const int weekly = percentValue(ai.weekly);
  const String fiveLabel = fiveHour < 0 ? String("--") : String(ai.fiveHour + "%");
  const String weekLabel = weekly < 0 ? String("--") : String(ai.weekly + "%");

  oled.setFont(u8g2_font_6x10_tf);
  oled.drawStr(0, 55, "5H");
  drawLineChart(fiveHourHistory, 20, 48, 67, 8);
  oled.drawStr(94, 55, fiveLabel.c_str());
  oled.drawStr(0, 63, "WK");
  drawLineChart(weeklyHistory, 20, 56, 67, 8);
  oled.drawStr(94, 63, weekLabel.c_str());
}

void drawSystemOverview() {
  drawMiniChart("CPU", sys.cpu, cpuHistory, 1);
  drawMiniChart("RAM", sys.ram, ramHistory, 65);
}

void drawOfflineScreen(bool codexUnavailable) {
  oled.setFont(u8g2_font_6x12_tf);
  drawCentered(codexUnavailable ? "CODEX OFFLINE" : "PC OFFLINE", 32);
  oled.drawHLine(22, 37, 84);
  oled.setFont(u8g2_font_6x10_tf);
  drawCentered(codexUnavailable ? "No usage data" : "Waiting for QUARK", 48);
  drawCentered(codexUnavailable ? "Check Codex login" : "companion...", 59);
}

void drawDisplay() {
  oled.clearBuffer();
  drawHeader();

  const bool connected = aiMode ? aiConnected() : systemConnected();
  if (!connected) {
    drawOfflineScreen(aiMode && systemConnected());
    oled.sendBuffer();
    return;
  }

  if (aiMode) {
    switch (page) {
      case 0:
        drawAIOverview();
        break;

      case 1:
        drawTokenPage("TOKENS TODAY", ai.today, "LIFETIME", ai.lifetime);
        break;

      case 2: {
        drawQuotaPage("5-HOUR LEFT", ai.fiveHour, ai.fiveReset, fiveHourHistory);
        break;
      }

      case 3: {
        drawQuotaPage("WEEKLY LEFT", ai.weekly, ai.weekReset, weeklyHistory);
        break;
      }
    }
  } else {
    switch (page) {
      case 0: {
        drawSystemOverview();
        break;
      }

      case 1:
        drawMetricPage("CPU LOAD", sys.cpu, cpuHistory, "", String(""), false);
        break;

      case 2: {
        const String temperature = sys.gpuTemp == "--"
          ? String("--")
          : String(sys.gpuTemp + "C");
        drawMetricPage("GPU LOAD", sys.gpu, gpuHistory, "TEMP", temperature, false);
        break;
      }

      case 3:
        drawMetricPage("RAM USED", sys.ram, ramHistory, "", String(""), false);
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
  const bool reading = digitalRead(BUTTON_PIN);
  const unsigned long now = millis();

  if (reading != lastButtonState) {
    lastButtonState = reading;
    lastDebounce = now;
  }

  if (now - lastDebounce < DEBOUNCE_DELAY || reading == buttonState)
    return;

  buttonState = reading;
  if (buttonState == LOW) {
    pressStart = now;
    return;
  }

  // Select exactly one action after the debounced release.
  const unsigned long held = now - pressStart;
  if (held >= 800) {
    waitingRefresh = true;
    refreshStarted = now;
    updateStatusLED();
    toneFor(2600, 70, 0);
    Serial.println(aiMode ? "REFRESH|AI" : "REFRESH|SYS");
    return;
  }

  page = (page + 1) % totalPages;
  toneFor(2100, 25, 0);
  drawDisplay();
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
    LED_B,
    5000,
    8
  );

// ---------------- BUZZER ----------------

  buzzerOff();

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
