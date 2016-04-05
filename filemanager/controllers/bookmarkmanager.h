#ifndef BOOKMARKMANAGER_H
#define BOOKMARKMANAGER_H

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
#include "bookmark.h"

class FileInfo;

class BookMarkManager : public BaseManager
{
    Q_OBJECT
public:
    explicit BookMarkManager(QObject *parent = 0);
    ~BookMarkManager();
    void load();
    void save();
    QList<BookMark *> getBookmarks();
private:
    void loadJson(const QJsonObject &json);
    void writeJson(QJsonObject &json);
    QList<BookMark *> m_bookmarks;

signals:
    void updates(const QString &directory, const QList<FileInfo*> &infoList);

public slots:
    void writeIntoBookmark(const QString &name, const QString &url);
    void removeBookmark(const QString &name, const QString &url);
    void fetchFileInformation(const QString &url,
                              int filter = int(QDir::AllEntries | QDir::NoDotDot));
};

#endif // BOOKMARKMANAGER_H
