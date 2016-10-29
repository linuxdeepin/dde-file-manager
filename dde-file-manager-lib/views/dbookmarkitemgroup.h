#ifndef DBOOKMARKITEMGROUP_H
#define DBOOKMARKITEMGROUP_H

#include <QList>
#include <QObject>

class DBookmarkItem;
class DFMEvent;

class DBookmarkItemGroup : public QObject
{
    Q_OBJECT
public:
    DBookmarkItemGroup();
    void addItem(DBookmarkItem * item);
    void insert(int index, DBookmarkItem * item);
    void removeItem(DBookmarkItem * item);
    void deselectAll();
    QList<DBookmarkItem *> items() const;
private:
    QList<DBookmarkItem *> m_items;
signals:
    void url(const DFMEvent &e);
};

#endif // DBOOKMARKITEMGROUP_H
