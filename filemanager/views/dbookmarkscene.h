#ifndef DBOOKMARKSCENE_H
#define DBOOKMARKSCENE_H

#include <QGraphicsScene>

#define BOOKMARK_ITEM_HEIGHT 30

class DBookmarkItem;
class DBookmarkRootItem;
class DBookmarkItemGroup;
class FMEvent;

class DBookmarkScene : public QGraphicsScene
{
    Q_OBJECT
public:
    DBookmarkScene();
    void addItem(DBookmarkItem *item);
    void insert(int index, DBookmarkItem *item);
    void insert(DBookmarkItem * before, DBookmarkItem *item);
    void remove(int index);
    void clear(DBookmarkItem * item);
    void remove(DBookmarkItem * item);
    void setSceneRect(qreal x, qreal y, qreal w, qreal h);
    DBookmarkItemGroup * getGroup();
protected:
    void changed(const QList<QRectF> &region);
    void dragEnterEvent(QGraphicsSceneDragDropEvent *event);
    void dragLeaveEvent(QGraphicsSceneDragDropEvent *event);
signals:
    void urlChanged(const QString &url);
    void dragEntered();
    void dragLeft();
    void dropped();
public slots:
    void doDragFinished(const QPointF &point, DBookmarkItem *item);
    void currentUrlChanged(const FMEvent &event);
private:
    void increaseSize();
    DBookmarkRootItem * m_rootItem;
    DBookmarkItemGroup * m_itemGroup;
    QList<DBookmarkItem *> m_items;
    double m_totalHeight = 0;
};

#endif // DBOOKMARKSCENE_H
