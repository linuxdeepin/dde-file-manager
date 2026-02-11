// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILEDIALOGMANAGERDBUS_H
#define FILEDIALOGMANAGERDBUS_H

#include <QObject>
#include <QDBusObjectPath>

class FileDialogManagerDBus : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.filemanager.filedialog")

public:
    explicit FileDialogManagerDBus(QObject *parent = nullptr);

public slots:
    /// NOTE:For interface compatibility,
    /// the following interfaces cannot be changed even if they are not standardized!

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
    void onAppExit();
    void initEventsFilter();

    QMap<QDBusObjectPath, QObject *> curDialogObjectMap;
    bool lastWindowClosed { false };
};

#endif   // FILEDIALOGMANAGERDBUS_H
