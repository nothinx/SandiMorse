// LED bawaan berkedip SOS terus-menerus tanpa delay().
// loop() tetap bebas untuk pekerjaan lain.
#include <SandiMorse.h>

#ifndef LED_BUILTIN
#define LED_BUILTIN 2 // ESP32 DevKit
#endif

SandiMorse morse(LED_BUILTIN, 15); // 15 WPM

void setup() {
  morse.mulai();
}

void loop() {
  morse.perbarui();
  // Jeda 7 unit setelah huruf terakhir sudah termasuk, jadi SOS berulang tetap berjarak.
  if (!morse.sedangMengirim()) morse.kirim("SOS");
}
