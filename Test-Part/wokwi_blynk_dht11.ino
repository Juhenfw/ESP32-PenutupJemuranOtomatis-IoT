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

// Konfigurasi Koneksi Jaringan
const char* ssid = "Finnzy";       // Nama WiFi
const char* pass = "Keluargasantuy";    // Password WiFi
const char* auth = BLYNK_AUTH_TOKEN;    // Token Blynk

// Konfigurasi Sensor DHT
#define DHTPIN 13                        // Pin GPIO untuk sensor DHT
#define DHTTYPE DHT11                   // Tipe sensor DHT
DHT dht(DHTPIN, DHTTYPE);               // Inisialisasi sensor DHT

// Inisialisasi Blynk Timer
BlynkTimer timer;

// Variabel Global untuk Monitoring
float temperatureValue = 0.0;
float humidityValue = 0.0;
int connectionAttempts = 0;
bool isBlynkConnected = false;

// Fungsi Kirim Data Sensor
void sendSensorData() {
  // Cek koneksi Blynk sebelum mengirim data
  if (!Blynk.connected()) {
    Serial.println("[BLYNK] Tidak terhubung. Melewati pengiriman data.");
    return;
  }

  // Baca data sensor
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

// Fungsi Pengecekan Koneksi
void checkConnection() {
  // Cek koneksi WiFi
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[WIFI] Koneksi WiFi terputus. Mencoba reconnect...");
    WiFi.reconnect();
  }

  // Cek koneksi Blynk
  if (!Blynk.connected()) {
    Serial.println("[BLYNK] Koneksi terputus. Mencoba reconnect...");
    
    // Tampilkan status koneksi
    if (WiFi.status() == WL_CONNECTED) {
      Blynk.connect();
    } else {
      Serial.println("[FATAL] WiFi tidak terhubung. Tidak dapat reconnect Blynk.");
    }
  }
}

// Fungsi Setup Awal
void setup() {
  // Inisialisasi Komunikasi Serial
  Serial.begin(115200);
  delay(100);

  // Pesan Awal
  Serial.println("\n===== SISTEM MONITORING SUHU =====");
  Serial.println("Memulai Inisialisasi Sistem...");

  // Konfigurasi WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);

  // Tunggu Koneksi WiFi
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    connectionAttempts++;

    // Batasi percobaan koneksi
    if (connectionAttempts > 20) {
      Serial.println("\n[FATAL] Gagal terhubung ke WiFi!");
      break;
    }
  }

  // Informasi Koneksi
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n[SUKSES] Terhubung ke WiFi");
    Serial.print("Alamat IP: ");
    Serial.println(WiFi.localIP());
  }

  // Inisialisasi Sensor DHT
  dht.begin();

  // Koneksi Blynk
  Blynk.begin(auth, ssid, pass);

  // Set Interval Pengiriman Data dan Pengecekan Koneksi
  timer.setInterval(5000L, sendSensorData);   // Kirim data setiap 5 detik
  timer.setInterval(10000L, checkConnection); // Cek koneksi setiap 10 detik
}

// Fungsi Loop Utama
void loop() {
  // Jalankan Blynk
  Blynk.run();
  
  // Jalankan Timer
  timer.run();
}

// BLYNK VIRTUAL PIN HANDLER (Opsional)
BLYNK_WRITE(V3) {
  int pinValue = param.asInt(); 
  Serial.print("[BLYNK] Nilai dari V3: ");
  Serial.println(pinValue);
}