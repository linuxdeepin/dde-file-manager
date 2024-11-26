// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TEXTINDEXDBUS_H
#define TEXTINDEXDBUS_H

#include "service_textindex_global.h"

#include <QObject>
#include <QDBusContext>

SERVICETEXTINDEX_BEGIN_NAMESPACE
class TextIndexDBusPrivate;
SERVICETEXTINDEX_END_NAMESPACE

class TextIndexDBus : public QObject, public QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.deepin.Filemanager.TextIndex")

public:
    explicit TextIndexDBus(const char *name, QObject *parent = nullptr);
    ~TextIndexDBus();

public Q_SLOTS:
    bool CreateIndexTask(const QString &path);
    bool UpdateIndexTask(const QString &path);
    bool StopCurrentTask();
    bool HasRunningTask();
    bool IndexDatabaseExists();

Q_SIGNALS:
    void CreateFailed();
    void CreateSuccessful();
    void CreateIndexCountChanged(int);
    void UpdateFailed();
    void UpdateSuccessful();
    void UpdateIndexCountChanged(int);

private:
    QScopedPointer<SERVICETEXTINDEX_NAMESPACE::TextIndexDBusPrivate> d;
};

#endif   // TEXTINDEXDBUS_H
