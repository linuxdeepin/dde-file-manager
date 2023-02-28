// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFMPLUGIN_TITLEBAR_GLOBAL_H
#define DFMPLUGIN_TITLEBAR_GLOBAL_H

#define DPTITLEBAR_BEGIN_NAMESPACE namespace dfmplugin_titlebar {
#define DPTITLEBAR_END_NAMESPACE }
#define DPTITLEBAR_USE_NAMESPACE using namespace dfmplugin_titlebar;
#define DPTITLEBAR_NAMESPACE dfmplugin_titlebar

#include <QObject>
#include <QUrl>
#include <QVariantMap>
#include <QDateTime>

#include <functional>

DPTITLEBAR_BEGIN_NAMESPACE

namespace CustomKey {
inline constexpr char kUrl[] { "CrumbData_Key_Url" };
inline constexpr char kDisplayText[] { "CrumbData_Key_DisplayText" };
inline constexpr char kIconName[] { "CrumbData_Key_IconName" };

inline constexpr char kKeepAddressBar[] { "Property_Key_KeepAddressBar" };
inline constexpr char kHideListViewBtn[] { "Property_Key_HideListViewBtn" };
inline constexpr char kHideIconViewBtn[] { "Property_Key_HideIconViewBtn" };
inline constexpr char kHideDetailSpaceBtn[] { "Property_Key_HideDetailSpaceBtn" };
}   // namespace CustomKey

// Setting menu action list
enum MenuAction {
    kNewWindow,
    kConnectToServer,
    kSetUserSharePassword,
    kChangeDiskPassword,
    kSettings
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

    inline bool operator==(const IPHistroyData &other)
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
inline constexpr char kAcComputerTitleBarBtnBox[] { "button_box" };
inline constexpr char kAcComputerTitleBarSearchBtn[] { "search_button" };
inline constexpr char kAcComputerTitleBarDetailBtn[] { "detail_button" };
inline constexpr char kAcComputerTitleBarOptionBtnBox[] { "ContollerToolBar" };

inline constexpr char kAcComputerCrumbBarLeftArrow[] { "left_arrow" };
inline constexpr char kAcComputerCrumbBarRightArrow[] { "right_arrow" };
inline constexpr char kAcComputerCrumbBarListView[] { "crumb_list_view" };
}

DPTITLEBAR_END_NAMESPACE
Q_DECLARE_METATYPE(QList<QVariantMap> *);

#endif   // DFMPLUGIN_TITLEBAR_GLOBAL_H
