// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef GLOBAL_DCONF_DEFINES_H
#define GLOBAL_DCONF_DEFINES_H

namespace GlobalDConfDefines {

/*!
 * \brief DConfig配置文件路径
 */
namespace ConfigPath {
inline constexpr char kDefaultCfgPath[] { "org.deepin.dde.file-manager" };
inline constexpr char kPluginsDConfName[] { "org.deepin.dde.file-manager.plugins" };
inline constexpr char kViewDConfName[] { "org.deepin.dde.file-manager.view" };
inline constexpr char kAnimationDConfName[] { "org.deepin.dde.file-manager.animation" };
}   // namespace ConfigPath

/*!
 * \brief 基础配置项
 */
namespace BaseConfig {
inline constexpr char kTreeViewEnable[] { "dfm.treeview.enable" };
inline constexpr char kDisplayPreviewVisibleKey[] { "dfm.displaypreview.visible" };
inline constexpr char kSidebarVisibleKey[] { "dfm.sidebar.visible" };
inline constexpr char kOpenFolderWindowsInASeparateProcess[] { "dfm.open.in.single.process" };
inline constexpr char kCunstomFixedTabs[] { "dfm.custom.fixedtab" };
inline constexpr char kPinnedTabs[] { "dfm.pinned.tabs" };
}   // namespace BaseConfig

/*!
 * \brief 动画相关配置项
 */
namespace AnimationConfig {
// 全局动画开关
inline constexpr char kAnimationEnable[] { "dfm.animation.enable" };

// 工作区切换动画
inline constexpr char kAnimationEnterEnable[] { "dfm.animation.enter.enable" };
inline constexpr char kAnimationEnterDuration[] { "dfm.animation.enter.duration" };
inline constexpr char kAnimationEnterCurve[] { "dfm.animation.enter.curve" };
inline constexpr char kAnimationEnterScale[] { "dfm.animation.enter.scale" };
inline constexpr char kAnimationEnterOpacity[] { "dfm.animation.enter.opacity" };

// 布局动画
inline constexpr char kAnimationLayoutEnable[] { "dfm.animation.layout.enable" };
inline constexpr char kAnimationLayoutDuration[] { "dfm.animation.layout.duration" };
inline constexpr char kAnimationLayoutCurve[] { "dfm.animation.layout.curve" };

// 详情视图动画
inline constexpr char kAnimationDetailviewEnable[] { "dfm.animation.detailview.enable" };
inline constexpr char kAnimationDetailviewDuration[] { "dfm.animation.detailview.duration" };
inline constexpr char kAnimationDetailviewCurve[] { "dfm.animation.detailview.curve" };

// 侧边栏动画
inline constexpr char kAnimationSidebarEnable[] { "dfm.animation.sidebar.enable" };
inline constexpr char kAnimationSidebarDuration[] { "dfm.animation.sidebar.duration" };
inline constexpr char kAnimationSidebarCurve[] { "dfm.animation.sidebar.curve" };

// 重排序动画
inline constexpr char kAnimationResortEnable[] { "dd.animation.resort.enable" };
inline constexpr char kAnimationResortCustomDuration[] { "dd.animation.resort.custom.duration" };
inline constexpr char kAnimationResortCustomCurve[] { "dd.animation.resort.custom.curve" };
inline constexpr char kAnimationResortAlignDuration[] { "dd.animation.resort.align.duration" };
inline constexpr char kAnimationResortAlignCurve[] { "dd.animation.resort.align.curve" };
}   // namespace AnimationConfig

}   // namespace GlobalDConfDefines

#endif   // GLOBAL_DCONF_DEFINES_H
