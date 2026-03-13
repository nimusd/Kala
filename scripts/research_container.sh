#!/bin/bash
# Container Documentation Research Script
# Run this to gather all source information quickly

CONTAINER=$1

echo "=== Researching $CONTAINER ==="

# Find header file
echo "--- Header File ---"
find . -name "*$(echo $CONTAINER | tr '[:upper:]' '[:lower:]' | tr ' ' '-').h" -exec echo "File: {}" \; -exec head -50 {} \;

# Find implementation file  
echo -e "\n--- Implementation File ---"
find . -name "*$(echo $CONTAINER | tr '[:upper:]' '[:lower:]' | tr ' ' '-').cpp" -exec echo "File: {}" \; -exec head -100 {} \;

# Find integration points
echo -e "\n--- Integration Points ---"
grep -r "$CONTAINER" --include="*.cpp" --include="*.h" . | grep -E "(ports|inputs|outputs|parameters|default)" | head -10

# Find usage in container port specification
echo -e "\n--- Container Port Spec ---"
grep -A 20 -B 5 "$CONTAINER" "docs/md format/Container Port Specification v1.2.md"

# Find in sounitbuilder (defaults and port creation)
echo -e "\n--- SounitBuilder Integration ---"
grep -A 10 -B 2 "\"$CONTAINER\"" sounitbuilder.cpp

# Find in kalamain (inspector and settings)
echo -e "\n--- Inspector/Settings ---"
grep -A 10 -B 2 "$CONTAINER" kalamain.cpp