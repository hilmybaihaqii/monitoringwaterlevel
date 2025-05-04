#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#define TRIG_PIN 19
#define ECHO_PIN 18
#define BUZZER_PIN 13
#define LED_PIN 2
#define PUMP_PIN 32

// WiFi credentials
const char* ssid = "Iphone";
const char* password = "bayarduaribu";

// Server configuration
const char* serverUrl = "http://192.168.1.90:5000/api/esp32/data";  // Ganti dengan IP server Anda

// Waktu antara pengiriman data (milliseconds)
const unsigned long SEND_INTERVAL = 1000;  // Mengurangi interval pengiriman data menjadi lebih cepat
unsigned long lastSendTime = 0;

// Variabel untuk kontrol non-blocking buzzer
unsigned long lastBuzzerTime = 0;
const unsigned long BUZZER_INTERVAL = 1000; // Interval untuk mengaktifkan buzzer

void setup() {
  Serial.begin(115200);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(PUMP_PIN, OUTPUT);

  // Memastikan pompa dimatikan pada awalnya
  digitalWrite(PUMP_PIN, LOW);

  // Koneksi ke WiFi
  WiFi.begin(ssid, password);
  Serial.println("Menghubungkan ke WiFi...");
  
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
    delay(500);
    Serial.print(".");
  }
  
  digitalWrite(LED_PIN, HIGH); // LED menyala saat terhubung
  Serial.println("\nTerhubung ke WiFi!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  unsigned long currentMillis = millis();
  
  // Baca sensor dan kirim data setiap interval waktu
  if (currentMillis - lastSendTime >= SEND_INTERVAL) {
    lastSendTime = currentMillis;
    
    // Ukur jarak dari sensor
    float distance = measureDistance();
    Serial.print("Jarak terukur: ");
    Serial.print(distance);
    Serial.println(" cm");
    
    // Periksa status WiFi dan kirim data jika terhubung
    if (WiFi.status() == WL_CONNECTED) {
      sendDataToServer(distance);
    } else {
      Serial.println("WiFi terputus! Mencoba menghubungkan kembali...");
      WiFi.reconnect();
    }
    
    // Aktifkan buzzer sesuai level
    activateBuzzerBasedOnLevel(distance);

    // Kontrol pompa berdasarkan level air
    controlPumpBasedOnLevel(distance);
  }
}

float measureDistance() {
  // Clear the trigger pin
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  
  // Set trigger pin HIGH untuk 10 microseconds
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  // Baca waktu echo
  long duration = pulseIn(ECHO_PIN, HIGH);
  
  // Hitung jarak (kecepatan suara = 343m/s = 0.0343cm/μs)
  // Jarak = waktu * kecepatan / 2 (pergi-balik)
  float distance = duration * 0.0343 / 2;
  
  // Validasi jarak dalam rentang yang masuk akal
  if (distance < 5 || distance > 400) {
    distance = -1;  // Nilai negatif untuk menunjukkan pembacaan yang tidak valid
  }
  
  return distance;
}

void sendDataToServer(float distance) {
  HTTPClient http;
  
  // Siapkan JSON document
  StaticJsonDocument<200> jsonDoc;
  jsonDoc["distance"] = distance;
  
  // Tambahkan info perangkat
  JsonObject deviceInfo = jsonDoc.createNestedObject("device");
  deviceInfo["id"] = WiFi.macAddress();
  deviceInfo["ip"] = WiFi.localIP().toString();
  deviceInfo["rssi"] = WiFi.RSSI(); // Kekuatan sinyal WiFi
  
  String jsonPayload;
  serializeJson(jsonDoc, jsonPayload);
  
  // Kirim HTTP POST request
  http.begin(serverUrl);
  http.addHeader("Content-Type", "application/json");
  
  Serial.println("Mengirim data ke server: " + jsonPayload);
  int httpResponseCode = http.POST(jsonPayload);
  
  // Mengecek apakah data berhasil terkirim ke server
  if (httpResponseCode == 200) {
    Serial.println("Data berhasil dikirim ke server!");
    String response = http.getString();
    Serial.println("Response: " + response);
    
    // Visual feedback untuk berhasil
    digitalWrite(LED_PIN, LOW);
    delay(100);
    digitalWrite(LED_PIN, HIGH);
  } else {
    Serial.print("Gagal mengirim data. Error code: ");
    Serial.println(httpResponseCode);
    
    // Visual feedback untuk error
    for (int i = 0; i < 3; i++) {
      digitalWrite(LED_PIN, LOW);
      delay(100);
      digitalWrite(LED_PIN, HIGH);
      delay(100);
    }
  }
  
  http.end();
}

void activateBuzzerBasedOnLevel(float distance) {
  // Konfigurasi ambang batas - sesuaikan dengan pengaturan server
  const float DANGER_LEVEL = 20;  // 20cm
  const float WARNING_LEVEL = 30; // 30cm
  
  unsigned long currentMillis = millis();
  
  // Aktifkan buzzer dengan interval non-blocking
  if (distance <= DANGER_LEVEL) {
    if (currentMillis - lastBuzzerTime >= BUZZER_INTERVAL) {
      lastBuzzerTime = currentMillis;
      Serial.println("BAHAYA: Level air sangat tinggi!");
      tone(BUZZER_PIN, 3000); 
      delay(100);               // Tunda sebentar
      noTone(BUZZER_PIN);
    }
  } else if (distance <= WARNING_LEVEL) {
    if (currentMillis - lastBuzzerTime >= BUZZER_INTERVAL) {
      lastBuzzerTime = currentMillis;
      Serial.println("PERINGATAN: Level air tinggi!");
      tone(BUZZER_PIN, 3000); 
      delay(300);
      noTone(BUZZER_PIN);
    }
  }
}

void controlPumpBasedOnLevel(float distance) {
  const float STOP_LEVEL = 40;  // 40cm - pompa berhenti
  const float START_LEVEL = 20; // 30cm - pompa mulai

  if (distance <= START_LEVEL) {
    // Pompa menyala jika level air mencapai 30cm atau lebih rendah
    Serial.println("Pompa menyala!");
    digitalWrite(PUMP_PIN,LOW);
  } else if (distance >= STOP_LEVEL) {
    // Pompa mati jika level air mencapai 40cm atau lebih tinggi
    Serial.println("Pompa mati!");
    digitalWrite(PUMP_PIN,HIGH);
  }
}