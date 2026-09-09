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
    # La terza minore sta davanti alla fondamentale in ogni linea.
    #
    # Prima erano inchiodate su C: 56% delle note in Classic Acid, con l'Eb al
    # 13%, cioe' la nota piu' rara era proprio quella che rende minore la
    # linea. Sommata al filtro che esalta la quinta armonica — che nella serie
    # e' E naturale — l'orecchio riceveva C, E e G e sentiva un accordo di Do
    # MAGGIORE, mentre l'Eb passava in una nota su otto. Da qui il "suona
    # allegro" nonostante i banchi siano tutti scritti in minore.
    # Le pause non sono un dettaglio: con sedici gate su sedici step
    # l'inviluppo non torna mai giu' e la battuta esce come un suono continuo.
    # Misurato, i banchi a 14-16 gate restavano fra il 39% e il 74% del picco
    # anche nei momenti "vuoti", ed e' quello che si sentiva come pattern
    # sovrapposti. Nessuna linea supera i tredici gate.
    ("Classic Acid", "0,3,7,10", "la linea di riferimento: apre il set", [
        [48,1,1,0,0],[51,1,0,0,0],[48,1,0,1,0],[51,1,0,0,0],
        [55,1,1,0,0],[48,0,0,0,0],[58,1,0,0,0],[48,0,0,0,0],
        [48,1,1,0,0],[58,1,0,0,0],[55,1,0,1,0],[51,1,0,0,0],
        [48,0,0,0,0],[48,0,0,0,0],[58,1,0,0,0],[51,1,0,0,0],
    ]),
    ("Rollin Acid", "0,1,3,7,10", "rullante, il b2 da' la minaccia", [
        [48,1,1,0,0],[51,1,0,0,0],[49,1,0,1,0],[51,1,0,0,0],
        [55,1,1,0,0],[51,1,0,1,0],[48,0,0,0,0],[58,1,0,1,0],
        [48,1,1,0,0],[51,1,0,0,0],[58,1,0,1,0],[48,0,0,0,0],
        [49,1,0,0,0],[51,1,0,0,0],[48,0,0,0,0],[51,1,1,0,0],
    ]),
    ("Munich Sequence", "0,3,7", "l'antenato: arpeggio di triade, nessuno slide", [
        [48,1,1,0,0],[51,1,0,0,0],[55,1,0,0,0],[51,1,0,0,1],
        [55,1,0,0,0],[51,1,0,0,0],[48,1,0,0,0],[51,1,0,0,0],
        [48,1,1,0,0],[51,1,0,0,0],[55,1,0,0,0],[51,1,0,0,1],
        [55,1,0,0,1],[51,1,0,0,1],[55,1,0,0,0],[51,1,0,0,0],
    ]),
    ("Night Drive", "0,3,5,10", "rada: il groove sta nelle pause, decay lungo", [
        [48,1,1,0,0],[48,0,0,0,0],[48,0,0,0,0],[51,1,0,1,0],
        [53,1,0,0,0],[48,0,0,0,0],[58,1,0,1,0],[48,0,0,0,0],
        [51,1,1,0,0],[48,0,0,0,0],[58,1,0,0,0],[48,0,0,0,0],
        [53,1,0,1,0],[48,0,0,0,0],[51,1,0,0,0],[48,0,0,0,0],
    ]),
    ("Deep Squelch", "0,3,8,10", "il b6 porta la malinconia, quasi tutto legato", [
        [48,1,1,0,0],[48,0,0,0,0],[51,1,0,1,0],[56,1,0,1,0],
        [58,1,0,0,0],[48,0,0,0,0],[56,1,0,1,0],[48,0,0,0,0],
        [51,1,1,0,0],[48,0,0,0,0],[58,1,0,1,0],[56,1,0,0,0],
        [48,0,0,0,0],[51,1,0,1,0],[48,1,0,0,0],[48,0,0,0,0],
    ]),
    # Quattro accenti e non sei: con risonanza .93 ogni accento tiene su il
    # livello, e sei accenti su sedici gate lasciavano l'inviluppo al 74% del
    # picco per tutta la battuta. Era il banco che fondeva di piu'.
    ("Phrygian Drive", "0,1,3,6,10", "b2 e tritono: il banco piu' dissonante", [
        [48,1,1,0,0],[49,1,0,0,0],[51,1,1,0,0],[54,1,0,0,0],
        [51,1,0,0,0],[49,1,0,0,0],[48,0,0,0,0],[54,1,0,1,0],
        [48,1,1,0,0],[51,1,0,0,0],[54,1,1,0,0],[58,1,0,0,0],
        [51,1,0,0,0],[48,0,0,0,0],[51,1,0,0,0],[48,0,0,0,0],
    ]),
]

