/****************************************************************************
** Meta object code from reading C++ file 'kalamain.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.10.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../kalamain.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'kalamain.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.10.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {
struct qt_meta_tag_ZN8KalaMainE_t {};
} // unnamed namespace

template <> constexpr inline auto KalaMain::qt_create_metaobjectdata<qt_meta_tag_ZN8KalaMainE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "KalaMain",
        "stopAllPlayback",
        "",
        "toggleCanvasFocus",
        "loadProjectFile",
        "filePath",
        "saveProjectFile",
        "getProjectFilePath",
        "onTabChanged",
        "index",
        "onNewProject",
        "onOpenProject",
        "onSaveProject",
        "onSaveProjectAs",
        "onExportAudio",
        "onNewSounit",
        "onLoadSounit",
        "onSaveSounitAs",
        "onSounitNameChanged",
        "name",
        "onSounitCommentChanged",
        "comment",
        "onSounitSelectorChanged",
        "onSounitNameEdited",
        "onSounitCommentEdited",
        "onAddTrack",
        "onDeleteTrack",
        "deleteTrackAtIndex",
        "trackIndex",
        "refreshVariationsList",
        "onVariationSelectionChanged",
        "onDeleteVariation",
        "onRenameVariation",
        "onVariationsTrackNameEdited",
        "onVariationSelectorChanged",
        "updateVariationSelector",
        "onScaleEditClicked",
        "onScaleAddClicked",
        "onScaleMarkerChanged",
        "markerIndex",
        "updateScaleDisplay",
        "getSelectedScaleMarkerTime",
        "onTempoEditClicked",
        "onTempoAddClicked",
        "onTempoMarkerChanged",
        "updateTempoDisplay",
        "getSelectedTempoMarkerTime",
        "onContainerSelected",
        "Container*",
        "container",
        "onConnectionSelected",
        "connectionIndex",
        "onSelectionCleared",
        "onInstanceNameChanged",
        "onConnectionFunctionChanged",
        "onConnectionWeightChanged",
        "value",
        "onDisconnectClicked",
        "onConfigParameterChanged",
        "QListWidgetItem*",
        "item",
        "onDnaSelectChanged",
        "onNumHarmonicsChanged",
        "onNoteSelectionChanged",
        "onNoteStartChanged",
        "onNoteDurationChanged",
        "onNotePitchChanged",
        "onNoteVariationChanged",
        "onNoteVibratoToggled",
        "checked",
        "onNoteVibratoEditClicked",
        "onExpressiveCurveChanged",
        "onExpressiveCurveAdd",
        "onExpressiveCurveDelete",
        "onExpressiveCurveApply",
        "updateExpressiveCurvesDropdown",
        "updateSounitNameDisplay",
        "populateSettingsTab",
        "populateHarmonicGeneratorSettings",
        "QFormLayout*",
        "layout",
        "populateRolloffProcessorSettings",
        "populateFormantBodySettings",
        "populateNoiseColorFilterSettings",
        "populateBandpassEQSettings",
        "populateCombFilterSettings",
        "populateLowHighPassFilterSettings",
        "populatePhysicsSystemSettings",
        "populateDriftEngineSettings",
        "populateGateProcessorSettings",
        "populateKarplusStrongSettings",
        "populateAttackSettings",
        "populateLFOSettings",
        "populateWavetableSynthSettings",
        "populateFrequencyMapperSettings",
        "populateEasingSettings",
        "onResetSettingsToDefaults",
        "onSaveContainerSettings",
        "onLoadContainerSettings",
        "onKeyboardShortcuts",
        "onDocumentation"
    };

    QtMocHelpers::UintData qt_methods {
        // Slot 'stopAllPlayback'
        QtMocHelpers::SlotData<void()>(1, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'toggleCanvasFocus'
        QtMocHelpers::SlotData<void()>(3, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'loadProjectFile'
        QtMocHelpers::SlotData<bool(const QString &)>(4, 2, QMC::AccessPublic, QMetaType::Bool, {{
            { QMetaType::QString, 5 },
        }}),
        // Slot 'saveProjectFile'
        QtMocHelpers::SlotData<bool(const QString &)>(6, 2, QMC::AccessPublic, QMetaType::Bool, {{
            { QMetaType::QString, 5 },
        }}),
        // Slot 'getProjectFilePath'
        QtMocHelpers::SlotData<QString() const>(7, 2, QMC::AccessPublic, QMetaType::QString),
        // Slot 'onTabChanged'
        QtMocHelpers::SlotData<void(int)>(8, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 9 },
        }}),
        // Slot 'onNewProject'
        QtMocHelpers::SlotData<void()>(10, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onOpenProject'
        QtMocHelpers::SlotData<void()>(11, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onSaveProject'
        QtMocHelpers::SlotData<void()>(12, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onSaveProjectAs'
        QtMocHelpers::SlotData<void()>(13, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onExportAudio'
        QtMocHelpers::SlotData<void()>(14, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onNewSounit'
        QtMocHelpers::SlotData<void()>(15, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onLoadSounit'
        QtMocHelpers::SlotData<void()>(16, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onSaveSounitAs'
        QtMocHelpers::SlotData<void()>(17, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onSounitNameChanged'
        QtMocHelpers::SlotData<void(const QString &)>(18, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 19 },
        }}),
        // Slot 'onSounitCommentChanged'
        QtMocHelpers::SlotData<void(const QString &)>(20, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 21 },
        }}),
        // Slot 'onSounitSelectorChanged'
        QtMocHelpers::SlotData<void(int)>(22, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 9 },
        }}),
        // Slot 'onSounitNameEdited'
        QtMocHelpers::SlotData<void()>(23, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onSounitCommentEdited'
        QtMocHelpers::SlotData<void()>(24, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onAddTrack'
        QtMocHelpers::SlotData<void()>(25, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onDeleteTrack'
        QtMocHelpers::SlotData<void()>(26, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'deleteTrackAtIndex'
        QtMocHelpers::SlotData<void(int)>(27, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 28 },
        }}),
        // Slot 'refreshVariationsList'
        QtMocHelpers::SlotData<void()>(29, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onVariationSelectionChanged'
        QtMocHelpers::SlotData<void()>(30, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onDeleteVariation'
        QtMocHelpers::SlotData<void()>(31, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onRenameVariation'
        QtMocHelpers::SlotData<void()>(32, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onVariationsTrackNameEdited'
        QtMocHelpers::SlotData<void()>(33, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onVariationSelectorChanged'
        QtMocHelpers::SlotData<void(int)>(34, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 9 },
        }}),
        // Slot 'updateVariationSelector'
        QtMocHelpers::SlotData<void()>(35, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onScaleEditClicked'
        QtMocHelpers::SlotData<void()>(36, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onScaleAddClicked'
        QtMocHelpers::SlotData<void()>(37, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onScaleMarkerChanged'
        QtMocHelpers::SlotData<void(int)>(38, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 39 },
        }}),
        // Slot 'updateScaleDisplay'
        QtMocHelpers::SlotData<void()>(40, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'getSelectedScaleMarkerTime'
        QtMocHelpers::SlotData<double() const>(41, 2, QMC::AccessPrivate, QMetaType::Double),
        // Slot 'onTempoEditClicked'
        QtMocHelpers::SlotData<void()>(42, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onTempoAddClicked'
        QtMocHelpers::SlotData<void()>(43, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onTempoMarkerChanged'
        QtMocHelpers::SlotData<void(int)>(44, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 39 },
        }}),
        // Slot 'updateTempoDisplay'
        QtMocHelpers::SlotData<void()>(45, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'getSelectedTempoMarkerTime'
        QtMocHelpers::SlotData<double() const>(46, 2, QMC::AccessPrivate, QMetaType::Double),
        // Slot 'onContainerSelected'
        QtMocHelpers::SlotData<void(Container *)>(47, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 48, 49 },
        }}),
        // Slot 'onConnectionSelected'
        QtMocHelpers::SlotData<void(int)>(50, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 51 },
        }}),
        // Slot 'onSelectionCleared'
        QtMocHelpers::SlotData<void()>(52, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onInstanceNameChanged'
        QtMocHelpers::SlotData<void()>(53, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onConnectionFunctionChanged'
        QtMocHelpers::SlotData<void(int)>(54, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 9 },
        }}),
        // Slot 'onConnectionWeightChanged'
        QtMocHelpers::SlotData<void(double)>(55, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Double, 56 },
        }}),
        // Slot 'onDisconnectClicked'
        QtMocHelpers::SlotData<void()>(57, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onConfigParameterChanged'
        QtMocHelpers::SlotData<void(QListWidgetItem *)>(58, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 59, 60 },
        }}),
        // Slot 'onDnaSelectChanged'
        QtMocHelpers::SlotData<void(int)>(61, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 9 },
        }}),
        // Slot 'onNumHarmonicsChanged'
        QtMocHelpers::SlotData<void(int)>(62, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 56 },
        }}),
        // Slot 'onNoteSelectionChanged'
        QtMocHelpers::SlotData<void()>(63, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onNoteStartChanged'
        QtMocHelpers::SlotData<void(double)>(64, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Double, 56 },
        }}),
        // Slot 'onNoteDurationChanged'
        QtMocHelpers::SlotData<void(double)>(65, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Double, 56 },
        }}),
        // Slot 'onNotePitchChanged'
        QtMocHelpers::SlotData<void(int)>(66, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 56 },
        }}),
        // Slot 'onNoteVariationChanged'
        QtMocHelpers::SlotData<void(int)>(67, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 9 },
        }}),
        // Slot 'onNoteVibratoToggled'
        QtMocHelpers::SlotData<void(bool)>(68, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Bool, 69 },
        }}),
        // Slot 'onNoteVibratoEditClicked'
        QtMocHelpers::SlotData<void()>(70, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onExpressiveCurveChanged'
        QtMocHelpers::SlotData<void(int)>(71, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 9 },
        }}),
        // Slot 'onExpressiveCurveAdd'
        QtMocHelpers::SlotData<void()>(72, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onExpressiveCurveDelete'
        QtMocHelpers::SlotData<void()>(73, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onExpressiveCurveApply'
        QtMocHelpers::SlotData<void()>(74, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'updateExpressiveCurvesDropdown'
        QtMocHelpers::SlotData<void()>(75, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'updateSounitNameDisplay'
        QtMocHelpers::SlotData<void(const QString &)>(76, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 19 },
        }}),
        // Slot 'populateSettingsTab'
        QtMocHelpers::SlotData<void()>(77, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'populateHarmonicGeneratorSettings'
        QtMocHelpers::SlotData<void(QFormLayout *)>(78, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 79, 80 },
        }}),
        // Slot 'populateRolloffProcessorSettings'
        QtMocHelpers::SlotData<void(QFormLayout *)>(81, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 79, 80 },
        }}),
        // Slot 'populateFormantBodySettings'
        QtMocHelpers::SlotData<void(QFormLayout *)>(82, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 79, 80 },
        }}),
        // Slot 'populateNoiseColorFilterSettings'
        QtMocHelpers::SlotData<void(QFormLayout *)>(83, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 79, 80 },
        }}),
        // Slot 'populateBandpassEQSettings'
        QtMocHelpers::SlotData<void(QFormLayout *)>(84, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 79, 80 },
        }}),
        // Slot 'populateCombFilterSettings'
        QtMocHelpers::SlotData<void(QFormLayout *)>(85, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 79, 80 },
        }}),
        // Slot 'populateLowHighPassFilterSettings'
        QtMocHelpers::SlotData<void(QFormLayout *)>(86, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 79, 80 },
        }}),
        // Slot 'populatePhysicsSystemSettings'
        QtMocHelpers::SlotData<void(QFormLayout *)>(87, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 79, 80 },
        }}),
        // Slot 'populateDriftEngineSettings'
        QtMocHelpers::SlotData<void(QFormLayout *)>(88, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 79, 80 },
        }}),
        // Slot 'populateGateProcessorSettings'
        QtMocHelpers::SlotData<void(QFormLayout *)>(89, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 79, 80 },
        }}),
        // Slot 'populateKarplusStrongSettings'
        QtMocHelpers::SlotData<void(QFormLayout *)>(90, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 79, 80 },
        }}),
        // Slot 'populateAttackSettings'
        QtMocHelpers::SlotData<void(QFormLayout *)>(91, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 79, 80 },
        }}),
        // Slot 'populateLFOSettings'
        QtMocHelpers::SlotData<void(QFormLayout *)>(92, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 79, 80 },
        }}),
        // Slot 'populateWavetableSynthSettings'
        QtMocHelpers::SlotData<void(QFormLayout *)>(93, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 79, 80 },
        }}),
        // Slot 'populateFrequencyMapperSettings'
        QtMocHelpers::SlotData<void(QFormLayout *)>(94, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 79, 80 },
        }}),
        // Slot 'populateEasingSettings'
        QtMocHelpers::SlotData<void(QFormLayout *)>(95, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 79, 80 },
        }}),
        // Slot 'onResetSettingsToDefaults'
        QtMocHelpers::SlotData<void()>(96, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onSaveContainerSettings'
        QtMocHelpers::SlotData<void()>(97, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onLoadContainerSettings'
        QtMocHelpers::SlotData<void()>(98, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onKeyboardShortcuts'
        QtMocHelpers::SlotData<void()>(99, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onDocumentation'
        QtMocHelpers::SlotData<void()>(100, 2, QMC::AccessPrivate, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<KalaMain, qt_meta_tag_ZN8KalaMainE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject KalaMain::staticMetaObject = { {
    QMetaObject::SuperData::link<QMainWindow::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN8KalaMainE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN8KalaMainE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN8KalaMainE_t>.metaTypes,
    nullptr
} };

void KalaMain::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<KalaMain *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->stopAllPlayback(); break;
        case 1: _t->toggleCanvasFocus(); break;
        case 2: { bool _r = _t->loadProjectFile((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])));
            if (_a[0]) *reinterpret_cast<bool*>(_a[0]) = std::move(_r); }  break;
        case 3: { bool _r = _t->saveProjectFile((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])));
            if (_a[0]) *reinterpret_cast<bool*>(_a[0]) = std::move(_r); }  break;
        case 4: { QString _r = _t->getProjectFilePath();
            if (_a[0]) *reinterpret_cast<QString*>(_a[0]) = std::move(_r); }  break;
        case 5: _t->onTabChanged((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 6: _t->onNewProject(); break;
        case 7: _t->onOpenProject(); break;
        case 8: _t->onSaveProject(); break;
        case 9: _t->onSaveProjectAs(); break;
        case 10: _t->onExportAudio(); break;
        case 11: _t->onNewSounit(); break;
        case 12: _t->onLoadSounit(); break;
        case 13: _t->onSaveSounitAs(); break;
        case 14: _t->onSounitNameChanged((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 15: _t->onSounitCommentChanged((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 16: _t->onSounitSelectorChanged((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 17: _t->onSounitNameEdited(); break;
        case 18: _t->onSounitCommentEdited(); break;
        case 19: _t->onAddTrack(); break;
        case 20: _t->onDeleteTrack(); break;
        case 21: _t->deleteTrackAtIndex((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 22: _t->refreshVariationsList(); break;
        case 23: _t->onVariationSelectionChanged(); break;
        case 24: _t->onDeleteVariation(); break;
        case 25: _t->onRenameVariation(); break;
        case 26: _t->onVariationsTrackNameEdited(); break;
        case 27: _t->onVariationSelectorChanged((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 28: _t->updateVariationSelector(); break;
        case 29: _t->onScaleEditClicked(); break;
        case 30: _t->onScaleAddClicked(); break;
        case 31: _t->onScaleMarkerChanged((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 32: _t->updateScaleDisplay(); break;
        case 33: { double _r = _t->getSelectedScaleMarkerTime();
            if (_a[0]) *reinterpret_cast<double*>(_a[0]) = std::move(_r); }  break;
        case 34: _t->onTempoEditClicked(); break;
        case 35: _t->onTempoAddClicked(); break;
        case 36: _t->onTempoMarkerChanged((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 37: _t->updateTempoDisplay(); break;
        case 38: { double _r = _t->getSelectedTempoMarkerTime();
            if (_a[0]) *reinterpret_cast<double*>(_a[0]) = std::move(_r); }  break;
        case 39: _t->onContainerSelected((*reinterpret_cast<std::add_pointer_t<Container*>>(_a[1]))); break;
        case 40: _t->onConnectionSelected((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 41: _t->onSelectionCleared(); break;
        case 42: _t->onInstanceNameChanged(); break;
        case 43: _t->onConnectionFunctionChanged((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 44: _t->onConnectionWeightChanged((*reinterpret_cast<std::add_pointer_t<double>>(_a[1]))); break;
        case 45: _t->onDisconnectClicked(); break;
        case 46: _t->onConfigParameterChanged((*reinterpret_cast<std::add_pointer_t<QListWidgetItem*>>(_a[1]))); break;
        case 47: _t->onDnaSelectChanged((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 48: _t->onNumHarmonicsChanged((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 49: _t->onNoteSelectionChanged(); break;
        case 50: _t->onNoteStartChanged((*reinterpret_cast<std::add_pointer_t<double>>(_a[1]))); break;
        case 51: _t->onNoteDurationChanged((*reinterpret_cast<std::add_pointer_t<double>>(_a[1]))); break;
        case 52: _t->onNotePitchChanged((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 53: _t->onNoteVariationChanged((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 54: _t->onNoteVibratoToggled((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1]))); break;
        case 55: _t->onNoteVibratoEditClicked(); break;
        case 56: _t->onExpressiveCurveChanged((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 57: _t->onExpressiveCurveAdd(); break;
        case 58: _t->onExpressiveCurveDelete(); break;
        case 59: _t->onExpressiveCurveApply(); break;
        case 60: _t->updateExpressiveCurvesDropdown(); break;
        case 61: _t->updateSounitNameDisplay((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 62: _t->populateSettingsTab(); break;
        case 63: _t->populateHarmonicGeneratorSettings((*reinterpret_cast<std::add_pointer_t<QFormLayout*>>(_a[1]))); break;
        case 64: _t->populateRolloffProcessorSettings((*reinterpret_cast<std::add_pointer_t<QFormLayout*>>(_a[1]))); break;
        case 65: _t->populateFormantBodySettings((*reinterpret_cast<std::add_pointer_t<QFormLayout*>>(_a[1]))); break;
        case 66: _t->populateNoiseColorFilterSettings((*reinterpret_cast<std::add_pointer_t<QFormLayout*>>(_a[1]))); break;
        case 67: _t->populateBandpassEQSettings((*reinterpret_cast<std::add_pointer_t<QFormLayout*>>(_a[1]))); break;
        case 68: _t->populateCombFilterSettings((*reinterpret_cast<std::add_pointer_t<QFormLayout*>>(_a[1]))); break;
        case 69: _t->populateLowHighPassFilterSettings((*reinterpret_cast<std::add_pointer_t<QFormLayout*>>(_a[1]))); break;
        case 70: _t->populatePhysicsSystemSettings((*reinterpret_cast<std::add_pointer_t<QFormLayout*>>(_a[1]))); break;
        case 71: _t->populateDriftEngineSettings((*reinterpret_cast<std::add_pointer_t<QFormLayout*>>(_a[1]))); break;
        case 72: _t->populateGateProcessorSettings((*reinterpret_cast<std::add_pointer_t<QFormLayout*>>(_a[1]))); break;
        case 73: _t->populateKarplusStrongSettings((*reinterpret_cast<std::add_pointer_t<QFormLayout*>>(_a[1]))); break;
        case 74: _t->populateAttackSettings((*reinterpret_cast<std::add_pointer_t<QFormLayout*>>(_a[1]))); break;
        case 75: _t->populateLFOSettings((*reinterpret_cast<std::add_pointer_t<QFormLayout*>>(_a[1]))); break;
        case 76: _t->populateWavetableSynthSettings((*reinterpret_cast<std::add_pointer_t<QFormLayout*>>(_a[1]))); break;
        case 77: _t->populateFrequencyMapperSettings((*reinterpret_cast<std::add_pointer_t<QFormLayout*>>(_a[1]))); break;
        case 78: _t->populateEasingSettings((*reinterpret_cast<std::add_pointer_t<QFormLayout*>>(_a[1]))); break;
        case 79: _t->onResetSettingsToDefaults(); break;
        case 80: _t->onSaveContainerSettings(); break;
        case 81: _t->onLoadContainerSettings(); break;
        case 82: _t->onKeyboardShortcuts(); break;
        case 83: _t->onDocumentation(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
        case 39:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< Container* >(); break;
            }
            break;
        }
    }
}

const QMetaObject *KalaMain::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *KalaMain::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN8KalaMainE_t>.strings))
        return static_cast<void*>(this);
    return QMainWindow::qt_metacast(_clname);
}

int KalaMain::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 84)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 84;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 84)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 84;
    }
    return _id;
}
QT_WARNING_POP
