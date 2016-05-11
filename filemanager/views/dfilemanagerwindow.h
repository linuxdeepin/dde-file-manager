#ifndef DFILEMANAGERWINDOW_H
#define DFILEMANAGERWINDOW_H

#include "dmovablemainwindow.h"
#include <dtitlebar.h>
#include "../models/durl.h"
#include "widgets/dwindowframe.h"
#include <QMainWindow>

#define DEFAULT_WINDOWS_WIDTH 950
#define DEFAULT_WINDOWS_HEIGHT 600
#define LEFTSIDEBAR_MAX_WIDTH 200
#define LEFTSIDEBAR_MIN_WIDTH 48
#define TITLE_FIXED_HEIGHT 40

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
class DSplitter;

class DMainWindow;
class DFileManagerWindow;
class ExtendView;
class QStackedLayout;



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
    void initExtendView();

    void initCentralWidget();
    void initStatusBar();
    void initConnect();

    DUrl currentUrl() const;

    int getFileViewMode() const;
    int getFileViewSortRole() const;

public slots:
    void showMinimized();
    void setFileViewMode(int viewMode);
    void setFileViewSortRole(int sortRole);
    void setIconView();
    void setListView();
    void setExtendView();


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
    QFrame * m_titleFrame = NULL;
    ExtendView* m_extendView = NULL;
    QStackedLayout* m_viewStackLayout;
};

class DMainWindow : public DWindowFrame{
    Q_OBJECT
public:
    explicit DMainWindow(QWidget *parent = 0);
    ~DMainWindow();
    void initUI();
    void initConnect();

    DFileManagerWindow *fileManagerWindow() const;

signals:
    void aboutToClose();

public slots:
    void moveCenter();
    void moveTopRight();
    void moveCenterByRect(QRect rect);
    void moveTopRightByRect(QRect rect);

protected:
    void closeEvent(QCloseEvent* event);

private:
    DFileManagerWindow *m_fileManagerWindow;
};

#endif // DFILEMANAGERWINDOW_H
