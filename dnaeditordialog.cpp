#include "dnaeditordialog.h"
#include "spectrumvisualizer.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QSlider>
#include <QSpinBox>
#include <QLabel>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFile>
#include <QMessageBox>
#include <QSettings>
#include <QFileInfo>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <cmath>
#include <algorithm>

// ---------------------------------------------------------------------------
// Exact rational arithmetic via __int128 — no floating-point digit ceiling
// ---------------------------------------------------------------------------
namespace {

using i128 = __int128;

i128 gcd128(i128 a, i128 b)
{
    if (a < 0) a = -a;
    if (b < 0) b = -b;
    while (b) { i128 t = b; b = a % b; a = t; }
    return a;
}

struct Rat {
    i128 num {0};
    i128 den {1};   // always > 0 after reduced()

    Rat reduced() const {
        if (num == 0) return {0, 1};
        i128 g = gcd128(num < 0 ? -num : num, den < 0 ? -den : den);
        i128 sign = (den < 0) ? -1 : 1;
        return {sign * num / g, sign * den / g};
    }

    // Convert a double (up to 6 decimal places) to exact rational
    static Rat from(double v) {
        i128 n = static_cast<int64_t>(std::llround(v * 1000000.0));
        i128 d = 1000000;
        i128 g = gcd128(n < 0 ? -n : n, d);
        return Rat{n / g, d / g};
    }
};

Rat operator+(Rat a, Rat b) { return Rat{a.num*b.den + b.num*a.den, a.den*b.den}.reduced(); }
Rat operator-(Rat a, Rat b) { return Rat{a.num*b.den - b.num*a.den, a.den*b.den}.reduced(); }
Rat operator*(Rat a, Rat b) { return Rat{a.num*b.num, a.den*b.den}.reduced(); }
Rat operator/(Rat a, Rat b) { return Rat{a.num*b.den, a.den*b.num}.reduced(); }

Rat ratPow(Rat base, int exp)
{
    bool neg = (exp < 0);
    if (neg) exp = -exp;
    Rat r{1, 1};
    for (int i = 0; i < exp; ++i) r = r * base;
    if (neg) return Rat{r.den, r.num}.reduced();
    return r;
}

// Extract exactly 'count' decimal digits from the fractional part of r.
// For repeating decimals the cycle just repeats naturally — no length limit.
QString ratFracDigits(Rat r, int count)
{
    if (r.den == 0) return {};
    i128 num = r.num < 0 ? -r.num : r.num;
    i128 den = r.den < 0 ? -r.den : r.den;
    i128 rem = num % den;

    QString out;
    out.reserve(count);
    for (int i = 0; i < count; ++i) {
        rem *= 10;
        int d = static_cast<int>(rem / den);
        d = std::clamp(d, 0, 9);
        out += QChar('0' + d);
        rem %= den;
    }
    return out;
}

} // anonymous namespace
// ---------------------------------------------------------------------------

