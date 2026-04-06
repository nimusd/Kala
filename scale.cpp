#include "scale.h"
#include <cmath>
#include <QJsonObject>

// Default constructor
Scale::Scale(const QString &name, int scaleId)
    : name(name), scaleId(scaleId), tonicIndex(-1)
{
    // Default to Just Intonation
    ratios = {1.0, 9.0/8.0, 5.0/4.0, 4.0/3.0, 3.0/2.0, 5.0/3.0, 15.0/8.0};
    noteNames = {"C", "D", "E", "F", "G", "A", "B"};
}

// Private constructor with custom ratios and names
Scale::Scale(const QString &name, int scaleId, const QVector<double> &ratios, const QVector<QString> &noteNames,
             const QVector<bool> &accidentals, int tonicIdx)
    : name(name), scaleId(scaleId), ratios(ratios), noteNames(noteNames), accidentals(accidentals), tonicIndex(tonicIdx)
{
}

double Scale::getRatio(int degree) const
{
    if (degree >= 0 && degree < ratios.size()) {
        return ratios[degree];
    }
    return 1.0;  // Fallback to tonic
}

QString Scale::getNoteName(int degree) const
{
    if (degree >= 0 && degree < noteNames.size()) {
        return noteNames[degree];
    }
    return "?";
}

bool Scale::getIsAccidental(int degree) const
{
    if (degree >= 0 && degree < accidentals.size()) {
        return accidentals[degree];
    }
    return false;
}

// ============================================================================
// WESTERN SCALES
// ============================================================================

Scale Scale::justIntonation()
{
    QVector<double> ratios = {
        1.0,        // C (Tonic) - 1/1
        9.0/8.0,    // D - 9/8
        5.0/4.0,    // E - 5/4 (pure major third)
        4.0/3.0,    // F - 4/3
        3.0/2.0,    // G (Fifth) - 3/2
        5.0/3.0,    // A - 5/3
        15.0/8.0    // B - 15/8
    };
    QVector<QString> names = {"C", "D", "E", "F", "G", "A", "B"};
    return Scale("Just Intonation", 0, ratios, names);
}

Scale Scale::pythagorean()
{
    QVector<double> ratios = {
        1.0,         // C (Tonic) - 1/1
        9.0/8.0,     // D - 9/8
        81.0/64.0,   // E - 81/64 (Pythagorean major third)
        4.0/3.0,     // F - 4/3
        3.0/2.0,     // G (Fifth) - 3/2
        27.0/16.0,   // A - 27/16
        243.0/128.0  // B - 243/128
    };
    QVector<QString> names = {"C", "D", "E", "F", "G", "A", "B"};
    return Scale("Pythagorean", 1, ratios, names);
}

Scale Scale::equalTemperament()
{
    // Full 12-tone equal temperament chromatic scale: each semitone is 2^(1/12)
    // Accidentals (sharps) are flagged so the canvas draws them dark grey
    QVector<double> ratios = {
        1.0,                     // C  - 0 semitones  (diatonic)
        std::pow(2.0, 1.0/12),   // C# - 1 semitone   (accidental)
        std::pow(2.0, 2.0/12),   // D  - 2 semitones  (diatonic)
        std::pow(2.0, 3.0/12),   // D# - 3 semitones  (accidental)
        std::pow(2.0, 4.0/12),   // E  - 4 semitones  (diatonic)
        std::pow(2.0, 5.0/12),   // F  - 5 semitones  (diatonic)
        std::pow(2.0, 6.0/12),   // F# - 6 semitones  (accidental)
        std::pow(2.0, 7.0/12),   // G  - 7 semitones  (diatonic)
        std::pow(2.0, 8.0/12),   // G# - 8 semitones  (accidental)
        std::pow(2.0, 9.0/12),   // A  - 9 semitones  (diatonic)
        std::pow(2.0, 10.0/12),  // A# - 10 semitones (accidental)
        std::pow(2.0, 11.0/12)   // B  - 11 semitones (diatonic)
    };
    QVector<QString> names = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
    QVector<bool> acc      = { false, true, false, true, false, false, true, false, true, false, true, false };
    return Scale("Equal Temperament", 2, ratios, names, acc, 0);  // tonicIndex=0 means C is default tonic
}

Scale Scale::equalTemperamentWithTonic(int tonicIndex)
{
    if (tonicIndex < 0 || tonicIndex > 11) {
        tonicIndex = 0;  // Default to C
    }

    // Full 12-tone equal temperament - keep original ratios (C=1/1, C#=2^(1/12), etc.)
    // The tonicIndex only affects which degree gets the "tonic" color
    // Frequencies stay the same regardless of tonic - only colors change
    QVector<double> ratios = {
        1.0,
        std::pow(2.0, 1.0/12),
        std::pow(2.0, 2.0/12),
        std::pow(2.0, 3.0/12),
        std::pow(2.0, 4.0/12),
        std::pow(2.0, 5.0/12),
        std::pow(2.0, 6.0/12),
        std::pow(2.0, 7.0/12),
        std::pow(2.0, 8.0/12),
        std::pow(2.0, 9.0/12),
        std::pow(2.0, 10.0/12),
        std::pow(2.0, 11.0/12)
    };
    QVector<QString> names = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
    QVector<bool> acc = { false, true, false, true, false, false, true, false, true, false, true, false };

    return Scale("Equal Temperament", 2, ratios, names, acc, tonicIndex);
}

