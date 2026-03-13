#include "scaledialog.h"
#include "ui_scaledialog.h"

// Returns an HTML description for the given scale ID.
static QString getScaleDescription(int scaleId)
{
    switch (scaleId) {

    // ---- Western ----

    case 0:
        return "<b>Just Intonation</b> &mdash; 7 notes &mdash; Western<br><br>"
               "Default scale. Pure harmonic ratios derived from the overtone series. "
               "Features pure major thirds (5/4) and perfect fifths (3/2).<br><br>"
               "<b>Notes:</b> C &middot; D &middot; E &middot; F &middot; G &middot; A &middot; B<br>"
               "<b>Ratios:</b> 1/1 &middot; 9/8 &middot; 5/4 &middot; 4/3 &middot; 3/2 &middot; 5/3 &middot; 15/8";

    case 1:
        return "<b>Pythagorean</b> &mdash; 7 notes &mdash; Western<br><br>"
               "Based on stacked perfect fifths (3/2). Major thirds are sharper (~22 cents) "
               "than Just Intonation.<br><br>"
               "<b>Notes:</b> C &middot; D &middot; E &middot; F &middot; G &middot; A &middot; B<br>"
               "<b>Ratios:</b> 1/1 &middot; 9/8 &middot; 81/64 &middot; 4/3 &middot; 3/2 &middot; 27/16 &middot; 243/128";

    case 2:
        return "<b>Equal Temperament</b> &mdash; 7 notes &mdash; Western<br><br>"
               "Modern standard tuning. Each semitone = 2^(1/12). Perfectly even spacing; "
               "compromise tuning allowing all keys to sound equally acceptable.<br><br>"
               "<b>Notes:</b> C &middot; D &middot; E &middot; F &middot; G &middot; A &middot; B<br>"
               "<b>Steps:</b> 0 &middot; 2 &middot; 4 &middot; 5 &middot; 7 &middot; 9 &middot; 11 semitones";

    case 3:
        return "<b>Quarter-Comma Meantone</b> &mdash; 7 notes &mdash; Western<br><br>"
               "Renaissance temperament. Fifths are tempered so that major thirds are pure (5/4). "
               "Historically popular in keyboard music (1500s&ndash;1700s).<br><br>"
               "<b>Notes:</b> C &middot; D &middot; E &middot; F &middot; G &middot; A &middot; B<br>"
               "<b>Fifth size:</b> 5^(1/4) &asymp; 696.6 cents (vs 702 cents pure)";

    // ---- Maqam ----

    case 4:
        return "<b>Maqam Rast</b> &mdash; 7 notes &mdash; Maqam (Arabic/Turkish)<br><br>"
               "Foundational maqam, similar to the major scale. Basis for many other maqams.<br><br>"
               "<b>Notes:</b> C &middot; D &middot; E &middot; F &middot; G &middot; A &middot; B<br>"
               "<b>Ratios:</b> 1/1 &middot; 9/8 &middot; 5/4 &middot; 4/3 &middot; 3/2 &middot; 27/16 &middot; 15/8";

    case 5:
        return "<b>Maqam Bayati</b> &mdash; 7 notes &mdash; Maqam (Arabic/Turkish)<br><br>"
               "Popular maqam featuring characteristic quarter-tone inflections "
               "on the 2nd and 6th degrees.<br><br>"
               "<b>Notes:</b> C &middot; D&#9837;+ &middot; E&#9837; &middot; F &middot; G &middot; A&#9837;+ &middot; B&#9837;<br>"
               "<b>Ratios:</b> 1/1 &middot; 16/15 &middot; 6/5 &middot; 4/3 &middot; 3/2 &middot; 8/5 &middot; 9/5";

    case 6:
        return "<b>Maqam Saba</b> &mdash; 8 notes &mdash; Maqam (Arabic/Turkish)<br><br>"
               "Complex 8-note scale containing many microtones. "
               "Expresses sadness and deep longing.<br><br>"
               "<b>Notes:</b> C &middot; D&#9837; &middot; E&#9837; &middot; E &middot; F&#9839; &middot; A&#9837; &middot; B&#9837; &middot; C&#9839;<br>"
               "<b>Ratios:</b> 1/1 &middot; 16/15 &middot; 6/5 &middot; 27/20 &middot; 45/32 &middot; 8/5 &middot; 9/5 &middot; 15/8";

    case 7:
        return "<b>Maqam Hijaz</b> &mdash; 7 notes &mdash; Maqam (Arabic/Turkish)<br><br>"
               "Dramatic maqam with a characteristic augmented 2nd between D&#9837; and E.<br><br>"
               "<b>Notes:</b> C &middot; D&#9837; &middot; E &middot; F &middot; G &middot; A&#9837; &middot; B<br>"
               "<b>Ratios:</b> 1/1 &middot; 16/15 &middot; 5/4 &middot; 4/3 &middot; 3/2 &middot; 8/5 &middot; 15/8";

    case 8:
        return "<b>Maqam Nahawand</b> &mdash; 7 notes &mdash; Maqam (Arabic/Turkish)<br><br>"
               "Often called the &ldquo;minor maqam.&rdquo; Very similar to the natural minor scale.<br><br>"
               "<b>Notes:</b> C &middot; D &middot; E&#9837; &middot; F &middot; G &middot; A&#9837; &middot; B<br>"
               "<b>Ratios:</b> 1/1 &middot; 9/8 &middot; 6/5 &middot; 4/3 &middot; 3/2 &middot; 8/5 &middot; 15/8";

    case 9:
        return "<b>Maqam Kurd</b> &mdash; 7 notes &mdash; Maqam (Arabic/Turkish)<br><br>"
               "Similar to the Phrygian mode. Melancholic character.<br><br>"
               "<b>Notes:</b> C &middot; D&#9837; &middot; E&#9837; &middot; F &middot; G &middot; A&#9837; &middot; B&#9837;<br>"
               "<b>Ratios:</b> 1/1 &middot; 16/15 &middot; 6/5 &middot; 4/3 &middot; 3/2 &middot; 8/5 &middot; 9/5";

    // ---- Indian Ragas ----

    case 10:
        return "<b>Raga Bhairav</b> &mdash; 7 notes &mdash; Indian Hindustani (morning raga)<br><br>"
               "Sharp 3rd and 7th with flat 2nd and 6th. Deep, serene early-morning character.<br><br>"
               "<b>Notes:</b> Sa &middot; Re&#9837; &middot; Ga &middot; Ma &middot; Pa &middot; Dha&#9837; &middot; Ni<br>"
               "<b>Ratios:</b> 1/1 &middot; 16/15 &middot; 5/4 &middot; 4/3 &middot; 3/2 &middot; 8/5 &middot; 15/8";

    case 11:
        return "<b>Raga Yaman</b> &mdash; 7 notes &mdash; Indian Hindustani (evening raga)<br><br>"
               "All natural notes except a sharp 4th (Ma&#9839;). "
               "Very popular and accessible raga.<br><br>"
               "<b>Notes:</b> Sa &middot; Re &middot; Ga &middot; Ma&#9839; &middot; Pa &middot; Dha &middot; Ni<br>"
               "<b>Ratios:</b> 1/1 &middot; 9/8 &middot; 5/4 &middot; 45/32 &middot; 3/2 &middot; 5/3 &middot; 15/8";

    case 12:
        return "<b>Raga Todi</b> &mdash; 7 notes &mdash; Indian Hindustani (afternoon raga)<br><br>"
               "Distinctive sharp 4th with many flat notes. Expresses longing and devotion.<br><br>"
               "<b>Notes:</b> Sa &middot; Re&#9837; &middot; Ga&#9837; &middot; Ma&#9839; &middot; Pa &middot; Dha&#9837; &middot; Ni<br>"
               "<b>Ratios:</b> 1/1 &middot; 16/15 &middot; 6/5 &middot; 45/32 &middot; 3/2 &middot; 8/5 &middot; 15/8";

    case 13:
        return "<b>Raga Bhairavi</b> &mdash; 7 notes &mdash; Indian Hindustani (flexible, anytime)<br><br>"
               "All flat notes, very flexible raga. Often used to conclude concerts.<br><br>"
               "<b>Notes:</b> Sa &middot; Re&#9837; &middot; Ga&#9837; &middot; Ma &middot; Pa &middot; Dha&#9837; &middot; Ni&#9837;<br>"
               "<b>Ratios:</b> 1/1 &middot; 16/15 &middot; 6/5 &middot; 4/3 &middot; 3/2 &middot; 8/5 &middot; 9/5";

    case 14:
        return "<b>Raga Shankarabharana</b> &mdash; 7 notes &mdash; Indian Carnatic (29th Melakarta)<br><br>"
               "All shuddha (natural) swaras. Equivalent to the Bilawal/major scale. "
               "Foundation for many other ragas.<br><br>"
               "<b>Notes:</b> Sa &middot; Re &middot; Ga &middot; Ma &middot; Pa &middot; Dha &middot; Ni<br>"
               "<b>Ratios:</b> 1/1 &middot; 9/8 &middot; 5/4 &middot; 4/3 &middot; 3/2 &middot; 5/3 &middot; 15/8";

    case 15:
        return "<b>Raga Mayamalavagowla</b> &mdash; 7 notes &mdash; Indian Carnatic (15th Melakarta)<br><br>"
               "Important morning raga. Same interval structure as Bhairav, "
               "from the Carnatic tradition.<br><br>"
               "<b>Notes:</b> Sa &middot; Re&#9837; &middot; Ga &middot; Ma &middot; Pa &middot; Dha&#9837; &middot; Ni<br>"
               "<b>Ratios:</b> 1/1 &middot; 16/15 &middot; 5/4 &middot; 4/3 &middot; 3/2 &middot; 8/5 &middot; 15/8";

    // ---- Indonesian ----

    case 16:
        return "<b>Pelog</b> &mdash; 7 notes &mdash; Indonesian (Javanese gamelan)<br><br>"
               "7-note gamelan scale with strongly unequal intervals. "
               "Many regional tuning variations exist.<br><br>"
               "<b>Notes:</b> 1 Panunggul &middot; 2 Gulu &middot; 3 Dada &middot; 4 Pelog "
               "&middot; 5 Lima &middot; 6 Nem &middot; 7 Barang<br>"
               "<b>Ratios:</b> 1/1 &middot; 256/243 &middot; 32/27 &middot; 4/3 "
               "&middot; 1024/729 &middot; 128/81 &middot; 16/9";

    case 17:
        return "<b>Slendro</b> &mdash; 5 notes &mdash; Indonesian (Javanese gamelan)<br><br>"
               "5-note gamelan scale with roughly equal ~240 cent intervals. "
               "Older than Pelog; used in traditional gamelan music.<br><br>"
               "<b>Notes:</b> 1 Panunggul &middot; 2 Gulu &middot; 3 Dada &middot; 5 Lima &middot; 6 Nem<br>"
               "<b>Steps:</b> ~240 cents each &mdash; 2^(n/5)";

    // ---- Chinese ----

    case 18:
        return "<b>Chinese Pentatonic</b> &mdash; 5 notes &mdash; Chinese<br><br>"
               "Traditional Gong mode pentatonic. Similar to the major pentatonic scale. "
               "Foundation of Chinese classical music.<br><br>"
               "<b>Notes:</b> Gong &middot; Shang &middot; Jue &middot; Zhi &middot; Yu<br>"
               "<b>Ratios:</b> 1/1 &middot; 9/8 &middot; 5/4 &middot; 3/2 &middot; 5/3";

    // ---- Japanese ----

    case 19:
        return "<b>Hirajoshi</b> &mdash; 5 notes &mdash; Japanese<br><br>"
               "Traditional Japanese 5-note scale. "
               "Used widely in traditional Japanese music and modern compositions.<br><br>"
               "<b>Notes:</b> C &middot; D &middot; E&#9837; &middot; G &middot; A&#9837;<br>"
               "<b>Ratios:</b> 1/1 &middot; 9/8 &middot; 6/5 &middot; 3/2 &middot; 8/5";

    // ---- Western (additional) ----

    case 20:
        return "<b>Whole Tone</b> &mdash; 6 notes &mdash; Western<br><br>"
               "Perfectly symmetric: every interval is a whole step (200 cents). "
               "No leading tone, no perfect fifth. "
               "Associated with dreamlike and ambiguous harmony (Debussy, impressionism).<br><br>"
               "<b>Notes:</b> C &middot; D &middot; E &middot; F&#9839; &middot; G&#9839; &middot; A&#9839;<br>"
               "<b>Steps:</b> 200 cents each &mdash; 2^(n&middot;2/12)";

    case 21:
        return "<b>Half Diminished</b> &mdash; 7 notes &mdash; Western (Locrian mode)<br><br>"
               "Built on the 7th degree of the major scale. "
               "Fits the m7&#9837;5 (half-diminished) chord. "
               "Features a diminished 5th above the tonic.<br><br>"
               "<b>Notes:</b> C &middot; D&#9837; &middot; E&#9837; &middot; F &middot; G&#9837; &middot; A&#9837; &middot; B&#9837;<br>"
               "<b>Ratios:</b> 1/1 &middot; 16/15 &middot; 6/5 &middot; 4/3 &middot; tritone &middot; 8/5 &middot; 9/5";

    // ---- Indian Ragas (additional) ----

    case 22:
        return "<b>Raga Kafi</b> &mdash; 7 notes &mdash; Indian Hindustani (Kafi thaat)<br><br>"
               "Dorian-like scale associated with spring, rain, and playful emotion. "
               "Light and nimble character.<br><br>"
               "<b>Notes:</b> Sa &middot; Re &middot; Ga&#9837; &middot; Ma &middot; Pa &middot; Dha &middot; Ni&#9837;<br>"
               "<b>Ratios:</b> 1/1 &middot; 9/8 &middot; 6/5 &middot; 4/3 &middot; 3/2 &middot; 5/3 &middot; 9/5";

    case 23:
        return "<b>Raga Asavari</b> &mdash; 7 notes &mdash; Indian Hindustani (morning raga)<br><br>"
               "Associated with pathos and yearning. "
               "All notes flat except Re. Morning raga of serene melancholy.<br><br>"
               "<b>Notes:</b> Sa &middot; Re &middot; Ga&#9837; &middot; Ma &middot; Pa &middot; Dha&#9837; &middot; Ni&#9837;<br>"
               "<b>Ratios:</b> 1/1 &middot; 9/8 &middot; 6/5 &middot; 4/3 &middot; 3/2 &middot; 8/5 &middot; 9/5";

    case 24:
        return "<b>Raga Khamaj</b> &mdash; 7 notes &mdash; Indian Hindustani (evening raga)<br><br>"
               "Mixolydian-like character. Romantic and playful. "
               "Uses flat 7th (Ni&#9837;). Evening raga.<br><br>"
               "<b>Notes:</b> Sa &middot; Re &middot; Ga &middot; Ma &middot; Pa &middot; Dha &middot; Ni&#9837;<br>"
               "<b>Ratios:</b> 1/1 &middot; 9/8 &middot; 5/4 &middot; 4/3 &middot; 3/2 &middot; 5/3 &middot; 9/5";

    case 25:
        return "<b>Raga Marwa</b> &mdash; 7 notes &mdash; Indian Hindustani (late afternoon raga)<br><br>"
               "Flat 2nd (Re&#9837;) and sharp 4th (Ma&#9839;) create distinctive tension. "
               "Contemplative, serious character. Late afternoon raga.<br><br>"
               "<b>Notes:</b> Sa &middot; Re&#9837; &middot; Ga &middot; Ma&#9839; &middot; Pa &middot; Dha &middot; Ni<br>"
               "<b>Ratios:</b> 1/1 &middot; 16/15 &middot; 5/4 &middot; 45/32 &middot; 3/2 &middot; 5/3 &middot; 15/8";

    case 26:
        return "<b>Raga Purvi</b> &mdash; 7 notes &mdash; Indian Hindustani (late evening raga)<br><br>"
               "Flat 2nd, sharp 4th, and flat 6th combine for a grave, devotional atmosphere. "
               "Late evening raga.<br><br>"
               "<b>Notes:</b> Sa &middot; Re&#9837; &middot; Ga &middot; Ma&#9839; &middot; Pa &middot; Dha&#9837; &middot; Ni<br>"
               "<b>Ratios:</b> 1/1 &middot; 16/15 &middot; 5/4 &middot; 45/32 &middot; 3/2 &middot; 8/5 &middot; 15/8";

    // ---- Maqam (additional) ----

    case 27:
        return "<b>Maqam Sikah</b> &mdash; 7 notes &mdash; Maqam (Arabic/Turkish)<br><br>"
               "Built on a neutral 3rd. Uses 3/4-tone (~150 cent) steps; "
               "12/11 ratio approximates the characteristic microtonal interval. "
               "Expressive, melancholic character.<br><br>"
               "<b>Notes:</b> C &middot; D&#9837;+ &middot; E&#9837; &middot; F &middot; G &middot; A&#9837; &middot; B&#9837;<br>"
               "<b>Ratios:</b> 1/1 &middot; 12/11 &middot; 6/5 &middot; 4/3 &middot; 3/2 &middot; 8/5 &middot; 9/5";

    case 28:
        return "<b>Maqam Ajam</b> &mdash; 7 notes &mdash; Maqam (Arabic/Turkish)<br><br>"
               "Major tetrachord character with a minor 7th (B&#9837;). "
               "Bright, joyful sound. Common in Egyptian and Arab classical music.<br><br>"
               "<b>Notes:</b> C &middot; D &middot; E &middot; F &middot; G &middot; A &middot; B&#9837;<br>"
               "<b>Ratios:</b> 1/1 &middot; 9/8 &middot; 5/4 &middot; 4/3 &middot; 3/2 &middot; 5/3 &middot; 9/5";

    case 29:
        return "<b>Maqam Athar Kurd</b> &mdash; 7 notes &mdash; Maqam (Arabic/Turkish)<br><br>"
               "Kurd lower tetrachord (C D&#9837; E&#9837; F) + Athar upper tetrachord (G A&#9837; B C). "
               "Defining augmented 2nd between A&#9837; and B. Dark, mysterious character.<br><br>"
               "<b>Notes:</b> C &middot; D&#9837; &middot; E&#9837; &middot; F &middot; G &middot; A&#9837; &middot; B<br>"
               "<b>Ratios:</b> 1/1 &middot; 16/15 &middot; 6/5 &middot; 4/3 &middot; 3/2 &middot; 8/5 &middot; 15/8";

    // ---- Persian Dastgah ----

    case 30:
        return "<b>Persian Shur</b> &mdash; 7 notes &mdash; Persian Dastgah<br><br>"
               "Most fundamental Persian dastgah. Features koron (&#9837;k, ~50 cents below natural) "
               "on the 2nd and 6th degrees (~135 and ~835 cents). "
               "Melancholic and introspective character.<br><br>"
               "<b>Notes:</b> C &middot; D&#9837;k &middot; E&#9837; &middot; F &middot; G &middot; A&#9837;k &middot; B&#9837;<br>"
               "<b>Koron degrees:</b> ~135 cents (2nd), ~835 cents (6th)";

    case 31:
        return "<b>Persian Mahur</b> &mdash; 7 notes &mdash; Persian Dastgah<br><br>"
               "Persian equivalent of the major scale. "
               "Joyful, bright character with JI-based tuning.<br><br>"
               "<b>Notes:</b> C &middot; D &middot; E &middot; F &middot; G &middot; A &middot; B<br>"
               "<b>Ratios:</b> 1/1 &middot; 9/8 &middot; 5/4 &middot; 4/3 &middot; 3/2 &middot; 5/3 &middot; 15/8";

    case 32:
        return "<b>Persian Segah</b> &mdash; 7 notes &mdash; Persian Dastgah<br><br>"
               "&ldquo;Third position&rdquo; dastgah. Uses neutral (3/4-flat) 3rd (~350 cents) "
               "and neutral 7th (~1050 cents). Contemplative and devotional character.<br><br>"
               "<b>Notes:</b> C &middot; D &middot; E&#9837;+ &middot; F &middot; G &middot; A &middot; B&#9837;+<br>"
               "<b>Neutral degrees:</b> E&#9837;+ &asymp; 350&cent;, B&#9837;+ &asymp; 1050&cent;";

    case 33:
        return "<b>Persian Chahargah</b> &mdash; 7 notes &mdash; Persian Dastgah<br><br>"
               "&ldquo;Fourth position&rdquo; dastgah. Characteristic augmented 2nd between E&#9837;+ "
               "and F&#9839;. Powerful and dramatic character.<br><br>"
               "<b>Notes:</b> C &middot; D &middot; E&#9837;+ &middot; F&#9839; &middot; G &middot; A+ &middot; B&#9837;+<br>"
               "<b>Microtones:</b> E&#9837;+ &asymp; 342&cent;, F&#9839; &asymp; 567&cent;, A+ &asymp; 906&cent;, B&#9837;+ &asymp; 1044&cent;";

    case 34:
        return "<b>Persian Homayun</b> &mdash; 7 notes &mdash; Persian Dastgah<br><br>"
               "Hijaz-like lower tetrachord: augmented 2nd from D&#9837; to E. "
               "Sorrowful and expressive. Similar to Maqam Hijaz but with minor 7th (B&#9837;).<br><br>"
               "<b>Notes:</b> C &middot; D&#9837; &middot; E &middot; F &middot; G &middot; A&#9837; &middot; B&#9837;<br>"
               "<b>Ratios:</b> 1/1 &middot; 16/15 &middot; 5/4 &middot; 4/3 &middot; 3/2 &middot; 8/5 &middot; 9/5";

    default:
        return "";
    }
}