DnaEditorDialog::DnaEditorDialog(int numHarmonics, QWidget *parent)
    : QDialog(parent)
    , numHarmonics(numHarmonics)
{
    setWindowTitle("DNA Editor");
    setMinimumSize(500, 600);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Title
    QLabel *title = new QLabel("Edit Harmonic Distribution Pattern");
    QFont titleFont = title->font();
    titleFont.setPointSize(titleFont.pointSize() + 2);
    titleFont.setBold(true);
    title->setFont(titleFont);
    mainLayout->addWidget(title);

    // Parameters form
    QFormLayout *formLayout = new QFormLayout();
    formLayout->setSpacing(10);

    // Sequence dropdown
    comboSequence = new QComboBox();
    comboSequence->addItem("All Harmonics", 0);
    comboSequence->addItem("Odd Only (Clarinet)", 1);
    comboSequence->addItem("Even Only (Hollow)", 2);
    comboSequence->addItem("Odd Dominant", 3);
    comboSequence->addItem("Fundamental+ (Near-sine)", 4);
    comboSequence->addItem("Octaves Only (Organ)", 5);
    comboSequence->addItem("Fibonacci Sequence", 6);
    comboSequence->addItem("Prime Numbers", 7);
    comboSequence->addItem("Triangular Numbers", 8);
    comboSequence->addItem("Perfect Squares", 9);
    comboSequence->addItem("Bell Curve (Gaussian)", 10);
    comboSequence->addItem("Inverse (Bright)", 11);
    comboSequence->addItem("Digit Formula", 12);
    comboSequence->addItem("Custom", -1);  // Interactive editing
    formLayout->addRow("Sequence:", comboSequence);

    mainLayout->addLayout(formLayout);

    // Digit formula widget — shown only when "Digit Formula" is selected
    createDigitFormulaWidget();
    mainLayout->addWidget(digitFormulaWidget);

    // Spectrum preview
    QLabel *previewLabel = new QLabel("Spectrum Editor:");
    previewLabel->setStyleSheet("font-weight: bold; margin-top: 10px;");
    mainLayout->addWidget(previewLabel);

    QLabel *helpText = new QLabel("Drag bar tops to adjust • Double-click to restore • Delete key to zero");
    helpText->setStyleSheet("color: gray; font-size: 9pt; margin-bottom: 5px;");
    mainLayout->addWidget(helpText);

    spectrumPreview = new SpectrumVisualizer();
    spectrumPreview->setEditable(true);  // Enable editing in custom DNA dialog

    // Connect visualizer changes to update our internal pattern
    connect(spectrumPreview, &SpectrumVisualizer::spectrumChanged,
            this, [this](const QVector<double> &spectrum) {
        customPattern = spectrum;
    });

    mainLayout->addWidget(spectrumPreview);

    // Save/Load buttons
    QHBoxLayout *fileButtonLayout = new QHBoxLayout();
    QPushButton *btnSave = new QPushButton("Save DNA...");
    QPushButton *btnLoad = new QPushButton("Load DNA...");
    btnSave->setToolTip("Save custom DNA pattern to .json file");
    btnLoad->setToolTip("Load custom DNA pattern from .json file");
    fileButtonLayout->addWidget(btnSave);
    fileButtonLayout->addWidget(btnLoad);
    fileButtonLayout->addStretch();
    mainLayout->addLayout(fileButtonLayout);

    // Button box
    QDialogButtonBox *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    mainLayout->addWidget(buttonBox);

    // Connect signals
    connect(comboSequence, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &DnaEditorDialog::onSequenceChanged);
    connect(btnSave, &QPushButton::clicked, this, &DnaEditorDialog::onSaveCustomDna);
    connect(btnLoad, &QPushButton::clicked, this, &DnaEditorDialog::onLoadCustomDna);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    // Initialize with default pattern (only if not set by setCustomPattern later)
    customPattern.resize(numHarmonics);
    updatePattern();
}

void DnaEditorDialog::setCustomPattern(const QVector<double> &pattern)
{
    // Set the custom pattern directly
    customPattern = pattern;

    // Update the spectrum visualizer
    if (spectrumPreview) {
        spectrumPreview->setSpectrum(customPattern);
    }

    // Switch combo box to "Custom" mode to indicate this is a custom pattern
    int customIndex = comboSequence->findData(-1);
    if (customIndex >= 0) {
        comboSequence->blockSignals(true);
        comboSequence->setCurrentIndex(customIndex);
        comboSequence->blockSignals(false);
    }
}

DnaEditorDialog::~DnaEditorDialog()
{
}

bool DnaEditorDialog::isDigitFormulaMode() const
{
    return comboSequence->currentData().toInt() == 12;
}

QString DnaEditorDialog::getComputedDigitString(int needed) const
{
    if (!isDigitFormulaMode()) return {};
    return computeFormulaDigits(needed);
}

int DnaEditorDialog::getFormulaStartDigit() const
{
    if (!isDigitFormulaMode() || !spinStartDigit) return 0;
    return spinStartDigit->value();
}

