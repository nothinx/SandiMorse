# SandiMorse (English)

[Bahasa Indonesia](README.md)

An Arduino **Morse code** library: non-blocking sender for LEDs and buzzers, text ⇄ Morse translator with fixed buffers, and a key/button reader that turns taps into letters. The API and examples are in Indonesian. This page maps every function to English.

```cpp
#include <SandiMorse.h>

SandiMorse morse(LED_BUILTIN, 15); // sender on the built-in LED, 15 WPM

void setup() {
  morse.mulai();                   // begin(): sets the pin as OUTPUT
}

void loop() {
  morse.perbarui();                // update(): call every loop()
  if (!morse.sedangMengirim()) morse.kirim("SOS"); // if not sending, send
}
```

## Why

| Library | Sender | Key reader | From the source code |
|---|---|---|---|
| **SandiMorse** | ✅ non-blocking | ✅ | no `delay()`, no `String` |
| MorseEncoder 2.0.3 | ✅ | ❌ | `dot()`/`dash()` use `delay()` |
| CWW Morse Transmit 1.2.1 | ✅ | ❌ | `dot()`/`dash()` use `delay()` |
| Etherkit Morse 1.1.2 | ✅ non-blocking | ❌ | `update()` must run exactly every 1 ms |
| SimpleMorse 1.0.0 | ❌ | ✅ | `update()` calls `delay(50)`, uses `String` |

- Standard PARIS timing: 1 unit = 1200 / WPM ms; dot 1, dash 3, gaps 1 / 3 / 7.
- Pin mode or pinless mode (`nyala()` = "is on") for `tone()`, relays, or displays.
- 54 characters (A–Z, 0–9, punctuation) in a 63-byte PROGMEM table.
- 13 bytes of RAM per sender, 11 per reader on an Arduino Uno.
- Key reader thresholds sit halfway between standard lengths (±1 unit tolerance), with debounce.

## Function reference

| Indonesian | English | Notes |
|---|---|---|
| `SandiMorse(pin, wpm)` / `SandiMorse()` | sender | default 20 WPM; pinless: read `nyala()` |
| `mulai()` | begin | |
| `kirim(teks)` | send(text) | text is not copied; keep it alive while sending |
| `perbarui()` | update | returns `nyala()` |
| `nyala()` | is on | a dot/dash is sounding |
| `sedangMengirim()` | is sending | includes the final 7-unit gap |
| `berhenti()` | stop | |
| `aturWPM(wpm)` | set WPM | |
| `durasiUnit()` | unit length | ms |
| `PembacaMorse(wpm)` | key reader | default 10 WPM |
| `perbarui(bool ditekan)` | update(isPressed) | `true` if a new letter |
| `hurufBaru()` | new letter | |
| `huruf()` | letter | `' '` = word gap, `'*'` = unknown (`TIDAK_DIKENAL`) |
| `ubahKeMorse(teks, buffer)` | to Morse | `"SOS KU"` → `"... --- ... / -.- ..-"`, `false` if the buffer is too small |
| `ubahKeTeks(morse, buffer)` | to text | `"... --- ..."` → `"SOS"` |

## Examples

`KedipSOS` (blink SOS), `BuzzerMorse` (Serial text → passive buzzer), `KetukTombolKeTeks` (button taps → text), `LatihanMorse` (practice game with score), `TerjemahSerial` (text ⇄ Morse in the Serial Monitor).

## Status

Version 1.0.0 passes automated logic tests and compiles on Uno, Mega, ESP32, ESP32-C3, ESP32-S3, STM32 Blackpill F411, and Bluepill F103. It has **not yet been tested on real hardware**.

## License

MIT © 2026 Amadeo Wisesa.
