#ifndef BOOKMARKMANAGER_H
#define BOOKMARKMANAGER_H

#include "basemanager.h"
#include "bookmark.h"
#include "abstractfilecontroller.h"

#include <QDir>

class AbstractFileInfo;

class BookMarkManager : public AbstractFileController, public BaseManager
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

public slots:
    BookMark *writeIntoBookmark(int index, const QString &name, const DUrl &url);
    void removeBookmark(const QString &name, const DUrl &url);

    // AbstractFileController interface
public:
    const QList<AbstractFileInfoPointer> getChildren(const DUrl &fileUrl, QDir::Filters filter, bool &accepted) const;
    AbstractFileInfoPointer createFileInfo(const DUrl &fileUrl, bool &accepted) const;
};

#endif // BOOKMARKMANAGER_H
