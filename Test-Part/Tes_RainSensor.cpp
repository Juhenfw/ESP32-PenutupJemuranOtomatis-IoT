#define sensor_hujan 4  // Gantilah dengan pin GPIO yang sesuai dengan pin sensor hujan di ESP32

int read_hujan = 0;
int hold = 0;

void setup() {
  pinMode(sensor_hujan, INPUT);  // Set pin sensor hujan sebagai input
  Serial.begin(9600);  // Mulai komunikasi serial
}

void loop() {
  read_hujan = digitalRead(sensor_hujan);  // Baca status sensor hujan
  Serial.print("Status sensor hujan: ");
  Serial.println(read_hujan);  // Debug output status sensor

  if (read_hujan == 0 && hold == 0) {  // Jika sensor hujan terdeteksi (LOW)
    Serial.println("Hujan");
    hold = 1;  // Set hold untuk mencegah pengulangan
  } 
  else if (read_hujan == 1 && hold == 1) {  // Jika sensor hujan tidak terdeteksi (HIGH)
    Serial.println("Cerah");
    hold = 0;  // Reset hold
  }

  delay(1000);  // Tunggu 1 detik untuk membaca lagi
}
