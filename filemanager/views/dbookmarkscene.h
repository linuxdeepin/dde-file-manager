#ifndef DBOOKMARKSCENE_H
#define DBOOKMARKSCENE_H

#include <QGraphicsScene>
#include <QGraphicsWidget>
#include <QGraphicsLinearLayout>
#include "../app/fmevent.h"

#define BOOKMARK_ITEM_HEIGHT 30
#define SEPARATOR_ITEM_HEIGHT 6
#define BOOKMARK_ITEM_WIDTH 201
#define BOOKMARK_ITEM_SPACE 0

class DBookmarkItem;
class DBookmarkRootItem;
class DBookmarkItemGroup;
class UDiskDeviceInfo;
class DUrl;
class DiskInfo;

class DBookmarkScene : public QGraphicsScene
{
    Q_OBJECT
public:
    DBookmarkScene();

    void initData();
    void initUI();
    void initConnect();

    DBookmarkItem * createBookmarkByKey(const QString& key);

    DUrl getStandardPathByKey(const QString& key);


    void addItem(DBookmarkItem *item);
    void insert(int index, DBookmarkItem *item);
    void insert(DBookmarkItem * before, DBookmarkItem *item);
    void remove(int index);
    void remove(DBookmarkItem * item);
    void setSceneRect(qreal x, qreal y, qreal w, qreal h);
    void addSeparator();
    void insertSeparator(int index);
    DBookmarkItemGroup * getGroup();
    int count();
    int getCustomBookmarkItemInsertIndex();
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
    void keyPressEvent(QKeyEvent *event);
signals:
    void urlChanged(const QString &url);
    void dragEntered();
    void dragLeft();
    void dropped();
public slots:
    void doDragFinished(const QPointF &point, const QPointF &scenePoint, DBookmarkItem *item);
    void currentUrlChanged(const FMEvent &event);
    void setHomeItem(DBookmarkItem* item);
    void setDefaultDiskItem(DBookmarkItem* item);
    void setNetworkDiskItem(DBookmarkItem* item);
    void doBookmarkRemoved(const FMEvent &event);
    void bookmarkRename(const FMEvent &event);
    void doBookmarkRenamed(const QString &oldname, const QString &newname, const FMEvent &event);
    void doBookmarkAdded(const QString &name, const FMEvent &event);
    void doMoveBookmark(int from, int to, const FMEvent &event);

    void volumeAdded(UDiskDeviceInfo * device);
    void volumeRemoved(UDiskDeviceInfo * device);
    void mountAdded(UDiskDeviceInfo * device);
    void mountRemoved(UDiskDeviceInfo * device);

    void backHome();

    void chooseMountedItem(const FMEvent &event);
private:
    bool isBelowLastItem(const QPointF &point);
    void increaseSize();
    void decreaseSize();
    void moveBefore(DBookmarkItem * from, DBookmarkItem* to);
    void moveAfter(DBookmarkItem * from, DBookmarkItem* to);
    int m_defaultCount = 0;

    QMap<QString, QString> m_smallIcons;
    QMap<QString, QString> m_smallHoverIcons;
    QMap<QString, QString> m_smallCheckedIcons;
    QMap<QString, QString> m_bigIcons;
    QMap<QString, QString> m_bigHoverIcons;
    QMap<QString, QString> m_bigCheckedIcons;
    QStringList m_systemPathKeys;
    QMap<QString, QString> m_systemBookMarks;

    DBookmarkRootItem * m_rootItem;
    DBookmarkItem* m_homeItem;
    DBookmarkItem* m_defaultDiskItem;
    DBookmarkItem* m_networkDiskItem;
    DBookmarkItemGroup * m_itemGroup;
    QList<DBookmarkItem *> m_customItems;
    QMap<QString, DBookmarkItem *> m_diskItems;
    double m_totalHeight = 0;
    bool m_acceptDrop;
    bool m_isTightMode = false;
    bool m_delayCheckMountedItem = false;
    FMEvent m_delayCheckMountedEvent;

    QGraphicsLinearLayout * m_defaultLayout;

};

#endif // DBOOKMARKSCENE_H
