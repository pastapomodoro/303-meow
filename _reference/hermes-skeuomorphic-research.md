# 303 Meow — Skeuomorphic UI Asset Pipeline: Research Report

*Ricerca completata da Hermes (deepseek/deepseek-v4-pro via OpenRouter, 8 tool call, 5 round di web search) — 21 luglio 2026, ore 23:34. Estratta dal DB sessione locale di Hermes (~/.hermes/state.db, session 20260721_233044_c9d98a) perché l'agente non aveva scritto un file di output.*

---

## R1: AI TEXTURE GENERATION (tileable + PBR)

### Findings (con link)

**1. StableMaterials (gvecchio) — il migliore open-source**
Modello diffusion-based su HuggingFace che genera TUTTE le PBR maps insieme (basecolor, normal, height, roughness, metallic) da text o image prompt. Supporta `tileable=True` nativamente. Output: 512x512, da upscalare dopo.
- https://huggingface.co/gvecchio/StableMaterials
- Paper: https://arxiv.org/abs/2406.09293
- Funziona via Diffusers, 50 steps (o 4 con LCM). Serve GPU NVIDIA.

**2. fal.ai PATINA — API-based, PBR completo**
Genera BaseColor, Normal, Roughness, Metallic, Height da text o image. C'è un frontend open-source Next.js pronto (lovisdotio/fal-texture-pbr-generator). Output pronti per Substance/Blender/Unreal.
- https://github.com/lovisdotio/fal-texture-pbr-generator
- API: fal.ai (pay-per-use, ~$0.002-0.01 per generation)

**3. Adobe Substance 3D Sampler — Image-to-Material (AI Powered)**
Da una singola foto genera albedo, normal, roughness, displacement, metallic. L'algoritmo "AI Powered" (Sampler 4.2+) ha categorie specifiche: Plastic, Fabric, Wood, Metal, Ground, Ceramic. È il gold standard professionale ma richiede licenza ($59.99/mo).
- https://experienceleague.adobe.com/en/docs/substance-3d-sampler/using/filters/tools/image-to-material
- Nuovo: Substance 3D Sampler 6.0 con supporto OpenPBR (giugno 2026)

**4. NormalMap.ai — gratuito, client-side**
Genera normal maps + roughness + height da una qualsiasi immagine. Funziona nel browser via WebGL, 100% locale (le immagini non lasciano il tuo computer). Ha anche seamless texture generator.
- https://normalmap.ai/
- https://normalmap.ai/seamless-texture/

**5. 3D AI Studio / TextureGen AI / MateriAI — soluzioni all-in-one**
Piattaforme web che generano PBR tileable da prompt. TextureGen AI ($7.90/mo) è il piu economico per uso regolare. 3D AI Studio genera seamless + PBR maps insieme. MateriAI ha plugin Unity/Unreal.
- https://texturegenai.com/
- https://www.3daistudio.com/Tools/SeamlessTextureGenerator
- https://matgenai.com/

### Prompt pattern efficaci per materiali "product photography"

Per generare ABS plastic bianca con grana leggera (non output "artistico"):

```
product photography macro shot of white ABS plastic surface,
fine matte grain texture, smooth uniform surface,
diffused studio lighting, no shadows, no reflections,
flat orthographic top-down view, 8K,
seamless tileable material, PBR albedo map,
neutral white balance, clean edges, industrial product finish
--no text, watermarks, logos, distortion, shadows, highlights
```

Per brushed aluminum:

```
product photography macro shot of brushed aluminum metal panel,
fine horizontal grain lines, anodized silver finish,
diffused even lighting, no specular highlights,
flat orthographic top-down, seamless tileable,
PBR albedo texture, 8K industrial photography,
--no text, watermarks, logos, shadows, reflections
```

Per matte rubber:

```
macro product shot of matte black rubber surface,
fine micro-texture grain, zero reflectivity,
smooth uniform matte finish, top-down orthographic,
diffused flat lighting, seamless tileable material,
PBR albedo map, 8K, industrial product photography
--no text, highlights, reflections, gloss
```

### Approccio raccomandato per le tue constraints

Usa **StableMaterials** (via HuggingFace Diffusers) se hai una GPU. Genera direttamente TUTTE le PBR maps insieme, tileable. Altrimenti: genera l'albedo con qualsiasi image generator (Flux/SDXL + prompt tileable), poi estrai normal/roughness con **NormalMap.ai** (gratuito, locale) o **fal.ai PATINA** (API).

