/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lixiang<lixianga@uniontech.com>
 *
 * Maintainer: lixiang<lixianga@uniontech.com>
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
#include "computerpropertyhelper.h"
#include "propertydialog/views/computerpropertydialog.h"
#include "dfm-base/base/urlroute.h"
#include "dfm-base/utils/fileutils.h"

DFMBASE_USE_NAMESPACE
CPY_USE_NAMESPACE
QString ComputerPropertyHelper::scheme()
{
    return "computer";
}

QWidget *ComputerPropertyHelper::createComputerProperty(const QUrl &url)
{
    static ComputerPropertyDialog *widget = nullptr;
    if (UrlRoute::isRootUrl(url) || FileUtils::isComputerDesktopFile(url)) {
        if (!widget) {
            widget = new ComputerPropertyDialog;
            return widget;
        }
        return widget;
    }
    return nullptr;
}