Scale Scale::quarterCommaMeantone()
{
    // 1/4-comma meantone: fifths are tempered to make major thirds pure
    // Fifth = (5^(1/4)) = ~1.4953 (slightly flat of 3/2)
    double fifth = std::pow(5.0, 0.25);
    QVector<double> ratios = {
        1.0,                        // C
        std::pow(fifth, 2) / 2.0,   // D (two fifths up, one octave down)
        5.0/4.0,                    // E (pure major third)
        std::pow(fifth, -1) * 2.0,  // F (fifth down, octave up)
        fifth,                      // G (pure fifth)
        5.0/3.0,                    // A (pure major sixth)
        std::pow(fifth, 3) / 4.0    // B (three fifths up, two octaves down)
    };
    QVector<QString> names = {"C", "D", "E", "F", "G", "A", "B"};
    return Scale("Quarter-Comma Meantone", 3, ratios, names);
}

// ============================================================================
// MAQAM SCALES (Arabic/Turkish/Middle Eastern)
// ============================================================================

Scale Scale::maqamRast()
{
    // Maqam Rast: C D E♭+ F G A B♭+ C
    // Using Turkish/Arabic quarter-tone approximations
    QVector<double> ratios = {
        1.0,            // C (Rast)
        9.0/8.0,        // D
        5.0/4.0,        // E (slightly flat in practice, but using 5/4)
        4.0/3.0,        // F
        3.0/2.0,        // G (Nawa)
        27.0/16.0,      // A
        15.0/8.0        // B
    };
    QVector<QString> names = {"C", "D", "E", "F", "G", "A", "B"};
    return Scale("Maqam Rast", 4, ratios, names);
}

Scale Scale::maqamBayati()
{
    // Maqam Bayati: D E♭ F G A B♭ C D
    // Transposed to start on C: C D♭+ E♭ F G A♭+ B♭ C
    QVector<double> ratios = {
        1.0,            // C
        16.0/15.0,      // D♭ (quarter-tone approximation)
        6.0/5.0,        // E♭
        4.0/3.0,        // F
        3.0/2.0,        // G
        8.0/5.0,        // A♭
        9.0/5.0         // B♭
    };
    QVector<QString> names = {"C", "Db+", "Eb", "F", "G", "Ab+", "Bb"};
    return Scale("Maqam Bayati", 5, ratios, names);
}

Scale Scale::maqamSaba()
{
    // Maqam Saba: D E♭- E F♭+ G♭ A♭ B♭ C D
    // Complex scale with many microtones
    QVector<double> ratios = {
        1.0,            // D (starting note, transposed to C)
        16.0/15.0,      // E♭-
        6.0/5.0,        // E (natural)
        27.0/20.0,      // F#+
        45.0/32.0,      // G
        8.0/5.0,        // A♭
        9.0/5.0,        // B♭
        15.0/8.0        // C#
    };
    QVector<QString> names = {"C", "Db", "Eb", "E", "F#", "Ab", "Bb", "C#"};
    return Scale("Maqam Saba", 6, ratios, names);
}

Scale Scale::maqamHijaz()
{
    // Maqam Hijaz: D E♭ F# G A B♭ C D
    // Characteristic augmented 2nd between E♭ and F#
    QVector<double> ratios = {
        1.0,            // C
        16.0/15.0,      // D♭
        5.0/4.0,        // E
        4.0/3.0,        // F
        3.0/2.0,        // G
        8.0/5.0,        // A♭
        15.0/8.0        // B
    };
    QVector<QString> names = {"C", "Db", "E", "F", "G", "Ab", "B"};
    return Scale("Maqam Hijaz", 7, ratios, names);
}

Scale Scale::maqamNahawand()
{
    // Maqam Nahawand: similar to natural minor
    // C D E♭ F G A♭ B C
    QVector<double> ratios = {
        1.0,            // C
        9.0/8.0,        // D
        6.0/5.0,        // E♭
        4.0/3.0,        // F
        3.0/2.0,        // G
        8.0/5.0,        // A♭
        15.0/8.0        // B
    };
    QVector<QString> names = {"C", "D", "Eb", "F", "G", "Ab", "B"};
    return Scale("Maqam Nahawand", 8, ratios, names);
}

Scale Scale::maqamKurd()
{
    // Maqam Kurd: C D♭ E♭ F G A♭ B♭ C
    // Similar to Phrygian mode
    QVector<double> ratios = {
        1.0,            // C
        16.0/15.0,      // D♭
        6.0/5.0,        // E♭
        4.0/3.0,        // F
        3.0/2.0,        // G
        8.0/5.0,        // A♭
        9.0/5.0         // B♭
    };
    QVector<QString> names = {"C", "Db", "Eb", "F", "G", "Ab", "Bb"};
    return Scale("Maqam Kurd", 9, ratios, names);
}

// ============================================================================
// INDIAN RAGAS
// ============================================================================

Scale Scale::ragaBhairav()
{
    // Raga Bhairav: C Db E F G Ab B C
    // Characteristic sharp 3rd and 7th with flat 2nd and 6th
    QVector<double> ratios = {
        1.0,            // Sa (C)
        16.0/15.0,      // Re♭ (Db)
        5.0/4.0,        // Ga (E)
        4.0/3.0,        // Ma (F)
        3.0/2.0,        // Pa (G)
        8.0/5.0,        // Dha♭ (Ab)
        15.0/8.0        // Ni (B)
    };
    QVector<QString> names = {"Sa", "Re♭", "Ga", "Ma", "Pa", "Dha♭", "Ni"};
    return Scale("Raga Bhairav", 10, ratios, names);
}

Scale Scale::ragaYaman()
{
    // Raga Yaman (Kalyan): C D E F# G A B C
    // All natural notes except sharp 4th
    QVector<double> ratios = {
        1.0,            // Sa (C)
        9.0/8.0,        // Re (D)
        5.0/4.0,        // Ga (E)
        45.0/32.0,      // Ma# (F#)
        3.0/2.0,        // Pa (G)
        5.0/3.0,        // Dha (A)
        15.0/8.0        // Ni (B)
    };
    QVector<QString> names = {"Sa", "Re", "Ga", "Ma#", "Pa", "Dha", "Ni"};
    return Scale("Raga Yaman", 11, ratios, names);
}

