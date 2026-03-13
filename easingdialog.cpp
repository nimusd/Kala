#include "easingdialog.h"
#include "ui_easingdialog.h"
#include "containersettings.h"
#include "easingpreviewwidget.h"
#include <QVBoxLayout>

EasingDialog::EasingDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::EasingDialog)
{
    ui->setupUi(this);

    // Get settings for default values and ranges
    const auto &easingSettings = ContainerSettings::instance().easing;

    // Configure spinbox ranges and defaults from settings
    // Sine Amplitude
    ui->spinSineAmplitude->setRange(0.1, 3.0);
    ui->spinSineAmplitude->setValue(Easing::getDefaultSineAmplitude());
    ui->sliderSineAmplitude->setRange(10, 300);
    ui->sliderSineAmplitude->setValue(static_cast<int>(Easing::getDefaultSineAmplitude() * 100));

    // Sine Cycles
    ui->spinSineCycles->setRange(0.25, 4.0);
    ui->spinSineCycles->setValue(Easing::getDefaultSineCycles());
    ui->sliderSineCycles->setRange(25, 400);
    ui->sliderSineCycles->setValue(static_cast<int>(Easing::getDefaultSineCycles() * 100));

    // Power (Quad/Cubic/Quartic)
    ui->spinPower->setRange(0.5, 8.0);
    ui->spinPower->setValue(Easing::getDefaultPower());
    ui->sliderPower->setRange(50, 800);
    ui->sliderPower->setValue(static_cast<int>(Easing::getDefaultPower() * 100));

    // Expo Strength
    ui->spinExpoStrength->setRange(1.0, 20.0);
    ui->spinExpoStrength->setValue(Easing::getDefaultExpoStrength());
    ui->sliderExpoStrength->setRange(10, 200);
    ui->sliderExpoStrength->setValue(static_cast<int>(Easing::getDefaultExpoStrength() * 10));

    // Circ Strength
    ui->spinCircStrength->setRange(0.1, 3.0);
    ui->spinCircStrength->setValue(Easing::getDefaultCircStrength());
    ui->sliderCircStrength->setRange(10, 300);
    ui->sliderCircStrength->setValue(static_cast<int>(Easing::getDefaultCircStrength() * 100));

    // Bounce Count
    ui->spinBounceCount->setRange(1.0, 8.0);
    ui->spinBounceCount->setValue(Easing::getDefaultBounceCount());
    ui->sliderBounceCount->setRange(10, 80);
    ui->sliderBounceCount->setValue(static_cast<int>(Easing::getDefaultBounceCount() * 10));

    // Bounce Strength
    ui->spinBounceStrength->setRange(1.0, 20.0);
    ui->spinBounceStrength->setValue(Easing::getDefaultBounceStrength());
    ui->sliderBounceStrength->setRange(10, 200);
    ui->sliderBounceStrength->setValue(static_cast<int>(Easing::getDefaultBounceStrength() * 10));

    // Overshoot
    ui->spinOvershoot->setRange(easingSettings.overshootMin, easingSettings.overshootMax);
    ui->spinOvershoot->setValue(easingSettings.overshootDefault);
    ui->sliderOvershoot->setRange(
        static_cast<int>(easingSettings.overshootMin * 100),
        static_cast<int>(easingSettings.overshootMax * 100));
    ui->sliderOvershoot->setValue(static_cast<int>(easingSettings.overshootDefault * 100));

    // Amplitude
    ui->spinAmplitude->setRange(easingSettings.amplitudeMin, easingSettings.amplitudeMax);
    ui->spinAmplitude->setValue(easingSettings.amplitudeDefault);
    ui->sliderAmplitude->setRange(
        static_cast<int>(easingSettings.amplitudeMin * 100),
        static_cast<int>(easingSettings.amplitudeMax * 100));
    ui->sliderAmplitude->setValue(static_cast<int>(easingSettings.amplitudeDefault * 100));

    // Period
    ui->spinPeriod->setRange(easingSettings.periodMin, easingSettings.periodMax);
    ui->spinPeriod->setValue(easingSettings.periodDefault);
    ui->sliderPeriod->setRange(
        static_cast<int>(easingSettings.periodMin * 100),
        static_cast<int>(easingSettings.periodMax * 100));
    ui->sliderPeriod->setValue(static_cast<int>(easingSettings.periodDefault * 100));

    // Frequency
    ui->spinFrequency->setRange(easingSettings.frequencyMin, easingSettings.frequencyMax);
    ui->spinFrequency->setValue(easingSettings.frequencyDefault);
    ui->sliderFrequency->setRange(
        static_cast<int>(easingSettings.frequencyMin * 10),
        static_cast<int>(easingSettings.frequencyMax * 10));
    ui->sliderFrequency->setValue(static_cast<int>(easingSettings.frequencyDefault * 10));

    // Decay
    ui->spinDecay->setRange(easingSettings.decayMin, easingSettings.decayMax);
    ui->spinDecay->setValue(easingSettings.decayDefault);
    ui->sliderDecay->setRange(
        static_cast<int>(easingSettings.decayMin * 100),
        static_cast<int>(easingSettings.decayMax * 100));
    ui->sliderDecay->setValue(static_cast<int>(easingSettings.decayDefault * 100));

    populateEasingTypes();

    // Create and insert the curve preview widget after the easing type group
    previewWidget = new EasingPreviewWidget(this);
    QVBoxLayout *mainLayout = qobject_cast<QVBoxLayout*>(layout());
    if (mainLayout) {
        // Insert after the first item (easing type group)
        mainLayout->insertWidget(1, previewWidget);
    }

    // Connect easing type change to parameter visibility update and preview
    connect(ui->comboEasingType, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &EasingDialog::onEasingTypeChanged);
    connect(ui->comboEasingType, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &EasingDialog::updatePreview);

    // Slider <-> SpinBox synchronization (sliders use int * 100 or * 10, spinboxes use double)
    // Sine Amplitude: slider * 100
    connect(ui->sliderSineAmplitude, &QSlider::valueChanged, this, [this](int value) {
        ui->spinSineAmplitude->setValue(value / 100.0);
    });
    connect(ui->spinSineAmplitude, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double value) {
        ui->sliderSineAmplitude->setValue(static_cast<int>(value * 100));
    });

    // Sine Cycles: slider * 100
    connect(ui->sliderSineCycles, &QSlider::valueChanged, this, [this](int value) {
        ui->spinSineCycles->setValue(value / 100.0);
    });
    connect(ui->spinSineCycles, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double value) {
        ui->sliderSineCycles->setValue(static_cast<int>(value * 100));
    });

    // Power: slider * 100
    connect(ui->sliderPower, &QSlider::valueChanged, this, [this](int value) {
        ui->spinPower->setValue(value / 100.0);
    });
    connect(ui->spinPower, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double value) {
        ui->sliderPower->setValue(static_cast<int>(value * 100));
    });

    // Expo Strength: slider * 10
    connect(ui->sliderExpoStrength, &QSlider::valueChanged, this, [this](int value) {
        ui->spinExpoStrength->setValue(value / 10.0);
    });
    connect(ui->spinExpoStrength, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double value) {
        ui->sliderExpoStrength->setValue(static_cast<int>(value * 10));
    });

    // Circ Strength: slider * 100
    connect(ui->sliderCircStrength, &QSlider::valueChanged, this, [this](int value) {
        ui->spinCircStrength->setValue(value / 100.0);
    });
    connect(ui->spinCircStrength, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double value) {
        ui->sliderCircStrength->setValue(static_cast<int>(value * 100));
    });

    // Bounce Count: slider * 10
    connect(ui->sliderBounceCount, &QSlider::valueChanged, this, [this](int value) {
        ui->spinBounceCount->setValue(value / 10.0);
    });
    connect(ui->spinBounceCount, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double value) {
        ui->sliderBounceCount->setValue(static_cast<int>(value * 10));
    });

    // Bounce Strength: slider * 10
    connect(ui->sliderBounceStrength, &QSlider::valueChanged, this, [this](int value) {
        ui->spinBounceStrength->setValue(value / 10.0);
    });
    connect(ui->spinBounceStrength, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double value) {
        ui->sliderBounceStrength->setValue(static_cast<int>(value * 10));
    });

    // Overshoot: slider * 100
    connect(ui->sliderOvershoot, &QSlider::valueChanged, this, [this](int value) {
        ui->spinOvershoot->setValue(value / 100.0);
    });
    connect(ui->spinOvershoot, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double value) {
        ui->sliderOvershoot->setValue(static_cast<int>(value * 100));
    });

    // Amplitude: slider * 100
    connect(ui->sliderAmplitude, &QSlider::valueChanged, this, [this](int value) {
        ui->spinAmplitude->setValue(value / 100.0);
    });
    connect(ui->spinAmplitude, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double value) {
        ui->sliderAmplitude->setValue(static_cast<int>(value * 100));
    });

    // Period: slider * 100
    connect(ui->sliderPeriod, &QSlider::valueChanged, this, [this](int value) {
        ui->spinPeriod->setValue(value / 100.0);
    });
    connect(ui->spinPeriod, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double value) {
        ui->sliderPeriod->setValue(static_cast<int>(value * 100));
    });

    // Frequency: slider * 10
    connect(ui->sliderFrequency, &QSlider::valueChanged, this, [this](int value) {
        ui->spinFrequency->setValue(value / 10.0);
    });
    connect(ui->spinFrequency, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double value) {
        ui->sliderFrequency->setValue(static_cast<int>(value * 10));
    });

    // Decay: slider * 100
    connect(ui->sliderDecay, &QSlider::valueChanged, this, [this](int value) {
        ui->spinDecay->setValue(value / 100.0);
    });
    connect(ui->spinDecay, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double value) {
        ui->sliderDecay->setValue(static_cast<int>(value * 100));
    });

    // Weight slider <-> spinbox synchronization (slider * 100)
    connect(ui->sliderWeight, &QSlider::valueChanged, this, [this](int value) {
        ui->spinWeight->setValue(value / 100.0);
    });
    connect(ui->spinWeight, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double value) {
        ui->sliderWeight->setValue(static_cast<int>(value * 100));
    });

    // Connect all spinboxes to update the preview curve
    connect(ui->spinSineAmplitude, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &EasingDialog::updatePreview);
    connect(ui->spinSineCycles, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &EasingDialog::updatePreview);
    connect(ui->spinPower, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &EasingDialog::updatePreview);
    connect(ui->spinExpoStrength, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &EasingDialog::updatePreview);
    connect(ui->spinCircStrength, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &EasingDialog::updatePreview);
    connect(ui->spinBounceCount, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &EasingDialog::updatePreview);
    connect(ui->spinBounceStrength, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &EasingDialog::updatePreview);
    connect(ui->spinOvershoot, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &EasingDialog::updatePreview);
    connect(ui->spinAmplitude, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &EasingDialog::updatePreview);
    connect(ui->spinPeriod, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &EasingDialog::updatePreview);
    connect(ui->spinFrequency, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &EasingDialog::updatePreview);
    connect(ui->spinDecay, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &EasingDialog::updatePreview);

    // Initialize parameter visibility and preview
    updateParameterVisibility();
    updatePreview();
}

