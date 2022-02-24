/*
 * Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co., Ltd.
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
#ifndef WORKSPACE_DEFINES_H
#define WORKSPACE_DEFINES_H

#include "dfm_filemanager_service_global.h"
#include "dfm-base/interfaces/abstractfileinfo.h"

#include <QObject>
#include <QVariant>

#include <functional>

QT_BEGIN_NAMESPACE
class QWidget;
QT_END_NAMESPACE

DFMBASE_BEGIN_NAMESPACE
class AbstractFileInfo;
DFMBASE_END_NAMESPACE

DSB_FM_BEGIN_NAMESPACE

namespace Workspace {

// custom event type
namespace EventType {
extern const int kTabAdded;
extern const int kTabChanged;
extern const int kTabMoved;
extern const int kTabRemoved;
extern const int kCloseTabs;
extern const int kShowCustomTopWidget;
};   // namespace EventType

using CreateTopWidgetCallback = std::function<QWidget *()>;
using ShowTopWidgetCallback = std::function<bool(QWidget *, const QUrl &)>;
using FileViewFilterCallback = std::function<bool(DFMBASE_NAMESPACE::AbstractFileInfo *, QVariant)>;

struct CustomTopWidgetInfo
{
    QString scheme;
    bool keepShow { false };   // always show
    CreateTopWidgetCallback createTopWidgetCb { nullptr };
    ShowTopWidgetCallback showTopWidgetCb { nullptr };
};

}   // namespace Workspace

DSB_FM_END_NAMESPACE

Q_DECLARE_METATYPE(DSB_FM_NAMESPACE::Workspace::CustomTopWidgetInfo);
Q_DECLARE_METATYPE(DSB_FM_NAMESPACE::Workspace::CreateTopWidgetCallback);
//Q_DECLARE_METATYPE(DSB_FM_NAMESPACE::Workspace::ShowTopWidgetCallback);
Q_DECLARE_METATYPE(DSB_FM_NAMESPACE::Workspace::FileViewFilterCallback);

#endif   // WORKSPACE_DEFINES_H
