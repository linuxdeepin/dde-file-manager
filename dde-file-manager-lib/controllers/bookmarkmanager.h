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
    QList<BookMarkPointer> getBookmarks();
    static QString cachePath();
private:
    void loadJson(const QJsonObject &json);
    void writeJson(QJsonObject &json);
    QList<BookMarkPointer> m_bookmarks;

public slots:
    BookMarkPointer writeIntoBookmark(int index, const QString &name, const DUrl &url);
    void removeBookmark(BookMarkPointer bookmark);
    void renameBookmark(BookMarkPointer bookmark, const QString &newname);
    void moveBookmark(int from, int to);
    // AbstractFileController interface

    void reLoad();

public:
    const QList<DAbstractFileInfoPointer> getChildren(const QSharedPointer<DFMGetChildrensEvent> &event) const Q_DECL_OVERRIDE;
    const DAbstractFileInfoPointer createFileInfo(const QSharedPointer<DFMCreateFileInfoEvnet> &event) const Q_DECL_OVERRIDE;
};

#endif // BOOKMARKMANAGER_H
