#ifndef DFILEVIEW_H
#define DFILEVIEW_H

#include <dlistview.h>

#include <QFrame>
#include <QUrl>
#include <QContextMenuEvent>

class ItemDelegate;
class QFileSystemModel;
class FileController;
class FileMenuManager;

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

    QString currentUrl() const;

public slots:
    void cd(const QString &url);
    void switchListMode();

protected:
    void contextMenuEvent(QContextMenuEvent * event);

private:
    ItemDelegate *m_delegate;
    FileController *m_controller;
    FileMenuManager* m_fileMenuManager;
};

#endif // DFILEVIEW_H
