// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
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

    pddmDbg << "clear all virtual entry:" << handler->dropTable<VirtualEntryData>();
}

void VirtualEntryDbHandler::clearData(const QString &stdSmb)
{
    Q_ASSERT(handler);

    VirtualEntryData data;
    data.setKey(stdSmb);
    pddmDbg << "remove virtual entry:" << handler->remove<VirtualEntryData>(data) << stdSmb;
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
        pddmDbg << "remove host entry:" << smbHost;
    }
}

void VirtualEntryDbHandler::saveAggregatedAndSperated(const QString &stdSmb, const QString &displayName)
{
    VirtualEntryData data(stdSmb);
    data.setDisplayName(displayName);
    saveData(data);

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
    }
}

QList<QSharedPointer<VirtualEntryData>> VirtualEntryDbHandler::virtualEntries()
{
    Q_ASSERT(handler);

    auto ret = handler->query<VirtualEntryData>().toBeans();
    pddmDbg << "query all virtual entries:" << ret.count();
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
    if (u.path().isEmpty())
        return u.host();

    Q_ASSERT(handler);
    const auto &field = Expression::Field<VirtualEntryData>;
    auto data = handler->query<VirtualEntryData>().where(field("key") == path).toBean();
    if (data)
        return data->getDisplayName();
    return "";
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
        pddmWar << "The database is invalid! open error";
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
    pddmDbg << "start checking db info";
    checkDbExists();
    pddmDbg << "end checking db info";
}
