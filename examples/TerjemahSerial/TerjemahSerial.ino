// Penerjemah di Serial Monitor (115200, akhiri dengan Enter):
//   ketik teks         -> muncul sandi Morse,  contoh: SOS     -> ... --- ...
//   ketik sandi Morse  -> muncul teks,         contoh: .- -... -> AB
// Huruf dipisah spasi, kata dipisah '/'.
#include <SandiMorse.h>

char masukan[64];
char hasil[160];
uint8_t panjang = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("Ketik teks atau sandi Morse lalu Enter.");
}

void loop() {
  while (Serial.available()) {
    char c = Serial.read();
    if (c != '\n' && c != '\r') {
      if (panjang < sizeof(masukan) - 1) masukan[panjang++] = c;
      continue;
    }
    if (!panjang) continue;
    masukan[panjang] = '\0';
    panjang = 0;

    // Diawali titik atau garis berarti sandi Morse.
    bool sandi = masukan[0] == '.' || masukan[0] == '-';
    bool muat = sandi ? ubahKeTeks(masukan, hasil) : ubahKeMorse(masukan, hasil);
    Serial.println(hasil);
    if (!muat) Serial.println("(terpotong, buffer hasil kurang besar)");
  }
}
