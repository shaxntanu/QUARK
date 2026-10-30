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
  String peak = "--";
  String currentStreak = "--";
  String bestStreak = "--";
  String longestTurn = "--";

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
  String vram = "--";
  String memoryFree = "--";
  String disk = "--";
  String diskFree = "--";
  String download = "--";
  String upload = "--";
  String downloadPercent = "--";
  String uploadPercent = "--";
  String frequency = "--";
  String uptime = "--";
  String battery = "--";
  String ac = "--";
  String web = "--";

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
GraphHistory vramHistory;
GraphHistory diskHistory;
GraphHistory downloadHistory;
GraphHistory uploadHistory;
GraphHistory dailyTokenHistory;
GraphHistory fiveHourHistory;
GraphHistory weeklyHistory;

// =====================================================
// STATE
// =====================================================

int page = 0;
const int aiPageCount = 6;
const int systemPageCount = 7;

bool aiMode = true;

int activePageCount() {
  return aiMode ? aiPageCount : systemPageCount;
}

bool lastMode = true;
bool lastModeReading = true;

bool lastButtonState = HIGH;
bool buttonState = HIGH;

bool waitingRefresh = false;

unsigned long refreshStarted = 0;

unsigned long lastDebounce = 0;
unsigned long modeDebounceStarted = 0;
unsigned long pressStart = 0;
unsigned long modeActivatedAt = 0;
unsigned long connectionCandidateAt = 0;

unsigned long lastDisplayUpdate = 0;

const unsigned long DEBOUNCE_DELAY = 40;
const unsigned long REFRESH_TIMEOUT = 5000;
const unsigned long MODE_CONNECT_GRACE = 1500;
const unsigned long CONNECTION_STABLE_TIME = 400;
const size_t SERIAL_BUFFER_LIMIT = 160;
const int LOW_QUOTA_THRESHOLD = 15;

// AI is only updated about once/minute.
// System telemetry updates every ~2 seconds.

const unsigned long AI_TIMEOUT  = 90000;
const unsigned long SYS_TIMEOUT = 6000;

String serialBuffer;
bool serialLineOverflow = false;
bool acknowledgePlayedForMode = false;

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
  if (ai.lastRx == 0)
    return false;

  return (
    millis() - ai.lastRx
    <
    AI_TIMEOUT
  );
}

bool systemConnected() {
  if (sys.lastRx == 0)
    return false;

  return (
    millis() - sys.lastRx
    <
    SYS_TIMEOUT
  );
}

bool modeConnectionHealthy() {
  return aiMode
    ? (aiConnected() && !ai.error)
    : (systemConnected() && !sys.error);
}

bool modeConnectionGraceActive() {
  return modeActivatedAt != 0
    && millis() - modeActivatedAt < MODE_CONNECT_GRACE;
}

void updateConnectionAcknowledgement() {
  if (acknowledgePlayedForMode || modeConnectionGraceActive())
    return;

  if (!modeConnectionHealthy()) {
    connectionCandidateAt = 0;
    return;
  }

  const unsigned long now = millis();
  if (connectionCandidateAt == 0) {
    connectionCandidateAt = now;
    return;
  }

  if (now - connectionCandidateAt >= CONNECTION_STABLE_TIME) {
    acknowledgePlayedForMode = true;
    connectionAcknowledgeSound();
  }
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

  if (waitingRefresh || modeConnectionGraceActive())
    return STATUS_BUSY;

  if (aiMode) {

    if (ai.error)
      return STATUS_ERROR;

    if (aiConnected())
      return STATUS_NORMAL;

    return STATUS_ERROR;

  } else {

    if (sys.error)
      return STATUS_ERROR;

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
  if (valueLength < 1 || valueLength > 48)
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

bool validMetricLabel(const String &value) {
  if (value == "--")
    return true;
  if (!value.length() || value.length() > 8)
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
    } else if ((character == 'B' || character == 'K' || character == 'M'
                || character == 'G' || character == 'T')
               && !hasSuffix && i == value.length() - 1) {
      hasSuffix = true;
    } else {
      return false;
    }
  }
  return hasDigit;
}

bool validSmallCount(const String &value) {
  if (value == "--")
    return true;
  if (!value.length() || value.length() > 4)
    return false;
  for (size_t i = 0; i < value.length(); ++i) {
    if (value[i] < '0' || value[i] > '9')
      return false;
  }
  return true;
}

