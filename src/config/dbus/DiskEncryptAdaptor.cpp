// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "src/config/dbus/DiskEncryptAdaptor.h"
#include <QtCore/QMetaObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>

/*
 * Implementation of adaptor class DiskEncryptAdaptor
 */

DiskEncryptAdaptor::DiskEncryptAdaptor(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
    // constructor
    setAutoRelaySignals(true);
}

DiskEncryptAdaptor::~DiskEncryptAdaptor()
{
    // destructor
}

QString DiskEncryptAdaptor::ChangeEncryptPassphress(const QVariantMap &params)
{
    // handle method call org.deepin.Filemanager.DiskEncrypt.ChangeEncryptPassphress
    QString out0;
    QMetaObject::invokeMethod(parent(), "ChangeEncryptPassphress", Q_RETURN_ARG(QString, out0), Q_ARG(QVariantMap, params));
    return out0;
}

QString DiskEncryptAdaptor::DecryptDisk(const QVariantMap &params)
{
    // handle method call org.deepin.Filemanager.DiskEncrypt.DecryptDisk
    QString out0;
    QMetaObject::invokeMethod(parent(), "DecryptDisk", Q_RETURN_ARG(QString, out0), Q_ARG(QVariantMap, params));
    return out0;
}

int DiskEncryptAdaptor::EncryptStatus(const QString &device)
{
    // handle method call org.deepin.Filemanager.DiskEncrypt.EncryptStatus
    int out0;
    QMetaObject::invokeMethod(parent(), "EncryptStatus", Q_RETURN_ARG(int, out0), Q_ARG(QString, device));
    return out0;
}

bool DiskEncryptAdaptor::HasPendingTask()
{
    // handle method call org.deepin.Filemanager.DiskEncrypt.HasPendingTask
    bool out0;
    QMetaObject::invokeMethod(parent(), "HasPendingTask", Q_RETURN_ARG(bool, out0));
    return out0;
}

void DiskEncryptAdaptor::IgnoreParamRequest()
{
    // handle method call org.deepin.Filemanager.DiskEncrypt.IgnoreParamRequest
    QMetaObject::invokeMethod(parent(), "IgnoreParamRequest");
}

bool DiskEncryptAdaptor::IsWorkerRunning()
{
    // handle method call org.deepin.Filemanager.DiskEncrypt.IsWorkerRunning
    bool out0;
    QMetaObject::invokeMethod(parent(), "IsWorkerRunning", Q_RETURN_ARG(bool, out0));
    return out0;
}

QString DiskEncryptAdaptor::PrepareEncryptDisk(const QVariantMap &params)
{
    // handle method call org.deepin.Filemanager.DiskEncrypt.PrepareEncryptDisk
    QString out0;
    QMetaObject::invokeMethod(parent(), "PrepareEncryptDisk", Q_RETURN_ARG(QString, out0), Q_ARG(QVariantMap, params));
    return out0;
}

QString DiskEncryptAdaptor::QueryTPMToken(const QString &device)
{
    // handle method call org.deepin.Filemanager.DiskEncrypt.QueryTPMToken
    QString out0;
    QMetaObject::invokeMethod(parent(), "QueryTPMToken", Q_RETURN_ARG(QString, out0), Q_ARG(QString, device));
    return out0;
}

void DiskEncryptAdaptor::ResumeEncryption(const QString &device)
{
    // handle method call org.deepin.Filemanager.DiskEncrypt.ResumeEncryption
    QMetaObject::invokeMethod(parent(), "ResumeEncryption", Q_ARG(QString, device));
}

void DiskEncryptAdaptor::SetEncryptParams(const QVariantMap &params)
{
    // handle method call org.deepin.Filemanager.DiskEncrypt.SetEncryptParams
    QMetaObject::invokeMethod(parent(), "SetEncryptParams", Q_ARG(QVariantMap, params));
}

QString DiskEncryptAdaptor::UnfinishedDecryptJob()
{
    // handle method call org.deepin.Filemanager.DiskEncrypt.UnfinishedDecryptJob
    QString out0;
    QMetaObject::invokeMethod(parent(), "UnfinishedDecryptJob", Q_RETURN_ARG(QString, out0));
    return out0;
}

