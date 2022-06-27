/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuyangming<liuyangming@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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
#include "tagoperationhelper.h"
#include "events/tageventcaller.h"

#include <QUrl>

using namespace dfmplugin_tag;

bool TagOperationHelper::openFilesHandle(quint64 windowId, const QList<QUrl> urls, const QString *error)
{
    Q_UNUSED(error)

    QList<QUrl> redirectedFileUrls;
    for (QUrl url : urls) {
        if (url.fragment().isEmpty()) {
            redirectedFileUrls.append(url);
        } else {
            QUrl redirectUrl = QUrl::fromLocalFile(url.fragment(QUrl::FullyEncoded));
            redirectedFileUrls.append(redirectUrl);
        }
    }

    TagEventCaller::sendOpenFiles(windowId, redirectedFileUrls);
    return true;
}
