#ifndef DBOOKMARKITEMGROUP_H
#define DBOOKMARKITEMGROUP_H

#include <QList>
#include <QObject>

class DBookmarkItem;
class FMEvent;

class DBookmarkItemGroup : public QObject
{
    Q_OBJECT
public:
    DBookmarkItemGroup();
    void addItem(DBookmarkItem * item);
    void removeItem(DBookmarkItem * item);
    void deselectAll();
    QList<DBookmarkItem *> * items();
private:
    QList<DBookmarkItem *> m_items;
signals:
    void url(const FMEvent &e);
};

#endif // DBOOKMARKITEMGROUP_H