void DnaEditorDialog::onSequenceChanged(int /*index*/)
{
    int seqType = comboSequence->currentData().toInt();
    digitFormulaWidget->setVisible(seqType == 12);
    updatePattern();
}

void DnaEditorDialog::onSaveCustomDna()
{
    // Get current pattern (either from editor or generated from sequence/purity)
    QVector<double> patternToSave = customPattern;

    // If empty, use what's currently displayed in the visualizer
    if (patternToSave.isEmpty() && spectrumPreview) {
        patternToSave = spectrumPreview->getSpectrum();
    }

    if (patternToSave.isEmpty()) {
        QMessageBox::warning(this, "No Pattern", "No custom DNA pattern to save.");
        return;
    }

    // Open file dialog for save location
    QSettings settings;
    QString lastDir = settings.value("lastDirectory/dna", QDir::homePath()).toString();

    QString fileName = QFileDialog::getSaveFileName(
        this,
        "Save Custom DNA Pattern",
        lastDir,
        "Custom DNA Files (*.dna.json);;JSON Files (*.json);;All Files (*)"
    );

    if (fileName.isEmpty()) {
        return;  // User cancelled
    }

    settings.setValue("lastDirectory/dna", QFileInfo(fileName).absolutePath());

    // Ensure .dna.json extension
    if (!fileName.endsWith(".json", Qt::CaseInsensitive)) {
        fileName += ".dna.json";
    }

    // Create JSON object
    QJsonObject jsonObj;
    jsonObj["name"] = "Custom DNA Pattern";
    jsonObj["description"] = "Custom harmonic distribution created in Kala DNA Editor";
    jsonObj["version"] = "1.0";
    jsonObj["numHarmonics"] = patternToSave.size();

    // Store amplitudes array
    QJsonArray amplitudesArray;
    for (double amp : patternToSave) {
        amplitudesArray.append(amp);
    }
    jsonObj["amplitudes"] = amplitudesArray;

    // Write to file
    QJsonDocument doc(jsonObj);
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::critical(this, "Save Error",
            "Could not open file for writing:\n" + fileName);
        return;
    }

    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();

    QMessageBox::information(this, "Saved",
        "Custom DNA pattern saved successfully to:\n" + fileName);
}

void DnaEditorDialog::onLoadCustomDna()
{
    // Open file dialog for load location
    QSettings settings;
    QString lastDir = settings.value("lastDirectory/dna", QDir::homePath()).toString();

    QString fileName = QFileDialog::getOpenFileName(
        this,
        "Load Custom DNA Pattern",
        lastDir,
        "Custom DNA Files (*.dna.json);;JSON Files (*.json);;All Files (*)"
    );

    if (fileName.isEmpty()) {
        return;  // User cancelled
    }

    settings.setValue("lastDirectory/dna", QFileInfo(fileName).absolutePath());

    // Read file
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, "Load Error",
            "Could not open file for reading:\n" + fileName);
        return;
    }

    QByteArray jsonData = file.readAll();
    file.close();

    // Parse JSON
    QJsonDocument doc = QJsonDocument::fromJson(jsonData);
    if (doc.isNull() || !doc.isObject()) {
        QMessageBox::critical(this, "Load Error",
            "Invalid JSON format in file:\n" + fileName);
        return;
    }

    QJsonObject jsonObj = doc.object();

    // Validate required fields
    if (!jsonObj.contains("amplitudes") || !jsonObj["amplitudes"].isArray()) {
        QMessageBox::critical(this, "Load Error",
            "Missing or invalid 'amplitudes' field in JSON file.");
        return;
    }

    // Extract amplitudes
    QJsonArray amplitudesArray = jsonObj["amplitudes"].toArray();
    QVector<double> loadedPattern;

    for (const QJsonValue &val : amplitudesArray) {
        if (val.isDouble()) {
            loadedPattern.append(val.toDouble());
        } else {
            QMessageBox::warning(this, "Load Warning",
                "Some amplitude values were invalid and skipped.");
        }
    }

    if (loadedPattern.isEmpty()) {
        QMessageBox::critical(this, "Load Error",
            "No valid amplitude data found in file.");
        return;
    }

    // Load the pattern
    setCustomPattern(loadedPattern);

    // Store the filename as the loaded name
    loadedName = QFileInfo(fileName).fileName();

    QString name = jsonObj.value("name").toString("Custom DNA Pattern");
    QMessageBox::information(this, "Loaded",
        "Successfully loaded: " + name + "\n" +
        QString::number(loadedPattern.size()) + " harmonics");
}

