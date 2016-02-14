#ifndef DFILEVIEW_H
#define DFILEVIEW_H

#include <libdui/dlistview.h>

#include <QFrame>

class QFileSystemModel;
class ItemDelegate;

DUI_USE_NAMESPACE

class DFileView : public DListView
{
    Q_OBJECT
public:
    explicit DFileView(QWidget *parent = 0);
    ~DFileView();

signals:

public slots:
    void back();
    inline void previous()
    {cd("..");}
    void cd(const QString &dir);
    void switchListMode();

private:
    QFileSystemModel *m_model;
    ItemDelegate *m_delegate;
};

#endif // DFILEVIEW_H
