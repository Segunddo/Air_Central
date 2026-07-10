/****************************************************************************
** Meta object code from reading C++ file 'receivedata.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.10.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../receivedata.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'receivedata.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.10.3. It"
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
struct qt_meta_tag_ZN11ReceiveDataE_t {};
} // unnamed namespace

template <> constexpr inline auto ReceiveData::qt_create_metaobjectdata<qt_meta_tag_ZN11ReceiveDataE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "ReceiveData",
        "newDeviceDetected",
        "",
        "idEsp",
        "set_waited_command",
        "commandType",
        "read_data",
        "decode_data",
        "QJsonObject",
        "obj",
        "conectar",
        "nomePorta",
        "list_ports"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'newDeviceDetected'
        QtMocHelpers::SignalData<void(QString)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 3 },
        }}),
        // Slot 'set_waited_command'
        QtMocHelpers::SlotData<void(QString)>(4, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 5 },
        }}),
        // Slot 'read_data'
        QtMocHelpers::SlotData<void()>(6, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'decode_data'
        QtMocHelpers::SlotData<void(QJsonObject)>(7, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 8, 9 },
        }}),
        // Method 'conectar'
        QtMocHelpers::MethodData<bool(QString)>(10, 2, QMC::AccessPublic, QMetaType::Bool, {{
            { QMetaType::QString, 11 },
        }}),
        // Method 'list_ports'
        QtMocHelpers::MethodData<QStringList()>(12, 2, QMC::AccessPublic, QMetaType::QStringList),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<ReceiveData, qt_meta_tag_ZN11ReceiveDataE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject ReceiveData::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN11ReceiveDataE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN11ReceiveDataE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN11ReceiveDataE_t>.metaTypes,
    nullptr
} };

void ReceiveData::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<ReceiveData *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->newDeviceDetected((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 1: _t->set_waited_command((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 2: _t->read_data(); break;
        case 3: _t->decode_data((*reinterpret_cast<std::add_pointer_t<QJsonObject>>(_a[1]))); break;
        case 4: { bool _r = _t->conectar((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])));
            if (_a[0]) *reinterpret_cast<bool*>(_a[0]) = std::move(_r); }  break;
        case 5: { QStringList _r = _t->list_ports();
            if (_a[0]) *reinterpret_cast<QStringList*>(_a[0]) = std::move(_r); }  break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (ReceiveData::*)(QString )>(_a, &ReceiveData::newDeviceDetected, 0))
            return;
    }
}

const QMetaObject *ReceiveData::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ReceiveData::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN11ReceiveDataE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int ReceiveData::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 6)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 6;
    }
    return _id;
}

// SIGNAL 0
void ReceiveData::newDeviceDetected(QString _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}
QT_WARNING_POP
