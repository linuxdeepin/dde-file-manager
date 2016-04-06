#ifndef DFILEMANAGERWINDOW_H
#define DFILEMANAGERWINDOW_H

#include "dmovablemainwindow.h"
#include <dtitlebar.h>

class DLeftSideBar;
class DToolBar;
class DFileView;
class DDetailView;
class QStatusBar;
class QFrame;
class QHBoxLayout;
class QVBoxLayout;
class QSplitter;
class QResizeEvent;
class DSplitter;

DWIDGET_USE_NAMESPACE

class DFileManagerWindow : public DMovableMainWindow
{
    Q_OBJECT
public:
    explicit DFileManagerWindow(QWidget *parent = 0);
    ~DFileManagerWindow();

    static const int MinimumWidth;

    void initData();
    void initUI();
    void initTitleBar();
    void initSplitter();

    void initLeftSideBar();

    void initRightView();
    void initToolBar();
    void initFileView();
    void initDetailView();

    void initCentralWidget();
    void initStatusBar();
    void initConnect();
signals:

public slots:


protected:
    void resizeEvent(QResizeEvent* event);
    void keyPressEvent(QKeyEvent *e);

private:
    QFrame* m_centralWidget;
    DTitlebar* m_titleBar = NULL;
    DLeftSideBar* m_leftSideBar = NULL;
    QFrame* m_rightView = NULL;
    DToolBar* m_toolbar = NULL;
    DFileView* m_fileView = NULL;
    DDetailView* m_detailView = NULL;
    QStatusBar* m_statusBar = NULL;
    QVBoxLayout* m_mainLayout;
    QVBoxLayout* m_viewLayout;
    DSplitter* m_splitter;
};

#endif // DFILEMANAGERWINDOW_H
