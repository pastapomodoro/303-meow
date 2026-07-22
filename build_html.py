"""Assembles Resources/ui.html from the sketch (HTML/CSS+JS) + JUCE-bridge overlay."""
import os

BASE   = os.path.dirname(__file__)
SKETCH = os.path.join(BASE, "sketch definitivo.html")
OUT    = os.path.join(BASE, "Resources", "ui.html")

with open(SKETCH) as f:
    lines = f.readlines()

# Take everything up to (but not including) </body>
end_idx = next(i for i in range(len(lines)-1, -1, -1) if lines[i].strip() == "</body>")
full_body = "".join(lines[:end_idx])

# ── JUCE bridge: second <script> injected just before </body> ─────────────────
BRIDGE = r"""
<script>
// ═══════════════════════════════════════════════════════════════════════════
//  JUCE 8 WebView bridge — overlays on top of the sketch JS
//  Runs after DOM+sketch JS are both ready (deferred via DOMContentLoaded)
// ═══════════════════════════════════════════════════════════════════════════
(function(){
  if(typeof window.__JUCE__ === 'undefined') return; // standalone browser: no-op

  // ── Minimal JUCE helper wrappers ─────────────────────────────────────────
  class ListenerList {
    constructor(){ this._l=new Map(); this._id=0; }
    add(fn){ const id=this._id++; this._l.set(id,fn); return id; }
    remove(id){ this._l.delete(id); }
    fire(data){ this._l.forEach(fn=>fn(data)); }
  }
  class SliderState {
    constructor(name){
      this.name=name; this._id='__juce__slider'+name;
      this.scaledValue=0;
      this.properties={start:0,end:1,skew:1,name:'',label:'',numSteps:100,interval:0,parameterIndex:-1};
      this.valueChangedEvent=new ListenerList();
      this.propertiesChangedEvent=new ListenerList();
      window.__JUCE__.backend.addEventListener(this._id,ev=>this._onEvent(ev));
      window.__JUCE__.backend.emitEvent(this._id,{eventType:'requestInitialUpdate'});
    }
    _onEvent(ev){
      if(ev.eventType==='valueChanged'){this.scaledValue=ev.value;this.valueChangedEvent.fire();}
      if(ev.eventType==='propertiesChanged'){const{eventType,...r}=ev;this.properties=r;this.propertiesChangedEvent.fire();}
    }
    getNormalisedValue(){
      const{start,end,skew}=this.properties;
      if(end===start)return 0;
      return Math.pow((this.scaledValue-start)/(end-start),skew);
    }
    setNormalisedValue(v){
      const{start,end,skew}=this.properties;
      this.scaledValue=Math.pow(Math.max(0,Math.min(1,v)),1/skew)*(end-start)+start;
      window.__JUCE__.backend.emitEvent(this._id,{eventType:'valueChanged',value:this.scaledValue});
    }
    getScaledValue(){return this.scaledValue;}
    sliderDragStarted(){window.__JUCE__.backend.emitEvent(this._id,{eventType:'sliderDragStarted'});}
    sliderDragEnded()  {window.__JUCE__.backend.emitEvent(this._id,{eventType:'sliderDragEnded'});}
  }
  class ToggleState {
    constructor(name){
      this.name=name; this._id='__juce__toggle'+name;
      this.value=false; this.valueChangedEvent=new ListenerList();
      window.__JUCE__.backend.addEventListener(this._id,ev=>this._onEvent(ev));
      window.__JUCE__.backend.emitEvent(this._id,{eventType:'requestInitialUpdate'});
    }
    _onEvent(ev){if(ev.eventType==='valueChanged'){this.value=!!ev.value;this.valueChangedEvent.fire();}}
    getValue(){return this.value;}
    setValue(v){this.value=!!v;window.__JUCE__.backend.emitEvent(this._id,{eventType:'valueChanged',value:this.value});}
  }
  let _pid=0; const _pending=new Map();
  window.__JUCE__.backend.addEventListener('__juce__complete',({promiseId,result})=>{
    if(_pending.has(promiseId)){_pending.get(promiseId)(result);_pending.delete(promiseId);}
  });
  function getNativeFunction(name){
    return function(...args){
      return new Promise(resolve=>{
        const id=_pid++;
        _pending.set(id,resolve);
        window.__JUCE__.backend.emitEvent('__juce__invoke',{name,params:args,resultId:id});
      });
    };
  }
  const _sc={},_tc={};
  function S(n){if(!_sc[n])_sc[n]=new SliderState(n);return _sc[n];}
  function T(n){if(!_tc[n])_tc[n]=new ToggleState(n);return _tc[n];}

  // ── Native function handles ───────────────────────────────────────────────
  const fnSelectPattern  =getNativeFunction('selectPattern');
  const fnToggleStep     =getNativeFunction('toggleStep');
  const fnSetStepNote    =getNativeFunction('setStepNote');
  const fnSetAccent      =getNativeFunction('setStepAccent');
  const fnSetSlide       =getNativeFunction('setStepSlide');
  const fnSetResolution  =getNativeFunction('setResolution');
  const fnLoadPreset     =getNativeFunction('loadPreset');
  const fnLoadSynthPreset=getNativeFunction('loadSynthPreset');
  const fnSetMidiMode    =getNativeFunction('setMidiMode');

  // ── Step data mirror (updated from stateUpdate) ───────────────────────────
  const stepData = Array.from({length:16},(_,i)=>({
    gate:  [0,3,5,8,11,14].includes(i),
    note:  [45,48,45,43,45,40,43,45,50,45,48,45,43,41,40,38][i],
    accent:[0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0][i]===1,
    slide: [0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0][i]===1,
  }));
  let juceCurrentStep=0, jucePattern=0;

  // ── Knob drag helper ──────────────────────────────────────────────────────
  const knobNorm={};  // cache of current normalised value per knob element
  function setKnobAngle(knobEl,v){
    if(!knobEl)return;
    // Sprite-knob path (shared helper from the sketch); fall back to CSS rotate.
    if(window.setKnobFrame){ window.setKnobFrame(knobEl,v); }
    else { knobEl.style.transform=`rotate(${-140+v*280}deg)`; }
  }
  function makeDraggable(knobEl, onNorm){
    let sy=0,sv=0,drag=false;
    knobEl.style.cursor='ns-resize';
    knobEl.addEventListener('mousedown',e=>{
      drag=true; sy=e.clientY; sv=knobNorm[knobEl]||0; e.preventDefault();
    });
    knobEl.addEventListener('touchstart',e=>{
      drag=true; sy=e.touches[0].clientY; sv=knobNorm[knobEl]||0; e.preventDefault();
    },{passive:false});
    const move=cy=>{
      if(!drag)return;
      const v=Math.max(0,Math.min(1,sv+(sy-cy)/160));
      knobNorm[knobEl]=v; setKnobAngle(knobEl,v); onNorm(v);
    };
    document.addEventListener('mousemove',e=>{if(drag)move(e.clientY);});
    document.addEventListener('touchmove',e=>{if(drag)move(e.touches[0].clientY);},{passive:true});
    document.addEventListener('mouseup',()=>{drag=false;});
    document.addEventListener('touchend',()=>{drag=false;});
  }

  // ── Wire synth knobs ──────────────────────────────────────────────────────
  // Map: knob-label text → [paramId, formatFn, valSuffix]
  const KNOB_MAP = {
    'Cutoff':    ['cutoff',    v=>Math.round(v)+'Hz',  null],
    'Resonance': ['resonance', v=>v.toFixed(2),        null],
    'Env Mod':   ['envMod',    v=>v.toFixed(2),        null],
    'Decay':     ['decay',     v=>v.toFixed(2)+'s',    null],
    'Volume':    ['volume',    v=>v.toFixed(2),        null],
    'Distort':   ['distortion',v=>v.toFixed(2),        null],
    'Accent':    ['accent',    v=>v.toFixed(2),        null],
    'Tuning':    ['tuning',    v=>v.toFixed(2),        null],
  };

  document.querySelectorAll('.knob-wrap').forEach(wrap=>{
    const lbl = wrap.querySelector('.knob-lbl');
    const valEl = wrap.querySelector('.knob-val');
    const knob = wrap.querySelector('.knob');
    if(!lbl||!knob) return;
    knob.classList.add('sprite-knob');   // render via spritesheet, not CSS gradient
    const key = lbl.textContent.trim();
    const entry = KNOB_MAP[key];
    if(!entry) return;
    const [param, fmt] = entry;
    const state = S(param);

    // JUCE → visual  (sprite frame uses the normalised 0..1 value; the label
    // shows the real scaled value — Hz, seconds, semitones, …)
    state.valueChangedEvent.add(()=>{
      const nv=state.getNormalisedValue();
      knobNorm[knob]=nv; setKnobAngle(knob,nv);
      if(valEl) valEl.textContent=fmt(state.getScaledValue());
    });
    // Drag → JUCE
    makeDraggable(knob, nv=>{
      state.sliderDragStarted();
      state.setNormalisedValue(nv);
      state.sliderDragEnded();
      if(valEl) valEl.textContent=fmt(state.getScaledValue());
    });
    // Init visual
    const nv0=state.getNormalisedValue();
    knobNorm[knob]=nv0; setKnobAngle(knob,nv0);
    if(valEl) valEl.textContent=fmt(state.getScaledValue());
  });

  // ── Play / Stop ──────────────────────────────────────────────────────────
  const playState = T('play');
  playState.valueChangedEvent.add(()=>{
    playing = playState.getValue();
    updatePlayInd();
    if(!playing){clearInterval(iv);curStep=0;buildSteps();buildLCD('lcd-s');buildLCD('lcd-s2');}
  });
  window.togglePlay = function(){
    playing=!playing;
    playState.setValue(playing);
    updatePlayInd();
    if(playing){
      iv=setInterval(()=>{
        curStep=(curStep+1)%16;
        buildSteps();buildLCD('lcd-s');buildLCD('lcd-s2');
        const sl=document.getElementById('slcd');
        if(sl) sl.textContent=String(curStep+1).padStart(2,'0');
      },stepMs());
    } else {
      clearInterval(iv);curStep=0;buildSteps();buildLCD('lcd-s');buildLCD('lcd-s2');
    }
  };
  window.togglePlay2=window.togglePlay;
  function stepMs(){
    const bpmVal=S('tempo').getScaledValue()||120;
    return Math.round(60000/bpmVal/4*4);
  }

  // ── Waveform ──────────────────────────────────────────────────────────────
  const waveState = T('waveform');
  waveState.valueChangedEvent.add(()=>{
    const isSqr=waveState.getValue();
    document.querySelectorAll('.wave-btn').forEach((b,i)=>{
      b.classList.toggle('active',i===(isSqr?1:0));
    });
  });
  window.setWave = function(el){
    document.querySelectorAll('.wave-btn').forEach(b=>b.classList.remove('active'));
    el.classList.add('active');
    waveState.setValue(el.textContent.trim()==='SQR');
  };

  // ── BPM / Tempo ───────────────────────────────────────────────────────────
  const tempoState = S('tempo');
  // APVTS "tempo" range is 60-200 BPM -> normalise over span 140 (was wrongly 160)
  const BPM_MIN=60, BPM_MAX=200;
  const bpmDisplays=()=>['bpm-display','bpm-hdr','bpm-lcd','bpm-lcd2']
    .map(id=>document.getElementById(id)).filter(Boolean);
  const showBpm=v=>bpmDisplays().forEach(el=>{ el.textContent=v; });
  const pushBpm=()=>{
    bpm=Math.max(BPM_MIN,Math.min(BPM_MAX,bpm));   // match param range, not 40-220
    showBpm(bpm);
    const n=(bpm-BPM_MIN)/(BPM_MAX-BPM_MIN);
    tempoState.sliderDragStarted();tempoState.setNormalisedValue(n);tempoState.sliderDragEnded();
  };
  tempoState.valueChangedEvent.add(()=>{
    bpm=Math.round(tempoState.getScaledValue());
    showBpm(bpm);
  });
  const _origChangeBPM=window.changeBPM;
  window.changeBPM = function(d){ _origChangeBPM&&_origChangeBPM(d); pushBpm(); };
  const _origTapTempo=window.tapTempo;
  window.tapTempo = function(){ _origTapTempo&&_origTapTempo(); pushBpm(); };

  // ── MIDI mode ─────────────────────────────────────────────────────────────
  const _origToggleMidi=window.toggleMidi;
  window.toggleMidi = function(el){
    _origToggleMidi&&_origToggleMidi(el);
    fnSetMidiMode(midiOn);
  };

  // ── Pattern buttons ───────────────────────────────────────────────────────
  const _origBuildKeycaps=window.buildKeycaps;
  window.buildKeycaps = function(){
    const grid=document.getElementById('pat-btns'); if(!grid) return;
    grid.innerHTML='';
    for(let i=0;i<8;i++){
      const key=document.createElement('div');
      key.className='keycap'+(i===jucePattern?' active':'');
      key.innerHTML=`<div class="keycap-base"><div class="keycap-top" style="padding:22px 4px 20px;"><div class="keycap-num" style="font-size:20px;">${i+1}</div></div></div>`;
      key.onclick=()=>{
        jucePattern=i;
        grid.querySelectorAll('.keycap').forEach((k,j)=>k.classList.toggle('active',j===i));
        fnSelectPattern(i);
      };
      grid.appendChild(key);
    }
  };
  window.buildKeycaps();

  // ── Step resolution ───────────────────────────────────────────────────────
  // Two button sets: SETTINGS ("1/4 Note"...) and SEQUENCER toolbar ("1/4"...).
  // Wire both, and reflect the active resolution visually across both sets.
  const RES_MAP={'1/4':1,'1/8':2,'1/16':4,'1/4 note':1,'1/8 note':2,'1/16 note':4};
  const resKey=b=>b.textContent.trim().toLowerCase();
  const resButtons=()=>[...document.querySelectorAll('button.btn')]
    .filter(b=>RES_MAP[resKey(b)]!==undefined);
  function setResolutionUI(res){
    fnSetResolution(res);
    resButtons().forEach(b=>b.classList.toggle('acid', RES_MAP[resKey(b)]===res));
  }
  resButtons().forEach(b=>b.addEventListener('click',()=>setResolutionUI(RES_MAP[resKey(b)])));

  // ── Step grid (SEQUENCER tab) ─────────────────────────────────────────────
  const noteNames=['C','C#','D','D#','E','F','F#','G','G#','A','A#','B'];
  const midiToName=n=>noteNames[n%12]+(Math.floor(n/12)-1);

  window.buildSteps = function(){
    const g=document.getElementById('steps-grid');
    const sub=document.getElementById('step-sub');
    if(!g||!sub) return;
    g.innerHTML=''; sub.innerHTML='';
    for(let i=0;i<16;i++){
      const s=stepData[i];
      const pl=i===juceCurrentStep&&playing;
      const b=document.createElement('div');
      b.className='step'+(s.gate?' on':'')+(pl?' playing':'');
      b.innerHTML=`<div class="step-num">${String(i+1).padStart(2,'0')}</div>
        <div class="step-led"></div>
        <div class="step-note">${s.gate?midiToName(s.note):'—'}</div>`;
      b.onclick=()=>{ s.gate=!s.gate; fnToggleStep(i); window.buildSteps(); buildLCD('lcd-s'); };
      g.appendChild(b);
      const sc=document.createElement('div'); sc.className='step-sub-col';
      const acc=document.createElement('div');
      acc.className='sub'+(s.accent?' acc':''); acc.textContent='ACC';
      acc.onclick=e=>{e.stopPropagation();s.accent=!s.accent;fnSetAccent(i,s.accent);acc.className='sub'+(s.accent?' acc':'');};
      const sld=document.createElement('div');
      sld.className='sub'+(s.slide?' sld':''); sld.textContent='SLD';
      sld.onclick=e=>{e.stopPropagation();s.slide=!s.slide;fnSetSlide(i,s.slide);sld.className='sub'+(s.slide?' sld':'');};
      sc.appendChild(acc); sc.appendChild(sld); sub.appendChild(sc);
    }
  };

  // ── Piano roll: override canvas click to use JUCE ─────────────────────────
  const STEPS=16, ROWS=24, NOTE_H=18;
  setTimeout(()=>{
    const canvas=document.getElementById('piano-roll-canvas');
    if(!canvas) return;
    // Piano roll uses its own local `notes` Set — sync stepData ↔ notes
    function syncNotesToCanvas(){
      notes.clear();
      const ALL_NOTES=[];
      for(let oct=4;oct>=3;oct--){
        ['B','A#','A','G#','G','F#','F','E','D#','D','C#','C'].forEach(n=>ALL_NOTES.push(n+oct));
      }
      stepData.forEach((s,si)=>{
        if(!s.gate) return;
        const noteName=midiToName(s.note); // e.g. "A3"
        const row=ALL_NOTES.findIndex(n=>n===noteName);
        if(row>=0) notes.add(`${si}:${row}`);
      });
      if(window._drawPR) window._drawPR();
    }

    // Patch mousedown: after existing handler, also call JUCE
    canvas.addEventListener('mousedown', e=>{
      const rect=canvas.getBoundingClientRect();
      const W=rect.width, cW=W/STEPS;
      const si=Math.floor((e.clientX-rect.left)/cW);
      const row=Math.floor((e.clientY-rect.top)/NOTE_H);
      if(si<0||si>=STEPS||row<0||row>=ROWS) return;
      // midiNote from row: row 0=B4(71), row 23=C3(48)
      const midiNote=71-row;
      const s=stepData[si];
      if(s.gate&&s.note===midiNote){ s.gate=false; fnToggleStep(si); }
      else { s.note=midiNote; s.gate=true; fnSetStepNote(si,midiNote); }
      window.buildSteps(); buildLCD('lcd-s');
    });

    // Initial sync
    syncNotesToCanvas();
    window._syncNotesToCanvas=syncNotesToCanvas;
  }, 250);

  // ── FX knobs → JUCE (patch after initFXKnobs runs at 150ms) ─────────────
  setTimeout(()=>{
    const FX_PARAMS={delay:'delayMix', reverb:'reverbMix', dist:'distortion'};
    Object.entries(FX_PARAMS).forEach(([id,param])=>{
      const state=S(param);
      // JUCE → fxVals
      state.valueChangedEvent.add(()=>{
        fxVals[id]=Math.round(state.getNormalisedValue()*100);
        updateFX(id);
      });
      // Patch canvas mousedown to also push to JUCE
      const c=document.getElementById('knob-canvas-'+id);
      if(!c) return;
      c.addEventListener('mousedown',e=>{
        state.sliderDragStarted();
        const start=fxVals[id];
        const startY=e.clientY;
        const onMove=ev=>{
          const nv=Math.max(0,Math.min(100,start+Math.round((startY-ev.clientY)*0.6)));
          state.setNormalisedValue(nv/100);
        };
        const onUp=()=>{
          state.sliderDragEnded();
          document.removeEventListener('mousemove',onMove);
          document.removeEventListener('mouseup',onUp);
        };
        document.addEventListener('mousemove',onMove);
        document.addEventListener('mouseup',onUp);
      },{capture:true});
    });
  },300);

  // ── Preset loading ────────────────────────────────────────────────────────
  const _origLoadPreset=window.loadPreset;
  window.loadPreset = function(){
    _origLoadPreset&&_origLoadPreset();
    const fs=document.getElementById('factory-select');
    const ss=document.getElementById('synth-select');
    if(fs) fnLoadPreset(parseInt(fs.value));
    if(ss) fnLoadSynthPreset(parseInt(ss.value));
    setTimeout(()=>window.__JUCE__.backend.emitEvent('requestStateUpdate',{}),250);
  };

  // ── C++ → JS: stateUpdate event ──────────────────────────────────────────
  window.__JUCE__.backend.addEventListener('stateUpdate',data=>{
    if(data.step!==undefined)    juceCurrentStep=data.step;
    if(data.playing!==undefined) playing=data.playing;
    if(data.pattern!==undefined){
      jucePattern=data.pattern;
      document.querySelectorAll('#pat-btns .keycap').forEach((k,i)=>k.classList.toggle('active',i===jucePattern));
    }
    if(data.steps){
      data.steps.forEach((s,i)=>{
        stepData[i].gate=s.gate; stepData[i].note=s.note;
        stepData[i].accent=s.accent; stepData[i].slide=s.slide;
      });
    }
    curStep=juceCurrentStep;
    window.buildSteps(); buildLCD('lcd-s'); buildLCD('lcd-s2');
    updatePlayInd();
    if(window._syncNotesToCanvas) window._syncNotesToCanvas();
    // Guida la testina del piano roll con lo stato reale del sequencer
    window._prStep = juceCurrentStep; window.playing2 = playing;
    if(window._drawPR) window._drawPR();
    const sl=document.getElementById('slcd');
    if(sl) sl.textContent=String(curStep+1).padStart(2,'0');
  });

  // Request initial state
  setTimeout(()=>window.__JUCE__.backend.emitEvent('requestStateUpdate',{}),400);

})();
</script>
</body>
</html>
"""

with open(OUT, 'w') as f:
    f.write(full_body)
    f.write(BRIDGE)

print(f"Written {OUT} ({sum(1 for _ in open(OUT))} lines)")
