#include "PluginEditor.h"
#include <BinaryData.h>

// ─────────────────────────────────────────────────────────────────────────────
// Build WebBrowserComponent::Options with all relays + resource provider
// ─────────────────────────────────────────────────────────────────────────────
static juce::WebBrowserComponent::Options makeOptions(TB303Editor* e,
    juce::WebSliderRelay& cutoff, juce::WebSliderRelay& resonance,
    juce::WebSliderRelay& envMod, juce::WebSliderRelay& decay,
    juce::WebSliderRelay& accent, juce::WebSliderRelay& volume,
    juce::WebSliderRelay& tuning, juce::WebSliderRelay& distortion,
    juce::WebSliderRelay& tempo,
    juce::WebSliderRelay& delayTime, juce::WebSliderRelay& delayFb,
    juce::WebSliderRelay& delayMix, juce::WebSliderRelay& reverbSize,
    juce::WebSliderRelay& reverbMix,
    juce::WebToggleButtonRelay& play, juce::WebToggleButtonRelay& waveform)
{
    using Opts = juce::WebBrowserComponent::Options;

    return Opts{}
        .withNativeIntegrationEnabled(true)
        .withOptionsFrom(cutoff)
        .withOptionsFrom(resonance)
        .withOptionsFrom(envMod)
        .withOptionsFrom(decay)
        .withOptionsFrom(accent)
        .withOptionsFrom(volume)
        .withOptionsFrom(tuning)
        .withOptionsFrom(distortion)
        .withOptionsFrom(tempo)
        .withOptionsFrom(delayTime)
        .withOptionsFrom(delayFb)
        .withOptionsFrom(delayMix)
        .withOptionsFrom(reverbSize)
        .withOptionsFrom(reverbMix)
        .withOptionsFrom(play)
        .withOptionsFrom(waveform)
        // ── Native functions ──────────────────────────────────────
        .withNativeFunction("selectPattern",
            [e](const juce::Array<juce::var>& args, juce::WebBrowserComponent::NativeFunctionCompletion c){
                if(!args.isEmpty()) e->processor.getSequencer().selectPattern((int)args[0]);
                c(juce::var{});
            })
        .withNativeFunction("toggleStep",
            [e](const juce::Array<juce::var>& args, juce::WebBrowserComponent::NativeFunctionCompletion c){
                if(!args.isEmpty()){
                    int step = (int)args[0];
                    auto& pat = e->processor.getSequencer().getPattern(
                                    e->processor.getSequencer().getCurrentPatternIndex());
                    auto s = pat.getStep(step);
                    s.gate = !s.gate;
                    pat.setStep(step, s);
                }
                c(juce::var{});
            })
        .withNativeFunction("setStepNote",
            [e](const juce::Array<juce::var>& args, juce::WebBrowserComponent::NativeFunctionCompletion c){
                if(args.size() >= 2){
                    int step = (int)args[0], note = (int)args[1];
                    auto& pat = e->processor.getSequencer().getPattern(
                                    e->processor.getSequencer().getCurrentPatternIndex());
                    auto s = pat.getStep(step);
                    // Toggle: if same note + gate on → turn off, else set note + gate on
                    if(s.gate && s.note == note) s.gate = false;
                    else { s.note = note; s.gate = true; }
                    pat.setStep(step, s);
                }
                c(juce::var{});
            })
        .withNativeFunction("setStepAccent",
            [e](const juce::Array<juce::var>& args, juce::WebBrowserComponent::NativeFunctionCompletion c){
                if(args.size() >= 2){
                    int step = (int)args[0]; bool on = (bool)args[1];
                    auto& pat = e->processor.getSequencer().getPattern(
                                    e->processor.getSequencer().getCurrentPatternIndex());
                    auto s = pat.getStep(step); s.accent = on; pat.setStep(step, s);
                }
                c(juce::var{});
            })
        .withNativeFunction("setStepSlide",
            [e](const juce::Array<juce::var>& args, juce::WebBrowserComponent::NativeFunctionCompletion c){
                if(args.size() >= 2){
                    int step = (int)args[0]; bool on = (bool)args[1];
                    auto& pat = e->processor.getSequencer().getPattern(
                                    e->processor.getSequencer().getCurrentPatternIndex());
                    auto s = pat.getStep(step); s.slide = on; pat.setStep(step, s);
                }
                c(juce::var{});
            })
        .withNativeFunction("setResolution",
            [e](const juce::Array<juce::var>& args, juce::WebBrowserComponent::NativeFunctionCompletion c){
                if(!args.isEmpty()) e->processor.setStepResolution((int)args[0]);
                c(juce::var{});
            })
        .withNativeFunction("loadPreset",
            [e](const juce::Array<juce::var>& args, juce::WebBrowserComponent::NativeFunctionCompletion c){
                if(!args.isEmpty()) e->processor.loadPreset((int)args[0]);
                c(juce::var{});
            })
        .withNativeFunction("loadSynthPreset",
            [e](const juce::Array<juce::var>& args, juce::WebBrowserComponent::NativeFunctionCompletion c){
                if(!args.isEmpty()) e->processor.loadSynthPreset((int)args[0]);
                c(juce::var{});
            })
        .withNativeFunction("setMidiMode",
            [e](const juce::Array<juce::var>& args, juce::WebBrowserComponent::NativeFunctionCompletion c){
                if(!args.isEmpty()) e->processor.setMidiMode((bool)args[0]);
                c(juce::var{});
            })
        .withNativeFunction("clearPattern",
            [e](const juce::Array<juce::var>&, juce::WebBrowserComponent::NativeFunctionCompletion c){
                auto& seq = e->processor.getSequencer();
                auto& pat = seq.getPattern(seq.getCurrentPatternIndex());
                for(int i=0;i<16;++i){ auto s=pat.getStep(i); s.gate=false; pat.setStep(i,s); }
                juce::Array<juce::var> arr;
                for(int i=0;i<16;++i){ const auto& s=pat.getStep(i);
                    juce::DynamicObject::Ptr o=new juce::DynamicObject();
                    o->setProperty("gate",s.gate); o->setProperty("note",s.note);
                    o->setProperty("accent",s.accent); o->setProperty("slide",s.slide);
                    o->setProperty("octave",s.octave); arr.add(juce::var(o.get())); }
                c(juce::var(arr));
            })
        .withNativeFunction("randomPattern",
            [e](const juce::Array<juce::var>&, juce::WebBrowserComponent::NativeFunctionCompletion c){
                auto& seq = e->processor.getSequencer();
                auto& pat = seq.getPattern(seq.getCurrentPatternIndex());
                static const int scale[] = {48,50,51,53,55,58,60,62,63};
                juce::Random rnd;
                for(int i=0;i<16;++i){
                    auto s=pat.getStep(i);
                    bool g = rnd.nextFloat() < 0.70f;
                    s.gate=g;
                    if(g){ s.note=scale[rnd.nextInt(9)]; s.octave=0;
                           s.accent=rnd.nextFloat()<0.28f; s.slide=rnd.nextFloat()<0.18f; }
                    pat.setStep(i,s);
                }
                juce::Array<juce::var> arr;
                for(int i=0;i<16;++i){ const auto& s=pat.getStep(i);
                    juce::DynamicObject::Ptr o=new juce::DynamicObject();
                    o->setProperty("gate",s.gate); o->setProperty("note",s.note);
                    o->setProperty("accent",s.accent); o->setProperty("slide",s.slide);
                    o->setProperty("octave",s.octave); arr.add(juce::var(o.get())); }
                c(juce::var(arr));
            })
        // ── Resource provider: serve ui.html ─────────────────────
        .withResourceProvider(
            [e](const juce::String& url) { return e->getResource(url); });
}

