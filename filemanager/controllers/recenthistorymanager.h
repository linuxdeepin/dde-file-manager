#ifndef RECENTHISTORYMANAGER_H
#define RECENTHISTORYMANAGER_H

#include <QObject>
#include <QDir>

#include "basemanager.h"

class FileInfo;

class RecentHistoryManager : public BaseManager
{
    Q_OBJECT
public:
    explicit RecentHistoryManager(QObject *parent = 0);
    ~RecentHistoryManager();

    void load() Q_DECL_OVERRIDE;
    void save() Q_DECL_OVERRIDE;

public slots:
    void fetchFileInformation(const QString &url,
                              int filter = int(QDir::AllEntries | QDir::NoDotDot));

signals:
    void updates(const QString &directory, const QList<FileInfo*> &infoList);
    void addFileInfo(const QString &path, FileInfo* info);
};

#endif // RECENTHISTORYMANAGER_H
