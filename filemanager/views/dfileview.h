#ifndef DFILEVIEW_H
#define DFILEVIEW_H

#include <dlistview.h>

#include <QFrame>
#include <QUrl>

class ItemDelegate;
class QFileSystemModel;
class FileController;

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

    QUrl currentUrl() const;

public slots:
    void cd(const QUrl &url);
    void switchListMode();

private:
    ItemDelegate *m_delegate;
    FileController *m_controller;
};

#endif // DFILEVIEW_H
