#include "helpdialog.h"

#include <QSplitter>
#include <QTreeWidget>
#include <QTextBrowser>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QCoreApplication>
#include <QFile>
#include <QDir>
#include <QGuiApplication>
#include <QKeyEvent>
#include <QTimer>

HelpDialog::HelpDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUI();
    buildTableOfContents();
    navigateTo("introduction.html");
}

QString HelpDialog::helpPath()
{
    return QCoreApplication::applicationDirPath() + "/help";
}

void HelpDialog::setupUI()
{
    setWindowTitle("Kala Documentation");
    resize(1100, 750);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(6, 6, 6, 6);

    // Search bar
    auto *searchLayout = new QHBoxLayout;
    m_searchLine = new QLineEdit;
    m_searchLine->setPlaceholderText("Search in page...");
    m_searchLine->setClearButtonEnabled(true);
    m_searchPrev = new QPushButton("<");
    m_searchPrev->setFixedWidth(30);
    m_searchPrev->setToolTip("Find previous (Shift+Enter)");
    m_searchNext = new QPushButton(">");
    m_searchNext->setFixedWidth(30);
    m_searchNext->setToolTip("Find next (Enter)");
    searchLayout->addWidget(m_searchLine);
    searchLayout->addWidget(m_searchPrev);
    searchLayout->addWidget(m_searchNext);
    mainLayout->addLayout(searchLayout);

    // Splitter: TOC left, browser right
    m_splitter = new QSplitter(Qt::Horizontal);

    m_tocTree = new QTreeWidget;
    m_tocTree->setHeaderHidden(true);
    m_tocTree->setMinimumWidth(200);
    m_tocTree->setMaximumWidth(350);
    m_splitter->addWidget(m_tocTree);

    m_browser = new QTextBrowser;
    m_browser->setOpenLinks(false);
    m_browser->setSearchPaths({helpPath()});
    m_splitter->addWidget(m_browser);

    m_splitter->setSizes({250, 850});
    mainLayout->addWidget(m_splitter);

    // Connections
    connect(m_tocTree, &QTreeWidget::itemClicked, this, &HelpDialog::onTocItemClicked);
    connect(m_browser, &QTextBrowser::anchorClicked, this, &HelpDialog::onAnchorClicked);
    connect(m_searchNext, &QPushButton::clicked, this, &HelpDialog::onSearchNext);
    connect(m_searchPrev, &QPushButton::clicked, this, &HelpDialog::onSearchPrev);
    connect(m_searchLine, &QLineEdit::returnPressed, this, &HelpDialog::onSearchReturnPressed);
}

QTreeWidgetItem *HelpDialog::addTopLevel(const QString &label, const QString &file, const QString &anchor)
{
    auto *item = new QTreeWidgetItem(m_tocTree, {label});
    m_tocMap[item] = {file, anchor};
    return item;
}

QTreeWidgetItem *HelpDialog::addChild(QTreeWidgetItem *parent, const QString &label, const QString &file, const QString &anchor)
{
    auto *item = new QTreeWidgetItem(parent, {label});
    m_tocMap[item] = {file, anchor};
    return item;
}

QTreeWidgetItem *HelpDialog::addCategory(const QString &label)
{
    auto *item = new QTreeWidgetItem(m_tocTree, {label});
    item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
    QFont f = item->font(0);
    f.setBold(true);
    f.setItalic(true);
    item->setFont(0, f);
    return item;
}

