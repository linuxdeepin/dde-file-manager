// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
