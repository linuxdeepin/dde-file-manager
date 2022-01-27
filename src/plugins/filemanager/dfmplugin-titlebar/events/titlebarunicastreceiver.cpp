/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
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
#include "titlebarunicastreceiver.h"
#include "utils/crumbmanager.h"
#include "utils/optionbuttonmanager.h"
#include "utils/crumbinterface.h"

#include <QSharedPointer>

#include <dfm-framework/framework.h>

DPTITLEBAR_USE_NAMESPACE

#define STR1(s) #s
#define STR2(s) STR1(s)

using namespace DSB_FM_NAMESPACE::TitleBar;

inline QString topic(const QString &func)
{
    return QString(STR2(DSB_FM_NAMESPACE)) + "::" + func;
}

TitleBarUnicastReceiver *TitleBarUnicastReceiver::instance()
{
    static TitleBarUnicastReceiver receiver;
    return &receiver;
}

void TitleBarUnicastReceiver::connectService()
{
    dpfInstance.eventUnicast().connect(topic("TitleBarService::addCustomCrumbar"), this, &TitleBarUnicastReceiver::invokeAddCustomCrumbar);
}

bool TitleBarUnicastReceiver::invokeAddCustomCrumbar(const CustomCrumbInfo &info)
{
    Q_ASSERT(!info.scheme.isEmpty());
    if (CrumbManager::instance()->isRegisted(info.scheme)) {
        qWarning() << "Crumb sechme " << info.scheme << "has been resigtered!";
        return false;
    }

    int state { OptionButtonManager::kDoNotHide };
    if (info.hideListViewBtn)
        state |= OptionButtonManager::kHideListViewBtn;
    if (info.hideIconViewBtn)
        state |= OptionButtonManager::kHideIconViewBtn;
    if (info.hideDetailSpaceBtn)
        state |= OptionButtonManager::kHideDetailSpaceBtn;
    if (state != OptionButtonManager::kDoNotHide)
        OptionButtonManager::instance()->setOptBtnVisibleState(info.scheme, static_cast<OptionButtonManager::OptBtnVisibleState>(state));

    CrumbManager::instance()->registerCrumbCreator(info.scheme, [=]() {
        CrumbInterface *interface { new CrumbInterface };
        interface->setKeepAddressBar(info.keepAddressBar);
        interface->registewrSupportedUrlCallback(info.supportedCb);
        interface->registerSeprateUrlCallback(info.seperateCb);
        return interface;
    });

    return true;
}

TitleBarUnicastReceiver::TitleBarUnicastReceiver(QObject *parent)
    : QObject(parent)
{
}
