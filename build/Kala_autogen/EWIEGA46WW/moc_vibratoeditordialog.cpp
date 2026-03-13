/****************************************************************************
** Meta object code from reading C++ file 'vibratoeditordialog.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.10.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../vibratoeditordialog.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'vibratoeditordialog.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN19VibratoEditorDialogE_t {};
} // unnamed namespace

template <> constexpr inline auto VibratoEditorDialog::qt_create_metaobjectdata<qt_meta_tag_ZN19VibratoEditorDialogE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "VibratoEditorDialog",
        "onRateChanged",
        "",
        "value",
        "onPitchDepthChanged",
        "onAmplitudeDepthChanged",
        "onOnsetChanged",
        "onRegularitySliderChanged",
        "onEnvelopeChanged",
        "onLoadEnvelopeClicked",
        "onVibratoPresetChanged",
        "index",
        "onSaveVibratoPresetClicked"
    };

    QtMocHelpers::UintData qt_methods {
        // Slot 'onRateChanged'
        QtMocHelpers::SlotData<void(double)>(1, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Double, 3 },
        }}),
        // Slot 'onPitchDepthChanged'
        QtMocHelpers::SlotData<void(double)>(4, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Double, 3 },
        }}),
        // Slot 'onAmplitudeDepthChanged'
        QtMocHelpers::SlotData<void(double)>(5, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Double, 3 },
        }}),
        // Slot 'onOnsetChanged'
        QtMocHelpers::SlotData<void(double)>(6, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Double, 3 },
        }}),
        // Slot 'onRegularitySliderChanged'
        QtMocHelpers::SlotData<void(int)>(7, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 3 },
        }}),
        // Slot 'onEnvelopeChanged'
        QtMocHelpers::SlotData<void()>(8, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onLoadEnvelopeClicked'
        QtMocHelpers::SlotData<void()>(9, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onVibratoPresetChanged'
        QtMocHelpers::SlotData<void(int)>(10, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 11 },
        }}),
        // Slot 'onSaveVibratoPresetClicked'
        QtMocHelpers::SlotData<void()>(12, 2, QMC::AccessPrivate, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<VibratoEditorDialog, qt_meta_tag_ZN19VibratoEditorDialogE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject VibratoEditorDialog::staticMetaObject = { {
    QMetaObject::SuperData::link<QDialog::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN19VibratoEditorDialogE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN19VibratoEditorDialogE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN19VibratoEditorDialogE_t>.metaTypes,
    nullptr
} };

void VibratoEditorDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<VibratoEditorDialog *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->onRateChanged((*reinterpret_cast<std::add_pointer_t<double>>(_a[1]))); break;
        case 1: _t->onPitchDepthChanged((*reinterpret_cast<std::add_pointer_t<double>>(_a[1]))); break;
        case 2: _t->onAmplitudeDepthChanged((*reinterpret_cast<std::add_pointer_t<double>>(_a[1]))); break;
        case 3: _t->onOnsetChanged((*reinterpret_cast<std::add_pointer_t<double>>(_a[1]))); break;
        case 4: _t->onRegularitySliderChanged((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 5: _t->onEnvelopeChanged(); break;
        case 6: _t->onLoadEnvelopeClicked(); break;
        case 7: _t->onVibratoPresetChanged((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 8: _t->onSaveVibratoPresetClicked(); break;
        default: ;
        }
    }
}

const QMetaObject *VibratoEditorDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *VibratoEditorDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN19VibratoEditorDialogE_t>.strings))
        return static_cast<void*>(this);
    return QDialog::qt_metacast(_clname);
}

int VibratoEditorDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 9)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 9;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 9)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 9;
    }
    return _id;
}
QT_WARNING_POP
