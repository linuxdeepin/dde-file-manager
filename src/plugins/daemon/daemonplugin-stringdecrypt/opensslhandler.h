// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef OPENSSLHANDLER_H
#define OPENSSLHANDLER_H

#include <QObject>

#include <openssl/rsa.h>

namespace daemonplugin_stringdecrypt {

class OpenSSLHandler : public QObject
{
    Q_OBJECT

public:
    static OpenSSLHandler *instance();

    void initKeyPairs();

    QString pubKey() const;
    int decrypt(const QString &in, QString *out);

private:
    explicit OpenSSLHandler(QObject *parent = nullptr);
    ~OpenSSLHandler();

    RSA *rsa { nullptr };
    QPair<QString, QString> keys;
};
}

#endif   // OPENSSLHANDLER_H
