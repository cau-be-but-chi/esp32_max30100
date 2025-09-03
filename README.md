# Heart Rate & SpO2 Monitoring Device

## Mô tả
Thiết bị đo nhịp tim và SpO₂ sử dụng ESP32 + cảm biến MAX30100.
Dữ liệu được gửi lên Firebase và hiển thị trên web (HTML/CSS).

## Phần cứng
- ESP32
- MAX30100
- Breadboard, jumper

## Phần mềm
- Arduino IDE (ESP32 code)
- Firebase Realtime Database
- Web (HTML/CSS)

## Cách chạy
1. Mở `esp32_code/main.ino` trên Arduino IDE và nạp vào ESP32.
2. Cấu hình Firebase trong `firebase/firebase_config.txt`.
3. Mở `web/index.html` để xem dữ liệu.