void DnaEditorDialog::updatePattern()
{
    int sequenceType = comboSequence->currentData().toInt();
    if (sequenceType == 12) {
        generateDigitFormulaPattern();
    } else {
        generateSequencePattern(sequenceType);
    }

    // Update spectrum preview
    spectrumPreview->setSpectrum(customPattern);
}

void DnaEditorDialog::generateSequencePattern(int sequenceType)
{
    // Generate pattern according to sequence type
    customPattern.resize(numHarmonics);
    customPattern.fill(0.0);
    double totalAmp = 0.0;

    switch (sequenceType) {
    case 0: // All Harmonics
        for (int i = 0; i < numHarmonics; i++) {
            customPattern[i] = 1.0 / std::pow(i + 1, 1.5);  // Standard rolloff
            totalAmp += customPattern[i];
        }
        break;

    case 1: // Odd Only (Clarinet)
        for (int i = 0; i < numHarmonics; i++) {
            if ((i + 1) % 2 == 1) {  // Odd harmonics
                customPattern[i] = 1.0 / std::pow(i + 1, 1.4);
            } else {
                customPattern[i] = 0.0;
            }
            totalAmp += customPattern[i];
        }
        break;

    case 2: // Even Only (Hollow)
        for (int i = 0; i < numHarmonics; i++) {
            if ((i + 1) % 2 == 0) {  // Even harmonics
                customPattern[i] = 1.0 / std::pow(i + 1, 1.5);
            } else {
                customPattern[i] = 0.0;
            }
            totalAmp += customPattern[i];
        }
        break;

    case 3: // Odd Dominant
        for (int i = 0; i < numHarmonics; i++) {
            if ((i + 1) % 2 == 1) {  // Odd harmonics
                customPattern[i] = 1.0 / std::pow(i + 1, 1.3);
            } else {  // Even harmonics (much weaker)
                customPattern[i] = 0.2 / std::pow(i + 1, 2.0);
            }
            totalAmp += customPattern[i];
        }
        break;

    case 4: // Fundamental+ (Near-sine)
        customPattern[0] = 1.0;  // Fundamental
        totalAmp = 1.0;
        for (int i = 1; i < std::min(8, numHarmonics); i++) {
            customPattern[i] = 0.05 / (i + 1);  // Very weak upper harmonics
            totalAmp += customPattern[i];
        }
        break;

    case 5: // Octaves Only (Organ)
        for (int i = 0; i < numHarmonics; i++) {
            int harmonic = i + 1;
            // Check if harmonic is a power of 2 (1, 2, 4, 8, 16, 32, 64...)
            if ((harmonic & (harmonic - 1)) == 0) {
                customPattern[i] = 1.0 / harmonic;
            } else {
                customPattern[i] = 0.0;
            }
            totalAmp += customPattern[i];
        }
        break;

    case 6: // Fibonacci Sequence
        {
            // Generate Fibonacci sequence
            int fib1 = 1, fib2 = 1;
            for (int i = 0; i < numHarmonics; i++) {
                int harmonic = i + 1;
                // Check if current harmonic number is in Fibonacci sequence
                if (harmonic == fib1) {
                    customPattern[i] = 1.0 / std::pow(harmonic, 1.2);
                    // Advance to next Fibonacci number
                    int next = fib1 + fib2;
                    fib2 = fib1;
                    fib1 = next;
                } else {
                    customPattern[i] = 0.0;
                }
                totalAmp += customPattern[i];
            }
        }
        break;

    case 7: // Prime Numbers
        {
            auto isPrime = [](int n) {
                if (n <= 1) return false;
                if (n <= 3) return true;
                if (n % 2 == 0 || n % 3 == 0) return false;
                for (int i = 5; i * i <= n; i += 6) {
                    if (n % i == 0 || n % (i + 2) == 0) return false;
                }
                return true;
            };

            for (int i = 0; i < numHarmonics; i++) {
                int harmonic = i + 1;
                if (isPrime(harmonic)) {
                    customPattern[i] = 1.0 / std::pow(harmonic, 1.3);
                } else {
                    customPattern[i] = 0.0;
                }
                totalAmp += customPattern[i];
            }
        }
        break;

    case 8: // Triangular Numbers
        {
            // Triangular numbers: T(n) = n(n+1)/2 → 1, 3, 6, 10, 15, 21, 28, 36, 45, 55...
            int n = 1;
            int triangular = 1;
            for (int i = 0; i < numHarmonics; i++) {
                int harmonic = i + 1;
                if (harmonic == triangular) {
                    customPattern[i] = 1.0 / std::pow(harmonic, 1.2);
                    // Next triangular number
                    n++;
                    triangular = (n * (n + 1)) / 2;
                } else {
                    customPattern[i] = 0.0;
                }
                totalAmp += customPattern[i];
            }
        }
        break;

    case 9: // Perfect Squares
        {
            int n = 1;
            for (int i = 0; i < numHarmonics; i++) {
                int harmonic = i + 1;
                if (harmonic == n * n) {
                    customPattern[i] = 1.0 / std::pow(harmonic, 1.1);
                    n++;
                } else {
                    customPattern[i] = 0.0;
                }
                totalAmp += customPattern[i];
            }
        }
        break;

    case 10: // Bell Curve (Gaussian)
        {
            // Peak at middle harmonics, fade both directions
            double center = numHarmonics / 3.0;  // Peak around 1/3 point
            double width = numHarmonics / 6.0;   // Standard deviation
            for (int i = 0; i < numHarmonics; i++) {
                double distance = (i - center) / width;
                // Gaussian curve: e^(-(x²/2))
                double gaussian = std::exp(-(distance * distance) / 2.0);
                customPattern[i] = gaussian / std::pow(i + 1, 0.8);  // Gentle rolloff
                totalAmp += customPattern[i];
            }
        }
        break;

    case 11: // Inverse (Bright/Subharmonic)
        {
            // Amplitude increases with harmonic number (opposite of normal)
            for (int i = 0; i < numHarmonics; i++) {
                int harmonic = i + 1;
                // Inverse rolloff: higher harmonics are louder
                customPattern[i] = std::pow(harmonic, 0.5) / numHarmonics;
                totalAmp += customPattern[i];
            }
        }
        break;

    case -1: // Custom (for interactive editing)
        // Keep current pattern
        return;

    default:
        // Default to all harmonics
        for (int i = 0; i < numHarmonics; i++) {
            customPattern[i] = 1.0 / std::pow(i + 1, 1.5);
            totalAmp += customPattern[i];
        }
        break;
    }

    // Normalize
    if (totalAmp > 0.0) {
        for (int i = 0; i < numHarmonics; i++) {
            customPattern[i] /= totalAmp;
        }
    }
}