Scale Scale::ragaTodi()
{
    // Raga Todi: C Db Eb F# G Ab B C
    // Distinctive sharp 4th with many flat notes
    QVector<double> ratios = {
        1.0,            // Sa (C)
        16.0/15.0,      // Re♭ (Db)
        6.0/5.0,        // Ga♭ (Eb)
        45.0/32.0,      // Ma# (F#)
        3.0/2.0,        // Pa (G)
        8.0/5.0,        // Dha♭ (Ab)
        15.0/8.0        // Ni (B)
    };
    QVector<QString> names = {"Sa", "Re♭", "Ga♭", "Ma#", "Pa", "Dha♭", "Ni"};
    return Scale("Raga Todi", 12, ratios, names);
}

Scale Scale::ragaBhairavi()
{
    // Raga Bhairavi: C Db Eb F G Ab Bb C
    // All notes can be used, very flexible raga
    QVector<double> ratios = {
        1.0,            // Sa (C)
        16.0/15.0,      // Re♭ (Db)
        6.0/5.0,        // Ga♭ (Eb)
        4.0/3.0,        // Ma (F)
        3.0/2.0,        // Pa (G)
        8.0/5.0,        // Dha♭ (Ab)
        9.0/5.0         // Ni♭ (Bb)
    };
    QVector<QString> names = {"Sa", "Re♭", "Ga♭", "Ma", "Pa", "Dha♭", "Ni♭"};
    return Scale("Raga Bhairavi", 13, ratios, names);
}

Scale Scale::ragaShankarabharana()
{
    // Raga Shankarabharana (Carnatic) / Bilawal (Hindustani): C D E F G A B C
    // 29th Melakarta, equivalent to major scale
    // All shuddha (natural) swaras
    QVector<double> ratios = {
        1.0,            // Sa (C)
        9.0/8.0,        // Re (D)
        5.0/4.0,        // Ga (E)
        4.0/3.0,        // Ma (F)
        3.0/2.0,        // Pa (G)
        5.0/3.0,        // Dha (A)
        15.0/8.0        // Ni (B)
    };
    QVector<QString> names = {"Sa", "Re", "Ga", "Ma", "Pa", "Dha", "Ni"};
    return Scale("Raga Shankarabharana", 14, ratios, names);
}

Scale Scale::ragaMayamalavagowla()
{
    // Raga Mayamalavagowla: C Db E F G Ab B C
    // 15th Melakarta, foundational morning raga in Carnatic music
    // Equivalent to Bhairav in Hindustani, but listed separately for Carnatic tradition
    QVector<double> ratios = {
        1.0,            // Sa (C)
        16.0/15.0,      // Re♭ (Db) - R1
        5.0/4.0,        // Ga (E) - G3
        4.0/3.0,        // Ma (F) - M1
        3.0/2.0,        // Pa (G)
        8.0/5.0,        // Dha♭ (Ab) - D1
        15.0/8.0        // Ni (B) - N3
    };
    QVector<QString> names = {"Sa", "Re♭", "Ga", "Ma", "Pa", "Dha♭", "Ni"};
    return Scale("Raga Mayamalavagowla", 15, ratios, names);
}

// ============================================================================
// INDONESIAN SCALES
// ============================================================================

Scale Scale::pelog()
{
    // Pelog (Javanese): 7-note scale with unequal intervals
    // Many regional variations; this is one common tuning
    QVector<double> ratios = {
        1.0,            // 1 (Panunggul)
        256.0/243.0,    // 2 (Gulu)
        32.0/27.0,      // 3 (Dada)
        4.0/3.0,        // 4 (Pelog)
        1024.0/729.0,   // 5 (Lima)
        128.0/81.0,     // 6 (Nem)
        16.0/9.0        // 7 (Barang)
    };
    QVector<QString> names = {"1", "2", "3", "4", "5", "6", "7"};
    return Scale("Pelog", 14, ratios, names);
}

Scale Scale::slendro()
{
    // Slendro (Javanese): 5-note scale with roughly equal intervals
    // Ideally ~240 cents per step, approximated here
    QVector<double> ratios = {
        1.0,                    // 1 (Panunggul)
        std::pow(2.0, 1.0/5),   // 2 (Gulu) - ~240 cents
        std::pow(2.0, 2.0/5),   // 3 (Dada) - ~480 cents
        std::pow(2.0, 3.0/5),   // 5 (Lima) - ~720 cents
        std::pow(2.0, 4.0/5)    // 6 (Nem) - ~960 cents
    };
    QVector<QString> names = {"1", "2", "3", "5", "6"};
    return Scale("Slendro", 15, ratios, names);
}

// ============================================================================
// CHINESE SCALES
// ============================================================================

Scale Scale::chinesePentatonic()
{
    // Chinese Pentatonic (Gong mode): C D E G A C
    // Similar to major pentatonic
    QVector<double> ratios = {
        1.0,            // Gong (C)
        9.0/8.0,        // Shang (D)
        5.0/4.0,        // Jue (E)
        3.0/2.0,        // Zhi (G)
        5.0/3.0         // Yu (A)
    };
    QVector<QString> names = {"Gong", "Shang", "Jue", "Zhi", "Yu"};
    return Scale("Chinese Pentatonic", 16, ratios, names);
}

// ============================================================================
// JAPANESE SCALES
// ============================================================================

