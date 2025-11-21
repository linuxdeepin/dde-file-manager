// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "tpmcontroldbus.h"
#include "core/tpmwork.h"
#include "polkit/policykithelper.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDataStream>
#include <QIODevice>

#include <unistd.h>
#include <cerrno>

namespace service_tpmcontrol {
DFM_LOG_REGISTER_CATEGORY(SERVICETPMCONTROL_NAMESPACE)
}

SERVICETPMCONTROL_USE_NAMESPACE

TPMControlDBus::TPMControlDBus(const char *name, QObject *parent)
    : QObject(parent), QDBusContext(), m_tpmWork(new TPMWork(this))
{
    QDBusConnection::RegisterOptions opts =
            QDBusConnection::ExportAllSlots |
            QDBusConnection::ExportAllSignals |
            QDBusConnection::ExportAllProperties;

    QDBusConnection::connectToBus(QDBusConnection::SystemBus, QString(name))
            .registerObject("/org/deepin/Filemanager/TPMControl", this, opts);

    fmInfo() << "TPMControlDBus registered on SystemBus";
}

TPMControlDBus::~TPMControlDBus()
{
    fmDebug() << "TPMControlDBus destroyed";
}

bool TPMControlDBus::checkAuthentication(const QString &actionId)
{
    if (!PolicyKitHelper::instance()->checkAuthorization(actionId, message().service())) {
        fmWarning() << "Authentication failed for action:" << actionId;
        return false;
    }
    return true;
}

bool TPMControlDBus::parseCredentialsFromFd(const QDBusUnixFileDescriptor &fd, QVariantMap *args)
{
    if (!fd.isValid()) {
        fmWarning() << "Invalid file descriptor provided";
        return false;
    }

    int rawFd = fd.fileDescriptor();
    if (rawFd < 0) {
        fmWarning() << "Invalid fd value:" << rawFd;
        return false;
    }

    // Read all data from file descriptor with proper error handling
    QByteArray buffer;
    char readBuffer[1024];
    ssize_t bytesRead;

    while (true) {
        bytesRead = read(rawFd, readBuffer, sizeof(readBuffer));

        if (bytesRead > 0) {
            // Successfully read data
            buffer.append(readBuffer, bytesRead);
        } else if (bytesRead == 0) {
            // EOF reached
            break;
        } else {
            // Error occurred (bytesRead == -1)
            if (errno == EINTR) {
                // Interrupted by signal, retry
                continue;
            }
            // Other errors
            fmCritical() << "Read error from file descriptor, errno:" << errno;
            return false;
        }
    }

    if (buffer.isEmpty()) {
        fmWarning() << "No data received from pipe";
        return false;
    }

    // Deserialize data
    QDataStream stream(&buffer, QIODevice::ReadOnly);
    stream >> *args;

    if (stream.status() != QDataStream::Ok) {
        fmWarning() << "Failed to parse credentials, status:" << stream.status();
        return false;
    }

    fmDebug() << "Successfully parsed credentials from fd";
    return true;
}

bool TPMControlDBus::sendDataViaFd(const QByteArray &data, QDBusUnixFileDescriptor &fd)
{
    // Create pipe
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        fmCritical() << "Failed to create pipe";
        return false;
    }

    // Write data to pipe
    ssize_t written = write(pipefd[1], data.constData(), data.size());
    close(pipefd[1]);  // Close write end immediately

    if (written != data.size()) {
        fmCritical() << "Failed to write data to pipe, written:" << written << "expected:" << data.size();
        close(pipefd[0]);
        return false;
    }

    // Create file descriptor wrapper
    fd = QDBusUnixFileDescriptor(pipefd[0]);
    if (!fd.isValid()) {
        fmCritical() << "Failed to create valid file descriptor";
        close(pipefd[0]);
        return false;
    }

    // Note: pipefd[0] is now owned by QDBusUnixFileDescriptor
    // It will be closed when fd is transmitted over DBus
    close(pipefd[0]);

    fmDebug() << "Successfully created fd for data transmission";
    return true;
}

int TPMControlDBus::IsTPMAvailable()
{
    fmInfo() << "IsTPMAvailable called";

    if (!checkAuthentication(PolicyKitActionId::kQuery)) {
        return kAuthFailed;
    }

    return m_tpmWork->isTPMAvailable();
}

int TPMControlDBus::CheckTPMLockout()
{
    fmInfo() << "CheckTPMLockout called";

    if (!checkAuthentication(PolicyKitActionId::kQuery)) {
        return kAuthFailed;
    }

    return m_tpmWork->checkTPMLockout();
}

int TPMControlDBus::IsSupportAlgo(const QString &algoName, bool &support)
{
    fmInfo() << "IsSupportAlgo called for:" << algoName;

    if (!checkAuthentication(PolicyKitActionId::kQuery)) {
        return kAuthFailed;
    }

    return m_tpmWork->isSupportAlgo(algoName, &support);
}

int TPMControlDBus::OwnerAuthStatus()
{
    fmInfo() << "OwnerAuthStatus called";

    if (!checkAuthentication(PolicyKitActionId::kQuery)) {
        return kAuthFailed;
    }

    return m_tpmWork->ownerAuthStatus();
}

int TPMControlDBus::GetRandom(int size, QDBusUnixFileDescriptor &randomData)
{
    fmInfo() << "GetRandom called with size:" << size;

    if (!checkAuthentication(PolicyKitActionId::kEncrypt)) {
        return kAuthFailed;
    }

    QString output;
    int ret = m_tpmWork->getRandom(size, &output);
    if (ret != 0) {
        fmWarning() << "getRandom failed with code:" << ret;
        return ret;
    }

    // Send random data via file descriptor
    QByteArray data = output.toUtf8();
    if (!sendDataViaFd(data, randomData)) {
        return kFdCreateFailed;
    }

    fmDebug() << "Random data sent via fd successfully";
    return kNoError;
}

int TPMControlDBus::Encrypt(const QDBusUnixFileDescriptor &params)
{
    fmInfo() << "Encrypt called";

    if (!checkAuthentication(PolicyKitActionId::kEncrypt)) {
        return kAuthFailed;
    }

    // Parse parameters from file descriptor
    QVariantMap args;
    if (!parseCredentialsFromFd(params, &args)) {
        fmCritical() << "Failed to parse encrypt parameters from fd";
        return kFdReadFailed;
    }

    return m_tpmWork->encrypt(args);
}

int TPMControlDBus::Decrypt(const QDBusUnixFileDescriptor &params, QDBusUnixFileDescriptor &password)
{
    fmInfo() << "Decrypt called";

    if (!checkAuthentication(PolicyKitActionId::kDecrypt)) {
        return kAuthFailed;
    }

    // Parse parameters from file descriptor
    QVariantMap args;
    if (!parseCredentialsFromFd(params, &args)) {
        fmCritical() << "Failed to parse decrypt parameters from fd";
        return kFdReadFailed;
    }

    QString pwd;
    int ret = m_tpmWork->decrypt(args, &pwd);
    if (ret != 0) {
        fmWarning() << "decrypt failed with code:" << ret;
        return ret;
    }

    // Send decrypted password via file descriptor
    QByteArray data = pwd.toUtf8();
    if (!sendDataViaFd(data, password)) {
        return kFdCreateFailed;
    }

    fmDebug() << "Decrypted password sent via fd successfully";
    return kNoError;
}
