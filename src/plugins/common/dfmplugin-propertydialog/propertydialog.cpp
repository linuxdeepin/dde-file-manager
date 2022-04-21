/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lixiang<lixianga@uniontech.com>
 *
 * Maintainer: lixiang<lixianga@uniontech.com>
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
#include "propertydialog.h"
#include "events/propertyeventreceiver.h"
#include "utils/propertydialoghelper.h"
#include "menu/propertymenuscene.h"

#include "services/common/menu/menuservice.h"

DSC_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DPPROPERTYDIALOG_USE_NAMESPACE

void PropertyDialog::initialize()
{
    PropertyEventReceiver::instance()->connectService();
}

bool PropertyDialog::start()
{

    PropertyDialogHelper::propertyServiceInstance()->addComputerPropertyToPropertyService();

    MenuService::service()->registerScene(PropertyMenuCreator::name(), new PropertyMenuCreator);
    MenuService::service()->bind(PropertyMenuCreator::name(), "WorkspaceMenu");
    return true;
}

dpf::Plugin::ShutdownFlag PropertyDialog::stop()
{
    return kSync;
}
