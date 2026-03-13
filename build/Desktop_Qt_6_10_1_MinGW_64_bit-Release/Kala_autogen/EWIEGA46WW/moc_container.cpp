/****************************************************************************
** Meta object code from reading C++ file 'container.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.10.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../container.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'container.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN9ContainerE_t {};
} // unnamed namespace

template <> constexpr inline auto Container::qt_create_metaobjectdata<qt_meta_tag_ZN9ContainerE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "Container",
        "portClicked",
        "",
        "Container*",
        "container",
        "portName",
        "isOutput",
        "QPoint",
        "globalPos",
        "moved",
        "clicked",
        "parameterChanged",
        "moveCompleted",
        "oldPos",
        "newPos",
        "dragStarted"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'portClicked'
        QtMocHelpers::SignalData<void(Container *, const QString &, bool, QPoint)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 3, 4 }, { QMetaType::QString, 5 }, { QMetaType::Bool, 6 }, { 0x80000000 | 7, 8 },
        }}),
        // Signal 'moved'
        QtMocHelpers::SignalData<void()>(9, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'clicked'
        QtMocHelpers::SignalData<void(Container *)>(10, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 3, 4 },
        }}),
        // Signal 'parameterChanged'
        QtMocHelpers::SignalData<void()>(11, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'moveCompleted'
        QtMocHelpers::SignalData<void(Container *, const QPoint &, const QPoint &)>(12, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 3, 4 }, { 0x80000000 | 7, 13 }, { 0x80000000 | 7, 14 },
        }}),
        // Signal 'dragStarted'
        QtMocHelpers::SignalData<void(Container *)>(15, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 3, 4 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<Container, qt_meta_tag_ZN9ContainerE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject Container::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN9ContainerE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN9ContainerE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN9ContainerE_t>.metaTypes,
    nullptr
} };

void Container::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<Container *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->portClicked((*reinterpret_cast<std::add_pointer_t<Container*>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<bool>>(_a[3])),(*reinterpret_cast<std::add_pointer_t<QPoint>>(_a[4]))); break;
        case 1: _t->moved(); break;
        case 2: _t->clicked((*reinterpret_cast<std::add_pointer_t<Container*>>(_a[1]))); break;
        case 3: _t->parameterChanged(); break;
        case 4: _t->moveCompleted((*reinterpret_cast<std::add_pointer_t<Container*>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QPoint>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<QPoint>>(_a[3]))); break;
        case 5: _t->dragStarted((*reinterpret_cast<std::add_pointer_t<Container*>>(_a[1]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
        case 0:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< Container* >(); break;
            }
            break;
        case 2:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< Container* >(); break;
            }
            break;
        case 4:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< Container* >(); break;
            }
            break;
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
        if (QtMocHelpers::indexOfMethod<void (Container::*)(Container * , const QString & , bool , QPoint )>(_a, &Container::portClicked, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (Container::*)()>(_a, &Container::moved, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (Container::*)(Container * )>(_a, &Container::clicked, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (Container::*)()>(_a, &Container::parameterChanged, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (Container::*)(Container * , const QPoint & , const QPoint & )>(_a, &Container::moveCompleted, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (Container::*)(Container * )>(_a, &Container::dragStarted, 5))
            return;
    }
}

const QMetaObject *Container::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Container::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN9ContainerE_t>.strings))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int Container::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    }
    return _id;
}

// SIGNAL 0
void Container::portClicked(Container * _t1, const QString & _t2, bool _t3, QPoint _t4)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1, _t2, _t3, _t4);
}

// SIGNAL 1
void Container::moved()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void Container::clicked(Container * _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1);
}

// SIGNAL 3
void Container::parameterChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void Container::moveCompleted(Container * _t1, const QPoint & _t2, const QPoint & _t3)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 4, nullptr, _t1, _t2, _t3);
}

// SIGNAL 5
void Container::dragStarted(Container * _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 5, nullptr, _t1);
}
QT_WARNING_POP
