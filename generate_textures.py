#!/usr/bin/env python3
"""
generate_textures.py — Genera texture per 303 Meow via Gemini Imagen 3 API.

API key: variabile d'ambiente GEMINI_API_KEY o file .env (NON versionato).
Crea .env nella root del progetto con: GEMINI_API_KEY=AIza...

Uso:
    python3 generate_textures.py               # genera tutti i preset
    python3 generate_textures.py --pick 0 2    # genera solo i preset 0 e 2
    python3 generate_textures.py --list        # mostra i preset disponibili
    python3 generate_textures.py --open        # genera e apre in Preview (macOS)
"""

import os
import sys
import json
import base64
import argparse
import subprocess
from pathlib import Path
from datetime import datetime

try:
    import requests
except ImportError:
    print("Dipendenza mancante: pip install requests")
    sys.exit(1)

# ── API key: env var o file .env ──────────────────────────────────────────────

def load_api_key() -> str:
    key = os.environ.get("GEMINI_API_KEY", "")
    if key:
        return key

    for candidate in [Path(".env"), Path(__file__).parent / ".env"]:
        if candidate.exists():
            for line in candidate.read_text(encoding="utf-8").splitlines():
                line = line.strip()
                if line.startswith("GEMINI_API_KEY=") and not line.startswith("#"):
                    return line.split("=", 1)[1].strip().strip('"').strip("'")

    print(
        "\n[ERRORE] GEMINI_API_KEY non trovata.\n"
        "Crea un file .env nella root del progetto con:\n"
        "    GEMINI_API_KEY=AIza...\n"
        "oppure esporta la variabile d'ambiente:\n"
        "    export GEMINI_API_KEY=AIza...\n"
    )
    sys.exit(1)


# ── Preset texture ─────────────────────────────────────────────────────────────
#
# Ogni preset ha:
#   name   → nome del file PNG output (senza estensione)
#   prompt → prompt per Imagen 3
#   desc   → descrizione per il log / approvazione
#   css_var→ variabile CSS target in sketch definitivo.html

PRESETS = [
    {
        "name": "panel_aluminum_light",
        "prompt": (
            "seamless tileable brushed aluminum texture, "
            "very light grey color #d8d8d4, "
            "fine uniform horizontal grain from brushing process, "
            "matte anodized industrial finish, "
            "extremely subtle micro-specular highlights on the grain, "
            "professional audio synthesizer front panel material, "
            "overhead flat-lay, completely uniform lighting, "
            "no shadows, no reflections, no text, no logos, no scratches, "
            "512x512 repeating texture, photorealistic material"
        ),
        "desc": "Alluminio spazzolato chiaro — per il pannello principale (.chassis)",
        "css_var": "--panel-face",
    },
    {
        "name": "panel_aluminum_mid",
        "prompt": (
            "seamless tileable brushed aluminum texture, "
            "medium grey #b8bcc4, "
            "horizontal brushed grain, anodized matte surface, "
            "subtle directionality from mechanical brushing, "
            "slightly cooler tone than warm grey, "
            "audio equipment rack panel material, "
            "overhead flat-lay, uniform lighting, "
            "no shadows, no text, no logos, 512x512, photorealistic material"
        ),
        "desc": "Alluminio spazzolato medio — per il bordo/frame esterno del chassis",
        "css_var": "--panel-shad",
    },
    {
        "name": "panel_abs_plastic",
        "prompt": (
            "seamless tileable injection-molded ABS plastic texture, "
            "off-white light grey #e8e8e4, "
            "very fine orange-peel surface micro-texture, "
            "matte non-glossy finish, "
            "smooth molded plastic like Roland TR-808 front panel, "
            "synthesizer hardware enclosure material, "
            "overhead flat-lay, uniform lighting, "
            "no shadows, no text, no logos, no gloss, 512x512, photorealistic material"
        ),
        "desc": "Plastica ABS off-white — alternativa al metallo, estetica TB-303 originale",
        "css_var": "--panel-face",
    },
    {
        "name": "panel_aluminum_dark",
        "prompt": (
            "seamless tileable brushed aluminum texture, "
            "dark charcoal grey #3a3d42, "
            "fine horizontal grain, anodized dark finish, "
            "professional audio equipment dark panel, "
            "subtle metallic sheen, matte surface, "
            "overhead flat-lay, uniform lighting, "
            "no shadows, no text, 512x512, photorealistic material"
        ),
        "desc": "Alluminio scuro — per il topbar / nameplate (variante dark)",
        "css_var": "--body-grad background",
    },
]

# ── Gemini Imagen 3 API ───────────────────────────────────────────────────────

