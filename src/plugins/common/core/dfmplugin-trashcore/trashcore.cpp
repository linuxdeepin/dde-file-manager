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
#include "trashfileinfo.h"
#include "utils/trashcorehelper.h"
#include "events/trashcoreeventreceiver.h"
#include "events/trashcoreeventsender.h"

#include "dfm-base/base/urlroute.h"
#include "dfm-base/base/schemefactory.h"

using CustomViewExtensionView = std::function<QWidget *(const QUrl &url)>;
Q_DECLARE_METATYPE(CustomViewExtensionView)

using namespace dfmplugin_trashcore;

void TrashCore::initialize()
{
    TrashCoreEventSender::instance();

    DFMBASE_NAMESPACE::UrlRoute::regScheme(TrashCoreHelper::scheme(), "/", TrashCoreHelper::icon(), true, tr("Trash"));
    DFMBASE_NAMESPACE::InfoFactory::regClass<TrashFileInfo>(TrashCoreHelper::scheme());
}

bool TrashCore::start()
{
    dpfSlotChannel->connect(DPF_MACRO_TO_STR(DPTRASHCORE_NAMESPACE), "slot_TrashCore_EmptyTrash",
                            TrashCoreEventReceiver::instance(), &TrashCoreEventReceiver::handleEmptyTrash);

    CustomViewExtensionView func { TrashCoreHelper::createTrashPropertyDialog };
    dpfSlotChannel->push("dfmplugin_propertydialog", "slot_CustomView_Register",
                         func, TrashCoreHelper::scheme());

    dpfHookSequence->follow("dfmplugin_fileoperations", "hook_Operation_CutFromFile",
                            TrashCoreEventReceiver::instance(), &TrashCoreEventReceiver::cutFileFromTrash);

    return true;
}
