/*
 * 整合專案：多合一環境監控器 (V2 - 根據指定邏輯更新)
 * ----------------------------------------------------
 * 新的判斷邏輯:
 * 1. 最高危險: MQ2 > 2000 AND MQ135 > 300
 * 2. 高危險:    MQ2 > 2000
 * 3. 警示:      MQ135 > 300
 * 4. 正常:      其他情況
 */

#include <Wire.h>
#include <Adafruit_AHTX0.h>      // AHT20 驅動庫
#include <TFT_eSPI.h>            // TTGO T-Display LCD 驅動庫
#include <MQ135.h>               // MQ-135 驅動庫

// --- 硬體腳位定義 ---
#define SDA_PIN 21
#define SCL_PIN 22
#define MQ2_PIN   32
#define MQ135_PIN 33
#define RED_PIN   25
#define GREEN_PIN 26
#define BLUE_PIN  27

// --- 根據圖片定義新的感測器門檻 ---
#define MQ2_THRESHOLD 2050    // MQ-2 ADC 危險門檻
#define MQ135_THRESHOLD 80000   // MQ-135 PPM 警示門檻

// --- 全域物件宣告 ---
Adafruit_AHTX0 aht;
TFT_eSPI tft = TFT_eSPI();
MQ135 mq135_sensor(MQ135_PIN);

// 定義新的危險等級，方便管理狀態
enum DangerLevel {
  NORMAL,           // 正常
  WARNING,          // 警示
  HIGH_DANGER,      // 高危險
  CRITICAL_DANGER   // 最高危險
};

void setup() {
  Serial.begin(115200);

  // 1. 初始化螢幕
  tft.init();
  tft.setRotation(3);
  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setCursor(10, 30);
  tft.println("System Initializing...");
  delay(1000);

  // 2. 初始化 I2C
  Wire.begin(SDA_PIN, SCL_PIN);

  // 3. 初始化 AHT20
  if (!aht.begin()) {
    tft.fillScreen(TFT_RED);
    tft.setCursor(10, 30);
    tft.println("AHT20 INIT FAIL");
    while (1) delay(10);
  }

  // 4. 初始化 RGB LED PWM 通道
  ledcSetup(0, 5000, 8); ledcAttachPin(RED_PIN, 0);
  ledcSetup(1, 5000, 8); ledcAttachPin(GREEN_PIN, 1);
  ledcSetup(2, 5000, 8); ledcAttachPin(BLUE_PIN, 2);

  tft.fillScreen(TFT_BLACK);
  tft.setCursor(10, 30);
  tft.println("Sensors Warming Up...");
  tft.setTextSize(1);
  tft.setCursor(10, 60);
  tft.println("(Wait 1-2 mins for stable readings)");
  delay(5000); 
}

void loop() {
  // --- 1. 讀取所有感測器數據 ---
  sensors_event_t humidity, temp;
  aht.getEvent(&humidity, &temp);
  float temperature = temp.temperature;
  float humi = humidity.relative_humidity;
  int mq2_value = analogRead(MQ2_PIN);
  float correctedPPM = mq135_sensor.getCorrectedPPM(temperature, humi);

  // --- 2. 根據新的指定邏輯判斷危險等級 ---
  DangerLevel currentLevel;
  
  // 必須從最嚴格的條件開始檢查
  if (mq2_value > MQ2_THRESHOLD && correctedPPM > MQ135_THRESHOLD) {
    currentLevel = CRITICAL_DANGER; // 等級1: 最高危險
  } else if (mq2_value > MQ2_THRESHOLD) {
    currentLevel = HIGH_DANGER;     // 等級2: 高危險
  } else if (correctedPPM > MQ135_THRESHOLD) {
    currentLevel = WARNING;         // 等級3: 警示
  } else {
    currentLevel = NORMAL;          // 等級4: 正常
  }

  // --- 3. 根據危險等級設定 RGB LED 顏色與狀態文字 ---
  String statusText;
  int statusColor;

  switch (currentLevel) {
    case CRITICAL_DANGER: // 最高危險
      ledcWrite(0, 255); ledcWrite(1, 0); ledcWrite(2, 0); // 紅色
      statusText = "CRITICAL";
      statusColor = TFT_RED;
      break;
    case HIGH_DANGER: // 高危險 (火災/瓦斯)
      ledcWrite(0, 255); ledcWrite(1, 128); ledcWrite(2, 0); // 橘色
      statusText = "DANGER";
      statusColor = TFT_ORANGE;
      break;
    case WARNING: // 警示 (空品)
      ledcWrite(0, 255); ledcWrite(1, 255); ledcWrite(2, 0); // 黃色
      statusText = "WARNING";
      statusColor = TFT_YELLOW;
      break;
    case NORMAL: // 正常
    default:
      ledcWrite(0, 0); ledcWrite(1, 255); ledcWrite(2, 0); // 綠色
      statusText = "NORMAL";
      statusColor = TFT_GREEN;
      break;
  }

  // --- 4. 在 LCD 螢幕上顯示所有資訊 ---
  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(2);

  // 顯示狀態
  tft.setTextColor(statusColor);
  tft.setCursor(0, 5);
  tft.printf("Status: %s\n", statusText.c_str());
  
  // 顯示詳細數據
  tft.setTextColor(TFT_WHITE);
  tft.setCursor(0, 35);
  tft.printf("Temp: %.1f C\n", temperature);
  tft.printf("Humi: %.1f %%\n", humi);
  tft.printf("(MQ2)Gas/Smoke: %d\n", mq2_value);
  tft.printf("(MQ135)Air Quality: %.0f PPM", correctedPPM);

  delay(2000);
}