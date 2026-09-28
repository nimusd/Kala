#!/usr/bin/env python3
"""Dedup stacked invisible note copies in Kala project files.

Anima's duplicate_notes bug (empty noteIds -> whole phrase) left 33 stacked
copies of ~384 notes in Kala no 21.kala (and 6 copies of 2 notes in no 20).
Copies share start/duration/pitch/track + all curve data, differing only by
UUID, so they stack invisibly on the canvas while all rendering/mixing.

This script:
  - backs up the .kala and .kalarender files
  - keeps the first occurrence of each content-identical group in the .kala
  - rewrites the .kalarender keeping only entries for the surviving note IDs
    (IDs are preserved, so graphHash/noteHash validation still passes on load)

Usage: python dedup_kala_notes.py <path/to/project.kala> [--dry-run]
"""
import json
import os
import shutil
import struct
import sys
import time


def note_key(note):
    """Canonical content key: everything except the id."""
    d = {k: v for k, v in note.items() if k != "id"}
    return json.dumps(d, sort_keys=True)


def dedup_notes(notes):
    seen = {}
    kept_ids = set()
    removed = 0
    kept_notes = []
    for note in notes:
        k = note_key(note)
        if k in seen:
            removed += 1
            continue
        seen[k] = True
        kept_ids.add(note.get("id", ""))
        kept_notes.append(note)
    return kept_notes, kept_ids, removed


def read_render_cache(path):
    with open(path, "rb") as f:
        magic = f.read(4)
        if magic != b"CLMR":
            raise ValueError(f"Bad magic {magic!r}")
        version = struct.unpack("<I", f.read(4))[0]
        sr = struct.unpack("<d", f.read(8))[0]
        count = struct.unpack("<I", f.read(4))[0]
        f.read(12)  # reserved
        entries = []
        for _ in range(count):
            tid = struct.unpack("<i", f.read(4))[0]
            l = struct.unpack("<I", f.read(4))[0]
            nid = f.read(l).decode("utf-8")
            gh, nh = struct.unpack("<QQ", f.read(16))
            # v2 stores each render's own rate; v1 used the header rate for all
            rate = struct.unpack("<d", f.read(8))[0] if version >= 2 else sr
            sc = struct.unpack("<Q", f.read(8))[0]
            samples = f.read(sc * 4)
            if len(samples) != sc * 4:
                raise ValueError(f"Truncated cache at entry {len(entries)}")
            entries.append((tid, nid, gh, nh, rate, sc, samples))
    return sr, entries


def write_render_cache(path, sr, entries):
    with open(path, "wb") as f:
        f.write(b"CLMR")
        f.write(struct.pack("<I", 2))
        f.write(struct.pack("<d", sr))
        f.write(struct.pack("<I", len(entries)))
        f.write(b"\x00" * 12)
        for tid, nid, gh, nh, rate, sc, samples in entries:
            data = nid.encode("utf-8")
            f.write(struct.pack("<i", tid))
            f.write(struct.pack("<I", len(data)))
            f.write(data)
            f.write(struct.pack("<QQ", gh, nh))
            f.write(struct.pack("<d", rate))
            f.write(struct.pack("<Q", sc))
            f.write(samples)


def main():
    if len(sys.argv) < 2:
        print(__doc__)
        return 1
    kala_path = sys.argv[1]
    dry_run = "--dry-run" in sys.argv[2:]
    cache_path = os.path.splitext(kala_path)[0] + ".kalarender"
    stamp = time.strftime("%Y%m%d-%H%M%S")

    with open(kala_path, encoding="utf-8-sig") as f:
        project = json.load(f)

    notes = project.get("notes", [])
    print(f"{os.path.basename(kala_path)}: {len(notes)} notes")

    kept_notes, kept_ids, removed = dedup_notes(notes)
    print(f"  kept {len(kept_notes)}, removed {removed} stacked copies")
    if dry_run:
        print("  DRY RUN — nothing written")
        return 0

    # Backups
    for p in (kala_path, cache_path):
        if os.path.exists(p):
            shutil.copy2(p, f"{p}.bak-{stamp}")
            print(f"  backup: {os.path.basename(p)}.bak-{stamp}")

    # Rewrite project
    project["notes"] = kept_notes
    with open(kala_path, "w", encoding="utf-8") as f:
        json.dump(project, f, indent=4, ensure_ascii=False)
    print(f"  wrote {len(kept_notes)} notes to {os.path.basename(kala_path)}")

    # Filter render cache
    if os.path.exists(cache_path):
        sr, entries = read_render_cache(cache_path)
        kept_entries = [e for e in entries if e[1] in kept_ids]
        missing = kept_ids - {e[1] for e in entries}
        print(f"  cache: {len(entries)} -> {len(kept_entries)} entries"
              f" ({len(kept_ids) - len(kept_entries)} kept notes have no cached render)"
              + (f"; MISSING RENDERS: {sorted(missing)[:5]}..." if missing else ""))
        write_render_cache(cache_path, sr, kept_entries)
        print(f"  wrote filtered cache: {os.path.basename(cache_path)}")

    # Verify round-trip
    with open(kala_path, encoding="utf-8") as f:
        check = json.load(f)
    print(f"  verify: file now has {len(check.get('notes', []))} notes")
    return 0


if __name__ == "__main__":
    sys.exit(main())
