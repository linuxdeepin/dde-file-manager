// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "organizerutils.h"

using namespace ddplugin_organizer;

bool OrganizerUtils::isAllItemCategory(const ItemCategories &flags)
{
    return flags == kCatAll;
}

ItemCategories OrganizerUtils::buildBitwiseEnabledCategory(const ItemCategories &flags)
{
    auto result { flags };
    if (flags == kCatDefault || flags < 0) {
        result = kCatAll;
        result &= ~kCatOther;
        result &= ~kCatApplication;
    }

    return result;
}

OrganizerUtils::OrganizerUtils()
{
}
