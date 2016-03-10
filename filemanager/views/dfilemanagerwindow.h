#ifndef DFILEMANAGERWINDOW_H
#define DFILEMANAGERWINDOW_H

#include "dmovablemainwindow.h"


class DTitleBar;
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
    void toggleMaxNormal();
    void toggleLayout();

protected:
    void resizeEvent(QResizeEvent* event);

private:
    QFrame* m_centralWidget;
    DTitleBar* m_titleBar = NULL;
    DLeftSideBar* m_leftSideBar = NULL;
    QFrame* m_rightView = NULL;
    DToolBar* m_toolbar = NULL;
    DFileView* m_fileView = NULL;
    DDetailView* m_detailView = NULL;
    QStatusBar* m_statusBar = NULL;
    QVBoxLayout* m_mainLayout;
    QVBoxLayout* m_viewLayout;
    QSplitter* m_splitter;
};

#endif // DFILEMANAGERWINDOW_H
