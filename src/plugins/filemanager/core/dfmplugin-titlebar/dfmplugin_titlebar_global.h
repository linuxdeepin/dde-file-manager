// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFMPLUGIN_TITLEBAR_GLOBAL_H
#define DFMPLUGIN_TITLEBAR_GLOBAL_H

#include <dfm-base/dfm_log_defines.h>

#include <QObject>
#include <QUrl>
#include <QVariantMap>
#include <QDateTime>

#include <functional>

#define DPTITLEBAR_NAMESPACE dfmplugin_titlebar

#define DPTITLEBAR_BEGIN_NAMESPACE namespace DPTITLEBAR_NAMESPACE {
#define DPTITLEBAR_END_NAMESPACE }
#define DPTITLEBAR_USE_NAMESPACE using namespace DPTITLEBAR_NAMESPACE;

DPTITLEBAR_BEGIN_NAMESPACE
DFM_LOG_USE_CATEGORY(DPTITLEBAR_NAMESPACE)

inline constexpr int kIconLeftPadding { 9 };
inline constexpr int kTextLeftPadding { 32 };
inline constexpr int kIconWidth { 14 };
inline constexpr int kIconHeight { 14 };
inline constexpr int kItemHeight { 30 };
inline constexpr int kItemMargin { 8 };
inline constexpr int kCompleterMaxHeight { 260 };
inline constexpr int kToolButtonSize { 30 };
inline constexpr int kToolButtonIconSize { 14 };
inline constexpr int kMaxTabCount { 8 };
inline constexpr int kFolderItemHeight { 26 };
inline constexpr int kFolderIconSize { 16 };
inline constexpr int kFolderMinWidth { 120 };

namespace CustomKey {
inline constexpr char kUrl[] { "CrumbData_Key_Url" };
inline constexpr char kDisplayText[] { "CrumbData_Key_DisplayText" };
inline constexpr char kIconName[] { "CrumbData_Key_IconName" };

inline constexpr char kKeepAddressBar[] { "Property_Key_KeepAddressBar" };
inline constexpr char kHideListViewBtn[] { "Property_Key_HideListViewBtn" };
inline constexpr char kHideIconViewBtn[] { "Property_Key_HideIconViewBtn" };
inline constexpr char kHideTreeViewBtn[] { "Property_Key_HideTreeViewBtn" };
inline constexpr char kHideDetailSpaceBtn[] { "Property_Key_HideDetailSpaceBtn" };
}   // namespace CustomKey

// Setting menu action list
enum MenuAction {
    kNewWindow,
    kConnectToServer,
    kSetUserSharePassword,
    kChangeDiskPassword,
    kSettings,
    kOpenInNewTab
};

// error code of change disk password
enum DPCErrorCode {
    kNoError = 0,
    kAuthenticationFailed,
    kInitFailed,
    kDeviceLoadFailed,
    kPasswordChangeFailed,
    kPasswordWrong,
    kAccessDiskFailed,   // Unable to get the encrypted disk list
    kPasswordInconsistent   // Passwords of disks are different
};

using SeprateUrlCallback = std::function<QList<QVariantMap>(const QUrl &)>;

// item of CrumbBar
struct CrumbData
{
public:
    CrumbData(const QUrl &theUrl = QUrl(), const QString &theDisplayText = QString(), const QString &theIconName = QString())
        : url(theUrl), displayText(theDisplayText), iconName(theIconName)
    {
    }

    QUrl url {};
    QString displayText;
    QString iconName;
};

class IPHistroyData
{
public:
    IPHistroyData(const QString &str, const QDateTime &time)
        : lastAccessed(time)
    {
        int index = str.indexOf("://");
        accessedType = str.mid(0, index);
        ipData = str.mid(index + 3);
    }

    inline bool isRecentlyAccessed() const
    {
        QDateTime startTime = QDateTime::currentDateTime();
        QDateTime endTime = startTime.addDays(-7);

        return (lastAccessed <= startTime && lastAccessed >= endTime);
    }

    inline QVariantMap toVariantMap() const
    {
        QVariantMap map;
        QString ip = QString("%1://%2").arg(accessedType, ipData);
        map.insert("ip", ip);
        map.insert("lastAccessed", lastAccessed.toString(Qt::ISODate));

        return map;
    }

    inline bool operator==(const IPHistroyData &other) const
    {
        return (!this->ipData.compare(other.ipData)
                && !this->accessedType.compare(other.accessedType, Qt::CaseInsensitive));
    }

    QString accessedType;
    QString ipData;
    QDateTime lastAccessed;
};

namespace AcName {
inline constexpr char kAcComputerTitleBar[] { "computer_title_bar" };
inline constexpr char kAcComputerTitleBarAddress[] { "AddressToolBar" };
inline constexpr char kAcComputerTitleBarBackBtn[] { "back_button" };
inline constexpr char kAcComputerTitleBarForwardBtn[] { "forword_button" };
inline constexpr char kAcComputerTitleBarSearchBtn[] { "search_button" };
inline constexpr char kAcComputerTitleBarDetailBtn[] { "detail_button" };
inline constexpr char kAcComputerTitleBarViewOptionsBtn[] { "viewoptions_button" };
inline constexpr char kAcComputerTitleBarListViewBtn[] { "listView_button" };
inline constexpr char kAcComputerTitleBarIconViewBtn[] { "iconView_button" };
inline constexpr char kAcComputerTitleBarTreeViewBtn[] { "treeView_button" };
inline constexpr char kAcComputerTitleBarOptionBtnBox[] { "ContollerToolBar" };
inline constexpr char kAcComputerTitleBarAddBtn[] { "add_button" };
inline constexpr char kAcComputerTitleBarCollectionServerView[] { "collectionServer_view" };
inline constexpr char kAcComputerCrumbBarLeftArrow[] { "left_arrow" };
inline constexpr char kAcComputerCrumbBarRightArrow[] { "right_arrow" };
inline constexpr char kAcComputerCrumbBarListView[] { "crumb_list_view" };
inline constexpr char kAcViewTabBarNewButton[] { "NewTabButton" };
inline constexpr char kAcViewTabBar[] { "TabBar" };
}

DPTITLEBAR_END_NAMESPACE
Q_DECLARE_METATYPE(QList<QVariantMap> *);
Q_DECLARE_METATYPE(QUrl *);
Q_DECLARE_METATYPE(QString *);

#endif   // DFMPLUGIN_TITLEBAR_GLOBAL_H
