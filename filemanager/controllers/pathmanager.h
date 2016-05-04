#ifndef PATHMANAGER_H
#define PATHMANAGER_H

#include <QObject>
#include <QMap>
#include <QFileSystemWatcher>
#include "basemanager.h"
#include "../models/durl.h"

class PathManager : public QObject ,public BaseManager
{
    Q_OBJECT

public:
    explicit PathManager(QObject *parent = 0);
    ~PathManager();

    void initPaths();
    void initConnect();

    QString getSystemPath(QString key);
    QString getSystemPathDisplayName(QString key);
    static QString getSystemCachePath();

    QMap<QString, QString> systemPaths() const;

signals:

public slots:

    void saveSystemPaths();
    void loadSystemPaths();
    void mkPath(const QString& path);
    void handleDirectoryChanged(const QString& path);

private:
    QMap<QString, QString> m_systemPaths;
    QFileSystemWatcher* m_fileSystemWatcher = NULL;
};

#endif // PATHMANAGER_H