// ─────────────────────────────────────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────────────────────────────────────
TB303Editor::TB303Editor(TB303Processor& p)
    : AudioProcessorEditor(p),
      processor(p),
      browser(makeOptions(this,
          cutoffRelay, resonanceRelay, envModRelay, decayRelay,
          accentRelay, volumeRelay, tuningRelay, distortionRelay, tempoRelay,
          delayTimeRelay, delayFbRelay, delayMixRelay, reverbSizeRelay, reverbMixRelay,
          playRelay, waveformRelay))
{
    auto& apvts = processor.getAPVTS();

    // Slider attachments
    cutoffAtt     = std::make_unique<SA>(*apvts.getParameter("cutoff"),        cutoffRelay);
    resonanceAtt  = std::make_unique<SA>(*apvts.getParameter("resonance"),     resonanceRelay);
    envModAtt     = std::make_unique<SA>(*apvts.getParameter("envMod"),        envModRelay);
    decayAtt      = std::make_unique<SA>(*apvts.getParameter("decay"),         decayRelay);
    accentAtt     = std::make_unique<SA>(*apvts.getParameter("accent"),        accentRelay);
    volumeAtt     = std::make_unique<SA>(*apvts.getParameter("volume"),        volumeRelay);
    tuningAtt     = std::make_unique<SA>(*apvts.getParameter("tuning"),        tuningRelay);
    distortionAtt = std::make_unique<SA>(*apvts.getParameter("distortion"),    distortionRelay);
    tempoAtt      = std::make_unique<SA>(*apvts.getParameter("tempo"),         tempoRelay);
    delayTimeAtt  = std::make_unique<SA>(*apvts.getParameter("delayTime"),     delayTimeRelay);
    delayFbAtt    = std::make_unique<SA>(*apvts.getParameter("delayFeedback"), delayFbRelay);
    delayMixAtt   = std::make_unique<SA>(*apvts.getParameter("delayMix"),      delayMixRelay);
    reverbSizeAtt = std::make_unique<SA>(*apvts.getParameter("reverbSize"),    reverbSizeRelay);
    reverbMixAtt  = std::make_unique<SA>(*apvts.getParameter("reverbMix"),     reverbMixRelay);

    // Toggle attachments
    playAtt     = std::make_unique<TA>(*apvts.getParameter("play"),     playRelay);
    waveformAtt = std::make_unique<TA>(*apvts.getParameter("waveform"), waveformRelay);

    addAndMakeVisible(browser);
    browser.goToURL("juce://juce.backend/index.html");

    setSize(1160, 700);
    startTimer(50); // 20 fps for step/state updates
}

