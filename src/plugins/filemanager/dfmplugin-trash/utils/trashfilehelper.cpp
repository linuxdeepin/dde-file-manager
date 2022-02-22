/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: huangyu<huangyub@uniontech.com>
 *             liuyangming<liuyangming@uniontech.com>
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

#include "trashfilehelper.h"
#include "trashhelper.h"
#include "events/trasheventcaller.h"

#include "dfm-base/base/schemefactory.h"
#include "dfm-base/utils/dialogmanager.h"

#include <QFileInfo>

DPTRASH_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

bool TrashFileHelper::openFilesHandle(quint64 windowId, const QList<QUrl> urls, const QString *error)
{
    Q_UNUSED(error)

    bool isOpenFile = false;
    QList<QUrl> redirectedFileUrls;
    for (const QUrl &url : urls) {
        QUrl redirectedFileUrl = TrashHelper::toLocalFile(url);
        QFileInfo fileInfo(redirectedFileUrl.path());
        if (fileInfo.isFile()) {
            isOpenFile = true;
            continue;
        }
        redirectedFileUrls << url;
    }
    if (!redirectedFileUrls.isEmpty())
        TrashEventCaller::sendOpenFiles(windowId, redirectedFileUrls);

    // Todo(yanghao)
    if (isOpenFile) {
        QString strMsg = QObject::tr("Unable to open items in the trash, please restore it first");
        DialogManagerInstance->showMessageDialog(DialogManager::kMsgWarn, strMsg);
    }
    return true;
}

bool TrashFileHelper::writeToClipBoardHandle(const quint64 windowId, const ClipBoard::ClipboardAction action, const QList<QUrl> urls)
{
    QList<QUrl> redirectedFileUrls;
    for (QUrl url : urls) {
        url.setScheme(SchemeTypes::kFile);
        redirectedFileUrls << url;
    }
    // Todo(yanghao)
    return true;
}
