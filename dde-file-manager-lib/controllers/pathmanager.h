#ifndef PATHMANAGER_H
#define PATHMANAGER_H

#include <QObject>
#include <QMap>
#include <QSet>
#include <QFileSystemWatcher>

#include "basemanager.h"
#include "durl.h"

class PathManager : public QObject, public BaseManager
{
    Q_OBJECT

public:
    explicit PathManager(QObject *parent = 0);
    ~PathManager();

    void initPaths();
    void initConnect();

    QString getSystemPath(QString key);
    QString getSystemPathDisplayName(QString key);
    QString getSystemPathDisplayNameByPath(const QString& path);
    QString getSystemPathIconName(QString key);
    QString getSystemPathIconNameByPath(const QString& path);

    static QString getSystemCachePath();

    QMap<QString, QString> systemPathsMap() const;
    QMap<QString, QString> systemPathDisplayNamesMap() const;

    inline bool isSystemPath(const QString& path)
    { return m_systemPathsSet.contains(path);}

public slots:
    void loadSystemPaths();
    void mkPath(const QString& path);
    void handleDirectoryChanged(const QString& path);

private:
    QMap<QString, QString> m_systemPathsMap;
    QMap<QString, QString> m_systemPathDisplayNamesMap;
    QMap<QString, QString> m_systemPathIconNamesMap;
    QSet<QString> m_systemPathsSet;
    QFileSystemWatcher* m_fileSystemWatcher = NULL;
};

#endif // PATHMANAGER_H
