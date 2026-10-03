// SandiMorse - sandi Morse: kirim teks ke LED/buzzer tanpa delay(), terjemahkan
// teks <-> Morse, dan ubah ketukan tombol menjadi huruf.
// Copyright (c) 2026 Amadeo Wisesa. Lisensi MIT.
//
// - Huruf A-Z, angka 0-9, dan tanda baca . , ? ' ! / ( ) & : ; = + - _ " $ @
//   (tabel 63 byte di PROGMEM).
// - Timing standar PARIS: 1 unit = 1200 / WPM ms. Titik 1 unit, garis 3,
//   jeda antarsimbol 1, antarhuruf 3, antarkata 7.
// - Karakter yang tidak ada di tabel dilewati saat dikirim. Kode Morse yang
//   tidak dikenal diterjemahkan menjadi '*'.
#pragma once
#include <Arduino.h>

const char TIDAK_DIKENAL = '*';

// Teks -> Morse, contoh "SOS KU" -> "... --- ... / -.- ..-".
// Huruf dipisah spasi, kata dipisah " / ". false jika buffer tidak muat
// (isi buffer terpotong tapi tetap diakhiri '\0').
bool ubahKeMorse(const char *teks, char *buffer, size_t ukuran);
// Morse -> teks, contoh "... --- ..." -> "SOS". Huruf dipisah spasi, kata
// dipisah '/'. false jika buffer tidak muat.
bool ubahKeTeks(const char *morse, char *buffer, size_t ukuran);
// Untuk array: ukuran buffer dihitung otomatis.
template <size_t N> bool ubahKeMorse(const char *teks, char (&buffer)[N]) { return ubahKeMorse(teks, buffer, N); }
template <size_t N> bool ubahKeTeks(const char *morse, char (&buffer)[N]) { return ubahKeTeks(morse, buffer, N); }

// Pengirim: mengubah teks menjadi nyala/mati tanpa delay().
class SandiMorse {
public:
  // LED atau buzzer aktif di pin, dinyalakan (HIGH) saat simbol berbunyi.
  SandiMorse(uint8_t pin, uint8_t wpm = 20) : _pin(pin) { aturWPM(wpm); }
  // Tanpa pin: baca nyala() lalu kendalikan sendiri (tone(), relay, layar).
  SandiMorse() { aturWPM(20); }

  void mulai(); // panggil di setup(), mengatur pin sebagai OUTPUT

  // Mulai mengirim teks (menggantikan kiriman sebelumnya). Teks tidak disalin,
  // jadi harus tetap ada selama dikirim: string literal atau array global.
  // false jika tidak ada karakter yang bisa dikirim.
  bool kirim(const char *teks);
  // Panggil di setiap loop(). Mengembalikan nyala().
  bool perbarui();
  void berhenti(); // hentikan kiriman dan matikan keluaran

  bool nyala() const { return _nyala; }               // true saat titik/garis berbunyi
  // true sampai jeda 7 unit setelah huruf terakhir selesai, jadi kiriman
  // berulang tetap berjarak satu kata.
  bool sedangMengirim() const { return _durasi; }

  void aturWPM(uint8_t wpm) { _unit = 1200 / (wpm ? wpm : 1); } // default 20 WPM
  uint16_t durasiUnit() const { return _unit; }                  // ms

private:
  void ambilHuruf();
  void simbolBerikut();
  void tulis(bool nyala);

  const char *_teks = nullptr;
  uint32_t _waktu = 0;   // millis() saat fase sekarang dimulai
  uint16_t _unit = 60;   // ms per unit
  uint8_t _pin = 0xFF;   // 0xFF = tanpa pin
  uint8_t _kode = 0;     // kode huruf sekarang, bit 1 teratas = penanda awal
  uint8_t _sisa = 0;     // simbol yang belum dikirim dari huruf sekarang
  uint8_t _durasi = 0;   // lama fase sekarang dalam unit, 0 = diam
  bool _nyala = false;
};

// Pembaca: mengubah ketukan tombol menjadi huruf.
// Tekan < 2 unit = titik, >= 2 unit = garis. Lepas >= 2 unit = huruf selesai,
// >= 5 unit = spasi. Batas di tengah sehingga ketukan boleh meleset ±1 unit.
class PembacaMorse {
public:
  PembacaMorse(uint8_t wpm = 10) { aturWPM(wpm); }

  // Panggil di setiap loop() dengan status tombol (true = ditekan).
  // Mengembalikan true jika ada huruf baru. Getaran kontak di bawah 1/4 unit diabaikan.
  bool perbarui(bool ditekan);
  bool hurufBaru() const { return _baru; } // true hanya selama satu perbarui()
  // Huruf terakhir: 'A'-'Z', '0'-'9', tanda baca, ' ' untuk jeda kata,
  // atau '*' (TIDAK_DIKENAL) jika kode tidak ada di tabel.
  char huruf() const { return _huruf; }

  void aturWPM(uint8_t wpm) { _unit = 1200 / (wpm ? wpm : 1); } // default 10 WPM, cocok untuk pemula
  uint16_t durasiUnit() const { return _unit; }                  // ms

private:
  uint32_t _waktu = 0;  // millis() saat tombol terakhir berubah
  uint16_t _unit = 120;
  uint8_t _kode = 1;    // simbol yang sudah diketuk, 1 = kosong, 0 = terlalu panjang
  char _huruf = 0;
  bool _ditekan = false, _baru = false, _bolehSpasi = false;
};
