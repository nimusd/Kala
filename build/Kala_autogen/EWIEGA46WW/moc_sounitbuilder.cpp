/****************************************************************************
** Meta object code from reading C++ file 'sounitbuilder.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.10.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../sounitbuilder.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'sounitbuilder.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN13SounitBuilderE_t {};
} // unnamed namespace

template <> constexpr inline auto SounitBuilder::qt_create_metaobjectdata<qt_meta_tag_ZN13SounitBuilderE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "SounitBuilder",
        "playbackStarted",
        "",
        "variationsChanged",
        "noteModeChanged",
        "active",
        "onAddContainer",
        "name",
        "QColor",
        "color",
        "inputs",
        "outputs",
        "triggerPlay",
        "onPortClicked",
        "Container*",
        "container",
        "portName",
        "isOutput",
        "QPoint",
        "globalPos",
        "onPlay",
        "onStop",
        "onPlaybackTick",
        "onRenderStarted",
        "onRenderProgressChanged",
        "percentage",
        "onRenderCompleted",
        "onRenderCancelled",
        "stopPlayback",
        "stopAudioEngine",
        "onCreateVariation",
        "rebuildGraph",
        "trackIndex",
        "rebuildTrackGraph",
        "setCurrentEditingTrack"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'playbackStarted'
        QtMocHelpers::SignalData<void()>(1, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'variationsChanged'
        QtMocHelpers::SignalData<void()>(3, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'noteModeChanged'
        QtMocHelpers::SignalData<void(bool)>(4, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 5 },
        }}),
        // Slot 'onAddContainer'
        QtMocHelpers::SlotData<void(const QString &, const QColor &, const QStringList &, const QStringList &)>(6, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 7 }, { 0x80000000 | 8, 9 }, { QMetaType::QStringList, 10 }, { QMetaType::QStringList, 11 },
        }}),
        // Slot 'triggerPlay'
        QtMocHelpers::SlotData<void()>(12, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'onPortClicked'
        QtMocHelpers::SlotData<void(Container *, const QString &, bool, QPoint)>(13, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 14, 15 }, { QMetaType::QString, 16 }, { QMetaType::Bool, 17 }, { 0x80000000 | 18, 19 },
        }}),
        // Slot 'onPlay'
        QtMocHelpers::SlotData<void()>(20, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onStop'
        QtMocHelpers::SlotData<void()>(21, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onPlaybackTick'
        QtMocHelpers::SlotData<void()>(22, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onRenderStarted'
        QtMocHelpers::SlotData<void()>(23, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onRenderProgressChanged'
        QtMocHelpers::SlotData<void(int)>(24, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 25 },
        }}),
        // Slot 'onRenderCompleted'
        QtMocHelpers::SlotData<void()>(26, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onRenderCancelled'
        QtMocHelpers::SlotData<void()>(27, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'stopPlayback'
        QtMocHelpers::SlotData<void(bool)>(28, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 29 },
        }}),
        // Slot 'stopPlayback'
        QtMocHelpers::SlotData<void()>(28, 2, QMC::AccessPublic | QMC::MethodCloned, QMetaType::Void),
        // Slot 'onCreateVariation'
        QtMocHelpers::SlotData<void()>(30, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'rebuildGraph'
        QtMocHelpers::SlotData<void(int, bool)>(31, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 32 }, { QMetaType::Bool, 33 },
        }}),
        // Slot 'rebuildGraph'
        QtMocHelpers::SlotData<void(int)>(31, 2, QMC::AccessPublic | QMC::MethodCloned, QMetaType::Void, {{
            { QMetaType::Int, 32 },
        }}),
        // Slot 'rebuildGraph'
        QtMocHelpers::SlotData<void()>(31, 2, QMC::AccessPublic | QMC::MethodCloned, QMetaType::Void),
        // Slot 'setCurrentEditingTrack'
        QtMocHelpers::SlotData<void(int)>(34, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 32 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<SounitBuilder, qt_meta_tag_ZN13SounitBuilderE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject SounitBuilder::staticMetaObject = { {
    QMetaObject::SuperData::link<QMainWindow::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN13SounitBuilderE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN13SounitBuilderE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN13SounitBuilderE_t>.metaTypes,
    nullptr
} };

void SounitBuilder::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<SounitBuilder *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->playbackStarted(); break;
        case 1: _t->variationsChanged(); break;
        case 2: _t->noteModeChanged((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1]))); break;
        case 3: _t->onAddContainer((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QColor>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<QStringList>>(_a[3])),(*reinterpret_cast<std::add_pointer_t<QStringList>>(_a[4]))); break;
        case 4: _t->triggerPlay(); break;
        case 5: _t->onPortClicked((*reinterpret_cast<std::add_pointer_t<Container*>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<bool>>(_a[3])),(*reinterpret_cast<std::add_pointer_t<QPoint>>(_a[4]))); break;
        case 6: _t->onPlay(); break;
        case 7: _t->onStop(); break;
        case 8: _t->onPlaybackTick(); break;
        case 9: _t->onRenderStarted(); break;
        case 10: _t->onRenderProgressChanged((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 11: _t->onRenderCompleted(); break;
        case 12: _t->onRenderCancelled(); break;
        case 13: _t->stopPlayback((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1]))); break;
        case 14: _t->stopPlayback(); break;
        case 15: _t->onCreateVariation(); break;
        case 16: _t->rebuildGraph((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<bool>>(_a[2]))); break;
        case 17: _t->rebuildGraph((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 18: _t->rebuildGraph(); break;
        case 19: _t->setCurrentEditingTrack((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
        case 5:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< Container* >(); break;
            }
            break;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (SounitBuilder::*)()>(_a, &SounitBuilder::playbackStarted, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (SounitBuilder::*)()>(_a, &SounitBuilder::variationsChanged, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (SounitBuilder::*)(bool )>(_a, &SounitBuilder::noteModeChanged, 2))
            return;
    }
}

const QMetaObject *SounitBuilder::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *SounitBuilder::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN13SounitBuilderE_t>.strings))
        return static_cast<void*>(this);
    return QMainWindow::qt_metacast(_clname);
}

int SounitBuilder::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 20)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 20;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 20)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 20;
    }
    return _id;
}

// SIGNAL 0
void SounitBuilder::playbackStarted()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void SounitBuilder::variationsChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void SounitBuilder::noteModeChanged(bool _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1);
}
QT_WARNING_POP
