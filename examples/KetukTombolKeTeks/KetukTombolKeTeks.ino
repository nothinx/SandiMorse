// Ketuk Morse dengan tombol, hurufnya muncul di Serial Monitor (115200).
// Tekan singkat = titik, tekan lama = garis. Diam sebentar = huruf selesai,
// diam lebih lama = spasi.
//
// Sambungan: tombol antara pin 2 dan GND (tanpa resistor).
// LED bawaan menyala selama tombol ditekan.
#include <SandiMorse.h>

#ifndef LED_BUILTIN
#define LED_BUILTIN 2 // ESP32 DevKit
#endif

const uint8_t PIN_TOMBOL = 2;

PembacaMorse pembaca(10); // 10 WPM: titik 120 ms, garis 360 ms

void setup() {
  Serial.begin(115200);
  pinMode(PIN_TOMBOL, INPUT_PULLUP);
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.println("Mulai mengetuk. Terlalu cepat/lambat? Ubah angka WPM.");
}

void loop() {
  bool ditekan = digitalRead(PIN_TOMBOL) == LOW;
  digitalWrite(LED_BUILTIN, ditekan);

  if (pembaca.perbarui(ditekan)) Serial.print(pembaca.huruf());
}