# tempo, wave, cutoff Hz, res, env, dec, acc, vol, dis, sub, dlyT, dlyF, dlyM, rvbS, rvbM
#
# I volumi sono riequilibrati sulla misura, non a occhio: rms per banco andava
# da 0.32 a 0.69, cioe' Phrygian entrava piu' del doppio di Night Drive, e il
# picco toccava 0.956 a un soffio dal clipping. Bersaglio ~0.42 di rms per
# tutti, cosi' passare da un banco all'altro dal vivo non cambia il livello.
#
# Risonanza sotto .85. Con .88 e .93 il feedback del ladder arriva a ~3.0 e il
# filtro auto-oscilla: canta da solo e riempie le pause a prescindere
# dall'inviluppo, che e' insieme il "troppo squelch" e i pattern che suonano
# sovrapposti. Il carattere acid viene dall'inviluppo che spazza una risonanza
# alta, non da tenerla al limite dell'innesco.
#
# Il decay va letto insieme al tempo. Un sedicesimo a 120 BPM dura 125 ms, e il
# motore e' monofonico con inviluppo che non riparte da zero (comportamento
# autentico della macchina): se il decay copre quattro o cinque step, ogni nota
# entra su un inviluppo ancora alto, senza attacco, e la frase si fonde in un
# suono continuo. I valori di prima erano .40, .62 e .78 contro step da ~120 ms,
# cioe' da 3 a 6 step coperti: era quello a far suonare i pattern sovrapposti.
# Qui nessun banco supera i ~2.5 step, e i piu' densi stanno sotto 1.5.
SYNTH = [
    (128, "SAW",  420, .74, .72, .22, .76, .70, .05, .00, .375, .28, .00, .35, .00),
    (134, "SAW",  340, .80, .78, .14, .82, .59, .30, .12, .250, .36, .06, .36, .02),
    (124, "SAW", 1150, .38, .26, .17, .58, .77, .02, .55, .300, .26, .18, .55, .14),
    (116, "SAW",  520, .52, .40, .32, .60, .76, .06, .45, .500, .42, .16, .60, .10),
    (126, "SQR",  240, .80, .74, .30, .68, .66, .12, .70, .250, .44, .12, .52, .08),
    (140, "SQR",  380, .82, .80, .16, .86, .38, .34, .26, .214, .38, .05, .40, .03),
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


MARKER = "  // ── timbri dei banchi (generati da apply_banks_v11.py) ──"


def emit_synth_extra():
    """I timbri dei banchi come voci della lista Synth.

    Il preset Pattern non scrive piu' il timbro, quindi la coppia
    linea+suono con cui un banco e' stato pensato non sarebbe piu'
    raggiungibile: qui diventa una voce Synth con lo stesso nome.

    cut in SYNTH_DATA e' normalizzato 0..1, non in Hz: la conversione e' la
    stessa v2n della UI, pow((hz-min)/(max-min), skew) sul range del knob
    Cutoff (120..5000, skew 0.4).
    """
    lo, hi, skew = 120.0, 5000.0, 0.4
    w = max(len(n) for n, _, _, _ in BANKS) + 3
    out = [MARKER]
    for (name, _, _, _), s in zip(BANKS, SYNTH):
        t, wave, cut, res, env, dec, acc, vol, dis, sub, dT, dF, dM, rS, rM = s
        n = (max(0.0, min(1.0, (cut - lo) / (hi - lo))) ** skew)
        nm = f"'{name}',"
        out.append(
            f"  {{name:{nm:<{w}}cut:{n:.6f}".rstrip("0").rstrip(".") + f",res:{d2(res)},"
            f"env:{d2(env)},dec:{d2(dec)},acc:{d2(acc)},vol:{d2(vol)},dis:{d2(dis)},"
            f"wave:'{wave}',sub:{d2(sub)},dlyT:{d3(dT)},dlyF:{d2(dF)},dlyM:{d2(dM)},"
            f"rvbS:{d2(rS)},rvbM:{d2(rM)}}},"
        )
    return "\n".join(out)


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

# SYNTH_DATA: i 12 timbri scritti a mano restano, i 6 dei banchi si
# rigenerano. Il marcatore rende l'operazione ripetibile senza duplicare.
m = re.search(r"(const SYNTH_DATA = \[\n)(.*?)(\n\];)", html, re.S)
if not m:
    sys.exit("SYNTH_DATA non trovato")
body = m.group(2)
if MARKER in body:
    body = body[:body.index(MARKER)].rstrip("\n")
html = html[:m.start(2)] + body + "\n" + emit_synth_extra() + html[m.end(2):]
print(f"  ok  SYNTH_DATA (+{len(BANKS)} timbri dei banchi)")

# Le <option> di synth-select sono markup statico: le rigenero dai nomi
# effettivamente presenti in SYNTH_DATA, cosi' gli indici non scivolano.
names = re.findall(r"\{name:'([^']+)'", re.search(
    r"const SYNTH_DATA = \[\n(.*?)\n\];", html, re.S).group(1))
opts = "\n".join(
    f'                <option value="{i}">{i + 1:02d} — {n}</option>'
    for i, n in enumerate(names))
ms = re.search(r"([ \t]*<option value=\"0\">01 — Classic Acid 303</option>\n"
               r"(?:[ \t]*<option value=\"\d+\">.*?</option>\n?)*)", html)
if not ms:
    sys.exit("<option> di synth-select non trovate")
html = html[:ms.start(1)] + opts + "\n" + html[ms.end(1):]
print(f"  ok  <option> Synth ({len(names)} voci)")

if html == orig:
    sys.exit("nessuna modifica")
TARGET.write_text(html)
print(f"\nindex.html: {len(orig)} -> {len(html)} byte")
