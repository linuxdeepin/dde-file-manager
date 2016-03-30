#ifndef DBOOKMARKSCENE_H
#define DBOOKMARKSCENE_H

#include <QGraphicsScene>

#define BOOKMARK_ITEM_HEIGHT 30

class DBookmarkItem;
class DBookmarkRootItem;
class DBookmarkItemGroup;

class DBookmarkScene : public QGraphicsScene
{
    Q_OBJECT
public:
    DBookmarkScene();
    void addItem(DBookmarkItem *item);
    void setSceneRect(qreal x, qreal y, qreal w, qreal h);
    DBookmarkItemGroup * getGroup();
protected:
    void changed(const QList<QRectF> &region);
signals:
    void urlChanged(const QString &url);
private:
    DBookmarkRootItem * m_rootItem;
    DBookmarkItemGroup * m_itemGroup;
};

#endif // DBOOKMARKSCENE_H