EasingDialog::~EasingDialog()
{
    delete ui;
}

void EasingDialog::populateEasingTypes()
{
    // Add easings grouped by category
    QVector<Easing> allEasings = Easing::getAllEasings();

    QString currentCategory;
    for (const Easing &easing : allEasings) {
        QString category = easing.getCategory();

        // Add category separator if category changed
        if (category != currentCategory && !currentCategory.isEmpty()) {
            ui->comboEasingType->insertSeparator(ui->comboEasingType->count());
        }
        currentCategory = category;

        // Add easing with its ID as user data
        ui->comboEasingType->addItem(easing.getName(), easing.getId());
    }

    // Default to QuadInOut (common choice for rhythmic easing)
    int defaultIndex = ui->comboEasingType->findData(static_cast<int>(Easing::Type::QuadInOut));
    if (defaultIndex >= 0) {
        ui->comboEasingType->setCurrentIndex(defaultIndex);
    }
}

void EasingDialog::onEasingTypeChanged(int index)
{
    Q_UNUSED(index);
    updateParameterVisibility();
}

void EasingDialog::updateParameterVisibility()
{
    int easingId = ui->comboEasingType->currentData().toInt();
    Easing easing = Easing::getById(easingId);

    Easing::Type type = easing.getType();

    // Power easings (Quad/Cubic/Quartic): power parameter
    bool isPower = (type == Easing::Type::QuadIn ||
                    type == Easing::Type::QuadOut ||
                    type == Easing::Type::QuadInOut ||
                    type == Easing::Type::CubicIn ||
                    type == Easing::Type::CubicOut ||
                    type == Easing::Type::CubicInOut ||
                    type == Easing::Type::QuarticIn ||
                    type == Easing::Type::QuarticOut ||
                    type == Easing::Type::QuarticInOut);
    ui->labelPower->setVisible(isPower);
    ui->sliderPower->setVisible(isPower);
    ui->spinPower->setVisible(isPower);

    // Update power spinbox default based on selected type
    if (isPower) {
        double defaultPower = easing.getPower();
        ui->spinPower->setValue(defaultPower);
    }

    // Sine parameters: amplitude and cycles
    bool isSine = (type == Easing::Type::SineIn ||
                   type == Easing::Type::SineOut ||
                   type == Easing::Type::SineInOut);
    ui->labelSineAmplitude->setVisible(isSine);
    ui->sliderSineAmplitude->setVisible(isSine);
    ui->spinSineAmplitude->setVisible(isSine);
    ui->labelSineCycles->setVisible(isSine);
    ui->sliderSineCycles->setVisible(isSine);
    ui->spinSineCycles->setVisible(isSine);

    // Expo parameters: strength
    bool isExpo = (type == Easing::Type::ExpoIn ||
                   type == Easing::Type::ExpoOut ||
                   type == Easing::Type::ExpoInOut);
    ui->labelExpoStrength->setVisible(isExpo);
    ui->sliderExpoStrength->setVisible(isExpo);
    ui->spinExpoStrength->setVisible(isExpo);

    // Circ parameters: curvature
    bool isCirc = (type == Easing::Type::CircIn ||
                   type == Easing::Type::CircOut ||
                   type == Easing::Type::CircInOut);
    ui->labelCircStrength->setVisible(isCirc);
    ui->sliderCircStrength->setVisible(isCirc);
    ui->spinCircStrength->setVisible(isCirc);

    // Bounce parameters: count and strength
    bool isBounce = (type == Easing::Type::BounceIn ||
                     type == Easing::Type::BounceOut ||
                     type == Easing::Type::BounceInOut);
    ui->labelBounceCount->setVisible(isBounce);
    ui->sliderBounceCount->setVisible(isBounce);
    ui->spinBounceCount->setVisible(isBounce);
    ui->labelBounceStrength->setVisible(isBounce);
    ui->sliderBounceStrength->setVisible(isBounce);
    ui->spinBounceStrength->setVisible(isBounce);

    // Back parameters: overshoot
    bool isBack = (type == Easing::Type::BackIn ||
                   type == Easing::Type::BackOut ||
                   type == Easing::Type::BackInOut);
    ui->labelOvershoot->setVisible(isBack);
    ui->sliderOvershoot->setVisible(isBack);
    ui->spinOvershoot->setVisible(isBack);

    // Elastic parameters: amplitude and period
    bool isElastic = (type == Easing::Type::ElasticIn ||
                      type == Easing::Type::ElasticOut ||
                      type == Easing::Type::ElasticInOut);
    ui->labelAmplitude->setVisible(isElastic);
    ui->sliderAmplitude->setVisible(isElastic);
    ui->spinAmplitude->setVisible(isElastic);
    ui->labelPeriod->setVisible(isElastic);
    ui->sliderPeriod->setVisible(isElastic);
    ui->spinPeriod->setVisible(isElastic);

    // Wobble parameters: frequency and decay
    bool isWobble = (type == Easing::Type::Wobble);
    ui->labelFrequency->setVisible(isWobble);
    ui->sliderFrequency->setVisible(isWobble);
    ui->spinFrequency->setVisible(isWobble);
    ui->labelDecay->setVisible(isWobble);
    ui->sliderDecay->setVisible(isWobble);
    ui->spinDecay->setVisible(isWobble);

    // Show/hide the parameters group box based on whether any parameters are visible
    bool hasParams = isPower || isSine || isExpo || isCirc || isBounce || isBack || isElastic || isWobble;
    ui->groupParameters->setVisible(hasParams);
}