Scale Scale::hirajoshi()
{
    // Hirajoshi scale: C D Eb G Ab C
    // 5-note Japanese scale
    QVector<double> ratios = {
        1.0,            // C
        9.0/8.0,        // D
        6.0/5.0,        // Eb
        3.0/2.0,        // G
        8.0/5.0         // Ab
    };
    QVector<QString> names = {"C", "D", "Eb", "G", "Ab"};
    return Scale("Hirajoshi", 17, ratios, names);
}

// ============================================================================
// WESTERN (ADDITIONAL)
// ============================================================================

Scale Scale::wholeTone()
{
    // Whole Tone: C D E F# G# A# (6 notes, all whole steps, 200 cents each)
    QVector<double> ratios = {
        1.0,                        // C
        std::pow(2.0, 2.0/12),      // D - 2 semitones
        std::pow(2.0, 4.0/12),      // E - 4 semitones
        std::pow(2.0, 6.0/12),      // F# - tritone
        std::pow(2.0, 8.0/12),      // G#
        std::pow(2.0, 10.0/12)      // A#
    };
    QVector<QString> names = {"C", "D", "E", "F#", "G#", "A#"};
    return Scale("Whole Tone", 20, ratios, names);
}

Scale Scale::halfDiminished()
{
    // Half Diminished (Locrian): C Db Eb F Gb Ab Bb
    // Fits the m7b5 (half-diminished) chord; natural scale on the 7th degree
    QVector<double> ratios = {
        1.0,                        // C
        16.0/15.0,                  // Db - minor 2nd
        6.0/5.0,                    // Eb - minor 3rd
        4.0/3.0,                    // F  - perfect 4th
        std::pow(2.0, 6.0/12),      // Gb - diminished 5th (tritone)
        8.0/5.0,                    // Ab - minor 6th
        9.0/5.0                     // Bb - minor 7th
    };
    QVector<QString> names = {"C", "Db", "Eb", "F", "Gb", "Ab", "Bb"};
    return Scale("Half Diminished", 21, ratios, names);
}

// ============================================================================
// INDIAN RAGAS (ADDITIONAL)
// ============================================================================

Scale Scale::ragaKafi()
{
    // Raga Kafi: C D Eb F G A Bb (Kafi thaat - Dorian-like)
    QVector<double> ratios = {
        1.0,            // Sa (C)
        9.0/8.0,        // Re (D)
        6.0/5.0,        // Ga♭ (Eb)
        4.0/3.0,        // Ma (F)
        3.0/2.0,        // Pa (G)
        5.0/3.0,        // Dha (A)
        9.0/5.0         // Ni♭ (Bb)
    };
    QVector<QString> names = {"Sa", "Re", "Ga♭", "Ma", "Pa", "Dha", "Ni♭"};
    return Scale("Raga Kafi", 22, ratios, names);
}

Scale Scale::ragaAsavari()
{
    // Raga Asavari: C D Eb F G Ab Bb (Asavari thaat)
    QVector<double> ratios = {
        1.0,            // Sa (C)
        9.0/8.0,        // Re (D)
        6.0/5.0,        // Ga♭ (Eb)
        4.0/3.0,        // Ma (F)
        3.0/2.0,        // Pa (G)
        8.0/5.0,        // Dha♭ (Ab)
        9.0/5.0         // Ni♭ (Bb)
    };
    QVector<QString> names = {"Sa", "Re", "Ga♭", "Ma", "Pa", "Dha♭", "Ni♭"};
    return Scale("Raga Asavari", 23, ratios, names);
}

Scale Scale::ragaKhamaj()
{
    // Raga Khamaj: C D E F G A Bb (Khamaj thaat - Mixolydian-like)
    QVector<double> ratios = {
        1.0,            // Sa (C)
        9.0/8.0,        // Re (D)
        5.0/4.0,        // Ga (E)
        4.0/3.0,        // Ma (F)
        3.0/2.0,        // Pa (G)
        5.0/3.0,        // Dha (A)
        9.0/5.0         // Ni♭ (Bb)
    };
    QVector<QString> names = {"Sa", "Re", "Ga", "Ma", "Pa", "Dha", "Ni♭"};
    return Scale("Raga Khamaj", 24, ratios, names);
}

Scale Scale::ragaMarwa()
{
    // Raga Marwa: C Db E F# G A B (Marwa thaat)
    // Late afternoon raga; flat 2nd and sharp 4th are characteristic
    QVector<double> ratios = {
        1.0,            // Sa (C)
        16.0/15.0,      // Re♭ (Db)
        5.0/4.0,        // Ga (E)
        45.0/32.0,      // Ma# (F#)
        3.0/2.0,        // Pa (G)
        5.0/3.0,        // Dha (A)
        15.0/8.0        // Ni (B)
    };
    QVector<QString> names = {"Sa", "Re♭", "Ga", "Ma#", "Pa", "Dha", "Ni"};
    return Scale("Raga Marwa", 25, ratios, names);
}

Scale Scale::ragaPurvi()
{
    // Raga Purvi: C Db E F# G Ab B (Purvi thaat)
    // Late evening raga; flat 2nd, sharp 4th, and flat 6th
    QVector<double> ratios = {
        1.0,            // Sa (C)
        16.0/15.0,      // Re♭ (Db)
        5.0/4.0,        // Ga (E)
        45.0/32.0,      // Ma# (F#)
        3.0/2.0,        // Pa (G)
        8.0/5.0,        // Dha♭ (Ab)
        15.0/8.0        // Ni (B)
    };
    QVector<QString> names = {"Sa", "Re♭", "Ga", "Ma#", "Pa", "Dha♭", "Ni"};
    return Scale("Raga Purvi", 26, ratios, names);
}

// ============================================================================
// MAQAM SCALES (ADDITIONAL)
// ============================================================================

