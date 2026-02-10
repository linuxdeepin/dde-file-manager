// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "virtualentrydbhandler.h"
#include "displaycontrol/info/protocolvirtualentryentity.h"
#include "displaycontrol/utilities/protocoldisplayutilities.h"

#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/base/application/settings.h>
#include <dfm-base/base/standardpaths.h>

#include <dfm-io/dfmio_utils.h>
#include <unistd.h>

DPSMBBROWSER_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

VirtualEntryDbHandler *VirtualEntryDbHandler::instance()
{
    static VirtualEntryDbHandler ins;
    return &ins;
}

VirtualEntryDbHandler::~VirtualEntryDbHandler()
{
    if (handler)
        delete handler;
    handler = nullptr;
}

void VirtualEntryDbHandler::clearData()
{
    Q_ASSERT(handler);

    fmDebug() << "clear all virtual entry:" << handler->dropTable<VirtualEntryData>();
}

void VirtualEntryDbHandler::clearData(const QString &stdSmb)
{
    Q_ASSERT(handler);

    VirtualEntryData data;
    data.setKey(stdSmb);
    fmDebug() << "remove virtual entry:" << handler->remove<VirtualEntryData>(data) << stdSmb;
}

void VirtualEntryDbHandler::removeData(const QString &stdSmb)
{
    Q_ASSERT(handler);

    const auto &field = Expression::Field<VirtualEntryData>;
    handler->remove<VirtualEntryData>(field("key") == stdSmb);

    // if last share of host is removed, remove the host entry from db.
    QStringList allSeperatedItem;
    allSmbIDs(nullptr, &allSeperatedItem);

    const QString &smbHost = protocol_display_utilities::getSmbHostPath(stdSmb);
    bool notLast = std::any_of(allSeperatedItem.cbegin(), allSeperatedItem.cend(),
                               [smbHost](const QString &smb) { return smb.startsWith(smbHost + "/"); });
    if (!notLast) {
        handler->remove<VirtualEntryData>(field("key") == smbHost);
        fmDebug() << "remove host entry:" << smbHost;
    }
}

void VirtualEntryDbHandler::saveAggregatedAndSperated(const QString &stdSmb, const QString &displayName)
{
    // seperate entry
    VirtualEntryData data(stdSmb);
    data.setDisplayName(displayName);
    {
        QString key(stdSmb);
        while (key.endsWith("/"))
            key.chop(1);
        static QString kRecordFilePath = QString("/tmp/dfm_smb_mount_%1.ini").arg(getuid());
        static QString kRecordGroup = "defaultSmbPath";
        static QRegularExpression kRegx { "/|\\.|:" };
        key = key.replace(kRegx, "_");
        QSettings sets(kRecordFilePath, QSettings::IniFormat);
        data.setTargetPath(sets.value(QString("%1/%2").arg(kRecordGroup).arg(key), "").toString());
    }
    saveData(data);
    data.setTargetPath("");

    data.setKey(protocol_display_utilities::getSmbHostPath(stdSmb));
    data.setDisplayName(data.getHost());
    saveData(data);
}

void VirtualEntryDbHandler::saveData(const VirtualEntryData &data)
{
    Q_ASSERT(handler);

    createTable();
    bool inserted = handler->insert<VirtualEntryData>(data, true) >= 0;
    if (!inserted) {
        // do update.
        const auto &field = Expression::Field<VirtualEntryData>;
        handler->update<VirtualEntryData>(field("targetPath") = data.getTargetPath(),
                                          field("key") == data.getKey());
    }
}

QList<QSharedPointer<VirtualEntryData>> VirtualEntryDbHandler::virtualEntries()
{
    Q_ASSERT(handler);

    auto ret = handler->query<VirtualEntryData>().toBeans();
    fmDebug() << "query all virtual entries:" << ret.count();
    return ret;
}

bool VirtualEntryDbHandler::hasOfflineEntry(const QString &stdSmb)
{
    const auto &allOfflined = allSmbIDs();
    return allOfflined.contains(stdSmb);
}

