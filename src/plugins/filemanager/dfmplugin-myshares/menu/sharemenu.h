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
#ifndef SHAREMENU_H
#define SHAREMENU_H

#include "dfmplugin_myshares_global.h"

#include "dfm-base/interfaces/abstractmenu.h"
#include "dfm-base/widgets/action/actiondatacontainer.h"
#include "dfm-base/dfm_actiontype_defines.h"

#include <QUrl>

DPMYSHARES_BEGIN_NAMESPACE

namespace ShareScene {
static constexpr char kShareScene[] { "myshare" };
}   // namespace ShareScene

class ShareMenu : public DFMBASE_NAMESPACE::AbstractMenu
{
public:
    explicit ShareMenu(QObject *parent = nullptr);

    virtual QMenu *build(QWidget *parent, MenuMode mode, const QUrl &rootUrl, const QUrl &focusedUrl, const QList<QUrl> &selected, QVariant customData) override;
    virtual void actionBusiness(QAction *act) override;

private:
    QMenu *buildEmptyMenu(QWidget *parent = nullptr);
    QMenu *buildFileMenu(const QList<QUrl> &selected, QWidget *parent = nullptr);
    QMenu *createMenuByContainer(const QVector<DFMBASE_NAMESPACE::ActionDataContainer> &containers, QWidget *parent = nullptr);
    void initShareActions();

private:
    enum ShareAction {
        kActCancelShare = DFMBASE_NAMESPACE::ActionType::kActMaxCustom,
    };

    QMap<int, int> actionMap;
    QMap<int, DFMBASE_NAMESPACE::ActionDataContainer> actContainers;

    QList<QUrl> selectedUrls;
    quint64 winId;
};

DPMYSHARES_END_NAMESPACE

#endif   // SHAREMENU_H
