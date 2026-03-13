#include "scale.h"
#include <cmath>
#include <QJsonObject>

// Default constructor
Scale::Scale(const QString &name, int scaleId)
    : name(name), scaleId(scaleId)
{
    // Default to Just Intonation
    ratios = {1.0, 9.0/8.0, 5.0/4.0, 4.0/3.0, 3.0/2.0, 5.0/3.0, 15.0/8.0};
    noteNames = {"C", "D", "E", "F", "G", "A", "B"};
}

// Private constructor with custom ratios and names
Scale::Scale(const QString &name, int scaleId, const QVector<double> &ratios, const QVector<QString> &noteNames)
    : name(name), scaleId(scaleId), ratios(ratios), noteNames(noteNames)
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
    // 12-tone equal temperament: each semitone is 2^(1/12)
    QVector<double> ratios = {
        1.0,                    // C - 0 semitones
        std::pow(2.0, 2.0/12),  // D - 2 semitones
        std::pow(2.0, 4.0/12),  // E - 4 semitones
        std::pow(2.0, 5.0/12),  // F - 5 semitones
        std::pow(2.0, 7.0/12),  // G - 7 semitones
        std::pow(2.0, 9.0/12),  // A - 9 semitones
        std::pow(2.0, 11.0/12)  // B - 11 semitones
    };
    QVector<QString> names = {"C", "D", "E", "F", "G", "A", "B"};
    return Scale("Equal Temperament", 2, ratios, names);
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

        default:
            return justIntonation();  // Default fallback
    }
}

QJsonObject Scale::toJson() const
{
    QJsonObject json;
    json["scaleId"] = scaleId;
    json["name"] = name;  // For human readability in the file
    return json;
}

Scale Scale::fromJson(const QJsonObject &json)
{
    int id = json["scaleId"].toInt(0);
    return getScaleById(id);
}