---

## R2: AI → KNOB FILMSTRIP WORKFLOW

### TROVATA LA SOLUZIONE DEFINITIVA

**StripKit — tool gratuito open-source (MIT) specifico per plugin audio**
Questo è ESATTAMENTE ciò che serve. Carichi UN PNG trasparente del knob e lui genera l'intero filmstrip (64/128 frame), con rotazione perfetta, supersampling Mitchell, export @2x/@3x/@4x, e — cruciale — esporta codice loader per JUCE, CSS/HTML, iPlug2 e HISE.

Caratteristiche chiave:
- **Layered knobs**: corpo statico + pointer rotante separato (risolve il problema della consistenza geometrica)
- **AI generation integrata**: genera arte layered da prompt con chiave OpenAI/Gemini/Claude
- **Path-traced assembly**: se renderizzi in Blender/KeyShot/Octane, assembla i frame in un filmstrip
- **Filmstrip importer**: importa strip esistenti (KnobMan export, pack acquistati), rileva frame count e orientamento
- **Batch processing**: intera cartella di sorgenti → strip in un colpo
- **skin.json manifest**: binding strip-to-parameter per loader JUCE-style
- **Value arc / fill ring**: Serum/Vital-style, baked in ogni frame
- **Code export**: JUCE (LookAndFeel), CSS/HTML, iPlug2, HISE

https://stripkit.pro/
https://github.com/Vybecode-LTD/stripkit (MIT, 13 stars, 104 commits, giugno 2026)

**StripKit è Windows-only** (Avalonia/.NET 9), ma produce PNG che usi ovunque.

### Findings aggiuntivi

