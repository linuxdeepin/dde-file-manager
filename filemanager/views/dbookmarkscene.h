#ifndef DBOOKMARKSCENE_H
#define DBOOKMARKSCENE_H

#include <QGraphicsScene>
#include <QGraphicsWidget>
#include <QGraphicsLinearLayout>

#define BOOKMARK_ITEM_HEIGHT 30
#define SEPARATOR_ITEM_HEIGHT 6
#define BOOKMARK_ITEM_WIDTH 200
#define BOOKMARK_ITEM_SPACE 5

class DBookmarkItem;
class DBookmarkRootItem;
class DBookmarkItemGroup;
class FMEvent;
class UDiskDeviceInfo;
class DUrl;
class DiskInfo;

class DBookmarkScene : public QGraphicsScene
{
    Q_OBJECT
public:
    DBookmarkScene();
    void addBookmark(DBookmarkItem *item);
    void insertBookmark(int index, DBookmarkItem *item);
    void addItem(DBookmarkItem *item);
    void addDefaultBookmark(DBookmarkItem *item);
    void insert(int index, DBookmarkItem *item);
    void insert(DBookmarkItem * before, DBookmarkItem *item);
    void remove(int index);
    void remove(DBookmarkItem * item);
    void setSceneRect(qreal x, qreal y, qreal w, qreal h);
    void addSeparator();
    DBookmarkItemGroup * getGroup();
    int count();
    int windowId();
    DBookmarkItem * hasBookmarkItem(const DUrl &url);
    DBookmarkItem * itemAt(const QPointF &point);
    int indexOf(DBookmarkItem * item);
    void setTightMode(bool v);
protected:
    void dragEnterEvent(QGraphicsSceneDragDropEvent *event);
    void dragLeaveEvent(QGraphicsSceneDragDropEvent *event);
    void dragMoveEvent(QGraphicsSceneDragDropEvent *event);
    void dropEvent(QGraphicsSceneDragDropEvent *event);
signals:
    void urlChanged(const QString &url);
    void dragEntered();
    void dragLeft();
    void dropped();
public slots:
    void doDragFinished(const QPointF &point, const QPointF &scenePoint, DBookmarkItem *item);
    void currentUrlChanged(const FMEvent &event);
    void bookmarkRemoved(const QString &url);
    void bookmarkMounted(int fd);
    void doBookmarkRemoved(const FMEvent &event);
    void bookmarkRename(const FMEvent &event);
    void doBookmarkRenamed(const QString &oldname, const QString &newname, const FMEvent &event);
    void doBookmarkAdded(const QString &name, const FMEvent &event);
    void doMoveBookmark(int from, int to, const FMEvent &event);
    void rootDropped(const QPointF& point);

    void volumeAdded(UDiskDeviceInfo * device);
    void volumeRemoved(UDiskDeviceInfo * device);
    void mountAdded(UDiskDeviceInfo * device);
    void mountRemoved(UDiskDeviceInfo * device);
private:
    void increaseSize();
    void decreaseSize();
    void move(DBookmarkItem * from, DBookmarkItem* to);
    int m_bookmarkCount = 0;
    int m_diskCount = 0;
    DBookmarkRootItem * m_rootItem;
    DBookmarkItemGroup * m_itemGroup;
    QList<DBookmarkItem *> m_items;
    QList<DBookmarkItem *> m_customItems;
    QMap<QString, DBookmarkItem *> m_diskItems;
    double m_totalHeight = 0;
    bool m_acceptDrop;
    bool m_isTightMode = false;

    QGraphicsWidget * m_defaultWidget;
    QGraphicsWidget * m_bookmarkWidget;
    QGraphicsLinearLayout * m_defaultLayout;
    QGraphicsLinearLayout * m_bookmarkLayout;
};

#endif // DBOOKMARKSCENE_H