ENDPOINT = (
    "https://generativelanguage.googleapis.com/v1beta"
    "/models/gemini-2.5-flash-image:generateContent"
)


def generate_one(api_key: str, prompt: str, sample_count: int = 1) -> list[bytes]:
    """Chiama gemini-2.5-flash-image e restituisce lista di PNG bytes."""
    results = []
    for _ in range(sample_count):
        resp = requests.post(
            f"{ENDPOINT}?key={api_key}",
            json={
                "contents": [{"parts": [{"text": prompt}]}],
                "generationConfig": {"responseModalities": ["TEXT", "IMAGE"]},
            },
            timeout=90,
        )

        if resp.status_code != 200:
            try:
                err = resp.json()
            except Exception:
                err = resp.text
            raise RuntimeError(f"API error {resp.status_code}: {json.dumps(err, indent=2)}")

        data = resp.json()
        for candidate in data.get("candidates", []):
            for part in candidate.get("content", {}).get("parts", []):
                inline = part.get("inlineData", {})
                if inline.get("mimeType", "").startswith("image/"):
                    results.append(base64.b64decode(inline["data"]))

    return results


# ── Main ──────────────────────────────────────────────────────────────────────

def main():
    parser = argparse.ArgumentParser(description="Genera texture via Gemini Imagen 3")
    parser.add_argument("--list", action="store_true", help="Mostra i preset disponibili ed esci")
    parser.add_argument("--pick", nargs="+", type=int, metavar="N",
                        help="Genera solo i preset con indice N (0-based)")
    parser.add_argument("--open", action="store_true",
                        help="Apri le immagini generate in Preview (macOS)")
    parser.add_argument("--variants", type=int, default=1, metavar="N",
                        help="Quante varianti per preset (default 1, max 4)")
    args = parser.parse_args()

    if args.list:
        print("\nPreset texture disponibili:\n")
        for i, p in enumerate(PRESETS):
            print(f"  [{i}] {p['name']}")
            print(f"       {p['desc']}")
            print(f"       CSS target: {p['css_var']}\n")
        return

    api_key = load_api_key()

    out_dir = Path(__file__).parent / "assets" / "textures"
    out_dir.mkdir(parents=True, exist_ok=True)

    to_generate = PRESETS if args.pick is None else [PRESETS[i] for i in args.pick]
    variants = max(1, min(4, args.variants))

    generated_paths: list[Path] = []
    manifest = {
        "generated_at": datetime.now().isoformat(),
        "textures": [],
    }

    for preset in to_generate:
        name = preset["name"]
        prompt = preset["prompt"]
        desc = preset["desc"]

        print(f"\n→ {name}")
        print(f"  {desc}")
        print(f"  Prompt: {prompt[:80]}...")
        print(f"  Generando {variants} variante/i ... ", end="", flush=True)

        try:
            images = generate_one(api_key, prompt, sample_count=variants)
        except RuntimeError as e:
            print(f"\n  [ERRORE] {e}")
            continue

        paths_for_preset = []
        for idx, img_bytes in enumerate(images):
            suffix = f"_v{idx+1}" if len(images) > 1 else ""
            out_path = out_dir / f"{name}{suffix}.png"
            out_path.write_bytes(img_bytes)
            size_kb = out_path.stat().st_size // 1024
            generated_paths.append(out_path)
            paths_for_preset.append(str(out_path))
            print(f"\n  ✓ {out_path.name} ({size_kb} KB)", end="", flush=True)

        manifest["textures"].append({
            "preset_name": name,
            "desc": desc,
            "css_target": preset["css_var"],
            "files": paths_for_preset,
        })

    print(f"\n\n{len(generated_paths)} file generati in: {out_dir}\n")

    # Salva manifest
    manifest_path = out_dir / "manifest.json"
    manifest_path.write_text(json.dumps(manifest, indent=2, ensure_ascii=False))
    print(f"Manifest: {manifest_path}")

    # Apri in Preview
    if args.open and generated_paths and sys.platform == "darwin":
        subprocess.run(["open"] + [str(p) for p in generated_paths], check=False)
        print("Aperte in macOS Preview.")

    # Istruzioni per il passo successivo
    print(
        "\n─────────────────────────────────────────────────────────────────\n"
        "PASSO SUCCESSIVO:\n"
        "  Valuta le texture in Preview.\n"
        "  Quando hai scelto, copia il file approvato in Resources/:\n"
        "      cp assets/textures/<nome>.png Resources/panel_texture.png\n"
        "  Poi di' a Claude di procedere con l'integrazione JUCE.\n"
        "─────────────────────────────────────────────────────────────────\n"
    )


if __name__ == "__main__":
    main()