Scale Scale::maqamSikah()
{
    // Maqam Sikah: built on a neutral (3/4-flat) 3rd degree
    // 12/11 (~151 cents) approximates the characteristic 3/4-tone step
    QVector<double> ratios = {
        1.0,            // C
        12.0/11.0,      // D♭+ (~150 cents, 3/4-tone step)
        6.0/5.0,        // E♭
        4.0/3.0,        // F
        3.0/2.0,        // G
        8.0/5.0,        // A♭
        9.0/5.0         // B♭
    };
    QVector<QString> names = {"C", "D♭+", "E♭", "F", "G", "A♭", "B♭"};
    return Scale("Maqam Sikah", 27, ratios, names);
}

Scale Scale::maqamAjam()
{
    // Maqam Ajam: major tetrachord character with minor 7th
    // C D E F G A Bb
    QVector<double> ratios = {
        1.0,            // C
        9.0/8.0,        // D
        5.0/4.0,        // E
        4.0/3.0,        // F
        3.0/2.0,        // G
        5.0/3.0,        // A
        9.0/5.0         // Bb
    };
    QVector<QString> names = {"C", "D", "E", "F", "G", "A", "Bb"};
    return Scale("Maqam Ajam", 28, ratios, names);
}

Scale Scale::maqamAtharKurd()
{
    // Maqam Athar Kurd: Kurd lower tetrachord + Athar upper tetrachord
    // C Db Eb F G Ab B — augmented 2nd between Ab and B
    QVector<double> ratios = {
        1.0,            // C
        16.0/15.0,      // Db
        6.0/5.0,        // Eb
        4.0/3.0,        // F
        3.0/2.0,        // G
        8.0/5.0,        // Ab
        15.0/8.0        // B (augmented 2nd above Ab)
    };
    QVector<QString> names = {"C", "Db", "Eb", "F", "G", "Ab", "B"};
    return Scale("Maqam Athar Kurd", 29, ratios, names);
}

// ============================================================================
// PERSIAN DASTGAH SYSTEM
// ============================================================================

Scale Scale::persianShur()
{
    // Persian Shur: most fundamental Persian dastgah
    // Uses koron (3/4-flat, ~135 cents) on 2nd and 6th degrees
    // C D♭k E♭ F G A♭k B♭
    QVector<double> ratios = {
        1.0,                             // C
        std::pow(2.0, 135.0/1200.0),    // D koron (~135 cents)
        6.0/5.0,                         // E♭
        4.0/3.0,                         // F
        3.0/2.0,                         // G
        std::pow(2.0, 835.0/1200.0),    // A koron (~835 cents)
        9.0/5.0                          // B♭
    };
    QVector<QString> names = {"C", "D♭k", "E♭", "F", "G", "A♭k", "B♭"};
    return Scale("Persian Shur", 30, ratios, names);
}

Scale Scale::persianMahur()
{
    // Persian Mahur: major-scale character, Persian equivalent of major
    // C D E F G A B with JI ratios
    QVector<double> ratios = {
        1.0,            // C
        9.0/8.0,        // D
        5.0/4.0,        // E
        4.0/3.0,        // F
        3.0/2.0,        // G
        5.0/3.0,        // A
        15.0/8.0        // B
    };
    QVector<QString> names = {"C", "D", "E", "F", "G", "A", "B"};
    return Scale("Persian Mahur", 31, ratios, names);
}

Scale Scale::persianSegah()
{
    // Persian Segah: "third position" dastgah
    // Uses neutral (3/4-flat) 3rd and 7th degrees (~350 and ~1050 cents)
    QVector<double> ratios = {
        1.0,                             // C
        9.0/8.0,                         // D
        std::pow(2.0, 350.0/1200.0),    // E♭+ (neutral 3rd)
        4.0/3.0,                         // F
        3.0/2.0,                         // G
        5.0/3.0,                         // A
        std::pow(2.0, 1050.0/1200.0)    // B♭+ (neutral 7th)
    };
    QVector<QString> names = {"C", "D", "E♭+", "F", "G", "A", "B♭+"};
    return Scale("Persian Segah", 32, ratios, names);
}

Scale Scale::persianChahargah()
{
    // Persian Chahargah: "fourth position", augmented 2nd intervals
    // C D E♭+ F# G A+ B♭+ with characteristic microtonal steps
    QVector<double> ratios = {
        1.0,                             // C
        9.0/8.0,                         // D
        std::pow(2.0, 342.0/1200.0),    // E♭+ (neutral/3-quarter flat)
        std::pow(2.0, 567.0/1200.0),    // F# (augmented 4th)
        3.0/2.0,                         // G
        std::pow(2.0, 906.0/1200.0),    // A+
        std::pow(2.0, 1044.0/1200.0)   // B♭+
    };
    QVector<QString> names = {"C", "D", "E♭+", "F#", "G", "A+", "B♭+"};
    return Scale("Persian Chahargah", 33, ratios, names);
}

Scale Scale::persianHomayun()
{
    // Persian Homayun: minor 2nd then major 3rd (Hijaz-like lower tetrachord)
    // C Db E F G Ab Bb — characteristic of South Persian music
    QVector<double> ratios = {
        1.0,            // C
        16.0/15.0,      // Db
        5.0/4.0,        // E (augmented 2nd above Db)
        4.0/3.0,        // F
        3.0/2.0,        // G
        8.0/5.0,        // Ab
        9.0/5.0         // Bb
    };
    QVector<QString> names = {"C", "Db", "E", "F", "G", "Ab", "Bb"};
    return Scale("Persian Homayun", 34, ratios, names);
}

// ============================================================================
// DIATONIC MODES (Equal Temperament)
// ============================================================================

