/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef TITLEBAR_DEFINES_H
#define TITLEBAR_DEFINES_H

#include "dfm_filemanager_service_global.h"

#include <QObject>
#include <QUrl>

#include <functional>

DSB_FM_BEGIN_NAMESPACE

namespace TitleBar {
// item of CrumbBar
struct CrumbData
{
public:
    CrumbData(QUrl theUrl = QUrl(), QString theDisplayText = QString(), QString theIconName = QString())
        : url(theUrl), displayText(theDisplayText), iconName(theIconName)
    {
    }

    QUrl url {};
    QString displayText;
    QString iconName;
};

using supportedUrlCallback = std::function<bool(const QUrl &)>;
using seprateUrlCallback = std::function<QList<CrumbData>(const QUrl &)>;

struct CustomCrumbInfo
{
    QString scheme;
    bool keepAddressBar { false };   // don't lost focus at addressbar, only search use it now
    bool hideListViewBtn { false };
    bool hideIconViewBtn { false };
    bool hideDetailSpaceBtn { false };
    supportedUrlCallback supportedCb { nullptr };
    seprateUrlCallback seperateCb { nullptr };
};

// custom event type
namespace EventType {
extern const int kSwitchMode;
extern const int kDoSearch;
extern const int kStopSearch;
extern const int kShowFilterView;
extern const int kStartSpinner;
extern const int kStopSpinner;
extern const int kShowFilterButton;
};   // namespace EventType

// setting menu action list
enum MenuAction {
    kNewWindow,
    kConnectToServer,
    kSetUserSharePassword,
    kSettings
};
}   // namespace TitleBar

DSB_FM_END_NAMESPACE

Q_DECLARE_METATYPE(DSB_FM_NAMESPACE::TitleBar::CustomCrumbInfo);
Q_DECLARE_METATYPE(DSB_FM_NAMESPACE::TitleBar::CrumbData);
Q_DECLARE_METATYPE(DSB_FM_NAMESPACE::TitleBar::MenuAction);
Q_DECLARE_METATYPE(DSB_FM_NAMESPACE::TitleBar::supportedUrlCallback);
Q_DECLARE_METATYPE(DSB_FM_NAMESPACE::TitleBar::seprateUrlCallback);

#endif   // TITLEBAR_DEFINES_H
