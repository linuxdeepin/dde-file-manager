/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyu@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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
#ifndef RECENTEVENTRECVER_H
#define RECENTEVENTRECVER_H

#include "recentlog.h"
#include "recentutil.h"

#include "windowservice/windowservice.h"
#include "windowservice/containerkeys.h"

#include "dfm-framework/event/eventhandler.h"
#include "dfm-framework/event/eventcallproxy.h"
#include "dfm-base/widgets/dfmsidebar/sidebaritem.h"

DSB_FM_USE_NAMESPACE

namespace SaveBlock {
    extern DSB_FM_NAMESPACE::WindowService * windowServiceTemp;
    const QString sidebarDisplayText = QObject::tr("Recent");
}

class RecentEventRecver : public dpf::SynchEventHandler
{
    Q_OBJECT
public:
    RecentEventRecver(){}

    virtual void eventProcess(const dpf::Event &event) override
    {
        if (event.topic() == EventKeys::TOPIC_WINDOW_EVENT)
            windowEvent(event);
    }

    void windowEvent(const dpf::Event &event)
    {
        if (event.data() == EventKeys::DATA_OPEN_NEW_WINDOW)
        {
            qCCritical(RecentPlugin) << SaveBlock::windowServiceTemp;
            if (SaveBlock::windowServiceTemp) {
                //do new sidebar and plugin all menu;
                int winIdx = event.property(EventKeys::PROPERTY_KEY_WINDOW_INDEX).toInt();
                qCCritical(RecentPlugin) << "recver:"
                                         << EventKeys::PROPERTY_KEY_WINDOW_INDEX
                                         << winIdx;

                QIcon recentIcon = QIcon::fromTheme(StandardPaths::iconName(StandardPaths::RecentPath));

                auto recentItem = new SideBarItem(recentIcon,
                                                  SaveBlock::sidebarDisplayText,
                                                  "core", RecentUtil::onlyRootUrl());

                recentItem->setFlags(recentItem->flags()&~(Qt::ItemIsEditable|Qt::ItemIsDragEnabled));

                SaveBlock::windowServiceTemp->insertSideBarItem(winIdx, 0, recentItem);
            }
        }
    }

};

#endif // RECENTEVENTRECVER_H
