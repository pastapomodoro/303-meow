#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class TB303Editor : public juce::AudioProcessorEditor,
                    public juce::Timer
{
public:
    explicit TB303Editor(TB303Processor& p);
    ~TB303Editor() override;

    void paint(juce::Graphics&) override {}
    void resized() override;
    void timerCallback() override;

    TB303Processor& processor;

    std::optional<juce::WebBrowserComponent::Resource>
        getResource(const juce::String& url);

private:

    // ── Slider relays (one per APVTS float param) ────────────────
    juce::WebSliderRelay cutoffRelay      { "cutoff"        };
    juce::WebSliderRelay resonanceRelay   { "resonance"     };
    juce::WebSliderRelay envModRelay      { "envMod"        };
    juce::WebSliderRelay decayRelay       { "decay"         };
    juce::WebSliderRelay accentRelay      { "accent"        };
    juce::WebSliderRelay volumeRelay      { "volume"        };
    juce::WebSliderRelay tuningRelay      { "tuning"        };
    juce::WebSliderRelay distortionRelay  { "distortion"    };
    juce::WebSliderRelay tempoRelay       { "tempo"         };
    juce::WebSliderRelay delayTimeRelay   { "delayTime"     };
    juce::WebSliderRelay delayFbRelay     { "delayFeedback" };
    juce::WebSliderRelay delayMixRelay    { "delayMix"      };
    juce::WebSliderRelay reverbSizeRelay  { "reverbSize"    };
    juce::WebSliderRelay reverbMixRelay   { "reverbMix"     };

    // ── Toggle relays (binary APVTS params) ──────────────────────
    juce::WebToggleButtonRelay playRelay     { "play"     };
    juce::WebToggleButtonRelay waveformRelay { "waveform" };

    // ── WebBrowserComponent (must be declared AFTER relays) ──────
    juce::WebBrowserComponent browser;

    // ── Parameter attachments ─────────────────────────────────────
    using SA = juce::WebSliderParameterAttachment;
    using TA = juce::WebToggleButtonParameterAttachment;
    std::unique_ptr<SA> cutoffAtt, resonanceAtt, envModAtt, decayAtt;
    std::unique_ptr<SA> accentAtt, volumeAtt, tuningAtt, distortionAtt, tempoAtt;
    std::unique_ptr<SA> delayTimeAtt, delayFbAtt, delayMixAtt;
    std::unique_ptr<SA> reverbSizeAtt, reverbMixAtt;
    std::unique_ptr<TA> playAtt, waveformAtt;

    // ── Step / pattern tracking for timer ────────────────────────
    int lastStep    = -1;
    int lastPattern = -1;

    // ── Helper: serialize current pattern to var ──────────────────
    juce::var buildStateVar();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TB303Editor)
};
