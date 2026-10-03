"""Compile + jalankan simulasi.cpp (kode library asli), lalu render grafik ke ../gambar/.

Jalankan dari folder ini:  python gambar.py   (butuh g++ dan matplotlib)
"""
import glob
import os
import subprocess
import sys
import tempfile

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

plt.rcParams.update({
    "figure.figsize": (8, 3.6), "figure.dpi": 100, "savefig.bbox": "tight", "savefig.pad_inches": 0.15,
    "figure.facecolor": "white", "axes.facecolor": "white", "savefig.facecolor": "white",
    "font.size": 10, "axes.titlesize": 11, "axes.titleweight": "bold", "axes.titlelocation": "left",
    "axes.spines.top": False, "axes.spines.right": False, "axes.edgecolor": "#9ca3af",
    "axes.grid": True, "grid.color": "#e5e7eb", "grid.linewidth": 0.8,
    "legend.frameon": False, "svg.fonttype": "path", "svg.hashsalt": "nothinx",
    "lines.linewidth": 1.8,
})
WARNA = {"utama": "#2563eb", "pembanding": "#dc2626", "ketiga": "#16a34a", "keempat": "#9333ea",
         "kelima": "#ea580c", "mentah": "#9ca3af", "target": "#111827"}

SINI = os.path.dirname(os.path.abspath(__file__))
KELUAR = os.path.join(SINI, "..", "gambar")


def koma(x, d=1):
    return f"{x:.{d}f}".replace(".", ",")


def jalankan():
    with tempfile.TemporaryDirectory() as tmp:
        exe = os.path.join(tmp, "sim")
        src = glob.glob(os.path.join(SINI, "..", "..", "src", "*.cpp"))
        hasil = subprocess.run(["g++", "-std=c++11", "-O2", "-I../test", "-I../../src", "simulasi.cpp", *src,
                                "-o", exe], cwd=SINI, capture_output=True, text=True)
        if hasil.returncode:
            sys.exit(hasil.stderr)
        teks = subprocess.run([exe], check=True, capture_output=True, text=True).stdout
    data, nama = {}, None
    for baris in teks.splitlines():
        if baris.startswith("# "):
            nama, data[baris[2:]] = baris[2:], []
        else:
            data[nama].append(baris)
    tabel = {k: [dict(zip(v[0].split(","), r.split(","))) for r in v[1:]] for k, v in data.items() if k != "teks"}
    return tabel, data["teks"]


def simpan(fig, nama):
    fig.savefig(os.path.join(KELUAR, nama), format="svg", metadata={"Date": None})
    plt.close(fig)


def ukur(ax, a, b, y, teks, kiri=False, warna=WARNA["target"]):
    """Panah ukuran a..b ms dengan teks di atasnya (kiri=True untuk rentang sempit)."""
    ax.annotate("", (a, y), (b, y), arrowprops=dict(arrowstyle="<->", color=warna, lw=1, shrinkA=0, shrinkB=0))
    ax.text(a if kiri else (a + b) / 2, y + 0.06, teks, ha="left" if kiri else "center", va="bottom",
            fontsize=8.5, color=warna)


def pengirim(t):
    info = t["kirim"][0]
    unit, kalimat = int(info["unit_ms"]), info["teks"]
    fase = [(int(r["ms"]), int(r["nyala"])) for r in t["fase"]]
    total = fase[-1][0]
    # sinyal tangga dan daftar (mulai, akhir) tiap simbol yang menyala
    xs, ys, nyala = [], [], []
    for (a, s), (b, _) in zip(fase, fase[1:]):
        xs += [a, b]
        ys += [s, s]
        if s:
            nyala.append((a, b))
    # kelompokkan simbol menjadi huruf: jeda >= 3 unit memisahkan huruf
    huruf, kel = [], [nyala[0]]
    for p, q in zip(nyala, nyala[1:]):
        if q[0] - p[1] >= 3 * unit:
            huruf.append(kel)
            kel = []
        kel.append(q)
    huruf.append(kel)

    fig, (a1, a2) = plt.subplots(2, 1, figsize=(8, 5.2), gridspec_kw={"height_ratios": [1, 1.25]})
    for ax in (a1, a2):
        ax.fill_between(xs, ys, color=WARNA["utama"], alpha=0.2, linewidth=0)
        ax.plot(xs, ys, color=WARNA["utama"])
        ax.set_yticks([0, 1], ["mati", "nyala"])
        ax.set_ylim(-0.05, 1.55)
        ax.grid(axis="y", visible=False)
    for kel, c in zip(huruf, kalimat.replace(" ", "")):
        a1.text((kel[0][0] + kel[-1][1]) / 2, 1.12, c, ha="center", va="bottom", fontweight="bold",
                color=WARNA["utama"])
    akhir_kata = huruf[2][-1][1]
    ukur(a1, akhir_kata, huruf[3][0][0], 1.12, f"jeda kata {huruf[3][0][0] - akhir_kata} ms")
    a1.set_xlim(0, total)
    a1.set_xlabel("Waktu (ms)")
    a1.set_title(f"“{kalimat}” pada 20 WPM: {total // unit} unit × {unit} ms = {koma(total / 1000, 2)} detik"
                 " (termasuk jeda akhir)")

    # perbesar huruf S dan O pertama
    s, o = huruf[0], huruf[1]
    ukur(a2, *s[0], 1.12, f"titik {s[0][1] - s[0][0]} ms", kiri=True)
    ukur(a2, s[0][1], s[1][0], 1.45, f"jeda simbol {s[1][0] - s[0][1]} ms", kiri=True)
    ukur(a2, s[-1][1], o[0][0], 1.12, f"jeda huruf {o[0][0] - s[-1][1]} ms")
    ukur(a2, *o[0], 1.12, f"garis {o[0][1] - o[0][0]} ms")
    a2.set_xlim(0, o[-1][1] + 3 * unit)
    a2.set_ylim(-0.05, 1.8)
    a2.set_xlabel("Waktu (ms)")
    a2.set_title("Diperbesar: huruf S dan O (1 unit = 1200 / WPM ms)")
    fig.tight_layout(h_pad=1.5)
    simpan(fig, "kirim_sos.svg")
    return unit, total


