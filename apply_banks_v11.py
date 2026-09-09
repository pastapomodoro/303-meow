"""Riscrive le 6 linee base e i parametri synth dei banchi in index.html (v1.1.0).

Perche' solo le linee base. In v1.1.0 c'e' questo, subito dopo i dati:

    FACTORY_BANKS.forEach(bank=>{
      const base=bank[0];
      bank.splice(0,bank.length,...Array.from({length:9},(_,i)=>makePerformanceVariation(base,i)));
    });

Lo splice sostituisce l'intero array con nove variazioni derivate da bank[0]:
le altre otto scritte a mano non suonano mai. Erano 48 pattern di dati morti,
quindi qui ogni banco ha una sola _P e le nove variazioni le genera il codice.

Perche' queste note. Quattro banchi su sei usavano lo stesso set (C Eb G Bb) con
gli stessi 4 accenti e 2 slide: all'ascolto era lo stesso pezzo sei volte. Ora
ogni banco ha un insieme intervallare suo, e densita', accenti, slide e ambito
sono spalmati su tutto il range. Tutti restano su root C con la terza minore,
che e' la condizione perche' saltare da un banco all'altro durante un set resti
consonante.
"""
import pathlib
import re
import sys

BASE = pathlib.Path("/Users/eugenio.bellini/Desktop/MISC/WEB/claudio")
TARGET = BASE / "index.html"

# [nota, gate, accent, slide, ottava] — root C3 = 48
BANKS = [
    ("Classic Acid", "0,3,7,10", "la linea di riferimento: apre il set", [
        [48,1,1,0,0],[48,0,0,0,0],[48,1,0,1,0],[51,1,0,0,0],
        [55,1,1,0,0],[48,0,0,0,0],[58,1,0,0,0],[48,1,0,0,1],
        [48,1,1,0,0],[58,1,0,0,0],[55,1,0,1,0],[51,1,0,0,0],
        [48,1,0,0,0],[48,0,0,0,0],[58,1,0,0,0],[48,1,0,0,0],
    ]),
    ("Rollin Acid", "0,1,3,7,10", "sedicesimi pieni, il b2 da' la minaccia", [
        [48,1,1,0,0],[48,1,0,0,0],[49,1,0,1,0],[51,1,0,0,0],
        [55,1,1,0,0],[51,1,0,1,0],[49,1,0,0,0],[48,1,0,0,0],
        [48,1,1,0,0],[58,1,0,0,0],[55,1,0,1,0],[51,1,1,0,0],
        [49,1,0,0,0],[48,1,0,0,1],[58,1,0,1,0],[48,1,1,0,0],
    ]),
    ("Munich Sequence", "0,3,7", "l'antenato: arpeggio di triade, nessuno slide", [
        [48,1,1,0,0],[51,1,0,0,0],[55,1,0,0,0],[48,1,0,0,1],
        [55,1,0,0,0],[51,1,0,0,0],[48,1,0,0,0],[51,1,0,0,0],
        [48,1,1,0,0],[51,1,0,0,0],[55,1,0,0,0],[48,1,0,0,1],
        [55,1,0,0,1],[51,1,0,0,1],[55,1,0,0,0],[51,1,0,0,0],
    ]),
    ("Night Drive", "0,3,5,10", "rada: il groove sta nelle pause, decay lungo", [
        [48,1,1,0,0],[48,0,0,0,0],[48,0,0,0,0],[51,1,0,1,0],
        [53,1,0,0,0],[48,0,0,0,0],[58,1,0,1,0],[48,0,0,0,0],
        [48,1,1,0,0],[48,0,0,0,0],[58,1,0,0,0],[48,0,0,0,0],
        [53,1,0,1,0],[48,0,0,0,0],[51,1,0,0,0],[48,0,0,0,0],
    ]),
    ("Deep Squelch", "0,3,8,10", "il b6 porta la malinconia, quasi tutto legato", [
        [48,1,1,0,0],[48,0,0,0,0],[51,1,0,1,0],[56,1,0,1,0],
        [58,1,0,0,0],[48,0,0,0,0],[56,1,0,1,0],[48,0,0,0,0],
        [48,1,1,0,0],[48,0,0,0,0],[58,1,0,1,0],[56,1,0,0,0],
        [48,0,0,0,0],[51,1,0,1,0],[48,1,0,0,0],[48,0,0,0,0],
    ]),
    ("Phrygian Drive", "0,1,3,6,10", "b2 e tritono: il banco piu' dissonante", [
        [48,1,1,0,0],[49,1,0,0,0],[51,1,1,0,0],[54,1,0,0,0],
        [48,1,1,0,0],[49,1,0,0,0],[58,1,0,0,0],[54,1,0,1,0],
        [48,1,1,0,0],[51,1,0,0,0],[54,1,1,0,0],[58,1,0,0,0],
        [48,1,1,0,0],[49,1,0,0,0],[51,1,0,0,0],[54,1,0,0,0],
    ]),
]