Scale Scale::modeIonian()
{
    // Ionian — the major scale: W W H W W W H
    // C D E F G A B  (0 2 4 5 7 9 11 semitones)
    QVector<double> ratios = {
        1.0,
        std::pow(2.0, 2.0/12),   // D
        std::pow(2.0, 4.0/12),   // E
        std::pow(2.0, 5.0/12),   // F
        std::pow(2.0, 7.0/12),   // G
        std::pow(2.0, 9.0/12),   // A
        std::pow(2.0, 11.0/12)   // B
    };
    QVector<QString> names = {"C", "D", "E", "F", "G", "A", "B"};
    return Scale("Ionian (Major)", 35, ratios, names);
}

Scale Scale::modeDorian()
{
    // Dorian — W H W W W H W
    // C D Eb F G A Bb  (0 2 3 5 7 9 10)
    // The quintessential jazz minor mode ("So What", "Impressions")
    QVector<double> ratios = {
        1.0,
        std::pow(2.0, 2.0/12),   // D
        std::pow(2.0, 3.0/12),   // Eb
        std::pow(2.0, 5.0/12),   // F
        std::pow(2.0, 7.0/12),   // G
        std::pow(2.0, 9.0/12),   // A  (major 6th — defines Dorian)
        std::pow(2.0, 10.0/12)   // Bb
    };
    QVector<QString> names = {"C", "D", "Eb", "F", "G", "A", "Bb"};
    return Scale("Dorian", 36, ratios, names);
}

Scale Scale::modePhrygian()
{
    // Phrygian — H W W W H W W
    // C Db Eb F G Ab Bb  (0 1 3 5 7 8 10)
    // Spanish / flamenco character; used in jazz over altered chords
    QVector<double> ratios = {
        1.0,
        std::pow(2.0, 1.0/12),   // Db (minor 2nd — defines Phrygian)
        std::pow(2.0, 3.0/12),   // Eb
        std::pow(2.0, 5.0/12),   // F
        std::pow(2.0, 7.0/12),   // G
        std::pow(2.0, 8.0/12),   // Ab
        std::pow(2.0, 10.0/12)   // Bb
    };
    QVector<QString> names = {"C", "Db", "Eb", "F", "G", "Ab", "Bb"};
    return Scale("Phrygian", 37, ratios, names);
}

Scale Scale::modeLydian()
{
    // Lydian — W W W H W W H
    // C D E F# G A B  (0 2 4 6 7 9 11)
    // Bright major sound; raised 4th is the signature
    QVector<double> ratios = {
        1.0,
        std::pow(2.0, 2.0/12),   // D
        std::pow(2.0, 4.0/12),   // E
        std::pow(2.0, 6.0/12),   // F# (raised 4th — defines Lydian)
        std::pow(2.0, 7.0/12),   // G
        std::pow(2.0, 9.0/12),   // A
        std::pow(2.0, 11.0/12)   // B
    };
    QVector<QString> names = {"C", "D", "E", "F#", "G", "A", "B"};
    return Scale("Lydian", 38, ratios, names);
}

Scale Scale::modeMixolydian()
{
    // Mixolydian — W W H W W H W
    // C D E F G A Bb  (0 2 4 5 7 9 10)
    // Dominant blues-jazz scale; major with flat 7th
    QVector<double> ratios = {
        1.0,
        std::pow(2.0, 2.0/12),   // D
        std::pow(2.0, 4.0/12),   // E
        std::pow(2.0, 5.0/12),   // F
        std::pow(2.0, 7.0/12),   // G
        std::pow(2.0, 9.0/12),   // A
        std::pow(2.0, 10.0/12)   // Bb (flat 7th — defines Mixolydian)
    };
    QVector<QString> names = {"C", "D", "E", "F", "G", "A", "Bb"};
    return Scale("Mixolydian", 39, ratios, names);
}

Scale Scale::modeAeolian()
{
    // Aeolian — W H W W H W W
    // C D Eb F G Ab Bb  (0 2 3 5 7 8 10)
    // Natural minor; foundation of Western minor tonality
    QVector<double> ratios = {
        1.0,
        std::pow(2.0, 2.0/12),   // D
        std::pow(2.0, 3.0/12),   // Eb
        std::pow(2.0, 5.0/12),   // F
        std::pow(2.0, 7.0/12),   // G
        std::pow(2.0, 8.0/12),   // Ab (flat 6th — differs from Dorian)
        std::pow(2.0, 10.0/12)   // Bb
    };
    QVector<QString> names = {"C", "D", "Eb", "F", "G", "Ab", "Bb"};
    return Scale("Aeolian (Natural Minor)", 40, ratios, names);
}

Scale Scale::modeLocrian()
{
    // Locrian — H W W H W W W
    // C Db Eb F Gb Ab Bb  (0 1 3 5 6 8 10)
    // Diminished 5th on tonic; rarely used as a tonal centre
    QVector<double> ratios = {
        1.0,
        std::pow(2.0, 1.0/12),   // Db
        std::pow(2.0, 3.0/12),   // Eb
        std::pow(2.0, 5.0/12),   // F
        std::pow(2.0, 6.0/12),   // Gb (diminished 5th — defines Locrian)
        std::pow(2.0, 8.0/12),   // Ab
        std::pow(2.0, 10.0/12)   // Bb
    };
    QVector<QString> names = {"C", "Db", "Eb", "F", "Gb", "Ab", "Bb"};
    return Scale("Locrian", 41, ratios, names);
}

// ============================================================================
// MELODIC MINOR AND ITS MODES (Jazz)
// ============================================================================

