// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ORGANIZERMENU_DEFINES_H
#define ORGANIZERMENU_DEFINES_H

#include "ddplugin_organizer_global.h"

namespace ddplugin_organizer {

namespace ActionID {
inline constexpr char kOrganizeEnable[] = "organize-enable";
inline constexpr char kOrganizeTrigger[] = "organize-trigger";
inline constexpr char kOrganizeOptions[] = "organize-options";
inline constexpr char kOrganizeBy[] = "organize-by";
inline constexpr char kOrganizeByType[] = "organize-by-type";
inline constexpr char kOrganizeByTimeAccessed[] = "organize-by-time-accessed";
inline constexpr char kOrganizeByTimeModified[] = "organize-by-time-modified";
inline constexpr char kOrganizeByTimeCreated[] = "organize-by-time-created";
inline constexpr char kOrganizeByCustom[] = "custom-collection";
inline constexpr char kCreateACollection[] = "create-a-collection";
}   // namespace ActionID

namespace CollectionMenuParams {
inline constexpr char kOnColletion[] = "OnColletion";
inline constexpr char kColletionView[] = "ColletionView";
}
}

#endif   // ORGANIZERMENU_DEFINES_H
