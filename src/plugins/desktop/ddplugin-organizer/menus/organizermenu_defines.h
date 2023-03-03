// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ORGANIZERMENU_DEFINES_H
#define ORGANIZERMENU_DEFINES_H

#include "ddplugin_organizer_global.h"

namespace ddplugin_organizer {

namespace ActionID {
inline constexpr char kOrganizeDesktop[] = "organize-desktop";
inline constexpr char kOrganizeOptions[] = "organize-options";
inline constexpr char kOrganizeBy[] = "organize-by";
inline constexpr char kOrganizeByType[] = "organize-by-type";
inline constexpr char kOrganizeByTimeAccessed[] = "organize-by-time-accessed";
inline constexpr char kOrganizeByTimeModified[] = "organize-by-time-modified";
inline constexpr char kOrganizeByTimeCreated[] = "organize-by-time-created";
inline constexpr char kOrganizeByCustom[] = "custom-collection";
inline constexpr char kCreateACollection[] = "create-a-collection";

#ifdef EnableDisplaySizeMenu
inline constexpr char kDisplaySize[] = "display-size";
inline constexpr char kDisplaySizeSmaller[] = "display-size-smaller";
inline constexpr char kDisplaySizeNormal[] = "display-size-normal";
inline constexpr char kDisplaySizeLarger[] = "display-size-larger";
#endif

} // namespace ActionID

namespace CollectionMenuParams {
inline constexpr char kOnColletion[] = "OnColletion";
inline constexpr char kColletionView[] = "ColletionView";
}
}

#endif // ORGANIZERMENU_DEFINES_H
