// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VIRTUALENTRYDBHANDLER_H
#define VIRTUALENTRYDBHANDLER_H

#include "dfmplugin_smbbrowser_global.h"
#include "virtualentrydata.h"

#include <dfm-base/base/db/sqlitehandle.h>

#include <QObject>
#include <QList>

DPSMBBROWSER_BEGIN_NAMESPACE

/*
 * The helper store datas like this.
 * it's easy to reuse it in ftp offline entry
 *  | key                      | protocol | host    | port | displayName      |
 *  | smb://1.2.3.4:1234/share | smb      | 1.2.3.4 | 1234 | share on 1.2.3.4 |
 *  | smb://2.3.4.5/share      | smb      | 2.3.4.5 | -1   | share on 2.3.4.5 |
 *  | ftp://3.4.5.6            | ftp      | 3.4.5.6 | -1   | ftp://3.4.5.6    |
 */

class VirtualEntryDbHandler : public QObject
{
    Q_OBJECT

public:
    static VirtualEntryDbHandler *instance();
    ~VirtualEntryDbHandler();

    void clearData();
    void clearData(const QString &stdSmb);
    void removeData(const QString &stdSmb);
    void saveAggregatedAndSperated(const QString &stdSmb, const QString &displayName);
    void saveData(const VirtualEntryData &data);

    bool hasOfflineEntry(const QString &stdSmb);
    QStringList allSmbIDs(QStringList *aggregated = nullptr, QStringList *seperated = nullptr);
    QString getDisplayNameOf(const QUrl &entryUrl);
    QString getFullSmbPath(const QString &stdSmb);
    QList<QSharedPointer<VirtualEntryData>> virtualEntries();

protected:
    bool checkDbExists();
    bool createTable();

private:
    explicit VirtualEntryDbHandler(QObject *parent = nullptr);

    void checkAndUpdateTable();

    DFMBASE_NAMESPACE::SqliteHandle *handler { nullptr };
};

DPSMBBROWSER_END_NAMESPACE

#endif   // VIRTUALENTRYDBHANDLER_H