Easing EasingDialog::getEasing() const
{
    int easingId = ui->comboEasingType->currentData().toInt();
    Easing easing = Easing::getById(easingId);

    // Apply custom parameters based on type
    Easing::Type type = easing.getType();

    if (type == Easing::Type::QuadIn ||
        type == Easing::Type::QuadOut ||
        type == Easing::Type::QuadInOut ||
        type == Easing::Type::CubicIn ||
        type == Easing::Type::CubicOut ||
        type == Easing::Type::CubicInOut ||
        type == Easing::Type::QuarticIn ||
        type == Easing::Type::QuarticOut ||
        type == Easing::Type::QuarticInOut) {
        easing.setPower(ui->spinPower->value());
    }
    else if (type == Easing::Type::SineIn ||
             type == Easing::Type::SineOut ||
             type == Easing::Type::SineInOut) {
        easing.setSineAmplitude(ui->spinSineAmplitude->value());
        easing.setSineCycles(ui->spinSineCycles->value());
    }
    else if (type == Easing::Type::ExpoIn ||
             type == Easing::Type::ExpoOut ||
             type == Easing::Type::ExpoInOut) {
        easing.setExpoStrength(ui->spinExpoStrength->value());
    }
    else if (type == Easing::Type::CircIn ||
             type == Easing::Type::CircOut ||
             type == Easing::Type::CircInOut) {
        easing.setCircStrength(ui->spinCircStrength->value());
    }
    else if (type == Easing::Type::BounceIn ||
             type == Easing::Type::BounceOut ||
             type == Easing::Type::BounceInOut) {
        easing.setBounceCount(ui->spinBounceCount->value());
        easing.setBounceStrength(ui->spinBounceStrength->value());
    }
    else if (type == Easing::Type::BackIn ||
             type == Easing::Type::BackOut ||
             type == Easing::Type::BackInOut) {
        easing.setOvershoot(ui->spinOvershoot->value());
    }
    else if (type == Easing::Type::ElasticIn ||
             type == Easing::Type::ElasticOut ||
             type == Easing::Type::ElasticInOut) {
        easing.setAmplitude(ui->spinAmplitude->value());
        easing.setPeriod(ui->spinPeriod->value());
    }
    else if (type == Easing::Type::Wobble) {
        easing.setFrequency(ui->spinFrequency->value());
        easing.setDecay(ui->spinDecay->value());
    }

    return easing;
}

EasingDialog::AnchorMode EasingDialog::getAnchorMode() const
{
    return static_cast<AnchorMode>(ui->comboAnchorMode->currentIndex());
}

double EasingDialog::getWeight() const
{
    return ui->spinWeight->value();
}

void EasingDialog::updatePreview()
{
    // Build the current easing with parameters and update preview widget
    Easing easing = getEasing();
    previewWidget->setEasing(easing);
}
