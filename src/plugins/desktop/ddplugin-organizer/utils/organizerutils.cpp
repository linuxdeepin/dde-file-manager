// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "organizerutils.h"

using namespace ddplugin_organizer;

bool OrganizerUtils::isAllItemCategory(const ItemCategories &flags)
{
    return flags == kCatDefault || flags == kCatAll;
}

OrganizerUtils::OrganizerUtils()
{

}
