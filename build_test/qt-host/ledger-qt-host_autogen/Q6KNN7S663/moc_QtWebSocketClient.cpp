/****************************************************************************
** Meta object code from reading C++ file 'QtWebSocketClient.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.9.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../qt-host/net/QtWebSocketClient.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'QtWebSocketClient.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.9.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_ledger__qt__QtWebSocketClient_t {
    QByteArrayData data[9];
    char stringdata0[145];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_ledger__qt__QtWebSocketClient_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_ledger__qt__QtWebSocketClient_t qt_meta_stringdata_ledger__qt__QtWebSocketClient = {
    {
QT_MOC_LITERAL(0, 0, 29), // "ledger::qt::QtWebSocketClient"
QT_MOC_LITERAL(1, 30, 20), // "onWebSocketConnected"
QT_MOC_LITERAL(2, 51, 0), // ""
QT_MOC_LITERAL(3, 52, 23), // "onWebSocketDisconnected"
QT_MOC_LITERAL(4, 76, 17), // "onMessageReceived"
QT_MOC_LITERAL(5, 94, 7), // "message"
QT_MOC_LITERAL(6, 102, 7), // "onError"
QT_MOC_LITERAL(7, 110, 28), // "QAbstractSocket::SocketError"
QT_MOC_LITERAL(8, 139, 5) // "error"

    },
    "ledger::qt::QtWebSocketClient\0"
    "onWebSocketConnected\0\0onWebSocketDisconnected\0"
    "onMessageReceived\0message\0onError\0"
    "QAbstractSocket::SocketError\0error"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_ledger__qt__QtWebSocketClient[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   34,    2, 0x08 /* Private */,
       3,    0,   35,    2, 0x08 /* Private */,
       4,    1,   36,    2, 0x08 /* Private */,
       6,    1,   39,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    5,
    QMetaType::Void, 0x80000000 | 7,    8,

       0        // eod
};

void ledger::qt::QtWebSocketClient::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        QtWebSocketClient *_t = static_cast<QtWebSocketClient *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->onWebSocketConnected(); break;
        case 1: _t->onWebSocketDisconnected(); break;
        case 2: _t->onMessageReceived((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 3: _t->onError((*reinterpret_cast< QAbstractSocket::SocketError(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 3:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QAbstractSocket::SocketError >(); break;
            }
            break;
        }
    }
}

const QMetaObject ledger::qt::QtWebSocketClient::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_ledger__qt__QtWebSocketClient.data,
      qt_meta_data_ledger__qt__QtWebSocketClient,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *ledger::qt::QtWebSocketClient::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ledger::qt::QtWebSocketClient::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ledger__qt__QtWebSocketClient.stringdata0))
        return static_cast<void*>(const_cast< QtWebSocketClient*>(this));
    if (!strcmp(_clname, "core::api::WebSocketClient"))
        return static_cast< core::api::WebSocketClient*>(const_cast< QtWebSocketClient*>(this));
    return QObject::qt_metacast(_clname);
}

int ledger::qt::QtWebSocketClient::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
