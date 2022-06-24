/*
* Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
*
* Author:     gongheng <gongheng@uniontech.com>
*
* Maintainer: zhengyouge <zhengyouge@uniontech.com>
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
#include "appendcompresshelper.h"

#include "dfm-framework/event/event.h"

#include <QProcess>
#include <QFileInfo>

DPUTILS_USE_NAMESPACE

AppendCompressHelper::AppendCompressHelper(QObject *parent)
    : QObject(parent)
{
}

bool AppendCompressHelper::setMouseStyle(const QUrl &toUrl, const QList<QUrl> &fromUrls, Qt::DropAction *dropAction)
{
    Q_ASSERT(dropAction);
    if (!fromUrls.isEmpty()) {
        if (canAppendCompress(fromUrls, toUrl)) {
            *dropAction = Qt::CopyAction;
            return true;
        }
    }
    return false;
}

bool AppendCompressHelper::dragDropCompress(const QUrl &toUrl, const QList<QUrl> &fromUrls)
{
    if (!fromUrls.isEmpty()) {
        if (canAppendCompress(fromUrls, toUrl)) {
            QString toFilePath = toUrl.toLocalFile();
            QStringList fromFilePath;
            int count = fromUrls.count();
            for (int i = 0; i < count; ++i) {
                fromFilePath << fromUrls.at(i).toLocalFile();
            }
            return appendCompress(toFilePath, fromFilePath);
        }
    }

    return false;
}

bool AppendCompressHelper::appendCompress(const QString &toFilePath, const QStringList &fromFilePaths)
{
    QStringList arguments { toFilePath };
    arguments << fromFilePaths;
    arguments << "dragdropadd";

    return QProcess::startDetached("deepin-compressor", arguments);
}

bool AppendCompressHelper::canAppendCompress(const QList<QUrl> &fromUrls, const QUrl &toUrl)
{
    if (!toUrl.isValid())
        return false;

    QString toFilePath = toUrl.toLocalFile();
    if (toFilePath.isEmpty())
        return false;

    if (dpfHookSequence->run("dfmplugin_utils", "hook_AppendCompress_Prohibit", fromUrls, toUrl)) {
        return false;
    }

    QFileInfo info(toFilePath);
    if (info.isFile() && info.isWritable() && (toFilePath.endsWith(".zip") || (toFilePath.endsWith(".7z") && !toFilePath.endsWith(".tar.7z")))) {
        return true;
    }

    return false;
}
