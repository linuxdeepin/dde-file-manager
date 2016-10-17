#ifndef BOOKMARKMANAGER_H
#define BOOKMARKMANAGER_H

#include "basemanager.h"
#include "dabstractfilecontroller.h"

#include "models/bookmark.h"

#include <QDir>

class DAbstractFileInfo;

class BookMarkManager : public DAbstractFileController, public BaseManager
{
    Q_OBJECT
public:
    explicit BookMarkManager(QObject *parent = 0);
    ~BookMarkManager();
    void load();
    void save();
    QList<BookMark *> getBookmarks();
    static QString cachePath();
private:
    void loadJson(const QJsonObject &json);
    void writeJson(QJsonObject &json);
    QList<BookMark *> m_bookmarks;

public slots:
    BookMark *writeIntoBookmark(int index, const QString &name, const DUrl &url);
    void removeBookmark(BookMark* bookmark);
    void renameBookmark(BookMark* bookmark, const QString &newname);
    void moveBookmark(int from, int to);
    // AbstractFileController interface
public:
    const QList<AbstractFileInfoPointer> getChildren(const DUrl &fileUrl, const QStringList &nameFilters,
                                                     QDir::Filters filters, QDirIterator::IteratorFlags flags,
                                                     bool &accepted) const Q_DECL_OVERRIDE;
    const AbstractFileInfoPointer createFileInfo(const DUrl &fileUrl, bool &accepted) const Q_DECL_OVERRIDE;
};

#endif // BOOKMARKMANAGER_H
