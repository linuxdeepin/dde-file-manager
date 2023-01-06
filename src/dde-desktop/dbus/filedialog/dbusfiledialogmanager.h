// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DBUSFILEDIALOGMANAGER_H
#define DBUSFILEDIALOGMANAGER_H

#include <QObject>
#include <QDBusObjectPath>

class DBusFileDialogManager : public QObject
{
public:
    explicit DBusFileDialogManager(QObject *parent = nullptr);

    QDBusObjectPath createDialog(QString key);
    void destroyDialog(const QDBusObjectPath &path);
    QList<QDBusObjectPath> dialogs() const;
    QString errorString() const;

    bool isUseFileChooserDialog() const;
    bool canUseFileChooserDialog(const QString &group, const QString &executableFileName) const;

    QStringList globPatternsForMime(const QString &mimeType) const;
    QStringList monitorFiles() const;

    void showBluetoothTransDialog(const QString &id, const QStringList &URIs);

private:
    void onDialogDestroy();

    bool initJobDone = false;

    QString m_errorString;
    QMap<QDBusObjectPath, QObject*> m_dialogObjectMap;
};

#endif // DBUSFILEDIALOGMANAGER_H
