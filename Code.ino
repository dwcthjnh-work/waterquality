#include <WiFi.h>
#include <FirebaseESP32.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <DHT.h>

// Thông tin WiFi của bạn
#define WIFI_SSID "R1OT_L1NH 1551"
#define WIFI_PASSWORD "1234567890"

// Firebase project details
#define FIREBASE_HOST "waterquality-e83e1-default-rtdb.firebaseio.com"  // URL của Firebase Realtime Database
#define FIREBASE_AUTH "dNS5qBc4tV1EQvnReJ8hfjmcN7aKiVbpBZG1OJaf"       // Database Secret

// Firebase objects
FirebaseData firebaseData;
FirebaseConfig config;
FirebaseAuth auth;

// DS18B20 setup
#define ONE_WIRE_BUS 2
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// DHT21 setup
#define DHTPIN 4
#define DHTTYPE DHT21
DHT dht(DHTPIN, DHTTYPE);

// pH sensor setup
#define PH_PIN 34 // Chân analog cho cảm biến pH
#define TDS_PIN 35       // Chân analog cho cảm biến TDS
#define TURBIDITY_PIN 32 // Chân analog cho cảm biến độ đục
void setup() {
  Serial.begin(9600);

  // Kết nối WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to WiFi");

  // Cấu hình Firebase
  config.host = FIREBASE_HOST;
  config.signer.tokens.legacy_token = FIREBASE_AUTH;

  // Kết nối Firebase
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  // Kiểm tra kết nối Firebase
  if (Firebase.ready()) {
    Serial.println("Connected to Firebase");
  } else {
    Serial.print("Failed to connect Firebase: ");
    Serial.println(firebaseData.errorReason());
  }

  // Khởi tạo cảm biến
  sensors.begin();
  dht.begin();
}

void loop() {
  // Đọc dữ liệu từ cảm biến DS18B20
  sensors.requestTemperatures();
  float tempDS18B20 = sensors.getTempCByIndex(0);

  // Đọc dữ liệu từ cảm biến DHT21
  float tempDHT = dht.readTemperature();
  float humidity = dht.readHumidity();

  // Đọc dữ liệu từ cảm biến pH
  float pH_value = readpHValue();
  float tdsValue = readTDS();
  float turbidityValue = readTurbidity();

  // Đẩy dữ liệu lên Firebase
  if (Firebase.setFloat(firebaseData, "/sensor/tempDS18B20", tempDS18B20)) {
    Serial.println("Temperature DS18B20 updated");
  } else {
    Serial.print("Failed to update DS18B20 temperature: ");
    Serial.println(firebaseData.errorReason());
  }

  if (Firebase.setFloat(firebaseData, "/sensor/tempDHT", tempDHT)) {
    Serial.println("Temperature DHT21 updated");
  } else {
    Serial.print("Failed to update DHT21 temperature: ");
    Serial.println(firebaseData.errorReason());
  }

  if (Firebase.setFloat(firebaseData, "/sensor/humidity", humidity)) {
    Serial.println("Humidity updated");
  } else {
    Serial.print("Failed to update humidity: ");
    Serial.println(firebaseData.errorReason());
  }

  if (Firebase.setFloat(firebaseData, "/sensor/pH", pH_value)) {
    Serial.println("pH value updated");
  } else {
    Serial.print("Failed to update pH value: ");
    Serial.println(firebaseData.errorReason());
  }
    if (Firebase.setFloat(firebaseData, "/sensor/tds", tdsValue)) {
    Serial.println("TDS value updated");
  } else {
    Serial.print("Failed to update TDS: ");
    Serial.println(firebaseData.errorReason());
  }

  if (Firebase.setFloat(firebaseData, "/sensor/turbidity", turbidityValue)) {
    Serial.println("Turbidity value updated");
  } else {
    Serial.print("Failed to update turbidity: ");
    Serial.println(firebaseData.errorReason());
  }

  // Đợi 15 giây trước khi gửi dữ liệu tiếp
  delay(500);
}

// Hàm đọc giá trị từ cảm biến pH
float readpHValue() {
  int analogValue = analogRead(PH_PIN); // Đọc giá trị từ cổng analog
  float voltage = analogValue * (3.3 / 4095.0); // Chuyển đổi giá trị analog sang điện áp
  float pH_value = 7 - (voltage - 2.5) * 3; // Công thức tính pH tùy thuộc vào cảm biến
  return pH_value;
}
// Hàm đọc giá trị từ cảm biến TDS
float readTDS() {
  int analogValue = analogRead(TDS_PIN);
  float voltage = analogValue * (3.3 / 4095.0);  // ESP32 dùng 3.3V
  float tdsValue = (133.42 * voltage * voltage * voltage - 255.86 * voltage * voltage + 857.39 * voltage) * 0.5;
  return tdsValue;
}

// Hàm đọc giá trị từ cảm biến độ đục
float readTurbidity() {
  int analogValue = analogRead(TURBIDITY_PIN);
  float voltage = analogValue * (3.3 / 4095.0);
  float turbidityValue = (voltage / 3.3) * 100;  // Giá trị độ đục tính phần trăm
  return turbidityValue;
}