Scale Scale::melodicMinor()
{
    // Melodic Minor (ascending / jazz form): W H W W W W H
    // C D Eb F G A B  (0 2 3 5 7 9 11)
    // Minor with raised 6th and 7th; the root of all melodic minor modes
    QVector<double> ratios = {
        1.0,
        std::pow(2.0, 2.0/12),   // D
        std::pow(2.0, 3.0/12),   // Eb
        std::pow(2.0, 5.0/12),   // F
        std::pow(2.0, 7.0/12),   // G
        std::pow(2.0, 9.0/12),   // A  (raised 6th vs Aeolian)
        std::pow(2.0, 11.0/12)   // B  (raised 7th vs Aeolian)
    };
    QVector<QString> names = {"C", "D", "Eb", "F", "G", "A", "B"};
    return Scale("Melodic Minor", 42, ratios, names);
}

Scale Scale::modeDorianB2()
{
    // Dorian b2 — 2nd mode of melodic minor
    // C Db Eb F G A Bb  (0 1 3 5 7 9 10)
    // Like Dorian but with Phrygian's minor 2nd; used over sus b9 chords
    QVector<double> ratios = {
        1.0,
        std::pow(2.0, 1.0/12),   // Db (flat 2nd)
        std::pow(2.0, 3.0/12),   // Eb
        std::pow(2.0, 5.0/12),   // F
        std::pow(2.0, 7.0/12),   // G
        std::pow(2.0, 9.0/12),   // A  (major 6th)
        std::pow(2.0, 10.0/12)   // Bb
    };
    QVector<QString> names = {"C", "Db", "Eb", "F", "G", "A", "Bb"};
    return Scale("Dorian b2", 43, ratios, names);
}

Scale Scale::modeLydianAugmented()
{
    // Lydian Augmented — 3rd mode of melodic minor
    // C D E F# G# A B  (0 2 4 6 8 9 11)
    // Lydian with raised 5th; used over maj7#5 chords
    QVector<double> ratios = {
        1.0,
        std::pow(2.0, 2.0/12),   // D
        std::pow(2.0, 4.0/12),   // E
        std::pow(2.0, 6.0/12),   // F# (raised 4th)
        std::pow(2.0, 8.0/12),   // G# (raised 5th — defines Lydian Aug)
        std::pow(2.0, 9.0/12),   // A
        std::pow(2.0, 11.0/12)   // B
    };
    QVector<QString> names = {"C", "D", "E", "F#", "G#", "A", "B"};
    return Scale("Lydian Augmented", 44, ratios, names);
}

Scale Scale::modeLydianDominant()
{
    // Lydian Dominant — 4th mode of melodic minor
    // C D E F# G A Bb  (0 2 4 6 7 9 10)
    // The iconic jazz dominant scale: raised 4th + flat 7th
    // Used over dom7#11 chords, tritone substitutions
    QVector<double> ratios = {
        1.0,
        std::pow(2.0, 2.0/12),   // D
        std::pow(2.0, 4.0/12),   // E
        std::pow(2.0, 6.0/12),   // F# (raised 4th / #11)
        std::pow(2.0, 7.0/12),   // G
        std::pow(2.0, 9.0/12),   // A
        std::pow(2.0, 10.0/12)   // Bb (flat 7th)
    };
    QVector<QString> names = {"C", "D", "E", "F#", "G", "A", "Bb"};
    return Scale("Lydian Dominant", 45, ratios, names);
}

Scale Scale::modeMixolydianB6()
{
    // Mixolydian b6 (Hindu) — 5th mode of melodic minor
    // C D E F G Ab Bb  (0 2 4 5 7 8 10)
    // Major with flat 6th and flat 7th; over dom7b13 chords
    QVector<double> ratios = {
        1.0,
        std::pow(2.0, 2.0/12),   // D
        std::pow(2.0, 4.0/12),   // E
        std::pow(2.0, 5.0/12),   // F
        std::pow(2.0, 7.0/12),   // G
        std::pow(2.0, 8.0/12),   // Ab (flat 6th — defines this mode)
        std::pow(2.0, 10.0/12)   // Bb
    };
    QVector<QString> names = {"C", "D", "E", "F", "G", "Ab", "Bb"};
    return Scale("Mixolydian b6", 46, ratios, names);
}

Scale Scale::modeLocrianSharp2()
{
    // Locrian #2 (Half-Diminished) — 6th mode of melodic minor
    // C D Eb F Gb Ab Bb  (0 2 3 5 6 8 10)
    // Like Locrian but with natural 2nd; over m7b5 chords in jazz
    QVector<double> ratios = {
        1.0,
        std::pow(2.0, 2.0/12),   // D  (major 2nd — #2 vs Locrian)
        std::pow(2.0, 3.0/12),   // Eb
        std::pow(2.0, 5.0/12),   // F
        std::pow(2.0, 6.0/12),   // Gb (diminished 5th)
        std::pow(2.0, 8.0/12),   // Ab
        std::pow(2.0, 10.0/12)   // Bb
    };
    QVector<QString> names = {"C", "D", "Eb", "F", "Gb", "Ab", "Bb"};
    return Scale("Locrian #2", 47, ratios, names);
}

Scale Scale::modeAltered()
{
    // Altered (Super Locrian) — 7th mode of melodic minor
    // C Db Eb E Gb Ab Bb  (0 1 3 4 6 8 10)
    // Every note above the 5th is altered (b9 #9 b5 b13); over alt dominant chords
    // The most dissonant and tension-rich scale in common jazz practice
    QVector<double> ratios = {
        1.0,
        std::pow(2.0, 1.0/12),   // Db (b9)
        std::pow(2.0, 3.0/12),   // Eb (#9)
        std::pow(2.0, 4.0/12),   // E  (major 3rd)
        std::pow(2.0, 6.0/12),   // Gb (b5 / #11)
        std::pow(2.0, 8.0/12),   // Ab (b13)
        std::pow(2.0, 10.0/12)   // Bb (b7)
    };
    QVector<QString> names = {"C", "Db", "Eb", "E", "Gb", "Ab", "Bb"};
    return Scale("Altered (Super Locrian)", 48, ratios, names);
}

