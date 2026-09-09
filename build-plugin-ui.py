#!/usr/bin/env python3
"""Genera Resources/ui.html (la WebView del plugin) da index.html + il bridge JUCE.

Perche' esiste questo script e non si usa build_html.py:

  build_html.py rigenera la pagina dal 'sketch definitivo.html', che e' fermo
  a settembre, e in fondo fa copyfile(ui.html, index.html): lanciarlo oggi
  cancellerebbe la web app. La sorgente vera e' index.html, e il plugin deve
  partire da quella.

Cosa fa:
  1. prende index.html e le toglie la chiusura </body></html>
  2. estrae il bridge JUCE da build_html.py, unica copia esistente
  3. gli sostituisce il pezzo del piano roll, scritto per la griglia vecchia
  4. gli aggiunge i ganci che mancavano, dentro la sua IIFE - quindi attivi
     solo nella WebView, dove window.__JUCE__ esiste
  5. scrive Resources/ui.html, che CMake incorpora come binary data
"""
import re, sys, os

BASE = os.path.dirname(os.path.abspath(__file__))
IDX  = os.path.join(BASE, 'index.html')
GEN  = os.path.join(BASE, 'build_html.py')
OUT  = os.path.join(BASE, 'Resources', 'ui.html')

pagina = open(IDX, encoding='utf8').read()
m = re.search(r'BRIDGE\s*=\s*r?"""(.*?)"""', open(GEN, encoding='utf8').read(), re.S)
if not m:
    sys.exit('bridge non trovato in build_html.py')
bridge = m.group(1)

# ── 1. il blocco del piano roll del bridge e' scritto per la griglia vecchia ──
# Installava un suo mousedown sulla canvas con ROWS=24, NOTE_H=18 e
# nota = 71 - riga. Da quando le righe sono i gradi della scala scelta quei tre
# numeri non descrivono piu' la griglia, e l'ottava visibile si sposta. L'hit
# test lo fa il piano roll della pagina, che chiama _prNoteHook passando anche
# la NOTA ASSOLUTA gia' risolta: qui si inoltra al C++ e basta.
vecchio = bridge[bridge.index('  // ── Piano roll ↔ C++ sequencer'):
                 bridge.index('  // ── CLEAR / RANDOM -> C++')]
nuovo = '''  // ── Piano roll ↔ C++ sequencer ────────────────────────────────────────────
  // L'hit test lo fa il piano roll della pagina: da quando le righe sono i
  // gradi della scala, 'riga * 18px' e 'nota = 71 - riga' non descrivono piu'
  // la griglia. _prNoteHook riceve step, riga e nota assoluta gia' risolta.
  function syncNotesToCanvas(){ if(window._prSetPattern) window._prSetPattern(stepData); }
  window._syncNotesToCanvas = syncNotesToCanvas;

  window._prNoteHook = function(si, riga, nota){
    if(!(si >= 0 && si < stepData.length) || !Number.isFinite(nota)) return;
    const s = stepData[si];
    if(s.gate && s.note === nota){ s.gate = false; fnToggleStep(si); }
    else { s.note = nota; s.gate = true; fnSetStepNote(si, nota); }
    syncNotesToCanvas(); window.buildSteps && window.buildSteps();
  };

  // Slide e accent dalle due righe sotto la griglia: prima cadevano nello
  // stato locale della pagina e non arrivavano al sequencer C++.
  window._prModifierHook = function(i, campo){
    const s = stepData[i]; if(!s) return;
    if(campo === 'accent'){ s.accent = !s.accent; fnSetAccent(i, s.accent); }
    else { s.slide = !s.slide; fnSetSlide(i, s.slide); }
    syncNotesToCanvas(); window.buildSteps && window.buildSteps();
  };

  // Due interruttori che la pagina legge da sola:
  //  - i nove tasti pattern parlano al C++ (selectPattern), quindi la pagina
  //    non deve cablarci sopra la sua coda WebAudio;
  //  - il motore WebAudio non deve nascere: il suono lo fa il DSP C++, e
  //    senza questo si ritrovano due sintetizzatori sovrapposti.
  window._jucePatternBridge = true;
  window._juceNoWebAudio    = true;

  setTimeout(syncNotesToCanvas, 250);

'''
bridge = bridge.replace(vecchio, nuovo, 1)

# ── 2. cucitura ───────────────────────────────────────────────────────────────
# Il bridge porta con se' </script></body></html>, quindi alla pagina si
# toglie la sua chiusura.
taglio = pagina.rfind('</body>')
if taglio < 0:
    sys.exit('</body> non trovato in index.html')
fuori = pagina[:taglio] + bridge

os.makedirs(os.path.dirname(OUT), exist_ok=True)
open(OUT, 'w', encoding='utf8').write(fuori)

# ── 3. controlli che avrebbero preso gli errori di ieri ──────────────────────
def conta(t, s): return t.count(s)
esiti = [
    ('bridge presente',            conta(fuori, '__JUCE__') >= 1),
    ('gate standalone del bridge', "if(typeof window.__JUCE__ === 'undefined') return;" in fuori),
    ('hook nota',                  conta(fuori, '_prNoteHook') >= 1),
    ('hook slide/accent',          conta(fuori, '_prModifierHook') >= 1),
    ('motore WebAudio spento',     conta(fuori, '_juceNoWebAudio') >= 1),
    ('griglia vecchia rimossa',    'const midiNote=71-row' not in fuori),
    ('piano roll a scala',         conta(fuori, 'noteDellaRiga') >= 1),
    ('un solo </html>',            conta(fuori, '</html>') == 1),
    ('un solo </body>',            conta(fuori, '</body>') == 1),
]
larghezza = max(len(n) for n, _ in esiti)
print('%s  %d byte' % (os.path.relpath(OUT, BASE), len(fuori.encode('utf8'))))
ko = 0
for nome, ok in esiti:
    print('  %-*s %s' % (larghezza, nome, 'ok' if ok else 'FALLITO'))
    if not ok: ko += 1
sys.exit(1 if ko else 0)