QStringList VirtualEntryDbHandler::allSmbIDs(QStringList *aggregated, QStringList *seperated)
{
    auto allEntries = virtualEntries();
    QStringList lst;
    std::for_each(allEntries.cbegin(), allEntries.cend(),
                  [&](const QSharedPointer<VirtualEntryData> data) {
                      lst.append(data->getKey());
                      if (aggregated && data->getHost() == data->getDisplayName())
                          aggregated->append(data->getKey());
                      if (seperated && data->getHost() != data->getDisplayName())
                          seperated->append(data->getKey());
                  });
    return lst;
}

QString VirtualEntryDbHandler::getDisplayNameOf(const QUrl &entryUrl)
{
    QString path = entryUrl.path();
    path.remove("." + QString(kVEntrySuffix));   // ==> smb://1.2.3.4/hello/
    QUrl u(path);
    if (u.path().isEmpty()) {
        fmDebug() << "Empty path for URL, returning host:" << u.host();
        return u.host();
    }

    Q_ASSERT(handler);
    const auto &field = Expression::Field<VirtualEntryData>;
    auto data = handler->query<VirtualEntryData>().where(field("key") == path).toBean();
    if (data) {
        fmDebug() << "Found display name for path:" << path << "name:" << data->getDisplayName();
        return data->getDisplayName();
    }

    fmWarning() << "No display name found for path:" << path;
    return "";
}

QString VirtualEntryDbHandler::getFullSmbPath(const QString &stdSmb)
{
    Q_ASSERT(handler);
    const auto &field = Expression::Field<VirtualEntryData>;
    auto data = handler->query<VirtualEntryData>().where(field("key") == stdSmb).toBean();
    if (data)
        return stdSmb + data->getTargetPath();
    return stdSmb;
}

bool VirtualEntryDbHandler::checkDbExists()
{
    using namespace dfmio;
    const auto &dbPath = DFMUtils::buildFilePath(StandardPaths::location(StandardPaths::kApplicationConfigPath).toLocal8Bit(),
                                                 "/deepin/dde-file-manager/database",
                                                 nullptr);

    QDir dir(dbPath);
    if (!dir.exists())
        dir.mkpath(dbPath);

    const auto &dbFilePath = DFMUtils::buildFilePath(dbPath.toLocal8Bit(),
                                                     Global::DataBase::kDfmDBName,
                                                     nullptr);
    handler = new SqliteHandle(dbFilePath);
    QSqlDatabase db { SqliteConnectionPool::instance().openConnection(dbFilePath) };
    if (!db.isValid() || db.isOpenError()) {
        fmCritical() << "The database is invalid! open error for path:" << dbFilePath;
        return false;
    }
    db.close();

    return true;
}

bool VirtualEntryDbHandler::createTable()
{
    Q_ASSERT(handler);

    return handler->createTable<VirtualEntryData>(SqliteConstraint::primary("key"),
                                                  SqliteConstraint::unique("key"));
}

VirtualEntryDbHandler::VirtualEntryDbHandler(QObject *parent)
    : QObject(parent)
{
    fmDebug() << "start checking db info";
    checkDbExists();
    fmDebug() << "end checking db info";
    fmDebug() << "start checking db struct";
    checkAndUpdateTable();
    fmDebug() << "end checking db struct";
}

void VirtualEntryDbHandler::checkAndUpdateTable()
{
    Q_ASSERT(handler);
    QString tableName = SqliteHelper::tableName<VirtualEntryData>();

    auto alterTable = [=] {
        bool ret = handler->excute(QString("ALTER TABLE %1 ADD COLUMN targetPath TEXT")
                                           .arg(tableName));
        fmInfo() << "alter table: " << ret;
    };
    handler->excute(QString("PRAGMA table_info(%1)").arg(tableName), [=](QSqlQuery *query) {
        while (query->next()) {
            if (query->value(1).toString() == "targetPath") {
                fmDebug() << "Table structure is up to date, targetPath column exists";
                return;
            }
        }
        alterTable();
    });
}
