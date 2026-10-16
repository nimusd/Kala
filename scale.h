#ifndef SCALE_H
#define SCALE_H

#include <QString>
#include <QVector>
#include <QJsonObject>

class Scale
{
public:
    // Scale constructor
    Scale(const QString &name = "Just Intonation", int scaleId = 0);

    // Getters
    QString getName() const { return name; }
    // Name for display: the scale's name, plus the reference degree's note
    // name when one is set — the key of a rotated ET ("Equal Temperament (F)")
    // or the tonic of a maqam/raga/JI stretch ("Just Intonation (G)",
    // "Raga Bhairav (Pa)"). Without it the bare name would read identically at
    // every reference move. For ET the key name is the sharp one — the ET
    // table is (Bb reads A#).
    QString getDisplayName() const;
    int getScaleId() const { return scaleId; }
    double getRatio(int degree) const;       // degree is 0 to (degreeCount-1), incorporates cent offset
    double getRawRatio(int degree) const;    // raw ratio without cent offset applied
    int getDegreeCount() const { return ratios.size(); }
    QString getNoteName(int degree) const;  // Get note name for this degree
    bool getIsAccidental(int degree) const; // True for chromatic degrees not in the diatonic core
    int getTonicIndex() const { return tonicIndex; }  // Reference (colour) degree, 0-based into THIS scale's table; -1 = none
    bool is24TETMaqam() const;  // True for the maqam scales tuned to the 24-TET quarter-tone grid

    // Per-degree cent offsets for microtonal inflection (±50¢ per degree)
    void setCentOffsets(const QVector<double> &offsets);
    const QVector<double>& getCentOffsets() const { return centOffsets; }

    // Static factory methods for predefined scales
    static Scale justIntonation();
    static Scale pythagorean();
    static Scale equalTemperament();
    static Scale quarterCommaMeantone();

    // Maqam scales
    static Scale maqamRast();
    static Scale maqamBayati();
    static Scale maqamSaba();
    static Scale maqamHijaz();
    static Scale maqamNahawand();
    static Scale maqamKurd();

    // Indian Ragas
    static Scale ragaBhairav();
    static Scale ragaYaman();
    static Scale ragaTodi();
    static Scale ragaBhairavi();
    static Scale ragaShankarabharana();
    static Scale ragaMayamalavagowla();

    // Indonesian
    static Scale pelog();
    static Scale slendro();

    // Chinese
    static Scale chinesePentatonic();

    // Japanese
    static Scale hirajoshi();

    // Western (additional)
    static Scale wholeTone();
    static Scale halfDiminished();

    // Diatonic modes (Equal Temperament)
    static Scale modeIonian();
    static Scale modeDorian();
    static Scale modePhrygian();
    static Scale modeLydian();
    static Scale modeMixolydian();
    static Scale modeAeolian();
    static Scale modeLocrian();

    // Melodic minor and its modes (jazz)
    static Scale melodicMinor();
    static Scale modeDorianB2();
    static Scale modeLydianAugmented();
    static Scale modeLydianDominant();
    static Scale modeMixolydianB6();
    static Scale modeLocrianSharp2();
    static Scale modeAltered();

    // Additional Ragas
    static Scale ragaKafi();
    static Scale ragaAsavari();
    static Scale ragaKhamaj();
    static Scale ragaMarwa();
    static Scale ragaPurvi();

    // Additional Maqams
    static Scale maqamSikah();
    static Scale maqamAjam();
    static Scale maqamAtharKurd();

    // Maqams in 5-limit just intonation (the pre-24-TET tuning, kept alongside)
    static Scale maqamRastJI();
    static Scale maqamBayatiJI();
    static Scale maqamSabaJI();
    static Scale maqamHijazJI();
    static Scale maqamNahawandJI();
    static Scale maqamKurdJI();
    static Scale maqamSikahJI();
    static Scale maqamAjamJI();
    static Scale maqamAtharKurdJI();

    // Persian Dastgah
    static Scale persianShur();
    static Scale persianMahur();
    static Scale persianSegah();
    static Scale persianChahargah();
    static Scale persianHomayun();

    static QVector<Scale> getAllScales();

    // Get scale by ID
    static Scale getScaleById(int id);

    // Create an ET scale rotated to a different tonic (base frequency stays A=440Hz, only colors change)
    static Scale equalTemperamentWithTonic(int tonicIndex);  // 0=C, 1=C#, ..., 9=A, 10=A#, 11=B

    // The same scale with its reference degree moved — 0-based index into this
    // scale's own degree table (ET: 0=C … 11=B; maqam/raga/JI: 0=root … n-1).
    // Ratios, cent offsets and the base frequency are untouched: a reference
    // move is a COLOUR change only, nothing retunes. Out of range (including
    // -1) returns the scale unchanged.
    Scale withTonic(int degreeIndex) const;

    // Serialization
    QJsonObject toJson() const;
    static Scale fromJson(const QJsonObject &json);

private:
    QString name;
    int scaleId;  // Unique identifier
    QVector<double> ratios;      // Ratios for scale degrees (variable count)
    QVector<QString> noteNames;  // Note names for each degree
    QVector<bool> accidentals;   // True for degrees that are chromatic/accidental (drawn dark grey)
    QVector<double> centOffsets; // Per-degree cent offsets (-50 to +50, default 0)
    int tonicIndex;             // Reference (colour) degree, 0-based into this scale's own table; -1 = none

    // Private constructor for setting custom ratios and names
    Scale(const QString &name, int scaleId, const QVector<double> &ratios, const QVector<QString> &noteNames,
          const QVector<bool> &accidentals = {}, int tonicIdx = -1);
};

#endif // SCALE_H
