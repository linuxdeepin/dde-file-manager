// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "smbvirtualentryupgradeunit.h"

#include <dfm-base/dfm_global_defines.h>

#include <QStandardPaths>
#include <QDir>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonParseError>

Q_DECLARE_LOGGING_CATEGORY(logToolUpgrade)

using namespace dfm_upgrade;
DFMBASE_USE_NAMESPACE

static constexpr char kConfigFile[] { "/deepin/dde-file-manager.json" };

SmbVirtualEntryUpgradeUnit::SmbVirtualEntryUpgradeUnit()
    : UpgradeUnit()
{
}

SmbVirtualEntryUpgradeUnit::~SmbVirtualEntryUpgradeUnit()
{
    if (handler)
        delete handler;
    handler = nullptr;
}

QString SmbVirtualEntryUpgradeUnit::name()
{
    return "SmbVirtualEntryUpgradeUnit";
}

bool SmbVirtualEntryUpgradeUnit::initialize(const QMap<QString, QString> &)
{
    return true;
}

bool SmbVirtualEntryUpgradeUnit::upgrade()
{
    if (!createDB()) {
        qCCritical(logToolUpgrade) << "Failed to create database for SMB virtual entries";
        return false;
    }

    const QList<VirtualEntryData> &&old = readOldItems();
    saveToDb(old);
    clearOldItems();

    return true;
}

bool SmbVirtualEntryUpgradeUnit::createDB()
{
    const QString &dbPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/deepin/dde-file-manager/database";

    QDir dir(dbPath);
    if (!dir.exists()) {
        qCDebug(logToolUpgrade) << "Database directory does not exist, creating:" << dbPath;
        dir.mkpath(dbPath);
    }

    const QString &dbFilePath = dbPath + "/" + DFMBASE_NAMESPACE::Global::DataBase::kDfmDBName;
    handler = new SqliteHandle(dbFilePath);
    QSqlDatabase db { SqliteConnectionPool::instance().openConnection(dbFilePath) };
    if (!db.isValid() || db.isOpenError()) {
        qCWarning(logToolUpgrade) << "The database is invalid! open error";
        return false;
    }
    db.close();

    return true;
}

bool SmbVirtualEntryUpgradeUnit::createTable()
{
    Q_ASSERT(handler);

    return handler->createTable<VirtualEntryData>(SqliteConstraint::primary("key"),
                                                  SqliteConstraint::unique("key"));
}

QList<VirtualEntryData> SmbVirtualEntryUpgradeUnit::readOldItems()
{
    const QString configPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + kConfigFile;
    QFile config(configPath);
    if (!config.open(QIODevice::ReadOnly)) {
        qCWarning(logToolUpgrade) << "Failed to open configuration file for reading:" << configPath;
        return {};
    }

    const auto &configs = config.readAll();
    config.close();

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(configs, &err);
    if (err.error != QJsonParseError::NoError) {
        qCWarning(logToolUpgrade) << "cannot parse config file:" << err.errorString();
        return {};
    }

    QList<VirtualEntryData> rets;
    QJsonObject rootObj = doc.object();
    if (rootObj.contains("RemoteMounts")) {
        const QJsonObject &remoteMountsObj = rootObj.value("RemoteMounts").toObject();
        std::for_each(remoteMountsObj.constBegin(), remoteMountsObj.constEnd(), [&](const QJsonValue &val) {
            if (!val.isObject()) {
                qCDebug(logToolUpgrade) << "Skipping non-object value in RemoteMounts";
                return;
            }

            const auto &data = convertFromMap(val.toObject().toVariantMap());
            if (data.getHost().isEmpty()) {
                qCDebug(logToolUpgrade) << "Skipping remote mount entry with empty host";
                return;
            }
            rets.append(data);
        });
    }

    if (rootObj.contains("StashedSmbDevices")) {
        const QJsonObject &stashedEntry = rootObj.value("StashedSmbDevices").toObject();
        if (stashedEntry.contains("SmbIntegrations")) {
            const QJsonArray &aggregatedList = stashedEntry.value("SmbIntegrations").toArray();
            std::for_each(aggregatedList.begin(), aggregatedList.end(), [&](const QJsonValue &val) {
                VirtualEntryData data(val.toString());
                if (data.getKey().isEmpty()) {
                    qCDebug(logToolUpgrade) << "Skipping SMB integration entry with empty key";
                    return;
                }
                rets.append(data);
            });
        }
    }

    return rets;
}

void SmbVirtualEntryUpgradeUnit::clearOldItems()
{
    QFile config(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + kConfigFile);
    if (!config.open(QIODevice::ReadOnly)) {
        qCWarning(logToolUpgrade) << "Failed to open configuration file for reading during cleanup";
        return;
    }

    const auto &configs = config.readAll();
    config.close();

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(configs, &err);
    if (err.error != QJsonParseError::NoError) {
        qCWarning(logToolUpgrade) << "cannot parse config file:" << err.errorString();
        return;
    }

    auto rootNode = doc.object();
    rootNode.remove("RemoteMounts");
    rootNode.remove("StashedSmbDevices");
    doc.setObject(rootNode);

    if (!config.open(QIODevice::ReadWrite | QIODevice::Truncate)) {
        qCWarning(logToolUpgrade) << "Failed to open configuration file for writing during cleanup";
        return;
    }

    config.write(doc.toJson());
    config.close();
}

VirtualEntryData SmbVirtualEntryUpgradeUnit::convertFromMap(const QVariantMap &map)
{
    VirtualEntryData data;
    const QString &protocol = map.value("protocol", "").toString();
    const QString &host = map.value("host", "").toString();
    const QString &share = map.value("share", "").toString();
    const QString &name = map.value("name", "").toString();
    if (protocol.isEmpty() || host.isEmpty() || share.isEmpty()) {
        qCWarning(logToolUpgrade) << "Invalid SMB entry data - missing required fields";
        return data;
    }

    qCDebug(logToolUpgrade) << "upgrade: smb entry: " << protocol << host
                            << share << name;

    data.setDisplayName(name);
    data.setHost(host);
    data.setProtocol(protocol);
    data.setKey(QString("%1://%2/%3/").arg(protocol).arg(host).arg(share));
    return data;
}

void SmbVirtualEntryUpgradeUnit::saveToDb(const QList<VirtualEntryData> &entries)
{
    createTable();
    std::for_each(entries.cbegin(), entries.cend(), [=](const VirtualEntryData &entry) {
        handler->insert<VirtualEntryData>(entry, true);
    });
}
