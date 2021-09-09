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
#ifndef CONTAINERKEYS_H
#define CONTAINERKEYS_H

#include "dfm_filemanager_service_global.h"
#include "dfm-framework/event/event.h"

DSB_FM_BEGIN_NAMESPACE

namespace EventKeys {

const QString TOPIC_WINDOW_EVENT {"WindowEvent"};
const QString DATA_OPEN_NEW_WINDOW {"OpenNewWindow"};
const QString PROPERTY_KEY_WINDOW_INDEX {"WindowIndex"};

}//namespace EventKeys

namespace SidebarKeys {

const QString GROUP_CORE {"core"};
const QString GROUP_DEVICE {"device"};

}//namespace SidebarKeys

DSB_FM_END_NAMESPACE


#endif // CONTAINERKEYS_H
