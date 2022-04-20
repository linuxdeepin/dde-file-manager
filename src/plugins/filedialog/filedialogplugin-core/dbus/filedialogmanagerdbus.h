/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
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

    QMap<QDBusObjectPath, QObject *> curDialogObjectMap;
};

#endif   // FILEDIALOGMANAGERDBUS_H
