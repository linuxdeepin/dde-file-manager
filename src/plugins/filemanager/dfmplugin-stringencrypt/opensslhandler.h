// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef OPENSSLHANDLER_H
#define OPENSSLHANDLER_H

#include <QObject>

#include <openssl/rsa.h>

namespace dfmplugin_stringencrypt {

class OpenSSLHandler : public QObject
{
    Q_OBJECT

public:
    static OpenSSLHandler *instance();
    int encrypt(const QString &in, QString *out);

private:
    explicit OpenSSLHandler(QObject *parent = nullptr);
    ~OpenSSLHandler();
    QString pubKey();
};
}

#endif   // OPENSSLHANDLER_H
