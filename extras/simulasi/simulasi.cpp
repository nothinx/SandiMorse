// Simulasi SandiMorse di PC: memakai kode library asli (../../src) dengan
// millis() palsu dari ../test/Arduino.h, dan mencetak data untuk gambar.py.
//   g++ -std=c++11 -O2 -I../test -I../../src simulasi.cpp ../../src/*.cpp -o sim && ./sim
// Keluaran: bagian diawali "# nama", lalu baris CSV.
#include <stdio.h>
#include <string.h>
#include "SandiMorse.h"

uint32_t waktuPalsu = 0;
int pinPalsu = -1;

// Acak sederhana dengan seed tetap (LCG Numerical Recipes), hasil sama di semua PC.
static uint32_t benih = 2026;
static float acak(float a, float b) {
  benih = benih * 1664525u + 1013904223u;
  return a + (b - a) * (benih >> 8) / 16777216.0f;
}

int main() {
  // --- Pengirim: "SOS SOS" pada 20 WPM, nyala() dibaca tiap 1 ms ---
  const char *KIRIM = "SOS SOS";
  SandiMorse morse; // tanpa pin, default 20 WPM
  printf("# kirim\nwpm,unit_ms,teks\n20,%u,%s\n", morse.durasiUnit(), KIRIM);
  printf("# fase\nms,nyala\n");
  waktuPalsu = 0;
  morse.kirim(KIRIM);
  bool status = morse.nyala();
  printf("0,%d\n", status);
  while (morse.sedangMengirim()) {
    waktuPalsu++;
    morse.perbarui();
    if (morse.nyala() != status) printf("%u,%d\n", waktuPalsu, status = morse.nyala());
  }
  printf("%u,%d\n", waktuPalsu, morse.nyala()); // akhir jeda 7 unit

  // --- Pembaca: ketukan manusia yang tidak rapi, 10 WPM (unit 120 ms) ---
  // Tiap durasi diacak di dalam toleransi pembaca (batas di tengah, ±1 unit).
  const char *KETUK = ".... .- .-.. --- / -.. ..- -. .. .-"; // HALO DUNIA
  PembacaMorse pembaca; // default 10 WPM
  const float u = pembaca.durasiUnit();
  printf("# ketuk\nunit_ms,pola\n%.0f,%s\n", u, KETUK);
  // Jadwal tekan/lepas dalam ms.
  uint32_t mulai[64], lama[64];
  int n = 0;
  float t = 500;
  for (const char *p = KETUK; *p; p++) {
    if (*p == '.' || *p == '-') {
      float d = *p == '.' ? acak(0.6f, 1.6f) : acak(2.4f, 4.0f);
      mulai[n] = (uint32_t)t;
      lama[n] = (uint32_t)(d * u);
      t += lama[n++];
      char s = p[1];
      t += (s == '.' || s == '-') ? acak(0.6f, 1.6f) * u : 0; // jeda antarsimbol
    } else if (*p == ' ' && p[1] != '/' && (p == KETUK || p[-1] != '/')) {
      t += acak(2.4f, 4.4f) * u; // jeda antarhuruf
    } else if (*p == '/') {
      t += acak(5.5f, 8.0f) * u; // jeda antarkata
    }
  }
  printf("# tekan\nmulai_ms,lama_ms\n");
  for (int i = 0; i < n; i++) printf("%u,%u\n", mulai[i], lama[i]);
  printf("# huruf\nms,huruf\n");
  int k = 0;
  for (waktuPalsu = 0; waktuPalsu < (uint32_t)t + 10 * u; waktuPalsu++) {
    bool ditekan = k < n && waktuPalsu >= mulai[k] && waktuPalsu < mulai[k] + lama[k];
    if (k < n && waktuPalsu >= mulai[k] + lama[k]) k++;
    if (pembaca.perbarui(ditekan)) printf("%u,%c\n", waktuPalsu, pembaca.huruf() == ' ' ? '_' : pembaca.huruf());
  }

  // --- Penerjemah teks <-> Morse ---
  printf("# teks\n");
  const char *TEKS[] = {"SOS", "Halo Dunia", "Jam 07:30, OK?"};
  char buf[128];
  for (const char *s : TEKS) {
    ubahKeMorse(s, buf);
    printf("ubahKeMorse(\"%s\") -> \"%s\"\n", s, buf);
  }
  const char *MORSE[] = {"... --- ...", ".- .-. -.. ..- .. -. --- / ..---", "-- --- .-. ... . / ........"};
  for (const char *s : MORSE) {
    ubahKeTeks(s, buf);
    printf("ubahKeTeks(\"%s\") -> \"%s\"\n", s, buf);
  }
  return 0;
}
