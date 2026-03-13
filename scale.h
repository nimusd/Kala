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
    int getScaleId() const { return scaleId; }
    double getRatio(int degree) const;  // degree is 0 to (degreeCount-1)
    int getDegreeCount() const { return ratios.size(); }
    QString getNoteName(int degree) const;  // Get note name for this degree

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

    // Persian Dastgah
    static Scale persianShur();
    static Scale persianMahur();
    static Scale persianSegah();
    static Scale persianChahargah();
    static Scale persianHomayun();

    static QVector<Scale> getAllScales();

    // Get scale by ID
    static Scale getScaleById(int id);

    // Serialization
    QJsonObject toJson() const;
    static Scale fromJson(const QJsonObject &json);

private:
    QString name;
    int scaleId;  // Unique identifier
    QVector<double> ratios;  // Ratios for scale degrees (variable count)
    QVector<QString> noteNames;  // Note names for each degree

    // Private constructor for setting custom ratios and names
    Scale(const QString &name, int scaleId, const QVector<double> &ratios, const QVector<QString> &noteNames);
};

#endif // SCALE_H
