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
#include "openwithhelper.h"
#include "views/openwith/openwithwidget.h"

#include "dfm-base/base/schemefactory.h"

DFMBASE_USE_NAMESPACE
DPUTILS_USE_NAMESPACE
OpenWithHelper::OpenWithHelper(QObject *parent)
    : QObject(parent)
{
}

QWidget *OpenWithHelper::createOpenWithWidget(const QUrl &url)
{
    if (url.isValid()) {
        AbstractFileInfoPointer fileInfo = InfoFactory::create<AbstractFileInfo>(url);
        if (fileInfo.isNull())
            return nullptr;
        if (fileInfo->isDir())
            return nullptr;
        OpenWithWidget *openWidget = new OpenWithWidget;
        openWidget->selectFileUrl(url);
        return openWidget;
    }
    return nullptr;
}
