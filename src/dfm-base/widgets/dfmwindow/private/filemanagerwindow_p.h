// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILEMANAGERWINDOW_P_H
#define FILEMANAGERWINDOW_P_H

#include <dfm-base/widgets/filemanagerwindow.h>
#include <dfm-base/widgets/dfmsplitter/splitter.h>
#include <dfm-base/interfaces/abstractbaseview.h>
#include <dfm-base/interfaces/abstractframe.h>

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

    int loadSidebarState() const;
    void saveSidebarState();

protected:
    QUrl currentUrl;
    static constexpr int kMinimumWindowWidth { 680 };
    static constexpr int kMinimumWindowHeight { 300 };
    static constexpr int kDefaultWindowWidth { 1100 };
    static constexpr int kDefaultWindowHeight { 700 };
    static constexpr int kMinimumLeftWidth { 95 };
    static constexpr int kMaximumLeftWidth { 600 };
    static constexpr int kDefaultLeftWidth { 200 };

    QFrame *centralView { nullptr };   // Central area (all except sidebar)
    QFrame *rightArea { nullptr };

    QHBoxLayout *midLayout { nullptr };
    QVBoxLayout *rightLayout { nullptr };
    QHBoxLayout *rightBottomLayout { nullptr };

    Splitter *splitter { nullptr };
    QPropertyAnimation *curSplitterAnimation { nullptr };
    int lastSidebarExpandedPostion { kDefaultLeftWidth };
    AbstractFrame *titleBar { nullptr };
    AbstractFrame *sideBar { nullptr };
    AbstractFrame *workspace { nullptr };
    AbstractFrame *detailSpace { nullptr };

    DIconButton *iconLabel { nullptr };
    DIconButton *expandButton { nullptr };
    DVerticalLine *sidebarSep { nullptr };

    std::once_flag titleBarFlag;
    std::once_flag titleMenuFlag;
    std::once_flag sideBarFlag;
    std::once_flag workspaceFlag;
    std::once_flag detailVewFlag;
};

}

#endif   // FILEMANAGERWINDOW_P_H