void HelpDialog::buildTableOfContents()
{
    // Introduction
    addTopLevel("Introduction", "introduction.html");

    // Keyboard shortcuts — quick reference
    addTopLevel("Keyboard Shortcuts", "keyboard-shortcuts.html");

    // Score Canvas
    auto *score = addTopLevel("The Score Canvas", "score-canvas.html");
    addChild(score, "Frequency Staff", "score-canvas.html", "the-frequency-staff-color-coded-scale-lines");
    addChild(score, "Modulations", "score-canvas.html", "modulations-when-the-staff-transforms");
    addChild(score, "Drawing Notes", "score-canvas.html", "drawing-notes-gesture-as-notation");
    addChild(score, "The Note Blob", "score-canvas.html", "the-note-blob-visualizing-expression");
    addChild(score, "Curves", "score-canvas.html", "the-curve-system-layers-of-expression");
    addChild(score, "Expressive Curves Inspector", "score-canvas.html", "expressive-curves-inspector");
    addChild(score, "Vibrato", "score-canvas.html", "vibrato-a-parameter-not-an-ornament");
    addChild(score, "Segments", "score-canvas.html", "segments-subdividing-continuous-notes");
    addChild(score, "Time and Navigation", "score-canvas.html", "time-and-navigation");
    addChild(score, "Canvas Interactions", "score-canvas.html", "canvas-interactions");
    addChild(score, "Right-Click: Notes", "score-canvas.html", "right-click-notes");
    addChild(score, "Right-Click: Segments", "score-canvas.html", "right-click-segments");

    // Sound Engine Overview
    auto *engine = addTopLevel("Sound Engine Overview", "sound-engine-overview.html");
    addChild(engine, "What is a Sounit?", "sound-engine-overview.html", "what-is-a-sounit");
    addChild(engine, "Container System", "sound-engine-overview.html", "the-container-system");
    addChild(engine, "  Essential (Blue)", "sound-engine-overview.html", "essential-blue");
    addChild(engine, "  Shaping (Orange)", "sound-engine-overview.html", "shaping-orange");
    addChild(engine, "  Modifiers (Green)", "sound-engine-overview.html", "modifiers-green");
    addChild(engine, "  Filters (Purple)", "sound-engine-overview.html", "filters-purple");
    addChild(engine, "Connections", "sound-engine-overview.html", "the-connection-system");
    addChild(engine, "Rendering Pipeline", "sound-engine-overview.html", "audio-rendering-pipeline");
    addChild(engine, "Synthesis Approaches", "sound-engine-overview.html", "synthesis-approaches");

    // Essential (Blue)
    auto *essential = addCategory("Containers: Essential (Blue)");
    addChild(essential, "Harmonic Generator", "harmonic-generator.html", "");
    addChild(essential, "Spectrum to Signal", "spectrum-to-signal.html", "");
    addChild(essential, "Karplus Strong", "karplus-strong.html", "");
    addChild(essential, "Recorder", "recorder.html", "");
    addChild(essential, "Bowed", "bowed.html", "");
    addChild(essential, "Reed", "reed.html", "");
    addChild(essential, "Signal Mixer", "signal-mixer.html", "");
    addChild(essential, "Attack", "attack.html", "");
    addChild(essential, "Wavetable Synth", "wavetable-synth.html", "");
    addChild(essential, "Note Tail", "note-tail.html", "");

    // Shaping (Orange)
    auto *shaping = addCategory("Containers: Shaping (Orange)");
    addChild(shaping, "Rolloff Processor", "rolloff-processor.html", "");
    addChild(shaping, "Spectrum Blender", "spectrum-blender.html", "");
    addChild(shaping, "Formant Body", "formant-body.html", "");
    addChild(shaping, "Breath Turbulence", "breath-turbulence.html", "");
    addChild(shaping, "Noise Color Filter", "noise-color-filter.html", "");

    // Modifiers (Green)
    auto *modifiers = addCategory("Containers: Modifiers (Green)");
    addChild(modifiers, "Physics System", "physics-system.html", "");
    addChild(modifiers, "Easing Applicator", "easing-applicator.html", "");
    addChild(modifiers, "Envelope Engine", "envelope-engine.html", "");
    addChild(modifiers, "Drift Engine", "drift-engine.html", "");
    addChild(modifiers, "LFO", "lfo.html", "");
    addChild(modifiers, "Frequency Mapper", "frequency-mapper.html", "");

    // Filters (Purple)
    auto *filters = addCategory("Containers: Filters (Purple)");
    addChild(filters, "10-Band EQ", "ten-band-eq.html", "");
    addChild(filters, "Comb Filter", "comb-filter.html", "");
    addChild(filters, "LP/HP Filter", "lp-hp-filter.html", "");
    addChild(filters, "IR Convolution", "ir-convolution.html", "");

    // Settings
    addTopLevel("Settings", "settings.html");

    // AI Companion
    auto *companion = addTopLevel("Kala Companion (AI)", "companion.html");
    addChild(companion, "Setting Up",          "companion.html", "setting-up");
    addChild(companion, "Build Sounits",       "companion.html", "build-sounits");
    addChild(companion, "Browse the Library",  "companion.html", "browse-the-library");
    addChild(companion, "Write Music",         "companion.html", "write-music");
    addChild(companion, "Shape Phrases",       "companion.html", "shape-phrases");
    addChild(companion, "Tips",                "companion.html", "tips");
    addChild(companion, "Limitations",         "companion.html", "limitations");

    // Dialogs
    auto *dialogs = addTopLevel("Dialogs", "dialogs.html");
    addChild(dialogs, "New Project", "dialogs.html", "new-project-dialog");
    addChild(dialogs, "Scale Dialog", "dialogs.html", "scale-dialog");
    addChild(dialogs, "Composition Settings", "dialogs.html", "composition-settings-dialog");
    addChild(dialogs, "Go To Dialog", "dialogs.html", "go-to-dialog");
    addChild(dialogs, "Easing Dialog", "dialogs.html", "easing-dialog");
    addChild(dialogs, "Vibrato Editor", "dialogs.html", "vibrato-editor-dialog");
    addChild(dialogs, "Dynamics Curve", "dialogs.html", "dynamics-curve-dialog");
    addChild(dialogs, "DNA Editor", "dialogs.html", "dna-editor-dialog");
    addChild(dialogs, "Export Audio", "dialogs.html", "export-audio-dialog");
    addChild(dialogs, "Envelope Library", "dialogs.html", "envelope-library-dialog");

    m_tocTree->expandAll();
}

