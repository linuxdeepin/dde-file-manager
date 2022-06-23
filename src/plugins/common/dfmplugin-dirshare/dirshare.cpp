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
#include "dirshare.h"
#include "sharemenu/sharemenuscene.h"
#include "widget/sharecontrolwidget.h"
#include "private/shareutils.h"

#include "services/common/menu/menuservice.h"
#include "services/common/propertydialog/propertydialogservice.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/dfm_global_defines.h"

#include <QLabel>
#include <QHBoxLayout>

DPDIRSHARE_USE_NAMESPACE
DSC_USE_NAMESPACE

void DirShare::initialize()
{
}

bool DirShare::start()
{
    MenuService::service()->registerScene(ShareMenuCreator::name(), new ShareMenuCreator);

    bindScene("CanvasMenu");
    bindScene("WorkspaceMenu");

    PropertyDialogService::service()->registerControlExpand(DirShare::createShareControlWidget, 2);

    return true;
}

dpf::Plugin::ShutdownFlag DirShare::stop()
{
    return kSync;
}

QWidget *DirShare::createShareControlWidget(const QUrl &url)
{
    DFMBASE_USE_NAMESPACE
    static QStringList supported { Global::Scheme::kFile, Global::Scheme::kUserShare };
    if (!supported.contains(url.scheme()))
        return nullptr;

    auto info = InfoFactory::create<AbstractFileInfo>(url);
    if (!ShareUtils::canShare(info))
        return nullptr;

    return new ShareControlWidget(url);
}

void DirShare::bindScene(const QString &parentScene)
{
    if (MenuService::service()->contains(parentScene)) {
        MenuService::service()->bind(ShareMenuCreator::name(), parentScene);
    } else {
        connect(MenuService::service(), &MenuService::sceneAdded, this, [=](const QString &scene) {
            if (scene == parentScene)
                MenuService::service()->bind(ShareMenuCreator::name(), scene);
        },
                Qt::DirectConnection);
    }
}
