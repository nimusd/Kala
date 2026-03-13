/****************************************************************************
** Meta object code from reading C++ file 'scorecanvas.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.10.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../scorecanvas.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'scorecanvas.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN11ScoreCanvasE_t {};
} // unnamed namespace

template <> constexpr inline auto ScoreCanvas::qt_create_metaobjectdata<qt_meta_tag_ZN11ScoreCanvasE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "ScoreCanvas",
        "zoomChanged",
        "",
        "pixelsPerHz",
        "slideModeChanged",
        "active",
        "frequencyRangeChanged",
        "minHz",
        "maxHz",
        "pressureChanged",
        "pressure",
        "cursorPositionChanged",
        "timeMs",
        "pitchHz",
        "notesChanged",
        "scaleSettingsChanged",
        "noteSelectionChanged",
        "tempoSettingsChanged",
        "requestAutoScroll",
        "dx",
        "dy"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'zoomChanged'
        QtMocHelpers::SignalData<void(double)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Double, 3 },
        }}),
        // Signal 'slideModeChanged'
        QtMocHelpers::SignalData<void(bool)>(4, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 5 },
        }}),
        // Signal 'frequencyRangeChanged'
        QtMocHelpers::SignalData<void(double, double)>(6, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Double, 7 }, { QMetaType::Double, 8 },
        }}),
        // Signal 'pressureChanged'
        QtMocHelpers::SignalData<void(double, bool)>(9, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Double, 10 }, { QMetaType::Bool, 5 },
        }}),
        // Signal 'cursorPositionChanged'
        QtMocHelpers::SignalData<void(double, double)>(11, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Double, 12 }, { QMetaType::Double, 13 },
        }}),
        // Signal 'notesChanged'
        QtMocHelpers::SignalData<void()>(14, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'scaleSettingsChanged'
        QtMocHelpers::SignalData<void()>(15, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'noteSelectionChanged'
        QtMocHelpers::SignalData<void()>(16, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'tempoSettingsChanged'
        QtMocHelpers::SignalData<void()>(17, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'requestAutoScroll'
        QtMocHelpers::SignalData<void(int, int)>(18, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 19 }, { QMetaType::Int, 20 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<ScoreCanvas, qt_meta_tag_ZN11ScoreCanvasE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject ScoreCanvas::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN11ScoreCanvasE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN11ScoreCanvasE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN11ScoreCanvasE_t>.metaTypes,
    nullptr
} };

void ScoreCanvas::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<ScoreCanvas *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->zoomChanged((*reinterpret_cast<std::add_pointer_t<double>>(_a[1]))); break;
        case 1: _t->slideModeChanged((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1]))); break;
        case 2: _t->frequencyRangeChanged((*reinterpret_cast<std::add_pointer_t<double>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<double>>(_a[2]))); break;
        case 3: _t->pressureChanged((*reinterpret_cast<std::add_pointer_t<double>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<bool>>(_a[2]))); break;
        case 4: _t->cursorPositionChanged((*reinterpret_cast<std::add_pointer_t<double>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<double>>(_a[2]))); break;
        case 5: _t->notesChanged(); break;
        case 6: _t->scaleSettingsChanged(); break;
        case 7: _t->noteSelectionChanged(); break;
        case 8: _t->tempoSettingsChanged(); break;
        case 9: _t->requestAutoScroll((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<int>>(_a[2]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (ScoreCanvas::*)(double )>(_a, &ScoreCanvas::zoomChanged, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (ScoreCanvas::*)(bool )>(_a, &ScoreCanvas::slideModeChanged, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (ScoreCanvas::*)(double , double )>(_a, &ScoreCanvas::frequencyRangeChanged, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (ScoreCanvas::*)(double , bool )>(_a, &ScoreCanvas::pressureChanged, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (ScoreCanvas::*)(double , double )>(_a, &ScoreCanvas::cursorPositionChanged, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (ScoreCanvas::*)()>(_a, &ScoreCanvas::notesChanged, 5))
            return;
        if (QtMocHelpers::indexOfMethod<void (ScoreCanvas::*)()>(_a, &ScoreCanvas::scaleSettingsChanged, 6))
            return;
        if (QtMocHelpers::indexOfMethod<void (ScoreCanvas::*)()>(_a, &ScoreCanvas::noteSelectionChanged, 7))
            return;
        if (QtMocHelpers::indexOfMethod<void (ScoreCanvas::*)()>(_a, &ScoreCanvas::tempoSettingsChanged, 8))
            return;
        if (QtMocHelpers::indexOfMethod<void (ScoreCanvas::*)(int , int )>(_a, &ScoreCanvas::requestAutoScroll, 9))
            return;
    }
}

const QMetaObject *ScoreCanvas::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ScoreCanvas::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN11ScoreCanvasE_t>.strings))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int ScoreCanvas::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 10)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 10;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 10)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 10;
    }
    return _id;
}

// SIGNAL 0
void ScoreCanvas::zoomChanged(double _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}

// SIGNAL 1
void ScoreCanvas::slideModeChanged(bool _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1);
}

// SIGNAL 2
void ScoreCanvas::frequencyRangeChanged(double _t1, double _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1, _t2);
}

// SIGNAL 3
void ScoreCanvas::pressureChanged(double _t1, bool _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 3, nullptr, _t1, _t2);
}

// SIGNAL 4
void ScoreCanvas::cursorPositionChanged(double _t1, double _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 4, nullptr, _t1, _t2);
}

// SIGNAL 5
void ScoreCanvas::notesChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}

// SIGNAL 6
void ScoreCanvas::scaleSettingsChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 6, nullptr);
}

// SIGNAL 7
void ScoreCanvas::noteSelectionChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 7, nullptr);
}

// SIGNAL 8
void ScoreCanvas::tempoSettingsChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 8, nullptr);
}

// SIGNAL 9
void ScoreCanvas::requestAutoScroll(int _t1, int _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 9, nullptr, _t1, _t2);
}
QT_WARNING_POP
