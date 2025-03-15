// Definisi Template dan Konfigurasi Blynk
#define BLYNK_TEMPLATE_ID "TMPL6puUJJ4Ab"
#define BLYNK_TEMPLATE_NAME "ESP32 Monitoring Suhu"
#define BLYNK_AUTH_TOKEN "hvrrIDwfozLmMqyB_iT7fapYCi6swfu7"

// Konfigurasi Debug dan Koneksi
#define BLYNK_PRINT Serial
#define BLYNK_DEBUG

// Library yang Dibutuhkan
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <Wire.h>
#include <BH1750.h>

// Konfigurasi Koneksi Jaringan
const char* ssid = "Finnzy";       // Nama WiFi
const char* pass = "Keluargasantuy";    // Password WiFi
const char* auth = BLYNK_AUTH_TOKEN;    // Token Blynk

// Konfigurasi Sensor DHT
#define DHTPIN 13                        // Pin GPIO untuk sensor DHT
#define DHTTYPE DHT11                   // Tipe sensor DHT
DHT dht(DHTPIN, DHTTYPE);               // Inisialisasi sensor DHT

// Konfigurasi Sensor Hujan
#define sensor_hujan 4  // Gantilah dengan pin GPIO yang sesuai dengan pin sensor hujan di ESP32

// Inisialisasi Sensor Cahaya (BH1750)
BH1750 lightMeter;

// Inisialisasi Blynk Timer
BlynkTimer timer;

// Variabel Global untuk Monitoring
float temperatureValue = 0.0;
float humidityValue = 0.0;
int connectionAttempts = 0;
bool isBlynkConnected = false;
int read_hujan = 0;
int hold = 0;  // Variabel untuk menahan status hujan agar tidak berubah terlalu cepat
unsigned long lastRainStatusChange = 0; // Variabel untuk memastikan perubahan status tidak terlalu cepat

// Fungsi Kirim Data Sensor Suhu dan Kelembaban
void sendSensorData() {
  if (!Blynk.connected()) {
    Serial.println("[BLYNK] Tidak terhubung. Melewati pengiriman data.");
    return;
  }

  // Baca data sensor suhu dan kelembaban
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  // Validasi Pembacaan Sensor
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("[SENSOR ERROR] Gagal membaca data sensor DHT!");
    return;
  }

  // Update Variabel Global
  temperatureValue = temperature;
  humidityValue = humidity;

  // Kirim Data ke Blynk
  Blynk.virtualWrite(V0, temperature);  // Virtual Pin Suhu
  Blynk.virtualWrite(V1, humidity);     // Virtual Pin Kelembaban

  // Cetak Data ke Serial Monitor
  Serial.println("===== DATA SENSOR =====");
  Serial.print("[SUHU] ");
  Serial.print(temperature);
  Serial.println("°C");
  Serial.print("[KELEMBABAN] ");
  Serial.print(humidity);
  Serial.println("%");

  // Notifikasi Suhu Tinggi
  if (temperature > 30) {
    Serial.println("[PERINGATAN] Suhu Melebihi 30°C!");
    Blynk.logEvent("temp_alert", "Suhu Tinggi: " + String(temperature) + "°C");
  }
}

// Fungsi untuk mengirim status sensor hujan
void sendRainStatus() {
  read_hujan = digitalRead(sensor_hujan);  // Baca status sensor hujan
  Serial.print("[STATUS HUJAN] ");
  Serial.println(read_hujan == LOW ? "Hujan" : "Cerah");

  if (read_hujan == LOW && hold == 0) {  // Jika sensor hujan terdeteksi (LOW)
    Serial.println("Hujan");
    Blynk.virtualWrite(V2, "Hujan");  // Kirim status hujan ke Virtual Pin V2
    hold = 1;  // Set hold untuk mencegah pengulangan status
  } 
  else if (read_hujan == HIGH && hold == 1) {  // Jika sensor hujan tidak terdeteksi (HIGH)
    Serial.println("Cerah");
    Blynk.virtualWrite(V2, "Cerah");  // Kirim status cerah ke Virtual Pin V2
    hold = 0;  // Reset hold agar status dapat berubah kembali ketika sensor hujan kering
  }
}


// Fungsi untuk mengirim data cahaya
void sendLightData() {
  float lux = lightMeter.readLightLevel();  // Baca nilai cahaya (lux)
  Serial.print("Light: ");
  Serial.print(lux);
  Serial.println(" lx");

  Blynk.virtualWrite(V3, lux);  // Kirim data cahaya ke Virtual Pin V3
}

// Fungsi Pengecekan Koneksi
void checkConnection() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[WIFI] Koneksi WiFi terputus. Mencoba reconnect...");
    WiFi.reconnect();
  }

  if (!Blynk.connected()) {
    Serial.println("[BLYNK] Koneksi terputus. Mencoba reconnect...");
    if (WiFi.status() == WL_CONNECTED) {
      Blynk.connect();
    } else {
      Serial.println("[FATAL] WiFi tidak terhubung. Tidak dapat reconnect Blynk.");
    }
  }
}

// Fungsi Setup Awal
void setup() {
  Serial.begin(115200);
  delay(100);

  Serial.println("\n===== SISTEM MONITORING =====");
  Serial.println("Memulai Inisialisasi Sistem...");

  // Konfigurasi WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    connectionAttempts++;

    if (connectionAttempts > 20) {
      Serial.println("\n[FATAL] Gagal terhubung ke WiFi!");
      break;
    }
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n[SUKSES] Terhubung ke WiFi");
    Serial.print("Alamat IP: ");
    Serial.println(WiFi.localIP());
  }

  // Inisialisasi Sensor DHT
  dht.begin();
  
  // Inisialisasi Sensor Cahaya
  Wire.begin();  // Inisialisasi I2C
  lightMeter.begin();  // Inisialisasi BH1750

  // Inisialisasi Sensor Hujan
  pinMode(sensor_hujan, INPUT);

  // Koneksi Blynk
  Blynk.begin(auth, ssid, pass);

  // Set Interval Pengiriman Data
  timer.setInterval(2000L, sendSensorData);    // Kirim data suhu dan kelembaban setiap 2 detik
  timer.setInterval(2000L, sendRainStatus);    // Kirim status hujan setiap 2 detik
  timer.setInterval(2000L, sendLightData);     // Kirim data cahaya setiap 2 detik
  timer.setInterval(5000L, checkConnection);   // Cek koneksi setiap 5 detik
}

// Fungsi Loop Utama
void loop() {
  Blynk.run();
  timer.run();
}
