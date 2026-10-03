// Benchmark SandiMorse di ATmega328P 16 MHz (simavr). Cara menjalankan dan
// angka hasilnya: README bagian "Kecepatan & memori".
// Siklus.h: Timer1 tanpa prescaler, UKUR(nama, ulang, kode) mencetak
// "BENCH nama siklus_per_panggilan". millis() berhenti selama UKUR, jadi
// perbarui() diukur di tengah satu fase (jalur yang paling sering).
#include <SandiMorse.h>
#include "Siklus.h"

SandiMorse pengirim(13, 20);
PembacaMorse pembaca(10);
char buffer[64];
volatile bool hasil, tombol;

void setup() {
  Serial.begin(115200);
  pengirim.mulai();
  pengirim.kirim("SOS");
  Serial.print(F("BENCH sizeof_SandiMorse "));
  Serial.println(sizeof(SandiMorse));
  Serial.print(F("BENCH sizeof_PembacaMorse "));
  Serial.println(sizeof(PembacaMorse));
  UKUR("kirim_perbarui", 1000, hasil = pengirim.perbarui());
  UKUR("baca_perbarui_lepas", 1000, hasil = pembaca.perbarui(tombol));
  tombol = true;
  pembaca.perbarui(tombol);
  UKUR("baca_perbarui_ditekan", 1000, hasil = pembaca.perbarui(tombol));
  UKUR("ubahKeMorse_SOS_KU", 100, hasil = ubahKeMorse("SOS KU", buffer));
  UKUR("ubahKeTeks_SOS_KU", 100, hasil = ubahKeTeks("... --- ... / -.- ..-", buffer));
  selesai();
}

void loop() {}