void HelpDialog::navigateTo(const QString &htmlFile, const QString &anchor)
{
    QString path = helpPath() + "/" + htmlFile;
    if (!QFile::exists(path))
        return;

    if (m_currentFile != htmlFile) {
        QFile f(path);
        if (f.open(QIODevice::ReadOnly)) {
            m_browser->setHtml(QString::fromUtf8(f.readAll()));
            f.close();
            m_currentFile = htmlFile;
        }
    }

    if (!anchor.isEmpty()) {
        m_browser->scrollToAnchor(anchor);
    } else {
        m_browser->moveCursor(QTextCursor::Start);
    }

    highlightTocItem(htmlFile, anchor);
}

void HelpDialog::navigateToContainer(const QString &containerName)
{
    auto map = containerToFileMap();
    QString file = map.value(containerName);
    if (!file.isEmpty()) {
        navigateTo(file);
    }
}

QMap<QString, QString> HelpDialog::containerToFileMap()
{
    static QMap<QString, QString> map = {
        // Essential (Blue)
        {"Harmonic Generator",  "harmonic-generator.html"},
        {"Spectrum to Signal",  "spectrum-to-signal.html"},
        {"Karplus Strong",      "karplus-strong.html"},
        {"Recorder",            "recorder.html"},
        {"Bowed",               "bowed.html"},
        {"Reed",                "reed.html"},
        {"Signal Mixer",        "signal-mixer.html"},
        {"Attack",              "attack.html"},
        {"Wavetable Synth",     "wavetable-synth.html"},
        {"Note Tail",           "note-tail.html"},
        // Shaping (Orange)
        {"Rolloff Processor",   "rolloff-processor.html"},
        {"Spectrum Blender",    "spectrum-blender.html"},
        {"Formant Body",        "formant-body.html"},
        {"Breath Turbulence",   "breath-turbulence.html"},
        {"Noise Color Filter",  "noise-color-filter.html"},
        // Modifiers (Green)
        {"Physics System",      "physics-system.html"},
        {"Easing Applicator",   "easing-applicator.html"},
        {"Envelope Engine",     "envelope-engine.html"},
        {"Drift Engine",        "drift-engine.html"},
        {"LFO",                 "lfo.html"},
        {"Frequency Mapper",    "frequency-mapper.html"},
        // Filters (Purple)
        {"10-Band EQ",          "ten-band-eq.html"},
        {"Comb Filter",         "comb-filter.html"},
        {"LP/HP Filter",        "lp-hp-filter.html"},
        {"IR Convolution",      "ir-convolution.html"},
    };
    return map;
}