bool validDailyTrend(const String &value) {
  if (value == "--")
    return true;
  int start = 0;
  int count = 0;
  while (start <= value.length()) {
    int end = value.indexOf(',', start);
    if (end < 0)
      end = value.length();
    const String sample = value.substring(start, end);
    if (!validPercent(sample))
      return false;
    ++count;
    if (count > 7 || end == value.length())
      break;
    start = end + 1;
  }
  return count == 7;
}

void setDailyTrend(const String &value) {
  if (value == "--")
    return;
  int start = 0;
  for (uint8_t point = 0; point < 7; ++point) {
    int end = value.indexOf(',', start);
    if (end < 0)
      end = value.length();
    dailyTokenHistory.values[point] = static_cast<int8_t>(value.substring(start, end).toInt());
    start = end + 1;
  }
  dailyTokenHistory.count = 7;
  dailyTokenHistory.next = 7;
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
    String today, lifetime, fiveHour, weekly, fiveReset, weekReset;
    String peak, currentStreak, bestStreak, longestTurn, dailyTrend;
    const bool hasToday = hasField(line, "TODAY");
    const bool hasLifetime = hasField(line, "LIFE");
    const bool hasFiveHour = hasField(line, "H5");
    const bool hasWeekly = hasField(line, "WEEK");
    const bool hasFiveReset = hasField(line, "H5R");
    const bool hasWeekReset = hasField(line, "WR");
    const bool hasPeak = hasField(line, "PEAK");
    const bool hasCurrentStreak = hasField(line, "STREAK");
    const bool hasBestStreak = hasField(line, "BEST");
    const bool hasLongestTurn = hasField(line, "LONGEST");
    const bool hasDailyTrend = hasField(line, "D7");

    // Validate the complete packet before changing any live values.
    if (hasToday && (!readField(line, "TODAY", today) || !validTokenLabel(today))) return;
    if (hasLifetime && (!readField(line, "LIFE", lifetime) || !validTokenLabel(lifetime))) return;
    if (hasFiveHour && (!readField(line, "H5", fiveHour) || !validPercent(fiveHour))) return;
    if (hasWeekly && (!readField(line, "WEEK", weekly) || !validPercent(weekly))) return;
    if (hasFiveReset && (!readField(line, "H5R", fiveReset) || !validResetLabel(fiveReset))) return;
    if (hasWeekReset && (!readField(line, "WR", weekReset) || !validResetLabel(weekReset))) return;
    if (hasPeak && (!readField(line, "PEAK", peak) || !validTokenLabel(peak))) return;
    if (hasCurrentStreak && (!readField(line, "STREAK", currentStreak) || !validSmallCount(currentStreak))) return;
    if (hasBestStreak && (!readField(line, "BEST", bestStreak) || !validSmallCount(bestStreak))) return;
    if (hasLongestTurn && (!readField(line, "LONGEST", longestTurn) || !validResetLabel(longestTurn))) return;
    if (hasDailyTrend && (!readField(line, "D7", dailyTrend) || !validDailyTrend(dailyTrend))) return;

    const bool updated = hasToday || hasLifetime || hasFiveHour || hasWeekly || hasFiveReset || hasWeekReset
      || hasPeak || hasCurrentStreak || hasBestStreak || hasLongestTurn || hasDailyTrend;
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
    if (hasPeak) ai.peak = peak;
    if (hasCurrentStreak) ai.currentStreak = currentStreak;
    if (hasBestStreak) ai.bestStreak = bestStreak;
    if (hasLongestTurn) ai.longestTurn = longestTurn;
    if (hasDailyTrend) setDailyTrend(dailyTrend);

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

  if (line.startsWith("SYS|")) {
    String cpu, gpu, ram, temperature, vram, disk, memoryFree, diskFree;
    String download, upload, downPercent, upPercent, frequency, uptime, battery, ac, web;
    const bool hasCpu = hasField(line, "CPU");
    const bool hasGpu = hasField(line, "GPU");
    const bool hasRam = hasField(line, "RAM");
    const bool hasTemperature = hasField(line, "TEMP");
    const bool hasVram = hasField(line, "VRAM");
    const bool hasDisk = hasField(line, "DISK");
    const bool hasMemoryFree = hasField(line, "FREE");
    const bool hasDiskFree = hasField(line, "DFREE");
    const bool hasDownload = hasField(line, "DN");
    const bool hasUpload = hasField(line, "UP");
    const bool hasDownPercent = hasField(line, "DNP");
    const bool hasUpPercent = hasField(line, "UPP");
    const bool hasFrequency = hasField(line, "FREQ");
    const bool hasUptime = hasField(line, "SYSUP");
    const bool hasBattery = hasField(line, "BAT");
    const bool hasAc = hasField(line, "AC");
    const bool hasWeb = hasField(line, "WEB");

    // Keep updates atomic so a malformed trailing value cannot partly apply.
    if (hasCpu && (!readField(line, "CPU", cpu) || !validPercent(cpu))) return;
    if (hasGpu && (!readField(line, "GPU", gpu) || !validPercent(gpu))) return;
    if (hasRam && (!readField(line, "RAM", ram) || !validPercent(ram))) return;
    if (hasTemperature && (!readField(line, "TEMP", temperature) || !validTemperature(temperature))) return;
    if (hasVram && (!readField(line, "VRAM", vram) || !validPercent(vram))) return;
    if (hasDisk && (!readField(line, "DISK", disk) || !validPercent(disk))) return;
    if (hasMemoryFree && (!readField(line, "FREE", memoryFree) || !validMetricLabel(memoryFree))) return;
    if (hasDiskFree && (!readField(line, "DFREE", diskFree) || !validMetricLabel(diskFree))) return;
    if (hasDownload && (!readField(line, "DN", download) || !validMetricLabel(download))) return;
    if (hasUpload && (!readField(line, "UP", upload) || !validMetricLabel(upload))) return;
    if (hasDownPercent && (!readField(line, "DNP", downPercent) || !validPercent(downPercent))) return;
    if (hasUpPercent && (!readField(line, "UPP", upPercent) || !validPercent(upPercent))) return;
    if (hasFrequency && (!readField(line, "FREQ", frequency) || !validMetricLabel(frequency))) return;
    if (hasUptime && (!readField(line, "SYSUP", uptime) || !validResetLabel(uptime))) return;
    if (hasBattery && (!readField(line, "BAT", battery) || !validPercent(battery))) return;
    if (hasAc && (!readField(line, "AC", ac) || (ac != "ON" && ac != "OFF" && ac != "--"))) return;
    if (hasWeb && (!readField(line, "WEB", web) || (web != "OK" && web != "NO" && web != "--"))) return;

    const bool updated = hasCpu || hasGpu || hasRam || hasTemperature || hasVram || hasDisk
      || hasMemoryFree || hasDiskFree || hasDownload || hasUpload || hasDownPercent
      || hasUpPercent || hasFrequency || hasUptime || hasBattery || hasAc || hasWeb;
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
    if (hasVram) {
      sys.vram = vram;
      appendSample(vramHistory, vram);
    }
    if (hasDisk) {
      sys.disk = disk;
      appendSample(diskHistory, disk);
    }
    if (hasMemoryFree) sys.memoryFree = memoryFree;
    if (hasDiskFree) sys.diskFree = diskFree;
    if (hasDownload) sys.download = download;
    if (hasUpload) sys.upload = upload;
    if (hasDownPercent) {
      sys.downloadPercent = downPercent;
      appendSample(downloadHistory, downPercent);
    }
    if (hasUpPercent) {
      sys.uploadPercent = upPercent;
      appendSample(uploadHistory, upPercent);
    }
    if (hasFrequency) sys.frequency = frequency;
    if (hasUptime) sys.uptime = uptime;
    if (hasBattery) sys.battery = battery;
    if (hasAc) sys.ac = ac;
    if (hasWeb) sys.web = web;

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

void drawFooterPair(const char *label, const String &value) {
  oled.setFont(u8g2_font_6x10_tf);
  oled.drawStr(0, 61, label);
  const int labelWidth = oled.getStrWidth(label);
  int valueWidth = oled.getStrWidth(value.c_str());
  int valueX = 127 - valueWidth;
  if (valueX < labelWidth + 5) {
    oled.setFont(u8g2_font_5x7_tf);
    valueWidth = oled.getStrWidth(value.c_str());
    valueX = 127 - valueWidth;
  }
  oled.drawStr(valueX < labelWidth + 3 ? labelWidth + 3 : valueX, 61, value.c_str());
  oled.setFont(u8g2_font_6x10_tf);
}

void drawHeader() {
  const bool connected = aiMode ? aiConnected() : systemConnected();
  const bool activeError = aiMode ? ai.error : sys.error;
  const char *statusText = waitingRefresh ? "BUSY"
    : (modeConnectionGraceActive() ? "LINK"
    : (activeError && connected ? "ERR" : (connected ? "LIVE" : "WAIT")));

  oled.setFont(u8g2_font_6x10_tf);
  // Tiny header-safe Q mark: ring plus a short tail.
  oled.drawCircle(6, 6, 4);
  oled.drawLine(8, 8, 12, 12);
  oled.drawStr(14, 10, "QUARK");

  oled.drawBox(51, 1, 23, 12);
  oled.setDrawColor(0);
  oled.drawStr(aiMode ? 58 : 54, 10, aiMode ? "AI" : "SYS");
  oled.setDrawColor(1);

  if (connected && !activeError)
    oled.drawDisc(80, 6, 2);
  else
    oled.drawCircle(80, 6, 2);

  oled.drawStr(85, 10, statusText);
  const String pageLabel = String(page + 1) + "/" + String(activePageCount());
  oled.drawStr(110, 10, pageLabel.c_str());
  oled.drawHLine(0, 14, 128);
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
    int sideWidth = oled.getStrWidth(sideValue.c_str());
    int sideX = 127 - sideWidth;
    if (sideX < 86) {
      oled.setFont(u8g2_font_5x7_tf);
      sideWidth = oled.getStrWidth(sideValue.c_str());
      sideX = 127 - sideWidth;
    }
    oled.drawStr(sideX < 86 ? 86 : sideX, 61, sideValue.c_str());
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
  drawMetricPage(title, percent < 0 ? String("--") : String(value), history,
                 "RESET", reset.length() ? reset : String("--"), lowWarning);
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

  drawFooterPair(footerLabel, footerValue);
}

void drawTrendPage(const char *title, const GraphHistory &history, const char *footerLabel, const String &footerValue) {
  oled.setFont(u8g2_font_6x12_tf);
  oled.drawStr(0, 25, title);
  drawLineChart(history, 2, 29, 124, 22);

  drawFooterPair(footerLabel, footerValue);
}

void drawStreakPage() {
  oled.setFont(u8g2_font_6x12_tf);
  drawCentered("CODEX STREAKS", 25);
  oled.drawVLine(64, 31, 29);

  const String current = ai.currentStreak == "--" ? String("--") : String(ai.currentStreak + "d");
  const String best = ai.bestStreak == "--" ? String("--") : String(ai.bestStreak + "d");
  oled.setFont(u8g2_font_6x10_tf);
  oled.drawStr(15, 38, "CURRENT");
  oled.drawStr(80, 38, "BEST");

  oled.setFont(u8g2_font_logisoso16_tf);
  int currentWidth = oled.getStrWidth(current.c_str());
  int bestWidth = oled.getStrWidth(best.c_str());
  if (currentWidth > 60 || bestWidth > 60)
    oled.setFont(u8g2_font_6x12_tf);
  currentWidth = oled.getStrWidth(current.c_str());
  bestWidth = oled.getStrWidth(best.c_str());
  oled.drawStr((64 - currentWidth) / 2, 59, current.c_str());
  oled.drawStr(64 + (64 - bestWidth) / 2, 59, best.c_str());
  oled.setFont(u8g2_font_6x10_tf);
}

void drawInfoPage(const char *title, const String &value, const char *footerLabel, const String &footerValue) {
  oled.setFont(u8g2_font_6x12_tf);
  drawCentered(title, 25);
  drawMetricValue(value, 49);
  drawFooterPair(footerLabel, footerValue);
}

void drawNetworkColumn(const char *label, String value, const GraphHistory &history, int x) {
  if (value != "--")
    value += "/s";
  oled.setFont(u8g2_font_6x10_tf);
  oled.drawStr(x + 3, 32, label);
  oled.setFont(u8g2_font_6x12_tf);
  int width = oled.getStrWidth(value.c_str());
  if (width > 58) {
    oled.setFont(u8g2_font_6x10_tf);
    width = oled.getStrWidth(value.c_str());
  }
  oled.drawStr(x + (60 - width) / 2, 47, value.c_str());
  oled.setFont(u8g2_font_6x10_tf);
  drawLineChart(history, x, 49, 62, 14);
}

void drawNetworkPage() {
  oled.setFont(u8g2_font_6x10_tf);
  drawCentered("NETWORK RATE", 23);
  drawNetworkColumn("DOWNLOAD", sys.download, downloadHistory, 1);
  drawNetworkColumn("UPLOAD", sys.upload, uploadHistory, 65);
}

void drawLinkPage() {
  oled.setFont(u8g2_font_6x12_tf);
  drawCentered("CHATGPT HTTPS", 25);
  String status = sys.web == "OK" ? String("REACHABLE")
    : (sys.web == "NO" ? String("NO ROUTE") : String("CHECKING"));
  drawCentered(status.c_str(), 47);
  oled.drawHLine(10, 51, 108);

  const String power = sys.battery == "--"
    ? String("BAT --")
    : String("BAT " + sys.battery + "%" + (sys.ac == "ON" ? " AC" : ""));
  oled.setFont(u8g2_font_5x7_tf);
  oled.drawStr(1, 61, (String("UP ") + sys.uptime).c_str());
  const int width = oled.getStrWidth(power.c_str());
  oled.drawStr(127 - width, 61, power.c_str());
}

void drawOfflineScreen(bool codexUnavailable, bool connecting) {
  oled.setFont(u8g2_font_6x12_tf);
  const char *title = connecting
    ? (aiMode ? "CONNECTING CODEX" : "CONNECTING PC")
    : (codexUnavailable ? "CODEX OFFLINE" : "PC OFFLINE");
  drawCentered(title, 32);
  oled.drawHLine(22, 37, 84);
  oled.setFont(u8g2_font_6x10_tf);
  if (connecting) {
    drawCentered("Checking the selected feed", 50);
  } else {
    drawCentered(codexUnavailable ? "No usage data" : "Waiting for QUARK", 48);
    drawCentered(codexUnavailable ? "Check Codex login" : "companion...", 59);
  }
}

void drawDisplay() {
  oled.clearBuffer();
  drawHeader();

  const bool connected = aiMode ? aiConnected() : systemConnected();
  if (modeConnectionGraceActive()) {
    drawOfflineScreen(false, true);
    oled.sendBuffer();
    return;
  }
  if (!connected) {
    drawOfflineScreen(aiMode && systemConnected(), false);
    oled.sendBuffer();
    return;
  }

  if (aiMode) {
    switch (page) {
      case 0:
        drawTokenPage("TOKENS TODAY", ai.today, "LIFETIME", ai.lifetime);
        break;

      case 1:
        drawTrendPage("7D TOKENS (REL)", dailyTokenHistory, "PEAK 7D", ai.peak);
        break;

      case 2:
        drawQuotaPage("5-HOUR LEFT", ai.fiveHour, ai.fiveReset, fiveHourHistory);
        break;

      case 3:
        drawQuotaPage("WEEKLY LEFT", ai.weekly, ai.weekReset, weeklyHistory);
        break;

      case 4:
        drawStreakPage();
        break;

      case 5:
        drawInfoPage("LONGEST TURN", ai.longestTurn, "APP SERVER", "CODEX");
        break;
    }
  } else {
    switch (page) {
      case 0:
        drawMetricPage("CPU LOAD", sys.cpu, cpuHistory, "FREQ", sys.frequency, false);
        break;

      case 1:
        drawMetricPage("MEMORY USED", sys.ram, ramHistory, "FREE", sys.memoryFree, false);
        break;

      case 2: {
        const String temperature = sys.gpuTemp == "--"
          ? String("--")
          : String(sys.gpuTemp + "C");
        drawMetricPage("GPU LOAD", sys.gpu, gpuHistory, "TEMP", temperature, false);
        break;
      }

      case 3:
        drawMetricPage("VRAM USED", sys.vram, vramHistory, "", String(""), false);
        break;

      case 4:
        drawMetricPage("DISK USED", sys.disk, diskHistory, "FREE", sys.diskFree, false);
        break;

      case 5:
        drawNetworkPage();
        break;

      case 6:
        drawLinkPage();
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
  modeActivatedAt = millis();
  connectionCandidateAt = 0;
  acknowledgePlayedForMode = false;

  if (aiMode) {
    Serial.println("MODE|AI");
  } else {
    Serial.println("MODE|SYS");
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

  page = (page + 1) % activePageCount();
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
  modeActivatedAt = millis();
  connectionCandidateAt = 0;
  acknowledgePlayedForMode = false;

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
  updateConnectionAcknowledgement();

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
