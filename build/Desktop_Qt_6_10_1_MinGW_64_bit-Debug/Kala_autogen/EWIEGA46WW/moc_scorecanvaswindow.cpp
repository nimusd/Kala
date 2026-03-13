/****************************************************************************
** Meta object code from reading C++ file 'scorecanvaswindow.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.10.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../scorecanvaswindow.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'scorecanvaswindow.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN17ScoreCanvasWindowE_t {};
} // unnamed namespace

template <> constexpr inline auto ScoreCanvasWindow::qt_create_metaobjectdata<qt_meta_tag_ZN17ScoreCanvasWindowE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "ScoreCanvasWindow",
        "playbackStarted",
        "",
        "compositionSettingsChanged",
        "onZoomIn",
        "onZoomOut",
        "zoomToFit",
        "onZoomToggled",
        "checked",
        "onPressureChanged",
        "pressure",
        "active",
        "onCursorPositionChanged",
        "timeMs",
        "pitchHz",
        "onCompositionSettingsTriggered",
        "onAddTrackTriggered",
        "onTrackSelected",
        "trackIndex",
        "onRenderStarted",
        "onRenderProgressChanged",
        "percentage",
        "onRenderCompleted",
        "onRenderCancelled",
        "onScoreAutoScroll",
        "dx",
        "dy",
        "stopPlayback",
        "stopAudioEngine",
        "play"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'playbackStarted'
        QtMocHelpers::SignalData<void()>(1, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'compositionSettingsChanged'
        QtMocHelpers::SignalData<void()>(3, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'onZoomIn'
        QtMocHelpers::SlotData<void()>(4, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'onZoomOut'
        QtMocHelpers::SlotData<void()>(5, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'zoomToFit'
        QtMocHelpers::SlotData<void()>(6, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'onZoomToggled'
        QtMocHelpers::SlotData<void(bool)>(7, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Bool, 8 },
        }}),
        // Slot 'onPressureChanged'
        QtMocHelpers::SlotData<void(double, bool)>(9, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Double, 10 }, { QMetaType::Bool, 11 },
        }}),
        // Slot 'onCursorPositionChanged'
        QtMocHelpers::SlotData<void(double, double)>(12, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Double, 13 }, { QMetaType::Double, 14 },
        }}),
        // Slot 'onCompositionSettingsTriggered'
        QtMocHelpers::SlotData<void()>(15, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onAddTrackTriggered'
        QtMocHelpers::SlotData<void()>(16, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onTrackSelected'
        QtMocHelpers::SlotData<void(int)>(17, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 18 },
        }}),
        // Slot 'onRenderStarted'
        QtMocHelpers::SlotData<void()>(19, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onRenderProgressChanged'
        QtMocHelpers::SlotData<void(int)>(20, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 21 },
        }}),
        // Slot 'onRenderCompleted'
        QtMocHelpers::SlotData<void()>(22, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onRenderCancelled'
        QtMocHelpers::SlotData<void()>(23, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onScoreAutoScroll'
        QtMocHelpers::SlotData<void(int, int)>(24, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 25 }, { QMetaType::Int, 26 },
        }}),
        // Slot 'stopPlayback'
        QtMocHelpers::SlotData<void(bool)>(27, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 28 },
        }}),
        // Slot 'stopPlayback'
        QtMocHelpers::SlotData<void()>(27, 2, QMC::AccessPublic | QMC::MethodCloned, QMetaType::Void),
        // Slot 'play'
        QtMocHelpers::SlotData<void()>(29, 2, QMC::AccessPublic, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<ScoreCanvasWindow, qt_meta_tag_ZN17ScoreCanvasWindowE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject ScoreCanvasWindow::staticMetaObject = { {
    QMetaObject::SuperData::link<QMainWindow::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN17ScoreCanvasWindowE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN17ScoreCanvasWindowE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN17ScoreCanvasWindowE_t>.metaTypes,
    nullptr
} };

void ScoreCanvasWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<ScoreCanvasWindow *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->playbackStarted(); break;
        case 1: _t->compositionSettingsChanged(); break;
        case 2: _t->onZoomIn(); break;
        case 3: _t->onZoomOut(); break;
        case 4: _t->zoomToFit(); break;
        case 5: _t->onZoomToggled((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1]))); break;
        case 6: _t->onPressureChanged((*reinterpret_cast<std::add_pointer_t<double>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<bool>>(_a[2]))); break;
        case 7: _t->onCursorPositionChanged((*reinterpret_cast<std::add_pointer_t<double>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<double>>(_a[2]))); break;
        case 8: _t->onCompositionSettingsTriggered(); break;
        case 9: _t->onAddTrackTriggered(); break;
        case 10: _t->onTrackSelected((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 11: _t->onRenderStarted(); break;
        case 12: _t->onRenderProgressChanged((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 13: _t->onRenderCompleted(); break;
        case 14: _t->onRenderCancelled(); break;
        case 15: _t->onScoreAutoScroll((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<int>>(_a[2]))); break;
        case 16: _t->stopPlayback((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1]))); break;
        case 17: _t->stopPlayback(); break;
        case 18: _t->play(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (ScoreCanvasWindow::*)()>(_a, &ScoreCanvasWindow::playbackStarted, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (ScoreCanvasWindow::*)()>(_a, &ScoreCanvasWindow::compositionSettingsChanged, 1))
            return;
    }
}

const QMetaObject *ScoreCanvasWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ScoreCanvasWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN17ScoreCanvasWindowE_t>.strings))
        return static_cast<void*>(this);
    return QMainWindow::qt_metacast(_clname);
}

int ScoreCanvasWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 19)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 19;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 19)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 19;
    }
    return _id;
}

// SIGNAL 0
void ScoreCanvasWindow::playbackStarted()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void ScoreCanvasWindow::compositionSettingsChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}
QT_WARNING_POP
