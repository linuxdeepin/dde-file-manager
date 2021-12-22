/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef FILEMANAGERWINDOW_P_H
#define FILEMANAGERWINDOW_P_H

#include "dfm-base/dfm_base_global.h"
#include "widgets/dfmsplitter/splitter.h"
#include "interfaces/abstractbaseview.h"
#include "interfaces/abstractframe.h"

#include <DTitlebar>
#include <DButtonBox>

#include <QObject>
#include <QFrame>
#include <QUrl>
#include <QHBoxLayout>

DWIDGET_USE_NAMESPACE
DFMBASE_BEGIN_NAMESPACE

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

private:
    QUrl currentUrl;
    static constexpr int kMinimumWindowWidth { 760 };
    static constexpr int kMinimumWindowHeight { 420 };
    static constexpr int kDefaultWindowWidth { 1100 };
    static constexpr int kDefaultWindowHeight { 700 };
    static constexpr int kMinimumLeftWidth { 120 };
    static constexpr int kMaximumLeftWidth { 200 };

    QFrame *centralView { nullptr };   // Central area (all except sidebar)
    QHBoxLayout *midLayout { nullptr };
    Splitter *splitter { nullptr };
    QFrame *leftView { nullptr };   // only splitter use it
    QFrame *rightView { nullptr };   // only splitter use it
    AbstractFrame *titleBar { nullptr };
    AbstractFrame *sideBar { nullptr };
    AbstractFrame *workspace { nullptr };
    AbstractFrame *detailSpace { nullptr };
};

DFMBASE_END_NAMESPACE

#endif   // FILEMANAGERWINDOW_P_H
