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

#include "dfm-base/base/schemefactory.h"

#include <dfm-framework/event/event.h>

#include <QProcess>

Q_DECLARE_METATYPE(QList<QUrl> *)

DPUTILS_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

AppendCompressHelper::AppendCompressHelper(QObject *parent)
    : QObject(parent)
{
}

bool AppendCompressHelper::setMouseStyle(const QUrl &toUrl, const QList<QUrl> &fromUrls, Qt::DropAction *dropAction)
{
    Q_ASSERT(dropAction);
    if (!fromUrls.isEmpty()) {
        if (isCompressedFile(toUrl)) {
            if (canAppendCompress(fromUrls, toUrl))
                *dropAction = Qt::CopyAction;
            else
                *dropAction = Qt::IgnoreAction;
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
    QStringList arguments;
    QString cmd = "deepin-compressor";
#ifdef COMPILE_ON_V23
    cmd = "ll-cli";
    arguments << "run";
    arguments << "org.deepin.compressor";
    arguments << "--exec";
    arguments << "deepin-compressor";
#endif
    arguments << toFilePath;
    arguments << fromFilePaths;
    arguments << "dragdropadd";
    return QProcess::startDetached(cmd, arguments);
}

bool AppendCompressHelper::canAppendCompress(const QList<QUrl> &fromUrls, const QUrl &toUrl)
{
    if (!toUrl.isValid())
        return false;

    QUrl localUrl = toUrl;
    QList<QUrl> urls {};
    bool ok = dpfHookSequence->run("dfmplugin_utils", "hook_UrlsTransform", QList<QUrl>() << localUrl, &urls);
    if (ok && !urls.isEmpty())
        localUrl = urls.first();

    QString toFilePath = localUrl.toLocalFile();
    if (toFilePath.isEmpty())
        return false;

    if (dpfHookSequence->run("dfmplugin_utils", "hook_AppendCompress_Prohibit", fromUrls, toUrl)) {
        return false;
    }

    const AbstractFileInfoPointer &info = InfoFactory::create<AbstractFileInfo>(toUrl);
    if (info && info->isAttributes(OptInfoType::kIsWritable) && isCompressedFile(toUrl))
        return true;

    return false;
}

bool AppendCompressHelper::isCompressedFile(const QUrl &toUrl)
{
    const AbstractFileInfoPointer &info = InfoFactory::create<AbstractFileInfo>(toUrl);
    if (info) {
        const QString &fileTypeName = info->nameOf(NameInfoType::kMimeTypeName);
        if (info->isAttributes(OptInfoType::kIsFile) && ((fileTypeName == "application/zip") || (fileTypeName == "application/x-7z-compressed" && !info->nameOf(NameInfoType::kFileName).endsWith(".tar.7z")))) {
            return true;
        }
    }
    return false;
}
