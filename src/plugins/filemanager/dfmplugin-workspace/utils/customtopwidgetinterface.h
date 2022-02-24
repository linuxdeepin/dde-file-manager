/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: liuyangming<liuyangming@uniontech.com>
 *             gongheng<gongheng@uniontech.com>
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
#ifndef CUSTOMTOPWIDGETINTERFACE_H
#define CUSTOMTOPWIDGETINTERFACE_H

#include "dfmplugin_workspace_global.h"

#include "services/filemanager/workspace/workspace_defines.h"

#include <QObject>

QT_BEGIN_NAMESPACE
class QWidget;
QT_END_NAMESPACE

DPWORKSPACE_BEGIN_NAMESPACE

using DSB_FM_NAMESPACE::Workspace::CreateTopWidgetCallback;
using DSB_FM_NAMESPACE::Workspace::ShowTopWidgetCallback;

class CustomTopWidgetInterface : public QObject
{
    Q_OBJECT
public:
    explicit CustomTopWidgetInterface(QObject *parent = nullptr);

    QWidget *create();
    bool isShowFromUrl(QWidget *w, const QUrl &url);
    void setKeepShow(bool keep);
    bool isKeepShow() const;
    void registeCreateTopWidgetCallback(const CreateTopWidgetCallback &func);
    void registeCreateTopWidgetCallback(const ShowTopWidgetCallback &func);

private:
    bool keepShow { false };
    CreateTopWidgetCallback createTopWidgetFunc;
    ShowTopWidgetCallback showTopWidgetFunc;
};

DPWORKSPACE_END_NAMESPACE
#endif   // CUSTOMTOPWIDGETINTERFACE_H
