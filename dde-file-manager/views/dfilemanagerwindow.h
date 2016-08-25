#ifndef DFILEMANAGERWINDOW_H
#define DFILEMANAGERWINDOW_H

#include "dmovablemainwindow.h"
#include "dtabbar.h"

#include "../models/durl.h"

#include <dtitlebar.h>

#include <QMainWindow>
#include <QDir>
#include <QMap>

#define DEFAULT_WINDOWS_WIDTH 960
#define DEFAULT_WINDOWS_HEIGHT 540
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

class DFileManagerWindow;
class DFileManagerWindow;
class ExtendView;
class QStackedLayout;
class QPushButton;

class DStatusBar;
class FMEvent;
class ComputerView;


DWIDGET_USE_NAMESPACE

class DFileManagerWindow : public QMainWindow
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
    void initTabBar();
    void initViewLayout();
    void initFileView();
    void initComputerView();


    void initCentralWidget();
    void initConnect();

    DUrl currentUrl() const;

    int getFileViewMode() const;
    int getFileViewSortRole() const;

    DTitlebar* getTitleBar();
    DToolBar* getToolBar();

    int windowId();

signals:
    void aboutToClose();

public slots:
    void moveCenter(const QPoint &cp);
    void moveTopRight();
    void moveCenterByRect(QRect rect);
    void moveTopRightByRect(QRect rect);

    void setFileViewMode(int viewMode);
    void setFileViewSortRole(int sortRole);
    void setIconView();
    void setListView();
    void preHandleCd(const FMEvent& event);
    void cd(const FMEvent& event);
    void showComputerView(const FMEvent& event);
    void openNewTab(const FMEvent& event);
    void createNewView(const FMEvent& event);
    void switchToView(const int index);
    void onFileViewCurrentUrlChanged(const DUrl &url);
    void onTabAddableChanged(bool addable);
    void onCurrentTabChanged(int tabIndex);
    void onCurrentTabClosed(const int index);

protected:
    void closeEvent(QCloseEvent* event);

private:
    QFrame* m_centralWidget = NULL;
    DTitlebar* m_titleBar = NULL;
    DLeftSideBar* m_leftSideBar = NULL;
    QFrame* m_rightView = NULL;
    DToolBar* m_toolbar = NULL;
    DTabBar* m_tabBar = NULL;
    QPushButton *m_newTabButton;
    DFileView* m_fileView = NULL;
    ComputerView* m_computerView = NULL;
    DDetailView* m_detailView = NULL;
    DStatusBar* m_statusBar = NULL;
    QVBoxLayout* m_mainLayout = NULL;
    DSplitter* m_splitter = NULL;
    QFrame * m_titleFrame = NULL;
    QStackedLayout* m_viewStackLayout=NULL;

    QMap<DUrl, QWidget*> m_views={};
};

#endif // DFILEMANAGERWINDOW_H
