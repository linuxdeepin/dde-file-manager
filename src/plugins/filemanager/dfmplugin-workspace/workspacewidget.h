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
#ifndef WORKSPACEWIDGET_H
#define WORKSPACEWIDGET_H

#include "dfmplugin_workspace_global.h"
#include "dfm-base/interfaces/abstractframe.h"

#include <QUrl>

namespace DFMBASE_NAMESPACE {
class AbstractBaseView;
}   // namespace dfmbase

class QVBoxLayout;
class QStackedLayout;

DPWORKSPACE_BEGIN_NAMESPACE

class FileView;
class WorkspaceWidget : public dfmbase::AbstractFrame
{
    Q_OBJECT
public:
    explicit WorkspaceWidget(QFrame *parent = nullptr);
    void setCurrentUrl(const QUrl &url) override;
    QUrl currentUrl() const override;

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void showEvent(QShowEvent *event) override;

private:
    void initializeUi();
    void initConnect();
    void initTabBar();
    void initViewLayout();
    void handleCtrlN();

private:
    QUrl workspaceUrl;
    QVBoxLayout *widgetLayout { nullptr };
    QStackedLayout *viewStackLayout { nullptr };
    QSharedPointer<DFMBASE_NAMESPACE::AbstractBaseView> fileView { nullptr };
};

DPWORKSPACE_END_NAMESPACE

#endif   // WORKSPACEWIDGET_H
