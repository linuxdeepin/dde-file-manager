// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFMPLUGIN_WORKSPACE_GLOBAL_H
#define DFMPLUGIN_WORKSPACE_GLOBAL_H

#include <dfm-base/dfm_log_defines.h>
#include <dfm-framework/event/eventhelper.h>

#include <dtkwidget_config.h>

#include <QList>
#include <QDir>
#include <QVariantMap>

#define DPWORKSPACE_NAMESPACE dfmplugin_workspace

#define DPWORKSPACE_BEGIN_NAMESPACE namespace DPWORKSPACE_NAMESPACE {
#define DPWORKSPACE_END_NAMESPACE }
#define DPWORKSPACE_USE_NAMESPACE using namespace DPWORKSPACE_NAMESPACE;

namespace dfmbase {
class FileInfo;
}

DPWORKSPACE_BEGIN_NAMESPACE
DFM_LOG_USE_CATEGORY(DPWORKSPACE_NAMESPACE)

enum class DirOpenMode : uint8_t {
    kOpenInCurrentWindow,
    kOpenNewWindow,
    kAwaysInCurrentWindow,
    // kForceOpenNewWindow // Todo(yanghao): ???
};

// view defines
inline QList<int> iconSizeList()
{
    return { 48, 64, 96, 128, 160, 192, 224, 256 };
}

inline QList<int> iconWidth()
{
    return { 44, 36, 28, 28, 28, 28, 28, 32 };
}

enum class RectOfItemType : uint8_t {
    kItemIconRect,
    kItemTreeArrowRect
};

// model defines
enum class ModelState : uint8_t {
    kIdle,
    kBusy
};
#ifdef DTKWIDGET_CLASS_DSizeMode
inline constexpr int kCompactIconViewSpacing { 0 };
inline constexpr int kCompactIconModeColumnPadding { 5 };
#endif

inline constexpr int kIconViewSpacing { 5 };   // icon模式下的间距的一半
inline constexpr int kListViewSpacing { 0 };
inline constexpr int kIconModeColumnPadding { 10 };
inline constexpr int kDefualtHeaderSectionWidth { 140 };
inline constexpr int kDefaultItemFileLastModifiedWidth { 145 };
inline constexpr int kMinimumHeaderSectionWidth { 120 };
inline constexpr int kListViewHeaderHeight { 36 };
inline constexpr int kListViewIconSize { 24 };
inline constexpr int kLeftPadding { 10 };
inline constexpr int kRightPadding { 10 };
inline constexpr int kListModeLeftMargin { 10 };
inline constexpr int kListModeRightMargin { 10 };
inline constexpr int kColumnPadding { 10 };
inline constexpr int kMinMoveLenght { 3 };
inline constexpr int kIconHorizontalMargin { 15 };   // 水平Margin的宽度
inline constexpr int kCompactIconHorizontalMargin { 10 };
inline constexpr int kIconVerticalTopMargin { 15 };   // 水平Margin的宽度
inline constexpr int kCompactIconVerticalTopMargin { 10 };
inline constexpr int kTreeItemIndent { 17 };
inline constexpr int kTreeExpandArrowWidth { 20 };
inline constexpr int kTreeExpandArrowHeight { 20 };
inline constexpr int kTreeArrowAndIconDistance { 8 };

// tab defines
inline constexpr int kMaxTabCount { 8 };

// view select box
inline constexpr int kSelectBoxLineWidth { 2 };

inline char kWorkspaceUri[] { "org.deepin.filemanager.workspace" };

namespace PropertyKey {
inline constexpr char kScheme[] { "Property_Key_Scheme" };
inline constexpr char kKeepShow[] { "Property_Key_KeepShow" };
inline constexpr char kCreateTopWidgetCallback[] { "Property_Key_CreateTopWidgetCallback" };
inline constexpr char kShowTopWidgetCallback[] { "Property_Key_ShowTopWidgetCallback" };
}

using CreateTopWidgetCallback = std::function<QWidget *()>;
using ShowTopWidgetCallback = std::function<bool(QWidget *, const QUrl &)>;
using FileViewFilterCallback = std::function<bool(dfmbase::FileInfo *, QVariant)>;
using FileViewRoutePrehaldler = std::function<void(quint64 winId, const QUrl &, std::function<void()>)>;

struct CustomTopWidgetInfo
{
    QString scheme;
    bool keepShow { false };   // always show
    CreateTopWidgetCallback createTopWidgetCb { nullptr };
    ShowTopWidgetCallback showTopWidgetCb { nullptr };

    CustomTopWidgetInfo() = default;
    inline CustomTopWidgetInfo(const QVariantMap &map)
        : scheme { map[PropertyKey::kScheme].toString() },
          keepShow { map[PropertyKey::kKeepShow].toBool() },
          createTopWidgetCb { DPF_NAMESPACE::paramGenerator<CreateTopWidgetCallback>(map[PropertyKey::kCreateTopWidgetCallback]) },
          showTopWidgetCb { DPF_NAMESPACE::paramGenerator<ShowTopWidgetCallback>(map[PropertyKey::kShowTopWidgetCallback]) }
    {
    }
};

namespace AcName {
inline constexpr char kAcFileView[] { "file_view" };
inline constexpr char kAcFileviewMenu[] { "fileview_menu" };
inline constexpr char kAcViewTabBar[] { "TabBar" };
inline constexpr char kAcViewTabBarNewButton[] { "NewTabButton" };
inline constexpr char kAcViewTabBarTopLine[] { "top_line" };
inline constexpr char kAcViewTabBarBottomLine[] { "bottom_line" };
}

namespace DConfigInfo {
inline constexpr char kConfName[] { "org.deepin.dde.file-manager.preview" };
inline constexpr char kMtpThumbnailKey[] { "mtpThumbnailEnable" };
}

DPWORKSPACE_END_NAMESPACE

Q_DECLARE_METATYPE(DPWORKSPACE_NAMESPACE::CreateTopWidgetCallback);
Q_DECLARE_METATYPE(DPWORKSPACE_NAMESPACE::ShowTopWidgetCallback);
Q_DECLARE_METATYPE(DPWORKSPACE_NAMESPACE::FileViewFilterCallback);
Q_DECLARE_METATYPE(DPWORKSPACE_NAMESPACE::FileViewRoutePrehaldler);
Q_DECLARE_METATYPE(QString *)
Q_DECLARE_METATYPE(QVariant *)
Q_DECLARE_METATYPE(QDir::Filters)

#endif   // DFMPLUGIN_WORKSPACE_GLOBAL_H
