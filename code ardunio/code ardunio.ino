#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <Servo.h>

// --- KHAI BÁO CHÂN ---
#define DHTPIN        D4
#define DHTTYPE       DHT22

#define GAS_PIN       A0
#define RAIN_PIN      D6
#define BUZZER_PIN    D7
#define SERVO_PIN     D5

DHT dht(DHTPIN, DHTTYPE);
Servo servo;

// --- NGƯỠNG ---
const int gasThreshold = 250;

// --- WiFi & MQTT ---
const char* ssid = "Fablab 2.4G";
const char* password = "Fira@2024";
const char* mqtt_server = "192.168.69.69";

WiFiClient espClient;
PubSubClient client(espClient);

// --- Biến trạng thái ---
bool isGasDetected = false;
unsigned long gasDetectedTime = 0;
bool buzzerOn = false;
bool doorOpened = false;

int rainCounter = 0;

void setup_wifi() {
  delay(100);
  Serial.println();
  Serial.print("Đang kết nối WiFi: ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\n✅ Kết nối WiFi thành công!");
  Serial.print("Địa chỉ IP: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Đang kết nối MQTT... ");
    String clientId = "NodeMCU-" + String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println("✅ Đã kết nối MQTT!");
    } else {
      Serial.print("❌ Kết nối MQTT thất bại. Lỗi: ");
      Serial.print(client.state());
      Serial.println(". Thử lại sau 5s...");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  pinMode(RAIN_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  analogWrite(BUZZER_PIN, 0);

  servo.attach(SERVO_PIN);
  servo.write(0); // Đóng cửa ban đầu

  setup_wifi();
  client.setServer(mqtt_server, 1884);

  Serial.println("🚀 Hệ thống NodeMCU giám sát môi trường khởi động...");
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  float temp = dht.readTemperature();
  float humi = dht.readHumidity();
  int gasValue = analogRead(GAS_PIN);
  bool rainDetected = digitalRead(RAIN_PIN) == LOW;
  unsigned long currentMillis = millis();

  Serial.println("------------");
  Serial.printf("Nhiệt độ: %.2f °C\n", temp);
  Serial.printf("Độ ẩm: %.2f %%\n", humi);
  Serial.printf("Gas: %d\n", gasValue);
  Serial.printf("Mưa: %s\n", rainDetected ? "Có" : "Không");

  // --- Xử lý khí gas ---
  if (gasValue > gasThreshold && !isGasDetected) {
    Serial.println("⚠️ Phát hiện khí gas! Bật còi và mở cửa.");
    gasDetectedTime = currentMillis;
    isGasDetected = true;

    // Bật còi 5 giây
    analogWrite(BUZZER_PIN, 100);
    buzzerOn = true;
    client.publish("node/buzzer", "on");

    // Mở cửa
    servo.write(170);
    doorOpened = true;
    client.publish("node/door", "open");
  }

  // Sau 5s thì tắt còi và đóng cửa nếu đã bật
  if (isGasDetected && (currentMillis - gasDetectedTime >= 5000)) {
    if (buzzerOn) {
      Serial.println("🔕 Tắt còi sau 5 giây.");
      analogWrite(BUZZER_PIN, 0);
      buzzerOn = false;
      client.publish("node/buzzer", "off");
    }

    if (doorOpened) {
      Serial.println("🚪 Đóng cửa sau 5 giây.");
      servo.write(0);
      doorOpened = false;
      client.publish("node/door", "closed");
    }

    isGasDetected = false;
  }

  // Gửi dữ liệu môi trường
  client.publish("node/temp", String(temp).c_str());
  client.publish("node/humi", String(humi).c_str());
  client.publish("node/gas", String(gasValue).c_str());

  // Gửi trạng thái mưa
  if (rainDetected) {
    rainCounter++;
    client.publish("node/rain", String(rainCounter).c_str());
  } else {
    rainCounter = 0;
    client.publish("node/rain", "0");
  }

  delay(1000);
}