// =============================================================================
//  Digit Formula
// =============================================================================

void DnaEditorDialog::createDigitFormulaWidget()
{
    digitFormulaWidget = new QWidget();
    digitFormulaWidget->setVisible(false);
    digitFormulaWidget->setStyleSheet(
        "QWidget#digitFormulaWidget { background: #1e2e1e; border: 1px solid #3a5a3a; "
        "border-radius: 4px; } QLabel { background: transparent; }");
    digitFormulaWidget->setObjectName("digitFormulaWidget");

    QVBoxLayout *vl = new QVBoxLayout(digitFormulaWidget);
    vl->setContentsMargins(10, 8, 10, 8);
    vl->setSpacing(6);

    QFormLayout *fl = new QFormLayout();
    fl->setSpacing(6);

    // Row 1: Named constant presets
    comboConstantPresets = new QComboBox();
    comboConstantPresets->addItem("— Custom formula —", 0);
    comboConstantPresets->addItem("π  (Pi = 3.14159…)",          1);
    comboConstantPresets->addItem("e  (Euler = 2.71828…)",       2);
    comboConstantPresets->addItem("φ  (Golden Ratio = 1.61803…)", 3);
    comboConstantPresets->addItem("√2  (= 1.41421…)",            4);
    comboConstantPresets->addItem("√3  (= 1.73205…)",            5);
    comboConstantPresets->addItem("√5  (= 2.23606…)",            6);
    fl->addRow("Constant:", comboConstantPresets);

    // Row 2: Formula inputs (A  op  B)
    QHBoxLayout *formulaRow = new QHBoxLayout();
    formulaRow->setSpacing(6);

    spinOperandA = new QDoubleSpinBox();
    spinOperandA->setRange(-999999.0, 999999.0);
    spinOperandA->setDecimals(6);
    spinOperandA->setSingleStep(1.0);
    spinOperandA->setValue(1.0);
    spinOperandA->setFixedWidth(110);

    comboOperator = new QComboBox();
    comboOperator->addItem("÷",         0);
    comboOperator->addItem("×",         1);
    comboOperator->addItem("+",         2);
    comboOperator->addItem("−",         3);
    comboOperator->addItem("√A  (unary)", 4);
    comboOperator->addItem("A²  (unary)", 5);
    comboOperator->addItem("Aᴮ  (power)", 6);
    comboOperator->setFixedWidth(110);

    labelOperandB = new QLabel("B:");
    spinOperandB = new QDoubleSpinBox();
    spinOperandB->setRange(-999999.0, 999999.0);
    spinOperandB->setDecimals(6);
    spinOperandB->setSingleStep(1.0);
    spinOperandB->setValue(7.0);
    spinOperandB->setFixedWidth(110);

    formulaRow->addWidget(new QLabel("A:"));
    formulaRow->addWidget(spinOperandA);
    formulaRow->addWidget(comboOperator);
    formulaRow->addWidget(labelOperandB);
    formulaRow->addWidget(spinOperandB);
    formulaRow->addStretch();

    QWidget *formulaContainer = new QWidget();
    formulaContainer->setLayout(formulaRow);
    fl->addRow("Formula:", formulaContainer);

    // Row 3: Start digit offset + normalize
    QHBoxLayout *optRow = new QHBoxLayout();
    optRow->setSpacing(8);

    spinStartDigit = new QSpinBox();
    spinStartDigit->setRange(0, 119);
    spinStartDigit->setValue(0);
    spinStartDigit->setFixedWidth(65);
    spinStartDigit->setToolTip("Skip this many fractional digits before reading amplitudes");

    checkNormalize = new QCheckBox("Normalize amplitude");
    checkNormalize->setChecked(true);
    checkNormalize->setToolTip(
        "Checked: distribute total loudness evenly (all patterns equally loud)\n"
        "Unchecked: raw digit/9 values — quiet zones stay quiet");

    optRow->addWidget(spinStartDigit);
    optRow->addSpacing(10);
    optRow->addWidget(checkNormalize);
    optRow->addStretch();

    QWidget *optContainer = new QWidget();
    optContainer->setLayout(optRow);
    fl->addRow("Start digit:", optContainer);

    vl->addLayout(fl);

    // Result info label
    labelResult = new QLabel("…");
    labelResult->setStyleSheet("color: #88cc88; font-size: 9pt;");
    labelResult->setWordWrap(true);
    vl->addWidget(labelResult);

    // --- Signal wiring ---

    // Constant preset changes: show/hide formula row
    connect(comboConstantPresets, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int idx) {
                bool isCustom = (idx == 0);
                spinOperandA->setVisible(isCustom);
                comboOperator->setVisible(isCustom);
                bool showB = isCustom && comboOperator->currentIndex() < 4;
                labelOperandB->setVisible(showB);
                spinOperandB->setVisible(showB);
                // Extend start-digit range for constants (120 digits available)
                spinStartDigit->setMaximum(isCustom ? 17 : 119);
                if (spinStartDigit->value() > spinStartDigit->maximum())
                    spinStartDigit->setValue(0);
                updatePattern();
            });

    // Operator changes: show/hide B for unary ops
    connect(comboOperator, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int opIdx) {
                bool unary = (opIdx == 4 || opIdx == 5);
                labelOperandB->setVisible(!unary);
                spinOperandB->setVisible(!unary);
                updatePattern();
            });

    connect(spinOperandA, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [this](double) { updatePattern(); });
    connect(spinOperandB, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [this](double) { updatePattern(); });
    connect(spinStartDigit, QOverload<int>::of(&QSpinBox::valueChanged),
            this, [this](int) { updatePattern(); });
    connect(checkNormalize, &QCheckBox::toggled,
            this, [this](bool) { updatePattern(); });
}

