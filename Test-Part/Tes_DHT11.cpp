// Library yang Dibutuhkan
#include <DHT.h>

// Konfigurasi Sensor DHT
#define DHTPIN 13                        // Pin GPIO untuk sensor DHT
#define DHTTYPE DHT11                   // Tipe sensor DHT
DHT dht(DHTPIN, DHTTYPE);               // Inisialisasi sensor DHT

// Fungsi Setup Awal
void setup() {
  // Inisialisasi Komunikasi Serial
  Serial.begin(115200);
  delay(100);

  // Pesan Awal
  Serial.println("\n===== SISTEM MONITORING SUHU =====");
  Serial.println("Memulai Inisialisasi Sensor DHT11...");

  // Inisialisasi Sensor DHT
  dht.begin();
}

// Fungsi Loop Utama
void loop() {
  // Baca data sensor
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  // Validasi Pembacaan Sensor
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("[SENSOR ERROR] Gagal membaca data sensor DHT!");
  } else {
    // Cetak Data ke Serial Monitor
    Serial.println("===== DATA SENSOR =====");
    Serial.print("[SUHU] ");
    Serial.print(temperature);
    Serial.println("°C");
    Serial.print("[KELEMBABAN] ");
    Serial.print(humidity);
    Serial.println("%");
  }

  // Tunggu 2 detik sebelum membaca sensor lagi
  delay(2000);
}
