/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: huangyu<huangyub@uniontech.com>
 *             liuyangming<liuyangming@uniontech.com>
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
#include "trashcore.h"
#include "utils/trashcorehelper.h"
#include "events/trashcoreunicastreceiver.h"
#include "events/trashcoreeventreceiver.h"
#include "events/trashcoreeventsender.h"
#include "services/common/propertydialog/propertydialogservice.h"

#include "services/common/trash/trash_defines.h"

DPTRASHCORE_USE_NAMESPACE

void TrashCore::initialize()
{
    TrashCoreUnicastReceiver::instance()->connectService();
    TrashCoreEventSender::instance();
}

bool TrashCore::start()
{
    DSC_USE_NAMESPACE
    dpfInstance.eventDispatcher().subscribe(Trash::EventType::kEmptyTrash,
                                            TrashCoreEventReceiver::instance(),
                                            &TrashCoreEventReceiver::handleEmptyTrash);
    propertyServIns->registerCustomizePropertyView(TrashCoreHelper::createTrashPropertyDialog, TrashCoreHelper::scheme());

    return true;
}

dpf::Plugin::ShutdownFlag TrashCore::stop()
{
    return kSync;
}
