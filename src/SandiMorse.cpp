#include "SandiMorse.h"

// Kode Morse untuk karakter '!' (33) sampai '_' (95), 0 = tidak ada.
// Dibaca dari bit 1 teratas (penanda awal) ke bawah: 0 = titik, 1 = garis.
// Contoh 'A' = 0b101 -> penanda, titik, garis = ".-".
static const uint8_t TABEL[] PROGMEM = {
  0x6B, 0x52, 0x00, 0x89, 0x00, 0x28, 0x5E, 0x36, // ! " # $ % & ' (
  0x6D, 0x00, 0x2A, 0x73, 0x61, 0x55, 0x32, 0x3F, // ) * + , - . / 0
  0x2F, 0x27, 0x23, 0x21, 0x20, 0x30, 0x38, 0x3C, // 1 2 3 4 5 6 7 8
  0x3E, 0x78, 0x6A, 0x00, 0x31, 0x00, 0x4C, 0x5A, // 9 : ; < = > ? @
  0x05, 0x18, 0x1A, 0x0C, 0x02, 0x12, 0x0E, 0x10, // A B C D E F G H
  0x04, 0x17, 0x0D, 0x14, 0x07, 0x06, 0x0F, 0x16, // I J K L M N O P
  0x1D, 0x0A, 0x08, 0x03, 0x09, 0x11, 0x0B, 0x19, // Q R S T U V W X
  0x1B, 0x1C, 0x00, 0x00, 0x00, 0x00, 0x4D,       // Y Z [ \ ] ^ _
};
const char AWAL_TABEL = '!';

static uint8_t kodeDari(char c) {
  if (c >= 'a' && c <= 'z') c -= 'a' - 'A';
  if (c < AWAL_TABEL || c >= AWAL_TABEL + (int)sizeof(TABEL)) return 0;
  return pgm_read_byte(&TABEL[c - AWAL_TABEL]);
}

static char hurufDari(uint8_t kode) {
  for (uint8_t i = 0; kode > 1 && i < sizeof(TABEL); i++)
    if (pgm_read_byte(&TABEL[i]) == kode) return AWAL_TABEL + i;
  return TIDAK_DIKENAL;
}

static bool spasi(char c) { return c == ' ' || c == '\n' || c == '\r' || c == '\t'; }

static uint8_t panjang(uint8_t kode) {
  uint8_t n = 0;
  while (kode > 1) { kode >>= 1; n++; }
  return n;
}

bool ubahKeMorse(const char *teks, char *buffer, size_t ukuran) {
  if (!ukuran) return false;
  size_t p = 0;
  bool muat = true, ada = false, jedaKata = false;
  auto tulis = [&](char c) { if (p + 1 < ukuran) buffer[p++] = c; else muat = false; };
  for (; teks && *teks && muat; teks++) {
    if (spasi(*teks)) { jedaKata = ada; continue; }
    uint8_t kode = kodeDari(*teks);
    if (!kode) continue;
    if (ada) {
      tulis(' ');
      if (jedaKata) { tulis('/'); tulis(' '); }
    }
    for (uint8_t n = panjang(kode); n--;) tulis((kode >> n) & 1 ? '-' : '.');
    ada = true;
    jedaKata = false;
  }
  buffer[p] = '\0';
  return muat;
}

bool ubahKeTeks(const char *morse, char *buffer, size_t ukuran) {
  if (!ukuran) return false;
  size_t p = 0;
  bool muat = true;
  uint8_t kode = 1; // 1 = kosong, 0 = tidak sah
  auto tulis = [&](char c) { if (p + 1 < ukuran) buffer[p++] = c; else muat = false; };
  for (const char *m = morse; m; m++) {
    char c = *m;
    if (c == '.' || c == '-') {
      if (kode) kode = kode >= 0x80 ? 0 : kode << 1 | (c == '-');
      continue;
    }
    if (c && !spasi(c) && c != '/') { kode = 0; continue; } // karakter asing
    if (kode != 1) { // huruf selesai
      tulis(kode ? hurufDari(kode) : TIDAK_DIKENAL);
      kode = 1;
    }
    if (c == '/' && p && buffer[p - 1] != ' ') tulis(' ');
    if (!c) break;
  }
  if (p && buffer[p - 1] == ' ') p--; // '/' di akhir
  buffer[p] = '\0';
  return muat;
}

void SandiMorse::mulai() {
  if (_pin == 0xFF) return;
  pinMode(_pin, OUTPUT);
  digitalWrite(_pin, LOW);
}

void SandiMorse::tulis(bool nyala) {
  _nyala = nyala;
  if (_pin != 0xFF) digitalWrite(_pin, nyala ? HIGH : LOW);
}

// Memuat huruf berikutnya ke _kode/_sisa dan mengisi jeda sebelum huruf itu
// (3 unit, atau 7 jika ada spasi). Jika teks habis, _sisa = 0 dan jeda 7 unit.
void SandiMorse::ambilHuruf() {
  _durasi = 3;
  _sisa = 0;
  for (; _teks && *_teks; _teks++) {
    if (spasi(*_teks)) { _durasi = 7; continue; }
    _kode = kodeDari(*_teks);
    if (_kode) {
      _sisa = panjang(_kode);
      _teks++;
      return;
    }
  }
  _teks = nullptr;
  _durasi = 7;
}

void SandiMorse::simbolBerikut() {
  _sisa--;
  _durasi = (_kode >> _sisa) & 1 ? 3 : 1;
  tulis(true);
}

bool SandiMorse::kirim(const char *teks) {
  berhenti();
  _teks = teks;
  ambilHuruf();
  if (!_sisa) {
    berhenti();
    return false;
  }
  _waktu = millis();
  simbolBerikut();
  return true;
}

void SandiMorse::berhenti() {
  _teks = nullptr;
  _sisa = 0;
  _durasi = 0;
  tulis(false);
}

bool SandiMorse::perbarui() {
  if (!_durasi) return false;
  uint32_t lama = (uint32_t)_durasi * _unit;
  if (millis() - _waktu < lama) return _nyala;
  _waktu += lama; // tanpa pergeseran: fase berikutnya dihitung dari batas fase ini
  if (_nyala) {
    tulis(false);
    if (_sisa) _durasi = 1; // jeda antarsimbol
    else ambilHuruf();      // jeda antarhuruf / antarkata / akhir
  } else if (_sisa) {
    simbolBerikut();
  } else {
    _durasi = 0; // jeda akhir selesai
  }
  return _nyala;
}

bool PembacaMorse::perbarui(bool ditekan) {
  _baru = false;
  uint32_t sekarang = millis();
  uint32_t lewat = sekarang - _waktu;

  // Debounce di sisi depan, sama seperti TombolPintar: perubahan pertama
  // langsung diterima, getaran sesudahnya diabaikan.
  if (ditekan != _ditekan && lewat >= _unit / 4u) {
    if (_ditekan && _kode) // tombol dilepas: satu simbol selesai
      _kode = _kode >= 0x80 ? 0 : _kode << 1 | (lewat >= 2u * _unit);
    _ditekan = ditekan;
    _waktu = sekarang;
    lewat = 0;
  }

  if (!_ditekan) {
    if (_kode != 1 && lewat >= 2u * _unit) {
      _huruf = _kode ? hurufDari(_kode) : TIDAK_DIKENAL;
      _kode = 1;
      _baru = _bolehSpasi = true;
    } else if (_bolehSpasi && lewat >= 5u * _unit) {
      _huruf = ' ';
      _baru = true;
      _bolehSpasi = false;
    }
  }
  return _baru;
}
