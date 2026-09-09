#!/usr/bin/env bash
# Pubblica la web app sui due progetti Vercel.
#
# Perche' serve uno script e non tre comandi a mano:
#
#  1. Vercel NON si aggiorna dal push su GitHub. I due progetti non sono
#     collegati al repo: se pushi e non deployi, sull'iPad resta la versione
#     vecchia. E' il passaggio che si dimentica sempre.
#  2. Il deploy non parte dalla root del repo, che contiene sorgenti C++,
#     cartelle di build e il plugin. Va spedita solo la pagina con i suoi
#     asset, quindi si costruisce un pacchetto a parte.
#  3. La lista degli asset si RICAVA da index.html invece di essere scritta
#     qui: se aggiungi un'immagine e la lista fosse fissa, il deploy andrebbe
#     online senza quel file e la pagina si romperebbe solo in produzione.
#
# Uso:  ./deploy-web.sh
#       ./deploy-web.sh --dry-run     costruisce e controlla, non pubblica
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROGETTI=(acidlab 303-meow)
# if e non '&& DRY=1': sotto set -e un && con condizione falsa fa uscire lo
# script, quindi senza argomenti non sarebbe partito niente.
DRY=0
if [[ "${1:-}" == "--dry-run" ]]; then DRY=1; fi

cd "$ROOT"
[[ -f index.html ]] || { echo "index.html non trovato in $ROOT"; exit 1; }

# ── 1. asset referenziati da index.html ──────────────────────────────────────
# Si leggono index.html E manifest.webmanifest. Il manifest e' una seconda
# sorgente di riferimenti, e guardando solo la pagina si perdevano tre icone:
# icon-1024 e le due maskable stanno solo nel manifest, esistevano su disco e
# rispondevano 404 online, perche' nessun deploy le aveva mai spedite.
#
# while read e non mapfile: mapfile e' un builtin di bash 4 e macOS ha la 3.2
ASSET=()
while IFS= read -r f; do
  ASSET+=("$f")
done < <(
  { grep -oE '(assets/[A-Za-z0-9_./-]+\.(webp|png|jpg|jpeg|svg|gif))|manifest\.webmanifest' index.html
    if [[ -f manifest.webmanifest ]]; then
      grep -oE 'assets/[A-Za-z0-9_./-]+\.(webp|png|jpg|jpeg|svg|gif)' manifest.webmanifest
    fi
  } | sort -u
)
echo "asset referenziati da pagina e manifest: ${#ASSET[@]}"

MANCANTI=()
for f in "${ASSET[@]}"; do [[ -f "$f" ]] || MANCANTI+=("$f"); done
if (( ${#MANCANTI[@]} )); then
  echo
  echo "FERMO: index.html referenzia file che non esistono su disco."
  printf '  %s\n' "${MANCANTI[@]}"
  echo "Pubblicare cosi' manderebbe online una pagina rotta."
  exit 1
fi

# ── 2. pacchetto in una cartella temporanea, fuori dal repo ──────────────────
PKG="$(mktemp -d)"
trap 'rm -rf "$PKG"' EXIT
cp index.html "$PKG/"
if [[ -f vercel.json ]]; then cp vercel.json "$PKG/"; fi
for f in "${ASSET[@]}"; do
  mkdir -p "$PKG/$(dirname "$f")"
  cp "$f" "$PKG/$f"
done
BYTE_LOCALI=$(wc -c < index.html | tr -d ' ')
echo "pacchetto: $(find "$PKG" -type f | wc -l | tr -d ' ') file, index.html $BYTE_LOCALI byte"

# Avviso, non blocco: si puo' voler provare online prima di committare.
if [[ -n "$(git -C "$ROOT" status --porcelain index.html 2>/dev/null)" ]]; then
  echo "nota: index.html ha modifiche non committate — pubblico comunque"
fi

if (( DRY )); then echo "--dry-run: non pubblico"; exit 0; fi

# ── 3. deploy su entrambi i progetti ─────────────────────────────────────────
cd "$PKG"
for p in "${PROGETTI[@]}"; do
  echo
  echo "── deploy su $p ──"
  rm -rf .vercel
  vercel link --yes --project "$p" >/dev/null
  vercel deploy --prod --yes >/dev/null
  echo "  fatto"
done

# ── 4. verifica che online ci sia davvero quello che hai in mano ─────────────
echo
sleep 5
ESITO=0
for p in "${PROGETTI[@]}"; do
  URL="https://$p.vercel.app"
  VIVI=$(curl -s --max-time 30 "$URL" -o "$PKG/live.html" -w '%{size_download}')
  if cmp -s "$ROOT/index.html" "$PKG/live.html"; then
    printf '  %-34s %s byte  identico\n' "$URL" "$VIVI"
  else
    printf '  %-34s %s byte  DIVERSO da index.html (%s)\n' "$URL" "$VIVI" "$BYTE_LOCALI"
    ESITO=1
  fi
done

echo
if (( ESITO )); then
  echo "Qualcosa non torna: la cache di Vercel puo' metterci qualche secondo."
  echo "Riprova la verifica fra poco:  curl -s https://acidlab.vercel.app | wc -c"
else
  echo "Online. Sull'iPad basta ricaricare la pagina: la pagina esce con"
  echo "cache-control max-age=0 must-revalidate e non c'e' service worker,"
  echo "quindi al primo caricamento prende la versione nuova."
fi
exit $ESITO