void HelpDialog::highlightTocItem(const QString &file, const QString &anchor)
{
    m_tocTree->blockSignals(true);

    // Find best matching item
    QTreeWidgetItem *bestMatch = nullptr;
    for (auto it = m_tocMap.constBegin(); it != m_tocMap.constEnd(); ++it) {
        if (it.value().file == file) {
            if (it.value().anchor == anchor) {
                bestMatch = it.key();
                break;
            }
            if (!bestMatch && it.value().anchor.isEmpty()) {
                bestMatch = it.key();
            }
        }
    }

    if (bestMatch) {
        m_tocTree->setCurrentItem(bestMatch);
        m_tocTree->scrollToItem(bestMatch);
    }

    m_tocTree->blockSignals(false);
}

void HelpDialog::onTocItemClicked(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column);
    auto it = m_tocMap.find(item);
    if (it != m_tocMap.end()) {
        navigateTo(it.value().file, it.value().anchor);
    }
}

void HelpDialog::onAnchorClicked(const QUrl &url)
{
    QString href = url.toString();

    // Handle internal links to other HTML pages
    if (href.endsWith(".html") || href.contains(".html#")) {
        QString file = href;
        QString anchor;
        int hashIdx = href.indexOf('#');
        if (hashIdx >= 0) {
            file = href.left(hashIdx);
            anchor = href.mid(hashIdx + 1);
        }
        navigateTo(file, anchor);
    } else if (href.startsWith('#')) {
        // Same-page anchor
        m_browser->scrollToAnchor(href.mid(1));
    }
}

void HelpDialog::onSearchNext()
{
    QString text = m_searchLine->text();
    if (text.isEmpty())
        return;

    if (!m_browser->find(text)) {
        // Wrap around: go to top and try again
        m_browser->moveCursor(QTextCursor::Start);
        if (!m_browser->find(text)) {
            // No match - brief red tint
            m_searchLine->setStyleSheet("QLineEdit { background-color: #3a2020; }");
            QTimer::singleShot(600, this, [this]() {
                m_searchLine->setStyleSheet("");
            });
        }
    }
}

void HelpDialog::onSearchPrev()
{
    QString text = m_searchLine->text();
    if (text.isEmpty())
        return;

    if (!m_browser->find(text, QTextDocument::FindBackward)) {
        // Wrap around: go to bottom and try again
        m_browser->moveCursor(QTextCursor::End);
        if (!m_browser->find(text, QTextDocument::FindBackward)) {
            m_searchLine->setStyleSheet("QLineEdit { background-color: #3a2020; }");
            QTimer::singleShot(600, this, [this]() {
                m_searchLine->setStyleSheet("");
            });
        }
    }
}

void HelpDialog::onSearchReturnPressed()
{
    if (QGuiApplication::keyboardModifiers() & Qt::ShiftModifier) {
        onSearchPrev();
    } else {
        onSearchNext();
    }
}
