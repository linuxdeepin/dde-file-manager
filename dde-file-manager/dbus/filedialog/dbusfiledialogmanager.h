/*
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

#ifndef DBUSFILEDIALOGMANAGER_H
#define DBUSFILEDIALOGMANAGER_H

#include <QObject>
#include <QDBusObjectPath>

class DBusFileDialogManager : public QObject
{
public:
    explicit DBusFileDialogManager(QObject *parent = 0);

    QDBusObjectPath createDialog(QString key);
    void destroyDialog(const QDBusObjectPath &path);
    QList<QDBusObjectPath> dialogs() const;
    QString errorString() const;

    bool isUseFileChooserDialog() const;
    bool canUseFileChooserDialog(const QString &group, const QString &executableFileName) const;

    QStringList globPatternsForMime(const QString &mimeType) const;

private:
    void onDialogDestroy();

    QString m_errorString;
    QMap<QDBusObjectPath, QObject*> m_dialogObjectMap;
};

#endif // DBUSFILEDIALOGMANAGER_H
