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
class FMEvent;
class ComputerView;
class TabBar;

DWIDGET_USE_NAMESPACE

class DFileManagerWindowPrivate;
class DFileManagerWindow : public DMainWindow
{
    Q_OBJECT
public:
    explicit DFileManagerWindow(QWidget *parent = 0);
    ~DFileManagerWindow();

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

    DToolBar* getToolBar() const;
    DFileView *getFileView() const;

    int windowId();

signals:
    void aboutToClose();
    void fileViewChanged(const DFileView* fileView);

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
    void setFileView(DFileView *view);
    void switchToView(const int index);
    void onFileViewCurrentUrlChanged(const DUrl &url);
    void onTabAddableChanged(bool addable);
    void onCurrentTabChanged(int tabIndex);
    void onCurrentTabClosed(const int index);

protected:
    void closeEvent(QCloseEvent* event)  Q_DECL_OVERRIDE;
    void mouseDoubleClickEvent(QMouseEvent *event) Q_DECL_OVERRIDE;

private:
    QScopedPointer<DFileManagerWindowPrivate> d_ptr;

    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), DFileManagerWindow)
};

#endif // DFILEMANAGERWINDOW_H
