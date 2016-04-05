#ifndef RECENTHISTORYMANAGER_H
#define RECENTHISTORYMANAGER_H

#include <QObject>
#include <QList>
#include <QJsonObject>
#include <QFile>
#include <stdlib.h>
#include <QJsonDocument>
#include <QJsonArray>
#include <QByteArray>
#include <QDateTime>
#include <QDir>
#include <QUrl>

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

private:
    void loadJson(const QJsonObject &json);
    void writeJson(QJsonObject &json);

public slots:
    void fetchFileInformation(const QString &url,
                              int filter = int(QDir::AllEntries | QDir::NoDotDot));

signals:
    void updates(const QString &directory, const QList<FileInfo*> &infoList);
    void fileCreated(const QString &path);

private slots:
    void addOpenedFile(const QString &url);

private:
    QList<QString> openedFileList;
};

#endif // RECENTHISTORYMANAGER_H