// ============================================================================
// SCALE MANAGEMENT
// ============================================================================

QVector<Scale> Scale::getAllScales()
{
    QVector<Scale> scales;

    // Western
    scales.append(justIntonation());
    scales.append(pythagorean());
    scales.append(equalTemperament());
    scales.append(quarterCommaMeantone());

    // Maqam
    scales.append(maqamRast());
    scales.append(maqamBayati());
    scales.append(maqamSaba());
    scales.append(maqamHijaz());
    scales.append(maqamNahawand());
    scales.append(maqamKurd());

    // Ragas
    scales.append(ragaBhairav());
    scales.append(ragaYaman());
    scales.append(ragaTodi());
    scales.append(ragaBhairavi());
    scales.append(ragaShankarabharana());
    scales.append(ragaMayamalavagowla());

    // Indonesian
    scales.append(pelog());
    scales.append(slendro());

    // Chinese
    scales.append(chinesePentatonic());

    // Japanese
    scales.append(hirajoshi());

    // Western (additional)
    scales.append(wholeTone());
    scales.append(halfDiminished());

    // Ragas (additional)
    scales.append(ragaKafi());
    scales.append(ragaAsavari());
    scales.append(ragaKhamaj());
    scales.append(ragaMarwa());
    scales.append(ragaPurvi());

    // Maqam (additional)
    scales.append(maqamSikah());
    scales.append(maqamAjam());
    scales.append(maqamAtharKurd());

    // Persian Dastgah
    scales.append(persianShur());
    scales.append(persianMahur());
    scales.append(persianSegah());
    scales.append(persianChahargah());
    scales.append(persianHomayun());

    // Diatonic modes (ET)
    scales.append(modeIonian());
    scales.append(modeDorian());
    scales.append(modePhrygian());
    scales.append(modeLydian());
    scales.append(modeMixolydian());
    scales.append(modeAeolian());
    scales.append(modeLocrian());

    // Melodic minor modes (Jazz)
    scales.append(melodicMinor());
    scales.append(modeDorianB2());
    scales.append(modeLydianAugmented());
    scales.append(modeLydianDominant());
    scales.append(modeMixolydianB6());
    scales.append(modeLocrianSharp2());
    scales.append(modeAltered());

    return scales;
}

Scale Scale::getScaleById(int id)
{
    switch (id) {
        // Western
        case 0: return justIntonation();
        case 1: return pythagorean();
        case 2: return equalTemperament();
        case 3: return quarterCommaMeantone();

        // Maqam
        case 4: return maqamRast();
        case 5: return maqamBayati();
        case 6: return maqamSaba();
        case 7: return maqamHijaz();
        case 8: return maqamNahawand();
        case 9: return maqamKurd();

        // Ragas
        case 10: return ragaBhairav();
        case 11: return ragaYaman();
        case 12: return ragaTodi();
        case 13: return ragaBhairavi();
        case 14: return ragaShankarabharana();
        case 15: return ragaMayamalavagowla();

        // Indonesian
        case 16: return pelog();
        case 17: return slendro();

        // Chinese
        case 18: return chinesePentatonic();

        // Japanese
        case 19: return hirajoshi();

        // Western (additional)
        case 20: return wholeTone();
        case 21: return halfDiminished();

        // Ragas (additional)
        case 22: return ragaKafi();
        case 23: return ragaAsavari();
        case 24: return ragaKhamaj();
        case 25: return ragaMarwa();
        case 26: return ragaPurvi();

        // Maqam (additional)
        case 27: return maqamSikah();
        case 28: return maqamAjam();
        case 29: return maqamAtharKurd();

        // Persian Dastgah
        case 30: return persianShur();
        case 31: return persianMahur();
        case 32: return persianSegah();
        case 33: return persianChahargah();
        case 34: return persianHomayun();

        // Diatonic modes (ET)
        case 35: return modeIonian();
        case 36: return modeDorian();
        case 37: return modePhrygian();
        case 38: return modeLydian();
        case 39: return modeMixolydian();
        case 40: return modeAeolian();
        case 41: return modeLocrian();

        // Melodic minor modes (Jazz)
        case 42: return melodicMinor();
        case 43: return modeDorianB2();
        case 44: return modeLydianAugmented();
        case 45: return modeLydianDominant();
        case 46: return modeMixolydianB6();
        case 47: return modeLocrianSharp2();
        case 48: return modeAltered();

        default:
            return justIntonation();  // Default fallback
    }
}

QJsonObject Scale::toJson() const
{
    QJsonObject json;
    json["scaleId"] = scaleId;
    json["name"] = name;  // For human readability in the file
    json["tonicIndex"] = tonicIndex;
    return json;
}

Scale Scale::fromJson(const QJsonObject &json)
{
    int id = json["scaleId"].toInt(0);
    // Check if tonicIndex is stored (for ET scales with rotated tonic)
    if (json.contains("tonicIndex")) {
        int tonicIdx = json["tonicIndex"].toInt(-1);
        // Only apply tonic rotation for Equal Temperament (scaleId == 2)
        if (id == 2) {
            if (tonicIdx >= 0 && tonicIdx <= 11) {
                return equalTemperamentWithTonic(tonicIdx);
            }
            // tonicIdx == -1 means default C (tonicIndex = 0)
            // fall through to getScaleById which returns ET with tonicIndex=0
        }
        // For non-ET scales, tonicIdx is ignored (scale's tonicIndex remains -1)
    }
    return getScaleById(id);
}
