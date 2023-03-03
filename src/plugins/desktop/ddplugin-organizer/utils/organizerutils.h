// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ORGANIZERUTILS_H
#define ORGANIZERUTILS_H

#include "organizer_defines.h"

namespace ddplugin_organizer {

class OrganizerUtils
{
public:
    static int covertIconLevel(int lv, bool toDisplay);
    static bool isAllItemCategory(const ItemCategories &flags);
private:
    OrganizerUtils();
};

}
#endif // ORGANIZERUTILS_H
