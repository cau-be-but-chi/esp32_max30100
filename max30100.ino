#include <Wire.h>
#include "MAX30100_PulseOximeter.h"
#include <WiFi.h>
#include <HTTPClient.h>


const char* ssid = "Anh Phuc";
const char* password = "dahieu468";


const char* firebaseHost = "https://esp32-health-monitor-5ac7a-default-rtdb.asia-southeast1.firebasedatabase.app";


const char* hrPath = "/sensors/heartRate";
const char* spo2Path = "/sensors/spo2";

#define REPORTING_PERIOD_MS 2000 

PulseOximeter pox;
uint32_t tsLastReport = 0;
bool beatDetected = false;

void onBeatDetected() {
  beatDetected = true;
  Serial.println("Phát hiện nhịp đập!");
}


void sensorUpdateTask(void *pvParameters) {
  for (;;) {
    pox.update();  
    vTaskDelay(1); 
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("Đang khởi động...");

  // Kết nối Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Đang kết nối Wi-Fi");
  int wifiTimeout = 0;
  while (WiFi.status() != WL_CONNECTED && wifiTimeout < 20) { 
    delay(500);
    Serial.print(".");
    wifiTimeout++;
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nĐã kết nối Wi-Fi! IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nKết nối Wi-Fi thất bại!");
  }

  // Khởi tạo MAX30100
  if (!pox.begin()) {
    Serial.println("Không tìm thấy MAX30100 - Kiểm tra kết nối I2C!");
    while (1);  // Dừng nếu không khởi tạo được
  } else {
    Serial.println("MAX30100 khởi động thành công");
    pox.setOnBeatDetectedCallback(onBeatDetected);
    pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);  
  }

  // Tạo task update cảm biến trên core 1
  xTaskCreatePinnedToCore(
    sensorUpdateTask,  // Hàm task
    "SensorUpdate",    // Tên task
    10000,             // Stack size
    NULL,              // Parameter
    1,                 // Priority
    NULL,              // Handle
    1                  // Core 1
  );
}

void loop() {
  if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
    float heartRate = pox.getHeartRate();
    int spo2 = pox.getSpO2();

    Serial.print("HR raw: ");
    Serial.print(heartRate);
    Serial.print(" bpm | SpO2 raw: ");
    Serial.print(spo2);
    Serial.println(" %");

    if (beatDetected && heartRate > 30 && spo2 > 80) {
      Serial.print("HR hợp lệ: ");
      Serial.print(heartRate);
      Serial.print(" bpm | SpO2 hợp lệ: ");
      Serial.print(spo2);
      Serial.println(" %");

      if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;

        String hrUrl = String(firebaseHost) + hrPath + ".json";
        http.begin(hrUrl);
        http.addHeader("Content-Type", "application/json");
        String hrPayload = String(heartRate);
        int hrHttpCode = http.PUT(hrPayload);
        if (hrHttpCode == HTTP_CODE_OK) {
          Serial.println("Gửi HR thành công");
        } else {
          Serial.printf("Lỗi gửi HR, mã: %d\n", hrHttpCode);
        }
        http.end();

        String spo2Url = String(firebaseHost) + spo2Path + ".json";
        http.begin(spo2Url);
        http.addHeader("Content-Type", "application/json");
        String spo2Payload = String(spo2);
        int spo2HttpCode = http.PUT(spo2Payload);
        if (spo2HttpCode == HTTP_CODE_OK) {
          Serial.println("Gửi SpO2 thành công");
        } else {
          Serial.printf("Lỗi gửi SpO2, mã: %d\n", spo2HttpCode);
        }
        http.end();
      }
      beatDetected = false;
    } else {
      Serial.println("Dữ liệu không hợp lệ - Kiểm tra ngón tay!");
    }

    tsLastReport = millis();
  }
}