// ============================================================================

ScaleDialog::ScaleDialog(const Scale &currentScale, double currentBaseFreq, bool isAtTimeZero, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ScaleDialog)
{
    ui->setupUi(this);

    // Populate scale dropdown with all available scales
    QVector<Scale> scales = Scale::getAllScales();
    for (const Scale &scale : scales) {
        ui->comboScale->addItem(scale.getName(), scale.getScaleId());
    }

    // Set current scale as selected
    int currentIndex = ui->comboScale->findData(currentScale.getScaleId());
    if (currentIndex >= 0) {
        ui->comboScale->setCurrentIndex(currentIndex);
    }

    // Set current base frequency
    ui->spinBaseFreq->setValue(currentBaseFreq);

    // Configure delete button
    ui->btnDelete->setEnabled(!isAtTimeZero);
    connect(ui->btnDelete, &QPushButton::clicked, this, &ScaleDialog::onDeleteClicked);

    // Wire description panel
    connect(ui->comboScale, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ScaleDialog::onScaleChanged);
    onScaleChanged(ui->comboScale->currentIndex());
}

ScaleDialog::~ScaleDialog()
{
    delete ui;
}

Scale ScaleDialog::getSelectedScale() const
{
    int scaleId = ui->comboScale->currentData().toInt();
    return Scale::getScaleById(scaleId);
}

double ScaleDialog::getBaseFrequency() const
{
    return ui->spinBaseFreq->value();
}

void ScaleDialog::onDeleteClicked()
{
    deleteRequested = true;
    accept();
}

void ScaleDialog::onScaleChanged(int index)
{
    if (index < 0) return;
    int scaleId = ui->comboScale->itemData(index).toInt();
    ui->textScaleInfo->setHtml(getScaleDescription(scaleId));
}
