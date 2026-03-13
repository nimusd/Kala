#!/usr/bin/env bash
# build_help.sh — Convert Kala markdown docs to help HTML via pandoc
# Run from the project root: bash scripts/build_help.sh

PANDOC="/c/Users/nimus/AppData/Local/Pandoc/pandoc.exe"
SRC="docs/Kala Documentation"
DEST="help"
TEMPLATE="help/pandoc-template.html"

convert() {
    local md="$1"
    local html="$2"
    local title="$3"
    "$PANDOC" "$SRC/$md" \
        --from=markdown \
        --to=html5 \
        --standalone \
        --template="$TEMPLATE" \
        --metadata title="$title" \
        -o "$DEST/$html"
    echo "  $html"
}

echo "Building Kala help files..."

# --- General ---
convert "liberation from the grid.md"          introduction.html          "Introduction"
convert "score canvas overview.md"             score-canvas.html          "The Score Canvas"
convert "Kala Sound Engine Overview.md"     sound-engine-overview.html "Sound Engine Overview"
convert "Kala dialog documentation.md"      dialogs.html               "Dialogs"
convert "sound_engine_settings.md"             settings.html              "Settings"

# --- Essential (Blue) ---
convert "harmonic generator documentation.md"  harmonic-generator.html    "Harmonic Generator"
convert "spectrum to signal documentation.md"  spectrum-to-signal.html    "Spectrum to Signal"
convert "karplus strong attack documentation.md" karplus-strong.html      "Karplus Strong"
convert "signal mixer documentation.md"        signal-mixer.html          "Signal Mixer"
convert "attack documentation.md"              attack.html                "Attack"
convert "wavetable synth documentation.md"     wavetable-synth.html       "Wavetable Synth"

# --- Shaping (Orange) ---
convert "rolloff processor documentation.md"   rolloff-processor.html     "Rolloff Processor"
convert "spectrum blender documentation.md"    spectrum-blender.html      "Spectrum Blender"
convert "formant body documentation.md"        formant-body.html          "Formant Body"
convert "breath turbulence documentation.md"   breath-turbulence.html     "Breath Turbulence"
convert "noise color filter documentation.md"  noise-color-filter.html    "Noise Color Filter"

# --- Modifiers (Green) ---
convert "physics system documentation.md"      physics-system.html        "Physics System"
convert "easing applicator documentation.md"   easing-applicator.html     "Easing Applicator"
convert "envelope engine documentation.md"     envelope-engine.html       "Envelope Engine"
convert "drift engine documentation.md"        drift-engine.html          "Drift Engine"
convert "lfo documentation.md"                 lfo.html                   "LFO"
convert "frequency mapper documentation.md"    frequency-mapper.html      "Frequency Mapper"

# --- Filters (Purple) ---
convert "10-band EQ documentation.md"          ten-band-eq.html           "10-Band EQ"
convert "comb filter documentation.md"         comb-filter.html           "Comb Filter"
convert "lp hp filter documentation.md"        lp-hp-filter.html          "LP/HP Filter"
convert "ir convolution documentation.md"      ir-convolution.html        "IR Convolution"

echo "Done. $(ls help/*.html | wc -l) HTML files in help/"
