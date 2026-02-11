// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CANVASMENU_DEFINES_H
#define CANVASMENU_DEFINES_H

namespace ddplugin_canvas {

namespace ActionID {
inline constexpr char kSortBy[] { "sort-by" };
inline constexpr char kDisplaySettings[] { "display-settings" };
inline constexpr char kWallpaperSettings[] { "wallpaper-settings" };
inline constexpr char kRefresh[] { "refresh" };
inline constexpr char kIconSize[] { "icon-size" };   // 1071: removed
inline constexpr char kAutoArrange[] { "auto-arrange" };   // 1071: removed

// sort by
inline constexpr char kSrtName[] { "sort-by-name" };
inline constexpr char kSrtTimeModified[] { "sort-by-time-modified" };
inline constexpr char kSrtTimeCreated[] { "sort-by-time-created" };
inline constexpr char kSrtSize[] { "sort-by-size" };
inline constexpr char kSrtType[] { "sort-by-type" };

// icon size (1071: removed)
inline constexpr char kIconSizeTiny[] { "tiny" };
inline constexpr char kIconSizeSmall[] { "small" };
inline constexpr char kIconSizeMedium[] { "medium" };
inline constexpr char kIconSizeLarge[] { "large" };
inline constexpr char kIconSizeSuperLarge[] { "super-large" };

}

namespace CanvasMenuParams {
inline constexpr char kDesktopGridPos[] { "DesktopGridPos" };
inline constexpr char kDesktopCanvasView[] { "DesktopCanvasView" };
}

}

#endif   // CANVASMENU_DEFINES_H
