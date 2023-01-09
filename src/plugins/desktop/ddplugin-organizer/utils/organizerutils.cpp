/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
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

#include "organizerutils.h"

using namespace ddplugin_organizer;

int OrganizerUtils::covertIconLevel(int lv, bool toDisplay)
{
    int ret = -1;
    if (toDisplay) { // 1 is small and 3 is large in canvas
        if (lv <= 1)
            ret = DisplaySize::kSmaller;
        else if (lv >= 3)
            ret = DisplaySize::kLarger;
        else
            ret = DisplaySize::kNormal;
    } else {
        switch (static_cast<DisplaySize>(lv)) {
        case DisplaySize::kSmaller:
            ret = 1;
            break;
        case DisplaySize::kNormal:
            ret = 2;
            break;
        case DisplaySize::kLarger:
            ret = 3;
            break;
        default:
            ret = 1;
            break;
        }
    }
    return ret;
}

bool OrganizerUtils::isAllItemCategory(const ItemCategories &flags)
{
    return flags == kCatDefault || flags == kCatAll;
}

OrganizerUtils::OrganizerUtils()
{

}
