# EnviroGuard: 多合一環境監控器 (V2)

## 專案簡介
本專案結合 AHT20 溫濕度感測、MQ-2 可燃氣體/煙霧偵測、MQ-135 空氣品質偵測，以及 TFT 螢幕與 RGB LED 指示燈，根據指定邏輯判斷並顯示環境危險等級。

### 危險等級判斷邏輯
1. **最高危險 (CRITICAL)**：MQ2 > 2050 且 MQ135 > 80000
2. **高危險 (DANGER)**：MQ2 > 2050
3. **警示 (WARNING)**：MQ135 > 80000
4. **正常 (NORMAL)**：其他情況

## 硬體清單
- ESP32 開發板
- Adafruit AHT20 溫濕度感測器模組
- MQ-2 可燃氣體/煙霧感測器模組
- MQ-135 空氣品質感測器模組
- TTGO T-Display (TFT_eSPI 驅動)
- RGB LED（三色共陽或共陰）
- 連接線若干

## 腳位配置
```text
ESP32 引腳      功能
---------------  ---------------------
21 (SDA)         AHT20 SDA
22 (SCL)         AHT20 SCL
32               MQ-2 (analogRead)
33               MQ-135 (analogRead)
25 (PWM_CH0)     RGB LED R
26 (PWM_CH1)     RGB LED G
27 (PWM_CH2)     RGB LED B

TFT 顯示器請依 TFT_eSPI 库設定 (User_Setup.h)
```

## 軟體依賴
- Arduino IDE (1.8.x 或更新)
- [Adafruit_AHTX0](https://github.com/adafruit/Adafruit_AHTX0)
- [TFT_eSPI](https://github.com/Bodmer/TFT_eSPI)
- [MQ135](https://github.com/thatmrmartin/MQ135)

請在 Arduino IDE 的「管理函式庫」中安裝上述套件。

## 程式流程
1. **初始化**：
   - TFT 螢幕 & I2C (SDA/SCL)
   - AHT20 感測器
   - RGB LED PWM 通道
2. **讀取感測值**：每 2 秒讀取溫度、濕度、MQ2 (ADC) 與 MQ135 (PPM)
3. **判斷等級**：依據 MQ2 與 MQ135 門檻，更新 `DangerLevel`
4. **顯示結果**：
   - TFT 螢幕顯示狀態文字與感測數值
   - RGB LED 顯示對應顏色：
     - 紅 (CRITICAL)
     - 橘 (DANGER)
     - 黃 (WARNING)
     - 綠 (NORMAL)

## 使用方式
1. 確認已安裝所需函式庫。  
2. 開啟 `EnviroGuard.ino`，並選擇 ESP32 開發板與對應通訊埠。  
3. 如需調整感測器門檻，可修改開頭 `MQ2_THRESHOLD` 與 `MQ135_THRESHOLD`。  
4. 將程式上傳至開發板，並等待螢幕與感測器熱機完成後，即可開始顯示。

## 範例程式片段
```cpp
// 根據條件設定危險等級
if (mq2_value > MQ2_THRESHOLD && correctedPPM > MQ135_THRESHOLD) {
  currentLevel = CRITICAL_DANGER;
} else if (mq2_value > MQ2_THRESHOLD) {
  currentLevel = HIGH_DANGER;
} else if (correctedPPM > MQ135_THRESHOLD) {
  currentLevel = WARNING;
} else {
  currentLevel = NORMAL;
}

// 更新顏色與顯示文字
// ...existing code...
```

## 授權
本專案採用 MIT License，自由使用與修改。
