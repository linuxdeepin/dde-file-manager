// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "opensslhandler.h"

#include <dfm-base/dfm_log_defines.h>
#include <dfm-base/utils/finallyutil.h>

#include <QDBusInterface>
#include <QDBusReply>
#include <QDebug>

#include <openssl/pem.h>

DFM_LOG_USE_CATEGORY(dfmplugin_stringencrypt)

using namespace dfmplugin_stringencrypt;

OpenSSLHandler *OpenSSLHandler::instance()
{
    static OpenSSLHandler ins;
    return &ins;
}

int OpenSSLHandler::encrypt(const QString &in, QString *out)
{
    const auto &&publicKey = pubKey();
    Q_ASSERT(!publicKey.isEmpty());
    Q_ASSERT(out);

    BIO *bio { nullptr };
    RSA *rsa { nullptr };
    unsigned char *cipherData { nullptr };

    dfmbase::FinallyUtil finalClear([=] {
        if (bio) BIO_free(bio);
        if (rsa) RSA_free(rsa);
        if (cipherData) delete[] cipherData;
    });

    bio = BIO_new(BIO_s_mem());
    BIO_write(bio, publicKey.toStdString().c_str(), publicKey.length());

    rsa = PEM_read_bio_RSA_PUBKEY(bio, nullptr, nullptr, nullptr);
    if (!rsa) {
        fmWarning() << "cannot read rsa by pubkey";
        return -1;
    }

    int rsaSize = RSA_size(rsa);
    cipherData = new unsigned char[rsaSize];
    int result = RSA_public_encrypt(in.length(),
                                    reinterpret_cast<const unsigned char *>(in.toStdString().c_str()),
                                    cipherData,
                                    rsa,
                                    RSA_PKCS1_PADDING);

    if (result == -1) {
        fmWarning() << "cannot encrypt by pubkey";
        return result;
    }

    QByteArray cipher(reinterpret_cast<char *>(cipherData), result);
    *out = QString(cipher.toBase64());
    return 0;
}

OpenSSLHandler::OpenSSLHandler(QObject *parent)
{
}

OpenSSLHandler::~OpenSSLHandler()
{
}

QString OpenSSLHandler::pubKey()
{
    QDBusInterface iface("com.deepin.filemanager.daemon",
                         "/com/deepin/filemanager/daemon/EncryptKeyHelper",
                         "com.deepin.filemanager.daemon.EncryptKeyHelper",
                         QDBusConnection::systemBus());
    if (iface.isValid()) {
        QDBusReply<QString> reply = iface.call("PublicKey");
        if (reply.isValid()) {
            QByteArray b64Pk = reply.value().toLocal8Bit();
            return QByteArray::fromBase64(b64Pk);
        }
    }
    fmWarning() << "dbus interface not valid or no public key returned";
    return "";
}