// -----------------------------------------------------------------------------

bool DnaEditorDialog::isRationalFormula() const
{
    if (comboConstantPresets->currentIndex() != 0) return false; // constants handled separately
    int op = comboOperator->currentIndex();
    if (op == 4) return false;  // √A — irrational in general
    if (op == 6) {              // Aᴮ — rational only for small integer B
        double b = spinOperandB->value();
        double bRound = std::round(b);
        return (std::abs(b - bRound) < 1e-9 && std::abs(bRound) <= 20);
    }
    return true; // ÷, ×, +, −, A² are always rational
}

QString DnaEditorDialog::computeFormulaDigits(int needed) const
{
    int constIdx = comboConstantPresets->currentIndex();

    if (constIdx > 0) {
        // Hardcoded 120-digit fractional expansions of well-known constants
        static const QString piDigits =
            "14159265358979323846264338327950288419716939937510"
            "58209749445923078164062862089986280348253421170679"
            "82148086513282306647093844609550582231725359408128";
        static const QString eDigits =
            "71828182845904523536028747135266249775724709369995"
            "95749669676277240766303535475945713821785251664274"
            "27466391932003059921817413596629043572900334295260";
        static const QString phiDigits =
            "61803398874989484820458683436563811772030917980576"
            "28621354486227052604628189024497072072041893911374"
            "84756536166187843615283192742044050518939965143143";
        static const QString sqrt2Digits =
            "41421356237309504880168872420969807856967187537694"
            "81060455752654541098227794358533786198554052395292"
            "89056997716766164890498826656646058662609974467217";
        static const QString sqrt3Digits =
            "73205080756887729352744634150587236694280525381038"
            "06280558069794519330169088000370811461867572485756"
            "26141415406701938331952783248765373185925854760402";
        static const QString sqrt5Digits =
            "23606797749978969640917366873127623544061835961152"
            "57242708972454105209256378048994144144083787822749"
            "69508176150773783049520898706952555263958994549787";

        switch (constIdx) {
        case 1: return piDigits;
        case 2: return eDigits;
        case 3: return phiDigits;
        case 4: return sqrt2Digits;
        case 5: return sqrt3Digits;
        case 6: return sqrt5Digits;
        default: return piDigits;
        }
    }

    int opIdx = comboOperator->currentIndex();

    // --- Rational path: exact, unlimited digits ---
    if (isRationalFormula()) {
        Rat a = Rat::from(spinOperandA->value());
        Rat b = Rat::from(spinOperandB->value());
        Rat result{0, 1};

        switch (opIdx) {
        case 0: // A ÷ B
            if (b.num == 0) return {};
            result = a / b;
            break;
        case 1: result = a * b; break;   // A × B
        case 2: result = a + b; break;   // A + B
        case 3: result = a - b; break;   // A − B
        case 5: result = a * a; break;   // A²
        case 6: {                         // Aᴮ, integer B ≤ 20
            int bInt = static_cast<int>(std::round(spinOperandB->value()));
            result = ratPow(a, bInt);
            break;
        }
        default: return {};
        }
        return ratFracDigits(result, needed);
    }

    // --- Irrational path: long double (~18 reliable digits) ---
    long double a = static_cast<long double>(spinOperandA->value());
    long double b = static_cast<long double>(spinOperandB->value());

    long double result = 0.0L;
    switch (opIdx) {
    case 4:                          // √A
        if (a < 0.0L) return {};
        result = sqrtl(a);
        break;
    case 6:                          // Aᴮ, non-integer or large B
        if (a < 0.0L && fabsl(b - roundl(b)) > 1e-12L) return {};
        result = powl(a, b);
        break;
    default: return {};
    }

    if (!std::isfinite(static_cast<double>(result))) return {};
    return extractLongDoubleDigits(result);
}