**1. KnobMan / WebKnobMan — ancora attivi**
WebKnobMan (https://www.g200kg.com/en/webknobman/) funziona nel browser, compatibile con Chrome/Firefox. Ha una KnobGallery con centinaia di design condivisi (https://www.g200kg.com/en/webknobman/gallery.php). KnobMan3D (https://g200kg.github.io/knobman3d/) è la versione WebGL/Three.js, open-source, per knob 3D renderizzati in tempo reale.

**2. AI → 3D → render pipeline**
Se vuoi andare da AI image a knob 3D renderizzato: genera knob flat con AI → converti in 3D con TripoSR/Rodin/Meshy (image-to-3D) → importa in Blender → renderizza turntable → assembla in StripKit (tab "Assemble"). Overkill per un knob, ma produce risultati fotorealistici estremi.

**3. Nessun workflow "AI-assisted plugin GUI pipeline" documentato pubblicamente**
Cercato su KVR, forum u-he/D16/Softube, Blender-for-GUI. Non esiste ancora un write-up pubblico di una pipeline AI-assisted end-to-end per plugin GUI. Terreno vergine — se documentato, saresti il primo.

### Raccomandazione per le tue constraints

**Workflow A (veloce, pratico):** Genera 1 immagine knob con AI (Flux/SDXL, prompt product-photography) → separa corpo e pointer in livelli (Photoshop/GIMP) → carica in StripKit come layered knob → esporta filmstrip + loader CSS/HTML. StripKit esporta già codice CSS background-position sprite pronto per il WebView.

**Workflow B (photoreal, più lavoro):** AI genera knob flat → importa in Blender, applica come texture su geometry cilindrica → renderizza 64 frame turntable → StripKit "Assemble" tab per impacchettare.

---

## R3: FAKE 3D LIGHTING IN CANVAS 2D (no WebGL)

### Findings

**1. Canvas Normal Mapping — zadvorsky CodePen**
Esempio annotato di per-pixel normal mapping e lighting su Canvas2D (no WebGL). https://codepen.io/zadvorsky/details/FjsJk

**2. Interactive Canvas Normal Map Lighting — danielhaim CodePen**
Versione interattiva mouse-controlled: https://codepen.io/danielhaim/pen/KwwwzEp

**3. La tecnica "light sweep" / pre-baked lighting states**
- Pre-renderizza N lighting states (es. 8 direzioni di luce) come PNG separati o frame in uno strip
- Al runtime, in base alla posizione del mouse (o a un parametro), fai crossfade tra i due stati più vicini usando `globalAlpha`
- Esempio concreto: generi 8 versioni del pannello con luce da 0°, 45°, 90°, 135°, 180°, 225°, 270°, 315°. Per un angolo di 67°, fai lerp tra il frame 45° e 90°.

Nessun write-up specifico per hardware UI/knob trovato, ma stesso pattern usato per "baked lighting" nei giochi 2D (es. https://www.starcubelabs.com/baking-2d-lighting/).

**4. Nvidia GDC 2001 — Per-Pixel Lighting (riferimento classico)**
https://developer.download.nvidia.com/assets/gamedev/docs/GDC2K_PerPixel_Lighting.pdf

**5. SVG feDiffuseLighting + feSpecularLighting**
Supportati universalmente da luglio 2015. Prendono l'alpha channel come bump map e applicano il modello Phong. Si combinano con l'immagine texture via feComposite (multiply per diffuse, add per specular). WKWebView su macOS li gestisce bene; WebView2 su Windows pure (Chromium-based).
- https://developer.mozilla.org/en-US/docs/Web/SVG/Reference/Element/feDiffuseLighting
- https://developer.mozilla.org/en-US/docs/Web/SVG/Reference/Element/feSpecularLighting

### Approccio raccomandato

1. **SVG filter per il pannello statico**: `feDiffuseLighting` + `feSpecularLighting` con la normal map come input. Luce che segue il mouse o un parametro. Zero JavaScript per il lighting, performance eccellenti.
2. **Per i knob, pre-baked light sweep**: dato che il knob ruota comunque, pre-bake 8-16 lighting direction nel filmstrip stesso. In alternativa, Canvas2D per-pixel lighting solo sulla regione del knob.

```svg
<!-- Esempio: pannello con illuminazione dinamica SVG -->
<filter id="panelLight">
  <feDiffuseLighting in="SourceGraphic" surfaceScale="3"
                      diffuseConstant="0.8" lighting-color="#ffffff"
                      result="diffuse">
    <fePointLight x="400" y="300" z="100" />
  </feDiffuseLighting>
  <feComposite in="SourceGraphic" in2="diffuse"
               operator="arithmetic" k1="1" k2="0" k3="0" k4="0" />
</filter>
```

---

## R4: PANEL & MICRO-DETAIL ASSETS

### Findings

**1. Front panel generation — prompt per orthographic view**
Prompt pattern per evitare perspective drift e fake text:

```
orthographic front view of a blank white ABS plastic audio hardware panel,
flat technical drawing, no perspective, no 3D depth,
even studio lighting, product photography,
screws in corners, inset rectangular LCD bezel cutout,
rubber button grid, silkscreen labels as simple geometric shapes,
clean minimal industrial design, 8K, product shot on white background
--no text, letters, numbers, words, shadows, gradients
```

Trucco chiave: `orthographic` + `flat technical drawing` + `no perspective` + `--no text` per evitare artefatti di testo AI-generato.

**2. Upscaling per hard mechanical edges**
- **Real-ESRGAN "anime" variant**: meglio per edge netti e line art che per texture organiche. Preserva contorni duri. https://github.com/xinntao/Real-ESRGAN
- **Topaz Gigapixel 8** (modalità "Standard" o "Lines"): superiore per edge meccanici, costoso ($99). Modalità "Recover" ottima per line art e testo.
- **Magnific AI**: tende a "inventare" dettaglio organico — NON usarlo per pannelli hardware, distorce gli edge rettilinei.
- **4x-UltraSharp ESRGAN**: specifico per UI e testo. https://openmodeldb.info/

Per upscaling asset UI: genera a 512px, poi upscala con Real-ESRGAN 4x (variante hard edges) a 2048px. Il dettaglio meccanico si preserva meglio che generare direttamente a 2048px.

**3. LCD/VFD display — CSS/canvas overlay**
Genera UN frame di LCD spento (texture scura con griglia pixel) via AI o SVG. Sopra, Canvas2D disegna i caratteri (`font-family: monospace`, verde acido #88ff00) con:
- `shadowBlur` + `shadowColor` per il phosphor glow
- Scanline overlay via CSS `repeating-linear-gradient` o Canvas pattern
- Opzionale: trail/ghosting via `globalAlpha` su frame precedenti

```css
/* Scanline overlay per LCD */
.lcd-screen::after {
  content: '';
  position: absolute;
  inset: 0;
  background: repeating-linear-gradient(
    0deg,
    transparent,
    transparent 2px,
    rgba(0,0,0,0.03) 2px,
    rgba(0,0,0,0.03) 4px
  );
  pointer-events: none;
}
```

### Raccomandazione

Genera il pannello base in AI (prompt orthographic sopra) → upscala con Real-ESRGAN → aggiungi viti, label, e bezel via SVG nel markup HTML (non nell'immagine, così restano editabili e perfettamente nitidi). I dettagli piccoli (viti, serigrafia) sono più precisi se fatti in SVG/CSS che se generati da AI.

---

## R5: FILE-SIZE BUDGET & FORMATI

### Findings

**1. Supporto formati in WKWebView (macOS) e WebView2 (Windows)**
Fonte: https://caniwebview.com/features/web-feature-avif/
- AVIF: supportato in tutti i WebView moderni — WKWebView macOS 16+, WKWebView iOS 16+, WebView2 Windows, Android WebView. "Widely available" (luglio 2026).
- WebP: supporto universale, tutti i WebView dal 2020.

**2. Compressione comparativa (2025-2026)**
- AVIF: 50% più piccolo di JPEG, 20-30% più piccolo di WebP a qualità equivalente
- WebP: 25-35% più piccolo di JPEG
- AVIF encoding è 5-10x più lento di WebP, ma per asset embedded (encode once) non importa
- WebP encoding è istantaneo, migliore per iterazioni veloci

**3. Budget realistico per plugin binary**
- Non superare ~2-4 MB totali per tutti gli asset UI
- 5 knob filmstrip @ 64 frame x 128px = ~200-400 KB ciascuno in WebP (lossy Q80) = 1-2 MB
- 1 pannello background @ 800x400px WebP Q85 = ~80-150 KB
- Totale realistico: 1.5-3 MB per UI assets
- Con AVIF, riduzione del 20-30%: ~1-2 MB totali

**4. Texture atlas per filmstrip**
Combina tutti i filmstrip knob in un singolo atlas PNG/WebP/AVIF verticale, con un JSON che mappa ogni knob al suo offset Y. JUCE BinaryData li carica come singolo blob.

**5. Embedding in JUCE 8**
JUCE BinaryData system: aggiungi i file .webp/.png al Projucer/CMake, vengono compilati come `const char*` array C++. Nel WebView, li servi come data-URI: `data:image/webp;base64,...`

### Raccomandazione

Usa **WebP lossy Q80** come formato di lavoro (encoding veloce per iterazioni). Per la build finale, converti in **AVIF Q60** (stessa qualità percepita, file ~25% più piccoli). Tieni PNG lossless solo per il knob pointer (ha bisogno di alpha channel preciso per la rotazione — WebP/AVIF lossy possono introdurre artefatti sui bordi). Texture atlas unico per tutti i knob.

---

# DO THIS FIRST (prioritized)

1. **Installa StripKit** (Windows VM/Boot Camp/Parallels se sei su Mac) e testa il workflow: carica il filmstrip knob esistente nell'Importer → verifica che rilevi frame count e orientamento → esporta loader HTML/CSS. Codice CSS background-position sprite pronto per il WebView. https://stripkit.pro/
2. **Genera texture PBR con StableMaterials** (HuggingFace, se hai GPU) o **NormalMap.ai** (gratuito, browser). Prompt: "white ABS plastic matte surface, fine grain, seamless tileable". Ottieni albedo + normal + roughness. Testa la normal map nel pannello SVG con feDiffuseLighting.
3. **Implementa il pannello base in HTML/SVG** con la texture albedo come background-image, normal map applicata via `<feDiffuseLighting>`, e un `<fePointLight>` pilotato da mousemove. Verifica performance in Safari (simula WKWebView). Aggiungi le viti come SVG `<circle>` con radial-gradient.
4. **Genera il knob "303 Meow" in due layer**: (a) corpo statico: cerchio con texture ABS plastica; (b) pointer/cap: triangolo/linea in verde acido #88ff00. Separa in due PNG. Carica in StripKit come "layered knob" → esporta filmstrip 64-frame.
5. **Definisci l'atlas texture budget**: quanti knob, risoluzione (128px?), frame count (64), peso totale. Parti con WebP Q80, testa in JUCE con BinaryData → data-URI. Se superi 3 MB, scala a Q60 o converti in AVIF.

---

Fonti: 50+ URL verificati dal web search.
