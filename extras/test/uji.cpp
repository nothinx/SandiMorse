// Uji logika SandiMorse di PC:
//   g++ -std=c++11 -Wall -Wextra -I. -I../../src uji.cpp ../../src/*.cpp -o uji && ./uji
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "SandiMorse.h"

uint32_t waktuPalsu = 1000;
int pinPalsu = -1;

// Menjalankan pengirim tiap 1 ms sampai selesai, mencatat lama setiap fase
// nyala/mati secara berurutan. Mengembalikan jumlah fase.
static int rekam(SandiMorse &m, uint32_t *fase, int maks) {
  int n = 0;
  uint32_t awal = millis();
  bool status = m.nyala();
  assert(pinPalsu == status);
  for (int i = 0; i < 100000 && m.sedangMengirim(); i++) {
    waktuPalsu++;
    m.perbarui();
    assert(pinPalsu == m.nyala());
    if (m.nyala() != status || !m.sedangMengirim()) {
      assert(n < maks);
      fase[n++] = millis() - awal;
      awal = millis();
      status = m.nyala();
    }
  }
  return n;
}

// Pembaca: tekan selama ms lalu lepas selama ms, perbarui() tiap 1 ms.
// Huruf yang muncul ditambahkan ke hasil.
static char hasil[64];
static void ketuk(PembacaMorse &p, bool ditekan, uint32_t ms) {
  for (uint32_t i = 0; i < ms; i++) {
    if (p.perbarui(ditekan)) {
      size_t n = strlen(hasil);
      hasil[n] = p.huruf();
      hasil[n + 1] = '\0';
    }
    waktuPalsu++;
  }
}
static void ketukKode(PembacaMorse &p, const char *kode, uint32_t titik, uint32_t garis, uint32_t jeda) {
  for (; *kode; kode++) {
    if (*kode == ' ') { ketuk(p, false, jeda * 2); continue; } // jeda huruf = 3 x jeda simbol
    ketuk(p, true, *kode == '.' ? titik : garis);
    ketuk(p, false, jeda);
  }
}

