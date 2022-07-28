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
#include "customtopwidgetinterface.h"

using namespace dfmplugin_workspace;
CustomTopWidgetInterface::CustomTopWidgetInterface(QObject *parent)
    : QObject(parent)
{
}

QWidget *CustomTopWidgetInterface::create()
{
    if (createTopWidgetFunc)
        return createTopWidgetFunc();
    return nullptr;
}

bool CustomTopWidgetInterface::isShowFromUrl(QWidget *w, const QUrl &url)
{
    return showTopWidgetFunc && showTopWidgetFunc(w, url);
}

void dfmplugin_workspace::CustomTopWidgetInterface::setKeepShow(bool keep)
{
    keepShow = keep;
}

bool CustomTopWidgetInterface::isKeepShow() const
{
    return keepShow;
}

void CustomTopWidgetInterface::registeCreateTopWidgetCallback(const ShowTopWidgetCallback &func)
{
    showTopWidgetFunc = func;
}

void dfmplugin_workspace::CustomTopWidgetInterface::registeCreateTopWidgetCallback(const CreateTopWidgetCallback &func)
{
    createTopWidgetFunc = func;
}
