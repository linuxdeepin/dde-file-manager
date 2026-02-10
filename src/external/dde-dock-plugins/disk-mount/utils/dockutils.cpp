// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dockutils.h"
#include "global_server_defines.h"

#include <DConfig>

#include <QLoggingCategory>
#include <QStringList>
#include <QProcess>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QFileInfo>
#include <QDir>
#include <QUrl>
#include <QIcon>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>

#include <libmount/libmount.h>

Q_LOGGING_CATEGORY(logAppDock, "org.deepin.dde.dock.plugin.disk-mount")
DCORE_USE_NAMESPACE

bool common_utils::isIntegratedByFilemanager()
{
    std::unique_ptr<DConfig> cfg {
        DConfig::create("org.deepin.dde.dock", "org.deepin.dde.dock.plugin.diskmount", "")
    };
    if (!cfg || !cfg->isValid()) {
        qCWarning(logAppDock) << "Failed to create or validate DConfig, using default integration";
        return true;
    }

    return cfg->value("filemanager-integration").toBool();
}

QString size_format::formatDiskSize(const quint64 num)
{
    QStringList list { " B", " KB", " MB", " GB", " TB" };
    qreal fileSize(num);

    QStringListIterator i(list);
    QString unit = i.next();

    int index = 0;
    while (i.hasNext()) {
        if (fileSize < 1024) {
            break;
        }

        unit = i.next();
        fileSize /= 1024;
        index++;
    }

    return QString("%1%2").arg(sizeString(QString::number(fileSize, 'f', 1)), unit);
}

QString size_format::sizeString(const QString &str)
{
    int beginPos = str.indexOf('.');

    if (beginPos < 0)
        return str;

    QString size = str;

    while (size.count() - 1 > beginPos) {
        if (!size.endsWith('0'))
            return size;

        size = size.left(size.count() - 1);
    }

    return size.left(size.count() - 1);
}

bool smb_utils::parseSmbInfo(const QString &smbPath, QString *host, QString *share, int *port)
{
    Q_ASSERT(host && share && port);
    static const QRegularExpression regx(R"(([:,]port=(?<port>\d*))?[,:]server=(?<host>[^/:,]+)(,share=(?<share>[^/:,]+))?)");
    auto match = regx.match(smbPath);
    if (!match.hasMatch()) {
        qCWarning(logAppDock) << "Failed to parse SMB info from path:" << smbPath;
        return false;
    }

    *host = match.captured("host");
    *share = match.captured("share");
    QString portStr = match.captured("port");
    *port = portStr.isEmpty() ? -1 : portStr.toInt();
    return true;
}

QString device_utils::blockDeviceName(const QVariantMap &data)
{
    QString idLabel = data.value(GlobalServerDefines::DeviceProperty::kIdLabel).toString();
    if (!idLabel.isEmpty()) {
        qCDebug(logAppDock) << "Using ID label as device name:" << idLabel;
        return idLabel;
    }

    quint64 size = data.value(GlobalServerDefines::DeviceProperty::kSizeTotal).toULongLong();
    return QObject::tr("%1 Volume").arg(size_format::formatDiskSize(size));
}

QString device_utils::protocolDeviceName(const QVariantMap &data)
{
    QString devName = data.value(GlobalServerDefines::DeviceProperty::kDisplayName).toString();
    QString host, share;
    int port;
    if (smb_utils::parseSmbInfo(devName, &host, &share, &port)) {
        devName = QObject::tr("%1 on %2").arg(share).arg(host);
        qCDebug(logAppDock) << "Updated SMB device name:" << devName;
    }

    return devName;
}

QString device_utils::blockDeviceIcon(const QVariantMap &data)
{
    if (data.value(GlobalServerDefines::DeviceProperty::kCryptoBackingDevice).toString() != "/")
        return "drive-removable-media-encrypted";
    if (data.value(GlobalServerDefines::DeviceProperty::kOpticalDrive).toBool())
        return "media-optical";
    return "drive-removable-media-usb";
}

QString device_utils::protocolDeviceIcon(const QVariantMap &data)
{
    auto iconLst = data.value(GlobalServerDefines::DeviceProperty::kDeviceIcon).toStringList();
    for (auto name : iconLst) {
        auto icon = QIcon::fromTheme(name);
        if (!icon.isNull())
            return name;
    }
    return "drive-network";
}

QUrl device_utils::blockDeviceTarget(const QVariantMap &data)
{
    if (data.value(GlobalServerDefines::DeviceProperty::kOpticalDrive).toBool()) {
        QString device = data.value(GlobalServerDefines::DeviceProperty::kDevice).toString();
        QUrl target;
        target.setScheme("burn");
        target.setPath(QString("%1/disc_files/").arg(device));
        return target;
    }
    QString mpt = data.value(GlobalServerDefines::DeviceProperty::kMountPoint).toString();
    return QUrl::fromLocalFile(mpt);
}

