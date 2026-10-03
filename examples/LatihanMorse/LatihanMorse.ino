// Latihan Morse: huruf acak muncul di Serial Monitor (115200), lalu ketuk
// sandinya dengan tombol. Benar atau salah langsung ditampilkan beserta skor.
//
// Sambungan: tombol antara pin 2 dan GND, buzzer aktif di pin 8 (opsional)
// untuk mendengar ketukan sendiri.
#include <SandiMorse.h>

const uint8_t PIN_TOMBOL = 2;
const uint8_t PIN_BUZZER = 8;
const bool TAMPILKAN_SANDI = true; // false jika sudah hafal

PembacaMorse pembaca(8); // pemula: 8 WPM, titik 150 ms
char soal;
uint16_t benar = 0, jumlah = 0;

void soalBaru() {
  soal = 'A' + random(26);
  Serial.print("\nKetuk huruf ");
  Serial.print(soal);
  if (TAMPILKAN_SANDI) {
    char kunci[2] = {soal, '\0'};
    char sandi[8];
    ubahKeMorse(kunci, sandi);
    Serial.print("  (");
    Serial.print(sandi);
    Serial.print(")");
  }
  Serial.print(": ");
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_TOMBOL, INPUT_PULLUP);
  pinMode(PIN_BUZZER, OUTPUT);
  randomSeed(analogRead(A0)); // pin analog yang tidak tersambung
  soalBaru();
}

void loop() {
  bool ditekan = digitalRead(PIN_TOMBOL) == LOW;
  digitalWrite(PIN_BUZZER, ditekan);

  if (!pembaca.perbarui(ditekan) || pembaca.huruf() == ' ') return;

  jumlah++;
  Serial.print(pembaca.huruf());
  if (pembaca.huruf() == soal) {
    benar++;
    Serial.print("  Benar!");
  } else {
    Serial.print("  Salah.");
  }
  Serial.print("  Skor ");
  Serial.print(benar);
  Serial.print("/");
  Serial.println(jumlah);
  soalBaru();
}
