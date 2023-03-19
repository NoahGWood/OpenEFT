/****************************************************************************
** Meta object code from reading C++ file 'io_nistraw.h'
**
** Created: Thu Feb 24 13:05:52 2011
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

/******************************************
* This file is modified by NIST - 06/2011 *
******************************************/

#include "io_nistraw.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'io_nistraw.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_NistrawIOPlugin[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

static const char qt_meta_stringdata_NistrawIOPlugin[] = {
    "NistrawIOPlugin\0"
};

const QMetaObject NistrawIOPlugin::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_NistrawIOPlugin,
      qt_meta_data_NistrawIOPlugin, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &NistrawIOPlugin::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *NistrawIOPlugin::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *NistrawIOPlugin::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_NistrawIOPlugin))
        return static_cast<void*>(const_cast< NistrawIOPlugin*>(this));
    if (!strcmp(_clname, "MeshIOInterface"))
        return static_cast< MeshIOInterface*>(const_cast< NistrawIOPlugin*>(this));
    if (!strcmp(_clname, "vcg.meshlab.MeshIOInterface/1.0"))
        return static_cast< MeshIOInterface*>(const_cast< NistrawIOPlugin*>(this));
    return QObject::qt_metacast(_clname);
}

int NistrawIOPlugin::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
QT_END_MOC_NAMESPACE