QUrl device_utils::protocolDeviceTarget(const QVariantMap &data)
{
    QString mpt = data.value(GlobalServerDefines::DeviceProperty::kMountPoint).toString();
    QString host, share;
    int port;
    if (smb_utils::parseSmbInfo(mpt, &host, &share, &port)) {
        QUrl target;
        target.setScheme("smb");
        target.setHost(host);
        target.setPort(port);
        target.setPath("/" + share);
        return target;
    }
    return QUrl::fromLocalFile(mpt);
}

bool device_utils::isDlnfsMount(const QString &mpt)
{
    auto unifyPath = [](const QString &path) {
        return path.endsWith("/") ? path : path + "/";
    };

    const QString &_mpt = unifyPath(mpt);

    libmnt_table *tab = mnt_new_table();
    libmnt_iter *iter = mnt_new_iter(MNT_ITER_BACKWARD);

    int ret = mnt_table_parse_mtab(tab, nullptr);
    if (ret != 0) {
        qCWarning(logAppDock) << "Failed to parse mtab for DLNFS check, error code:" << ret;
        mnt_free_table(tab);
        mnt_free_iter(iter);
        return false;
    }

    libmnt_fs *fs = nullptr;
    while (mnt_table_next_fs(tab, iter, &fs) == 0) {
        if (fs && strcmp("dlnfs", mnt_fs_get_source(fs)) == 0) {
            QString target = unifyPath(mnt_fs_get_target(fs));
            if (target == _mpt) {
                mnt_free_table(tab);
                mnt_free_iter(iter);
                return true;
            }
        }
    }

    mnt_free_table(tab);
    mnt_free_iter(iter);
    return false;
}

QString device_utils::queryDevice(const QString &mpt)
{
    auto unifyPath = [](const QString &path) {
        QString _path = path;
        while (_path.endsWith("/") && _path.length() > 1)
            _path.chop(1);
        return _path;
    };

    const QString &_mpt = unifyPath(mpt);

    libmnt_table *tab = mnt_new_table();
    libmnt_iter *iter = mnt_new_iter(MNT_ITER_BACKWARD);

    int ret = mnt_table_parse_mtab(tab, nullptr);
    if (ret != 0) {
        qCWarning(logAppDock) << "Failed to parse mtab for device query, error code:" << ret << "mount point:" << _mpt;
        mnt_free_table(tab);
        mnt_free_iter(iter);
        return "";
    }

    libmnt_fs *fs = nullptr;
    while (mnt_table_next_fs(tab, iter, &fs) == 0) {
        if (fs && strcmp(_mpt.toStdString().c_str(), mnt_fs_get_target(fs)) == 0) {
            QString source = mnt_fs_get_source(fs);
            mnt_free_table(tab);
            mnt_free_iter(iter);
            return source;
        }
    }

    mnt_free_table(tab);
    mnt_free_iter(iter);
    return "";
}

QString device_utils::protocolDeviceAlias(const QString &scheme, const QString &host)
{
    if (scheme.isEmpty() || host.isEmpty()) {
        qCDebug(logAppDock) << "Invalid protocol URL - missing scheme or host:" << scheme << host;
        return "";
    }

    static QString cfgPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)
            + "/deepin/dde-file-manager.json";

    QFile file(cfgPath);
    if (!file.open(QIODevice::ReadOnly)) {
        qCDebug(logAppDock) << "Failed to open config file:" << cfgPath;
        return "";
    }

    QJsonParseError jpe;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &jpe);
    if (jpe.error != QJsonParseError::NoError) {
        qCWarning(logAppDock) << "Failed to parse JSON config file:" << jpe.errorString();
        return "";
    }

    QJsonObject rootObj = doc.object();
    QJsonObject protocolAliasObj = rootObj.value("NetworkProtocolDeviceAlias").toObject();
    QJsonArray itemsArray = protocolAliasObj.value("Items").toArray();

    // Search for matching scheme and host
    for (const QJsonValue &item : itemsArray) {
        QJsonObject itemObj = item.toObject();
        QString itemScheme = itemObj.value("scheme").toString();
        if (itemScheme != scheme)
            continue;

        QJsonArray devicesArray = itemObj.value("devices").toArray();
        for (const QJsonValue &device : devicesArray) {
            QJsonObject deviceObj = device.toObject();
            QString deviceHost = deviceObj.value("host").toString();

            if (deviceHost == host) {
                QString alias = deviceObj.value("alias").toString();
                return alias;
            }
        }
    }

    return "";
}