TB303Editor::~TB303Editor()
{
    stopTimer();
}

// ─────────────────────────────────────────────────────────────────────────────
// Layout
// ─────────────────────────────────────────────────────────────────────────────
void TB303Editor::resized()
{
    browser.setBounds(getLocalBounds());
}

// ─────────────────────────────────────────────────────────────────────────────
// Timer: push state updates to JS
// ─────────────────────────────────────────────────────────────────────────────
juce::var TB303Editor::buildStateVar()
{
    auto& seq = processor.getSequencer();
    int patIdx  = seq.getCurrentPatternIndex();
    int curStep = seq.getCurrentStep();
    bool isPlaying = seq.isPlaying();

    auto& pat = seq.getPattern(patIdx);

    juce::Array<juce::var> stepsArr;
    for(int i = 0; i < 16; ++i){
        const auto& s = pat.getStep(i);
        juce::DynamicObject::Ptr obj = new juce::DynamicObject();
        obj->setProperty("gate",   s.gate);
        obj->setProperty("note",   s.note);
        obj->setProperty("accent", s.accent);
        obj->setProperty("slide",  s.slide);
        stepsArr.add(juce::var(obj.get()));
    }

    juce::DynamicObject::Ptr state = new juce::DynamicObject();
    state->setProperty("step",    curStep);
    state->setProperty("playing", isPlaying);
    state->setProperty("pattern", patIdx);
    state->setProperty("steps",   juce::var(stepsArr));
    return juce::var(state.get());
}

void TB303Editor::timerCallback()
{
    auto& seq = processor.getSequencer();
    int curStep = seq.getCurrentStep();
    int curPat  = seq.getCurrentPatternIndex();

    // Emit stateUpdate whenever step or pattern changes
    if(curStep != lastStep || curPat != lastPattern)
    {
        lastStep    = curStep;
        lastPattern = curPat;
        browser.emitEventIfBrowserIsVisible("stateUpdate", buildStateVar());
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Resource provider — serves ui.html from binary data
// ─────────────────────────────────────────────────────────────────────────────
std::optional<juce::WebBrowserComponent::Resource>
TB303Editor::getResource(const juce::String& url)
{
    DBG("[getResource] url = '" << url << "'");
    // Serve ui.html for any root/index request
    if(url == "/" || url == "/index.html" || url.isEmpty())
    {
        const auto* data = reinterpret_cast<const std::byte*>(BinaryData::ui_html);
        std::vector<std::byte> vec(data, data + BinaryData::ui_htmlSize);

        return juce::WebBrowserComponent::Resource{
            std::move(vec), "text/html"
        };
    }

    // Serve the knob spritesheet (61-frame vertical PNG used by all knobs)
    if(url.endsWith("acidlab-knob-61f-128px.png"))
    {
        const auto* data = reinterpret_cast<const std::byte*>(BinaryData::acidlabknob61f128px_png);
        std::vector<std::byte> vec(data, data + BinaryData::acidlabknob61f128px_pngSize);

        return juce::WebBrowserComponent::Resource{
            std::move(vec), "image/png"
        };
    }

    // Serve the panel texture (brushed/plastic background for the chassis)
    if(url == "/assets/panel.png")
    {
        const auto* data = reinterpret_cast<const std::byte*>(BinaryData::panel_texture_png);
        std::vector<std::byte> vec(data, data + BinaryData::panel_texture_pngSize);

        return juce::WebBrowserComponent::Resource{
            std::move(vec), "image/png"
        };
    }

    return {};
}
