// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILEMANAGERWINDOW_P_H
#define FILEMANAGERWINDOW_P_H

#include <dfm-base/widgets/filemanagerwindow.h>
#include <dfm-base/widgets/dfmsplitter/splitter.h>
#include <dfm-base/interfaces/abstractbaseview.h>
#include <dfm-base/interfaces/abstractframe.h>
#include <dfm-base/widgets/dfmcustombuttons/customiconbutton.h>

#include <DTitlebar>
#include <DButtonBox>
#include <DVerticalLine>

#include <QObject>
#include <QFrame>
#include <QUrl>
#include <QHBoxLayout>
#include <QPropertyAnimation>

#include <mutex>

DWIDGET_USE_NAMESPACE

namespace dfmbase {

class FileManagerWindow;
class FileManagerWindowPrivate : public QObject
{
    Q_OBJECT
    friend class FileManagerWindow;
    FileManagerWindow *const q;

public:
    explicit FileManagerWindowPrivate(const QUrl &url, FileManagerWindow *qq);
    bool processKeyPressEvent(QKeyEvent *event);
    int splitterPosition() const;
    void setSplitterPosition(int pos);
    void resetTitleBarSize();
    void resetSideBarSize();
    void animateSplitter(bool expanded);

    void loadWindowState();
    void saveWindowState();

    void showSideBar();
    void hideSideBar();
    void setupSidebarSepTracking();
    int loadSidebarState() const;
    void saveSidebarState();
    void updateSideBarState();
    void updateSideBarVisibility();
    void updateSidebarSeparator();
    void updateSideBarSeparatorStyle();
    void updateSideBarSeparatorPosition();

    void handleSplitterMoved(int pos, int index);

    // DetailSpace splitter management
    int detailSplitterPosition() const;
    void setDetailSplitterPosition(int detailWidth);
    int loadDetailSpaceState() const;
    void saveDetailSpaceState();
    void updateRightAreaMinWidth();   // Update rightArea minimum width for resize priority

protected:
    QUrl currentUrl;
    static constexpr int kMinimumWindowWidth { 540 };
    static constexpr int kMinimumWindowHeight { 300 };
    static constexpr int kDefaultWindowWidth { 1100 };
    static constexpr int kDefaultWindowHeight { 700 };
    static constexpr int kMinimumLeftWidth { 95 };
    static constexpr int kMaximumLeftWidth { 600 };
    static constexpr int kDefaultLeftWidth { 200 };
    static constexpr int kMinimumRightWidth { kMinimumWindowWidth };

    // DetailSpace size constraints
    static constexpr int kMinimumDetailWidth { 280 };
    static constexpr int kMaximumDetailWidth { 500 };
    static constexpr int kDefaultDetailWidth { 280 };
    static constexpr int kMinimumWorkspaceWidth { 260 };

    QFrame *centralView { nullptr };   // Central area (all except sidebar)
    QFrame *rightArea { nullptr };

    QHBoxLayout *midLayout { nullptr };
    QVBoxLayout *rightLayout { nullptr };
    QHBoxLayout *rightBottomLayout { nullptr };

    Splitter *splitter { nullptr };
    Splitter *detailSplitter { nullptr };   // Splitter for workspace and detailSpace
    QPropertyAnimation *curSplitterAnimation { nullptr };
    QPropertyAnimation *curDetailSplitterAnimation { nullptr };   // Separate animation for detail splitter
    int lastSidebarExpandedPostion { kDefaultLeftWidth };
    int lastDetailSpaceWidth { kDefaultDetailWidth };   // Remember detailSpace width
    AbstractFrame *titleBar { nullptr };
    AbstractFrame *sideBar { nullptr };
    AbstractFrame *workspace { nullptr };
    AbstractFrame *detailSpace { nullptr };

    DIconButton *iconLabel { nullptr };
    CustomDIconButton *expandButton { nullptr };
    DVerticalLine *sidebarSep { nullptr };
    QWidget *iconArea { nullptr };

    std::once_flag titleBarFlag;
    std::once_flag titleMenuFlag;
    std::once_flag sideBarFlag;
    std::once_flag workspaceFlag;
    std::once_flag detailVewFlag;

    bool sideBarAutoVisible { true };
    bool sideBarShrinking { false };

private:
    bool setupAnimation(bool expanded);
    void handleWindowResize(bool expanded);
    void configureAnimation(int start, int end);
    void connectAnimationSignals();
    bool isAnimationEnabled() const;

    // DetailSpace splitter internal methods
    void initDetailSplitter();
    void handleDetailSplitterMoved(int pos, int index);
    void animateDetailSplitter(bool show);
};

}

#endif   // FILEMANAGERWINDOW_P_H
