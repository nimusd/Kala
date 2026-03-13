#!/bin/bash
# Batch generate all container documentation

containers=(
    "Spectrum to Signal:Essential"
    "Formant Body:Shaping" 
    "Breath Turbulence:Shaping"
    "Noise Color Filter:Shaping"
    "Physics System:Modifiers"
    "Easing Applicator:Modifiers"
    "Envelope Engine:Modifiers"
    "Drift Engine:Modifiers"
    "Gate Processor:Modifiers"
    "LFO:Modifiers"
    "Karplus Strong Attack:Modifiers"
)

for container_info in "${containers[@]}"; do
    name="${container_info%:*}"
    category="${container_info#*:}"
    
    echo "Generating documentation for: $name ($category)"
    python scripts/generate_docs.py "$name" "$category"
done

echo "All container documentation generated!"