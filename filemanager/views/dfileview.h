#ifndef DFILEVIEW_H
#define DFILEVIEW_H

#include <dlistview.h>

#include <QFrame>

class ItemDelegate;
class QFileSystemModel;

DWIDGET_USE_NAMESPACE

class DFileSystemModel;
class DFileView : public DListView
{
    Q_OBJECT
public:
    explicit DFileView(QWidget *parent = 0);
    ~DFileView();

    void initUI();
    void initDelegate();
    void initModel();
    void initConnects();

    DFileSystemModel *model() const;

public slots:
    void back();
    inline void previous()
    {cd("..");}
    void cd(const QString &dir);
    void switchListMode();

private:
    ItemDelegate *m_delegate;
};

#endif // DFILEVIEW_H