def pembaca(t):
    unit = int(t["ketuk"][0]["unit_ms"])
    pola = t["ketuk"][0]["pola"]
    tekan = [(int(r["mulai_ms"]), int(r["lama_ms"])) for r in t["tekan"]]
    huruf = [(int(r["ms"]), r["huruf"]) for r in t["huruf"]]
    batas = 2 * unit
    fig, ax = plt.subplots(figsize=(8, 3.8))
    for m, d in tekan:
        w = WARNA["utama"] if d < batas else WARNA["keempat"]
        ax.plot([m / 1000, m / 1000], [0, d], color=w, linewidth=2.2, solid_capstyle="butt")
        ax.plot(m / 1000, d, "o", color=w, markersize=4)
    # label garis acuan di kanan (kosong); warnanya sekaligus keterangan warna titik/garis
    for y, teks, w in [(unit, f"titik ideal 1 unit = {unit} ms", WARNA["utama"]),
                       (3 * unit, f"garis ideal 3 unit = {3 * unit} ms", WARNA["keempat"])]:
        ax.axhline(y, color=WARNA["mentah"], linewidth=1.0, linestyle=":")
        ax.text(0.995, y + 6, teks, color=w, fontsize=8.5, ha="right", transform=ax.get_yaxis_transform())
    ax.axhline(batas, color=WARNA["target"], linewidth=1.2, linestyle="--")
    ax.text(0.995, batas + 6, f"batas titik/garis 2 unit = {batas} ms", fontsize=8.5, ha="right",
            transform=ax.get_yaxis_transform())
    # huruf hasil pembaca di atas kelompok ketukannya
    teks, i = "", 0
    for ms, c in huruf:
        if c == "_":
            teks += " "
            continue
        kel = []
        while i < len(tekan) and tekan[i][0] < ms:
            kel.append(tekan[i][0])
            i += 1
        ax.text((kel[0] + kel[-1]) / 2000, 4.7 * unit, c, ha="center", va="bottom", fontweight="bold",
                fontsize=12, color=WARNA["utama"])
        teks += c
    teks = teks.strip()
    ax.set_ylim(0, 5.4 * unit)
    ax.set_xlim(0, tekan[-1][0] / 1000 + 4.6)
    ax.set_xlabel("Waktu ketukan dimulai (detik)")
    ax.set_ylabel("Lama tekan (ms)")
    titik = [d for _, d in tekan if d < batas]
    garis = [d for _, d in tekan if d >= batas]
    ax.set_title(f"{len(tekan)} ketukan tidak rapi tetap terbaca “{teks}” (10 WPM)")
    simpan(fig, "ketukan.svg")
    return pola, teks, (min(titik), max(titik)), (min(garis), max(garis))


def main():
    os.makedirs(KELUAR, exist_ok=True)
    t, teks = jalankan()
    unit, total = pengirim(t)
    pola, hasil, titik, garis = pembaca(t)
    print(f"pengirim: unit {unit} ms, total {total} ms")
    print(f"pembaca: pola {pola} -> {hasil}; titik {titik} ms, garis {garis} ms")
    print("\n".join(teks))


if __name__ == "__main__":
    main()