// -----------------------------------------------------------------------------

QString DnaEditorDialog::extractLongDoubleDigits(long double value) const
{
    long double intPart;
    long double frac = modfl(fabsl(value), &intPart);

    QString digits;
    // long double (80-bit) gives ~18-19 reliable decimal digits
    for (int i = 0; i < 18; i++) {
        frac *= 10.0L;
        int d = static_cast<int>(frac);
        d = std::clamp(d, 0, 9);
        digits += QChar('0' + d);
        frac -= static_cast<long double>(d);
        if (frac < 0.0L) frac = 0.0L;
    }
    return digits;
}

// -----------------------------------------------------------------------------

void DnaEditorDialog::generateDigitFormulaPattern()
{
    // Determine digit availability and update the start-digit spinbox range
    bool isConst    = (comboConstantPresets->currentIndex() > 0);
    bool isRational = !isConst && isRationalFormula();
    bool isFloat    = !isConst && !isRational;

    int maxStart = isConst ? 119 : (isFloat ? 17 : 9999);
    spinStartDigit->blockSignals(true);
    spinStartDigit->setMaximum(maxStart);
    if (spinStartDigit->value() > maxStart) spinStartDigit->setValue(0);
    spinStartDigit->blockSignals(false);

    int startDigit = spinStartDigit->value();
    int needed     = startDigit + numHarmonics;

    QString digitStr = computeFormulaDigits(needed);

    if (digitStr.isEmpty()) {
        labelResult->setText("Cannot compute (division by zero, negative root, or overflow)");
        customPattern.resize(numHarmonics);
        customPattern.fill(0.0);
        return;
    }

    bool normalize = checkNormalize->isChecked();
    int available  = digitStr.length();

    customPattern.resize(numHarmonics);
    customPattern.fill(0.0);

    double totalAmp = 0.0;
    for (int i = 0; i < numHarmonics; i++) {
        int pos = startDigit + i;
        if (pos < available) {
            int d = digitStr[pos].digitValue();
            customPattern[i] = d / 9.0;
        }
        totalAmp += customPattern[i];
    }

    if (normalize && totalAmp > 0.0) {
        for (int i = 0; i < numHarmonics; i++)
            customPattern[i] /= totalAmp;
    }

    updateDigitResultLabel(digitStr);
}

// -----------------------------------------------------------------------------

void DnaEditorDialog::updateDigitResultLabel(const QString &digitStr)
{
    int startDigit = spinStartDigit->value();
    int available  = digitStr.length();
    int end        = std::min(startDigit + numHarmonics, available);
    int previewLen = std::min(24, end - startDigit);

    QString active = digitStr.mid(startDigit, previewLen);
    bool truncated = (startDigit + numHarmonics > available);
    if (truncated)
        active += " … (zeros fill remaining harmonics)";
    else if (startDigit + numHarmonics < available)
        active += "…";

    QString label = QString("digits [%1 – %2]:  %3\n(%4 digits available, %5 harmonics)")
        .arg(startDigit)
        .arg(startDigit + numHarmonics - 1)
        .arg(active)
        .arg(available)
        .arg(numHarmonics);

    labelResult->setText(label);
}
