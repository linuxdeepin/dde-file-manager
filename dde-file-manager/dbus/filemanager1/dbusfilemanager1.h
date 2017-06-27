#ifndef DBUSFILEMANAGER1_H
#define DBUSFILEMANAGER1_H

#include <QObject>

class DBusFileManager1 : public QObject
{
    Q_OBJECT

public:
    explicit DBusFileManager1(QObject *parent = 0);

    void ShowFolders(const QStringList &URIs, const QString &StartupId);
    void ShowItemProperties(const QStringList &URIs, const QString &StartupId);
    void ShowItems(const QStringList &URIs, const QString &StartupId);

    // debug function
    QStringList GetMonitorFiles() const;
};

#endif // DBUSFILEMANAGER1_H
