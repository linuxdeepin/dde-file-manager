#ifndef DFILEMANAGERWINDOW_H
#define DFILEMANAGERWINDOW_H

#include "durl.h"

#include <DMainWindow>

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
class DFMEvent;
class ComputerView;
class TabBar;
class ViewManager;

DWIDGET_USE_NAMESPACE

class DFileManagerWindowPrivate;
class DFileManagerWindow : public DMainWindow
{
    Q_OBJECT
public:
    explicit DFileManagerWindow(QWidget *parent = 0);
    explicit DFileManagerWindow(const DUrl &fileUrl, QWidget *parent = 0);
    ~DFileManagerWindow();

    DUrl currentUrl() const;

    int getFileViewMode() const;
    int getFileViewSortRole() const;

    DToolBar* getToolBar() const;
    DFileView *getFileView() const;
    DLeftSideBar *getLeftSideBar() const;
    ViewManager* getViewManager() const;

    int windowId();

    bool tabAddable() const;

signals:
    void aboutToClose();
    void fileViewChanged(const DFileView* fileView);
    void positionChanged(const QPoint &pos);

public slots:
    void moveCenter(const QPoint &cp);
    void moveTopRight();
    void moveCenterByRect(QRect rect);
    void moveTopRightByRect(QRect rect);

    void setFileViewMode(int viewMode);
    void setIconView();
    void setListView();
    void preHandleCd(const DUrl &fileUrl, int source);
    void preHandleCd(const DFMEvent& event);
    void cd(const DFMEvent& event);

    void showPluginView(const DUrl& fileUrl);
    void openNewTab(const DFMEvent& event);
    void createNewView(const DFMEvent& event);
    void switchToView(DFileView *view);
    void onTabAddableChanged(bool addable);
    void onCurrentTabChanged(int tabIndex);
    void onRequestCloseTab(const int index, const bool& remainState);
    void closeCurrentTab(const DFMEvent& event);
    void showNewTabButton();
    void hideNewTabButton();
    void onNewTabButtonClicked();

protected:
    void closeEvent(QCloseEvent* event)  Q_DECL_OVERRIDE;
    void mouseDoubleClickEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void moveEvent(QMoveEvent *event) Q_DECL_OVERRIDE;

    void initData();
    void initUI();

    void initTitleFrame();
    void initTitleBar();
    void initSplitter();
    void initViewManager();

    void initLeftSideBar();

    void initRightView();
    void initToolBar();
    void initTabBar();
    void initViewLayout();
    void initFileView(const DUrl &fileUrl);
    void initComputerView();
    void loadPluginRegisteredSchemes();

    void initCentralWidget();
    void initConnect();

private:
    QScopedPointer<DFileManagerWindowPrivate> d_ptr;

    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), DFileManagerWindow)
};

#endif // DFILEMANAGERWINDOW_H