int main() {
  const char *semua = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.,?'!/()&:;=+-_\"$@";
  char buf[64], balik[64];

  { // tabel lengkap bolak-balik, huruf kecil juga
    assert(strlen(semua) == 54);
    for (const char *c = semua; *c; c++) {
      char satu[2] = {*c, '\0'};
      assert(ubahKeMorse(satu, buf) && strlen(buf) >= 1);
      assert(ubahKeTeks(buf, balik) && balik[0] == *c && balik[1] == '\0');
    }
    for (char c = 'a'; c <= 'z'; c++) {
      char satu[2] = {c, '\0'};
      ubahKeMorse(satu, buf);
      ubahKeTeks(buf, balik);
      assert(balik[0] == c - 32);
    }
    // semua kode berbeda (tidak ada dua karakter dengan kode sama)
    for (const char *a = semua; *a; a++)
      for (const char *b = a + 1; *b; b++) {
        char sa[2] = {*a, 0}, sb[2] = {*b, 0}, ka[16], kb[16];
        ubahKeMorse(sa, ka);
        ubahKeMorse(sb, kb);
        assert(strcmp(ka, kb) != 0);
      }
  }
  { // contoh dari tabel ITU
    ubahKeMorse("A", buf); assert(!strcmp(buf, ".-"));
    ubahKeMorse("0", buf); assert(!strcmp(buf, "-----"));
    ubahKeMorse("?", buf); assert(!strcmp(buf, "..--.."));
    ubahKeMorse("$", buf); assert(!strcmp(buf, "...-..-")); // 7 simbol, terpanjang
    ubahKeMorse("_", buf); assert(!strcmp(buf, "..--.-"));
    assert(ubahKeMorse("SOS ku", buf) && !strcmp(buf, "... --- ... / -.- ..-"));
    assert(ubahKeTeks(buf, balik) && !strcmp(balik, "SOS KU"));
    assert(ubahKeMorse("  A  \n B ", buf) && !strcmp(buf, ".- / -..."));
  }
  { // karakter dan kode tidak dikenal
    assert(ubahKeMorse("A#B", buf) && !strcmp(buf, ".- -..."));
    assert(ubahKeMorse("#%", buf) && !strcmp(buf, ""));
    assert(ubahKeMorse(nullptr, buf) && !strcmp(buf, ""));
    assert(ubahKeTeks("........", buf) && !strcmp(buf, "*"));  // 8 simbol
    assert(ubahKeTeks("..--", buf) && !strcmp(buf, "*"));      // tidak ada di tabel
    assert(ubahKeTeks(".-x ...", buf) && !strcmp(buf, "*S")); // karakter asing
    assert(ubahKeTeks("/ ... // / --- /", buf) && !strcmp(buf, "S O"));
    assert(ubahKeTeks("...   ---", buf) && !strcmp(buf, "SO"));
    assert(ubahKeTeks("", buf) && !strcmp(buf, ""));
  }
  { // buffer kecil: false, isi terpotong tapi tetap diakhiri '\0'
    char b4[4], b2[2], b1[1];
    assert(!ubahKeMorse("SOS", b4) && !strcmp(b4, "..."));
    assert(ubahKeMorse("E", b2) && !strcmp(b2, "."));
    assert(!ubahKeMorse("E", b1) && b1[0] == '\0');
    assert(!ubahKeMorse("E", b4, 0));
    assert(!ubahKeTeks("... ---", b2) && !strcmp(b2, "S"));
    assert(ubahKeTeks("... ---", b4) && !strcmp(b4, "SO"));
  }

  { // timing pengirim pada 20 WPM: 1 unit = 1200 / 20 = 60 ms
    SandiMorse m(13, 20);
    m.mulai();
    assert(m.durasiUnit() == 60 && pinPalsu == LOW && !m.sedangMengirim());
    assert(m.kirim("SOS") && m.nyala() && pinPalsu == HIGH);
    uint32_t fase[32];
    const uint32_t harap[] = {
        60, 60, 60, 60, 60, 180,     // S: titik jeda titik jeda titik, jeda huruf
        180, 60, 180, 60, 180, 180,  // O
        60, 60, 60, 60, 60, 420};    // S, lalu jeda kata di akhir
    assert(rekam(m, fase, 32) == 18);
    for (int i = 0; i < 18; i++) assert(fase[i] == harap[i]);
    assert(!m.nyala() && pinPalsu == LOW && !m.perbarui());
  }
  { // spasi = jeda 7 unit, karakter asing dilewati, kiriman bisa diulang
    SandiMorse m(13, 20);
    uint32_t fase[16];
    assert(m.kirim("E e") && rekam(m, fase, 16) == 4);
    assert(fase[0] == 60 && fase[1] == 420 && fase[2] == 60 && fase[3] == 420);
    assert(m.kirim("E#T") && rekam(m, fase, 16) == 4);
    assert(fase[0] == 60 && fase[1] == 180 && fase[2] == 180 && fase[3] == 420);
    assert(!m.kirim("") && !m.kirim("#%^") && !m.kirim(nullptr) && !m.sedangMengirim());
  }
  { // berhenti() di tengah kiriman
    SandiMorse m(13, 20);
    m.kirim("TTT");
    waktuPalsu += 30;
    assert(m.perbarui() && pinPalsu == HIGH);
    m.berhenti();
    assert(!m.nyala() && !m.sedangMengirim() && pinPalsu == LOW);
    waktuPalsu += 1000;
    assert(!m.perbarui() && pinPalsu == LOW);
  }
  { // tanpa pin dan WPM lain; WPM 0 tidak membuat pembagian nol
    pinPalsu = -1;
    SandiMorse m;
    m.mulai();
    m.aturWPM(5);
    assert(m.durasiUnit() == 240);
    assert(m.kirim("T") && m.nyala() && pinPalsu == -1);
    waktuPalsu += 720;
    assert(!m.perbarui() && m.sedangMengirim());
    m.aturWPM(0);
    assert(m.durasiUnit() == 1200);
    pinPalsu = LOW;
  }
  { // millis() meluap di tengah kiriman
    waktuPalsu = 0xFFFFFF80u;
    SandiMorse m(13, 20);
    uint32_t fase[32];
    assert(m.kirim("SOS") && rekam(m, fase, 32) == 18);
    assert(fase[5] == 180 && fase[6] == 180 && fase[17] == 420);
    waktuPalsu = 1000;
  }

  { // pembaca pada 10 WPM (1 unit = 120 ms), ketukan rapi
    PembacaMorse p(10);
    hasil[0] = '\0';
    ketukKode(p, "... --- ...", 120, 360, 120);
    ketuk(p, false, 1000);
    assert(!strcmp(hasil, "SOS ")); // spasi muncul setelah jeda >= 5 unit
  }
  { // ketukan manusia: tidak rata tapi masih dalam toleransi ±1 unit
    PembacaMorse p(10);
    hasil[0] = '\0';
    ketukKode(p, ".-", 180, 260, 200);         // titik 1,5 unit, garis 2,2 unit
    ketuk(p, false, 300);                       // jeda total 4,2 unit -> huruf selesai
    ketukKode(p, "-...", 100, 450, 100);        // titik 0,8 unit, garis 3,8 unit
    ketuk(p, false, 900);                       // jeda 7,5 unit -> spasi
    ketukKode(p, "-", 340, 340, 100);
    ketuk(p, false, 300);
    assert(!strcmp(hasil, "AB T"));
  }
  { // huruf tersedia lewat hurufBaru()/huruf() hanya satu kali
    PembacaMorse p(10);
    ketuk(p, true, 120);
    int baru = 0;
    char h = 0;
    for (int i = 0; i < 1000; i++) {
      waktuPalsu++;
      bool ada = p.perbarui(false);
      assert(p.hurufBaru() == ada);
      if (ada) {
        assert(p.huruf() == (baru ? ' ' : 'E'));
        baru++;
        h = p.huruf();
      }
    }
    assert(baru == 2 && h == ' ' && !p.hurufBaru()); // 'E' lalu spasi
  }
  { // getaran kontak tidak menambah simbol
    PembacaMorse p(10);
    hasil[0] = '\0';
    for (int i = 0; i < 20; i++) ketuk(p, i % 2 == 0, 1); // bergetar 20 ms (< 30 ms)
    ketuk(p, true, 100);
    ketuk(p, false, 1000);
    assert(!strcmp(hasil, "E "));
  }
  { // kode terlalu panjang atau tidak ada di tabel -> '*'
    PembacaMorse p(10);
    hasil[0] = '\0';
    ketukKode(p, "........", 120, 360, 120); // 8 titik
    ketuk(p, false, 300);
    ketukKode(p, "..--", 120, 360, 120);
    ketuk(p, false, 300);
    ketukKode(p, "...-..-", 120, 360, 120); // 7 simbol, '$'
    ketuk(p, false, 300);
    assert(!strcmp(hasil, "**$"));
  }
  { // millis() meluap
    waktuPalsu = 0xFFFFFE00u;
    PembacaMorse p(10);
    hasil[0] = '\0';
    ketukKode(p, "-.-", 120, 360, 120);
    ketuk(p, false, 300);
    assert(!strcmp(hasil, "K"));
  }
  printf("Semua uji lolos (SandiMorse %u byte, PembacaMorse %u byte)\n", (unsigned)sizeof(SandiMorse),
         (unsigned)sizeof(PembacaMorse));
  return 0;
}
