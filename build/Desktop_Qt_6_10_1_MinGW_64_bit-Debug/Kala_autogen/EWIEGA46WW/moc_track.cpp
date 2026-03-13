/****************************************************************************
** Meta object code from reading C++ file 'track.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.10.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../track.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'track.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN5TrackE_t {};
} // unnamed namespace

template <> constexpr inline auto Track::qt_create_metaobjectdata<qt_meta_tag_ZN5TrackE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "Track",
        "nameChanged",
        "",
        "name",
        "colorChanged",
        "QColor",
        "color",
        "gainChanged",
        "gain",
        "sounitLoaded",
        "sounitName",
        "sounitDirtyChanged",
        "dirty",
        "graphRebuilt",
        "isValid",
        "noteAdded",
        "Note",
        "note",
        "noteRemoved",
        "noteId",
        "noteUpdated",
        "notesDirtyChanged",
        "renderStarted",
        "renderProgressChanged",
        "percent",
        "renderCompleted",
        "renderCancelled",
        "renderCacheInvalidated",
        "muteChanged",
        "muted",
        "soloChanged",
        "solo",
        "volumeChanged",
        "volume",
        "panChanged",
        "pan",
        "variationCreated",
        "index",
        "variationDeleted",
        "variationRenamed",
        "onCanvasGraphChanged",
        "onContainerParameterChanged",
        "onSounitNameChanged"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'nameChanged'
        QtMocHelpers::SignalData<void(const QString &)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 3 },
        }}),
        // Signal 'colorChanged'
        QtMocHelpers::SignalData<void(const QColor &)>(4, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 5, 6 },
        }}),
        // Signal 'gainChanged'
        QtMocHelpers::SignalData<void(float)>(7, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Float, 8 },
        }}),
        // Signal 'sounitLoaded'
        QtMocHelpers::SignalData<void(const QString &)>(9, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 10 },
        }}),
        // Signal 'sounitDirtyChanged'
        QtMocHelpers::SignalData<void(bool)>(11, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 12 },
        }}),
        // Signal 'graphRebuilt'
        QtMocHelpers::SignalData<void(bool)>(13, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 14 },
        }}),
        // Signal 'noteAdded'
        QtMocHelpers::SignalData<void(const Note &)>(15, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 16, 17 },
        }}),
        // Signal 'noteRemoved'
        QtMocHelpers::SignalData<void(const QString &)>(18, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 19 },
        }}),
        // Signal 'noteUpdated'
        QtMocHelpers::SignalData<void(const Note &)>(20, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 16, 17 },
        }}),
        // Signal 'notesDirtyChanged'
        QtMocHelpers::SignalData<void(bool)>(21, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 12 },
        }}),
        // Signal 'renderStarted'
        QtMocHelpers::SignalData<void()>(22, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'renderProgressChanged'
        QtMocHelpers::SignalData<void(int)>(23, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 24 },
        }}),
        // Signal 'renderCompleted'
        QtMocHelpers::SignalData<void()>(25, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'renderCancelled'
        QtMocHelpers::SignalData<void()>(26, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'renderCacheInvalidated'
        QtMocHelpers::SignalData<void()>(27, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'muteChanged'
        QtMocHelpers::SignalData<void(bool)>(28, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 29 },
        }}),
        // Signal 'soloChanged'
        QtMocHelpers::SignalData<void(bool)>(30, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 31 },
        }}),
        // Signal 'volumeChanged'
        QtMocHelpers::SignalData<void(float)>(32, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Float, 33 },
        }}),
        // Signal 'panChanged'
        QtMocHelpers::SignalData<void(float)>(34, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Float, 35 },
        }}),
        // Signal 'variationCreated'
        QtMocHelpers::SignalData<void(int, const QString &)>(36, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 37 }, { QMetaType::QString, 3 },
        }}),
        // Signal 'variationDeleted'
        QtMocHelpers::SignalData<void(int)>(38, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 37 },
        }}),
        // Signal 'variationRenamed'
        QtMocHelpers::SignalData<void(int, const QString &)>(39, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 37 }, { QMetaType::QString, 3 },
        }}),
        // Slot 'onCanvasGraphChanged'
        QtMocHelpers::SlotData<void()>(40, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onContainerParameterChanged'
        QtMocHelpers::SlotData<void()>(41, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onSounitNameChanged'
        QtMocHelpers::SlotData<void(const QString &)>(42, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 3 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<Track, qt_meta_tag_ZN5TrackE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject Track::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN5TrackE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN5TrackE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN5TrackE_t>.metaTypes,
    nullptr
} };

void Track::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<Track *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->nameChanged((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 1: _t->colorChanged((*reinterpret_cast<std::add_pointer_t<QColor>>(_a[1]))); break;
        case 2: _t->gainChanged((*reinterpret_cast<std::add_pointer_t<float>>(_a[1]))); break;
        case 3: _t->sounitLoaded((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 4: _t->sounitDirtyChanged((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1]))); break;
        case 5: _t->graphRebuilt((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1]))); break;
        case 6: _t->noteAdded((*reinterpret_cast<std::add_pointer_t<Note>>(_a[1]))); break;
        case 7: _t->noteRemoved((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 8: _t->noteUpdated((*reinterpret_cast<std::add_pointer_t<Note>>(_a[1]))); break;
        case 9: _t->notesDirtyChanged((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1]))); break;
        case 10: _t->renderStarted(); break;
        case 11: _t->renderProgressChanged((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 12: _t->renderCompleted(); break;
        case 13: _t->renderCancelled(); break;
        case 14: _t->renderCacheInvalidated(); break;
        case 15: _t->muteChanged((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1]))); break;
        case 16: _t->soloChanged((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1]))); break;
        case 17: _t->volumeChanged((*reinterpret_cast<std::add_pointer_t<float>>(_a[1]))); break;
        case 18: _t->panChanged((*reinterpret_cast<std::add_pointer_t<float>>(_a[1]))); break;
        case 19: _t->variationCreated((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2]))); break;
        case 20: _t->variationDeleted((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 21: _t->variationRenamed((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2]))); break;
        case 22: _t->onCanvasGraphChanged(); break;
        case 23: _t->onContainerParameterChanged(); break;
        case 24: _t->onSounitNameChanged((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (Track::*)(const QString & )>(_a, &Track::nameChanged, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (Track::*)(const QColor & )>(_a, &Track::colorChanged, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (Track::*)(float )>(_a, &Track::gainChanged, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (Track::*)(const QString & )>(_a, &Track::sounitLoaded, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (Track::*)(bool )>(_a, &Track::sounitDirtyChanged, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (Track::*)(bool )>(_a, &Track::graphRebuilt, 5))
            return;
        if (QtMocHelpers::indexOfMethod<void (Track::*)(const Note & )>(_a, &Track::noteAdded, 6))
            return;
        if (QtMocHelpers::indexOfMethod<void (Track::*)(const QString & )>(_a, &Track::noteRemoved, 7))
            return;
        if (QtMocHelpers::indexOfMethod<void (Track::*)(const Note & )>(_a, &Track::noteUpdated, 8))
            return;
        if (QtMocHelpers::indexOfMethod<void (Track::*)(bool )>(_a, &Track::notesDirtyChanged, 9))
            return;
        if (QtMocHelpers::indexOfMethod<void (Track::*)()>(_a, &Track::renderStarted, 10))
            return;
        if (QtMocHelpers::indexOfMethod<void (Track::*)(int )>(_a, &Track::renderProgressChanged, 11))
            return;
        if (QtMocHelpers::indexOfMethod<void (Track::*)()>(_a, &Track::renderCompleted, 12))
            return;
        if (QtMocHelpers::indexOfMethod<void (Track::*)()>(_a, &Track::renderCancelled, 13))
            return;
        if (QtMocHelpers::indexOfMethod<void (Track::*)()>(_a, &Track::renderCacheInvalidated, 14))
            return;
        if (QtMocHelpers::indexOfMethod<void (Track::*)(bool )>(_a, &Track::muteChanged, 15))
            return;
        if (QtMocHelpers::indexOfMethod<void (Track::*)(bool )>(_a, &Track::soloChanged, 16))
            return;
        if (QtMocHelpers::indexOfMethod<void (Track::*)(float )>(_a, &Track::volumeChanged, 17))
            return;
        if (QtMocHelpers::indexOfMethod<void (Track::*)(float )>(_a, &Track::panChanged, 18))
            return;
        if (QtMocHelpers::indexOfMethod<void (Track::*)(int , const QString & )>(_a, &Track::variationCreated, 19))
            return;
        if (QtMocHelpers::indexOfMethod<void (Track::*)(int )>(_a, &Track::variationDeleted, 20))
            return;
        if (QtMocHelpers::indexOfMethod<void (Track::*)(int , const QString & )>(_a, &Track::variationRenamed, 21))
            return;
    }
}

const QMetaObject *Track::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Track::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN5TrackE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int Track::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 25)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 25;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 25)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 25;
    }
    return _id;
}

// SIGNAL 0
void Track::nameChanged(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}

// SIGNAL 1
void Track::colorChanged(const QColor & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1);
}

// SIGNAL 2
void Track::gainChanged(float _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1);
}

// SIGNAL 3
void Track::sounitLoaded(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 3, nullptr, _t1);
}

// SIGNAL 4
void Track::sounitDirtyChanged(bool _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 4, nullptr, _t1);
}

// SIGNAL 5
void Track::graphRebuilt(bool _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 5, nullptr, _t1);
}

// SIGNAL 6
void Track::noteAdded(const Note & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 6, nullptr, _t1);
}

// SIGNAL 7
void Track::noteRemoved(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 7, nullptr, _t1);
}

// SIGNAL 8
void Track::noteUpdated(const Note & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 8, nullptr, _t1);
}

// SIGNAL 9
void Track::notesDirtyChanged(bool _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 9, nullptr, _t1);
}

// SIGNAL 10
void Track::renderStarted()
{
    QMetaObject::activate(this, &staticMetaObject, 10, nullptr);
}

// SIGNAL 11
void Track::renderProgressChanged(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 11, nullptr, _t1);
}

// SIGNAL 12
void Track::renderCompleted()
{
    QMetaObject::activate(this, &staticMetaObject, 12, nullptr);
}

// SIGNAL 13
void Track::renderCancelled()
{
    QMetaObject::activate(this, &staticMetaObject, 13, nullptr);
}

// SIGNAL 14
void Track::renderCacheInvalidated()
{
    QMetaObject::activate(this, &staticMetaObject, 14, nullptr);
}

// SIGNAL 15
void Track::muteChanged(bool _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 15, nullptr, _t1);
}

// SIGNAL 16
void Track::soloChanged(bool _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 16, nullptr, _t1);
}

// SIGNAL 17
void Track::volumeChanged(float _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 17, nullptr, _t1);
}

// SIGNAL 18
void Track::panChanged(float _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 18, nullptr, _t1);
}

// SIGNAL 19
void Track::variationCreated(int _t1, const QString & _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 19, nullptr, _t1, _t2);
}

// SIGNAL 20
void Track::variationDeleted(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 20, nullptr, _t1);
}

// SIGNAL 21
void Track::variationRenamed(int _t1, const QString & _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 21, nullptr, _t1, _t2);
}
QT_WARNING_POP
