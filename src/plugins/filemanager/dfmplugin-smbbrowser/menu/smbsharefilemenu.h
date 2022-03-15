/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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
#ifndef SMBSHAREFILEMENU_H
#define SMBSHAREFILEMENU_H

#include "dfmplugin_smbbrowser_global.h"

#include "dfm-base/interfaces/abstractmenu.h"
#include "dfm-base/widgets/action/actiondatacontainer.h"
#include "dfm-base/dfm_actiontype_defines.h"

#include <QUrl>

DPSMBBROWSER_BEGIN_NAMESPACE

namespace SmbBrowserScene {
static constexpr char kSmbBrowserScene[] { "smbbrowser" };
}

class SmbShareFileMenu : public DFMBASE_NAMESPACE::AbstractMenu
{
public:
    explicit SmbShareFileMenu(QObject *parent = nullptr);

    virtual QMenu *build(QWidget *parent, MenuMode mode, const QUrl &rootUrl, const QUrl &focusUrl, const QList<QUrl> &selected, QVariant customData) override;
    virtual void actionBusiness(QAction *act) override;

private:
    QMenu *createMenuByContainer(const QVector<DFMBASE_NAMESPACE::ActionDataContainer> &containers, QWidget *parent = nullptr);

private:
    quint64 winId;
    QUrl selectedUrl;
};

DPSMBBROWSER_END_NAMESPACE

#endif   // SMBSHAREFILEMENU_H
