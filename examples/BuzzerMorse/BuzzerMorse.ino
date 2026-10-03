// Ketik teks di Serial Monitor (115200, akhiri dengan Enter), lalu teks itu
// dibunyikan sebagai Morse di buzzer pasif.
//
// Sambungan: buzzer pasif antara pin 8 dan GND.
// Buzzer aktif (berbunyi sendiri saat diberi HIGH)? Pakai SandiMorse morse(8, 20);
// dan hapus bagian tone()/noTone().
#include <SandiMorse.h>

const uint8_t PIN_BUZZER = 8;
const unsigned int NADA = 700; // Hz, nada CW yang umum 600-800 Hz

SandiMorse morse; // tanpa pin: bunyi diatur sendiri lewat nyala()
char ketikan[64]; // teks yang sedang diketik
char dikirim[64]; // teks yang sedang dibunyikan (kirim() tidak menyalin teks)
uint8_t panjang = 0;

void setup() {
  Serial.begin(115200);
  morse.aturWPM(15);
  Serial.println("Ketik teks lalu Enter.");
}

void loop() {
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') {
      if (!panjang) continue;
      ketikan[panjang] = '\0';
      strcpy(dikirim, ketikan);
      panjang = 0;
      Serial.print("Membunyikan: ");
      Serial.println(dikirim);
      if (!morse.kirim(dikirim)) Serial.println("Tidak ada huruf yang bisa dikirim.");
    } else if (panjang < sizeof(ketikan) - 1) {
      ketikan[panjang++] = c;
    }
  }

  // tone() hanya dipanggil saat status berubah, bukan di setiap loop().
  static bool bunyi = false;
  if (morse.perbarui() != bunyi) {
    bunyi = morse.nyala();
    if (bunyi) tone(PIN_BUZZER, NADA);
    else noTone(PIN_BUZZER);
  }
}
