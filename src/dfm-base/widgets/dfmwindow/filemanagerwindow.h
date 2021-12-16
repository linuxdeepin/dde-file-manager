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
#ifndef FILEMANAGERWINDOW_H
#define FILEMANAGERWINDOW_H

#include "dfm-base/dfm_base_global.h"

#include <DMainWindow>

DWIDGET_USE_NAMESPACE

DFMBASE_BEGIN_NAMESPACE

class AbstractFrame;
class FileManagerWindowPrivate;
class FileManagerWindow : public DMainWindow
{
    Q_OBJECT
    friend class FileManagerWindowPrivate;

public:
    explicit FileManagerWindow(const QUrl &url, QWidget *parent = nullptr);
    virtual ~FileManagerWindow();

    void cd(const QUrl &url);
    QUrl currentUrl() const;
    void moveCenter(const QPoint &cp);
    void installTitleBar(AbstractFrame *w);
    void installTitleMenu(QMenu *menu);
    void installSideBar(AbstractFrame *w);
    void installWorkSpace(AbstractFrame *w);

    AbstractFrame *titleBar() const;
    AbstractFrame *sideBar() const;
    AbstractFrame *workSpace() const;

signals:
    void aboutToClose();
    void positionChanged(const QPoint &pos);
    void currentUrlChanged();
    void currentViewStateChanged();
    void selectUrlChanged(const QList<QUrl> &urlList);

private:
    void initializeUi();

private:
    QScopedPointer<FileManagerWindowPrivate> d;
};

DFMBASE_END_NAMESPACE

#endif   // FILEMANAGERWINDOW_H
