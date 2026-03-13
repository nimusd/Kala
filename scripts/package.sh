#!/usr/bin/env bash
# package.sh — Build a distributable Kala release folder
#
# Usage:  bash scripts/package.sh
# Run from the project root: C:\c++code\Kala\
#
# Output: dist/Kala/   (ready to zip)

set -e

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="$PROJECT_ROOT/build/Desktop_Qt_6_10_1_MinGW_64_bit-Release"
DIST_DIR="$PROJECT_ROOT/dist/Kala"
WINDEPLOYQT="/c/Qt/6.10.1/mingw_64/bin/windeployqt"
FFTW_DLL="$PROJECT_ROOT/fftw-3.3.5-dll64/libfftw3f-3.dll"

echo "=== Kala packager ==="
echo ""

# ── 1. Build ──────────────────────────────────────────────────────────────────
echo "[1/5] Building..."
cmake --build "$BUILD_DIR"
echo "      Build OK"

# ── 2. Prepare dist folder ────────────────────────────────────────────────────
echo "[2/5] Preparing dist folder..."
rm -rf "$DIST_DIR"
mkdir -p "$DIST_DIR"
cp "$BUILD_DIR/Kala.exe" "$DIST_DIR/"
echo "      Kala.exe copied"

# ── 3. Qt runtime (windeployqt) ───────────────────────────────────────────────
echo "[3/5] Running windeployqt..."
"$WINDEPLOYQT" --release "$DIST_DIR/Kala.exe"
echo "      Qt DLLs deployed"

# ── 4. FFTW DLL ───────────────────────────────────────────────────────────────
echo "[4/5] Copying libfftw3f-3.dll..."
cp "$FFTW_DLL" "$DIST_DIR/"
echo "      libfftw3f-3.dll copied"

# ── 5. Help documentation ─────────────────────────────────────────────────────
echo "[5/5] Copying help documentation..."
cp -r "$PROJECT_ROOT/help" "$DIST_DIR/"
echo "      help/ copied"

# ── 6. Zip ────────────────────────────────────────────────────────────────────
echo "[6/5] Creating Kala.zip..."
ZIP_PATH="$PROJECT_ROOT/dist/Kala.zip"
rm -f "$ZIP_PATH"
powershell -Command "Compress-Archive -Path '$(cygpath -w "$DIST_DIR")' -DestinationPath '$(cygpath -w "$ZIP_PATH")'"
echo "      Kala.zip created"

# ── Done ──────────────────────────────────────────────────────────────────────
echo ""
echo "=== Done! ==="
echo "    Folder : $DIST_DIR"
echo "    Zip    : $ZIP_PATH"
