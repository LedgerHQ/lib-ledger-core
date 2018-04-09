/****************************************************************************
** Meta object code from reading C++ file 'QtMainExecutionContext.hpp'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.9.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../qt-host/async/QtMainExecutionContext.hpp"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'QtMainExecutionContext.hpp' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.9.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_ledger__qt__QtMainExecutionContext_t {
    QByteArrayData data[6];
    char stringdata0[110];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_ledger__qt__QtMainExecutionContext_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_ledger__qt__QtMainExecutionContext_t qt_meta_stringdata_ledger__qt__QtMainExecutionContext = {
    {
QT_MOC_LITERAL(0, 0, 34), // "ledger::qt::QtMainExecutionCo..."
QT_MOC_LITERAL(1, 35, 12), // "postRunnable"
QT_MOC_LITERAL(2, 48, 0), // ""
QT_MOC_LITERAL(3, 49, 36), // "std::shared_ptr<core::api::Ru..."
QT_MOC_LITERAL(4, 86, 8), // "runnable"
QT_MOC_LITERAL(5, 95, 14) // "performExecute"

    },
    "ledger::qt::QtMainExecutionContext\0"
    "postRunnable\0\0std::shared_ptr<core::api::Runnable>\0"
    "runnable\0performExecute"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_ledger__qt__QtMainExecutionContext[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   24,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       5,    1,   27,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    4,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 3,    4,

       0        // eod
};

void ledger::qt::QtMainExecutionContext::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        QtMainExecutionContext *_t = static_cast<QtMainExecutionContext *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->postRunnable((*reinterpret_cast< std::shared_ptr<core::api::Runnable>(*)>(_a[1]))); break;
        case 1: _t->performExecute((*reinterpret_cast< std::shared_ptr<core::api::Runnable>(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (QtMainExecutionContext::*_t)(std::shared_ptr<core::api::Runnable> );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&QtMainExecutionContext::postRunnable)) {
                *result = 0;
                return;
            }
        }
    }
}

const QMetaObject ledger::qt::QtMainExecutionContext::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_ledger__qt__QtMainExecutionContext.data,
      qt_meta_data_ledger__qt__QtMainExecutionContext,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *ledger::qt::QtMainExecutionContext::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ledger::qt::QtMainExecutionContext::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ledger__qt__QtMainExecutionContext.stringdata0))
        return static_cast<void*>(const_cast< QtMainExecutionContext*>(this));
    if (!strcmp(_clname, "core::api::ExecutionContext"))
        return static_cast< core::api::ExecutionContext*>(const_cast< QtMainExecutionContext*>(this));
    return QObject::qt_metacast(_clname);
}

int ledger::qt::QtMainExecutionContext::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 2)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 2;
    }
    return _id;
}

// SIGNAL 0
void ledger::qt::QtMainExecutionContext::postRunnable(std::shared_ptr<core::api::Runnable> _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