# tempo, wave, cutoff Hz, res, env, dec, acc, vol, dis, sub, dlyT, dlyF, dlyM, rvbS, rvbM
SYNTH = [
    (128, "SAW",  420, .74, .72, .22, .76, .70, .05, .00, .375, .28, .00, .35, .00),
    (134, "SAW",  340, .88, .82, .14, .82, .72, .30, .12, .250, .36, .06, .36, .02),
    (124, "SAW", 1150, .38, .26, .40, .58, .74, .02, .55, .300, .26, .18, .55, .14),
    (116, "SAW",  520, .52, .40, .62, .60, .75, .06, .45, .500, .42, .16, .60, .10),
    (126, "SQR",  240, .80, .74, .78, .68, .73, .12, .70, .250, .44, .12, .52, .08),
    (140, "SQR",  380, .93, .88, .16, .86, .71, .34, .26, .214, .38, .05, .40, .03),
]


def d2(v):
    """.74 e non 0.74: e' la convenzione del file."""
    return f"{v:.2f}"[1:] if 0 <= v < 1 else f"{v:.2f}"


def d3(v):
    return f"{v:.3f}"[1:] if 0 <= v < 1 else f"{v:.3f}"


def emit_data():
    w = max(len(n) for n, _, _, _ in BANKS) + 3
    out = []
    for (name, _, _, _), s in zip(BANKS, SYNTH):
        t, wave, cut, res, env, dec, acc, vol, dis, sub, dT, dF, dM, rS, rM = s
        nm = f"'{name}',"
        out.append(
            f"  {{name:{nm:<{w}}tempo:{t},wave:'{wave}',cut:'{cut}Hz',"
            f"res:{d2(res)},env:{d2(env)},dec:{d2(dec)},acc:{d2(acc)},vol:{d2(vol)},"
            f"dis:{d2(dis)},sub:{d2(sub)},dlyT:{d3(dT)},dlyF:{d2(dF)},dlyM:{d2(dM)},"
            f"rvbS:{d2(rS)},rvbM:{d2(rM)}}},"
        )
    return "\n".join(out)


def emit_banks():
    out = []
    for name, gradi, nota, steps in BANKS:
        body = ",".join("[" + ",".join(str(v) for v in st) + "]" for st in steps)
        out.append(f"    [ // {name} — gradi {gradi} — {nota}")
        out.append(f"      _P([{body}])")
        out.append("    ],")
    res = "\n".join(out)
    return res[:-1] if res.endswith(",") else res


html = TARGET.read_text()
orig = html

m = re.search(r"const FACTORY_DATA = window\.FACTORY_DATA = \[\n(.*?)\n\];", html, re.S)
if not m:
    sys.exit("FACTORY_DATA non trovato")
html = html[:m.start(1)] + emit_data().rstrip(",") + html[m.end(1):]
print("  ok  FACTORY_DATA")

m = re.search(r"(  const FACTORY_BANKS=\[\n)(.*?)(\n  \];)", html, re.S)
if not m:
    sys.exit("FACTORY_BANKS non trovato")
removed = m.group(2).count("_P([[") - len(BANKS)
html = html[:m.start(2)] + emit_banks() + html[m.end(2):]
print(f"  ok  FACTORY_BANKS ({removed} pattern morte rimosse)")

# "Basic Acid" -> "Munich Sequence" nei testi iniziali e nel menu
n = 0
for pat, rep in [
    (r'(<div class="preset-name" id="lcd-preset">)[^<]*(</div>)', BANKS[0][0]),
    (r'(id="loaded-name">)[^<]*(</div>)', BANKS[0][0]),
]:
    html, k = re.subn(pat, lambda mm, r=rep: mm.group(1) + r + mm.group(2), html)
    n += k
opts = "\n".join(
    f'                <option value="{i}">{i + 1:02d} — {b[0]}</option>'
    for i, b in enumerate(BANKS))
mo = re.search(r"([ \t]*<option value=\"0\">01 — .*?</option>\n"
               r"(?:[ \t]*<option value=\"[1-9]\">.*?</option>\n?)*)", html)
if mo:
    html = html[:mo.start(1)] + opts + "\n" + html[mo.end(1):]
    print("  ok  <option> del menu")
print(f"  ok  testi iniziali ({n})")

if html == orig:
    sys.exit("nessuna modifica")
TARGET.write_text(html)
print(f"\nindex.html: {len(orig)} -> {len(html)} byte")
