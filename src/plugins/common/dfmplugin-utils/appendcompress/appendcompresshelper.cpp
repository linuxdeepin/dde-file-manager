// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "appendcompresshelper.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/device/deviceutils.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/utils/protocolutils.h>

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
    QList<QUrl> transformedUrls;
    UniversalUtils::urlsTransformToLocal(fromUrls, &transformedUrls);
    if (!transformedUrls.isEmpty()) {
        if (canAppendCompress(transformedUrls, toUrl)) {
            QString toFilePath = toUrl.toLocalFile();
            QStringList fromFilePath;
            int count = transformedUrls.count();

            for (int i = 0; i < count; ++i) {
                const auto &info = InfoFactory::create<FileInfo>(transformedUrls.at(i));
                if (info && info->canAttributes(CanableInfoType::kCanRedirectionFileUrl)) {
                    fromFilePath << info->urlOf(UrlInfoType::kRedirectedFileUrl).path();
                } else {
                    fromFilePath << transformedUrls.at(i).toLocalFile();
                }
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
    if (!toUrl.isValid() || fromUrls.isEmpty())
        return false;

    QUrl localUrl = toUrl;
    QList<QUrl> urls {};
    bool ok = UniversalUtils::urlsTransformToLocal({ localUrl }, &urls);
    if (ok && !urls.isEmpty())
        localUrl = urls.first();

    QString toFilePath = localUrl.toLocalFile();
    if (toFilePath.isEmpty())
        return false;

    if (ProtocolUtils::isFTPFile(fromUrls.at(0)) || ProtocolUtils::isFTPFile(toUrl))
        return false;

    if (dpfHookSequence->run("dfmplugin_utils", "hook_AppendCompress_Prohibit", fromUrls, toUrl)) {
        return false;
    }

    const FileInfoPointer &info = InfoFactory::create<FileInfo>(toUrl);
    if (info && info->isAttributes(OptInfoType::kIsWritable) && isCompressedFile(toUrl))
        return true;

    return false;
}

bool AppendCompressHelper::isCompressedFile(const QUrl &toUrl)
{
    const FileInfoPointer &info = InfoFactory::create<FileInfo>(toUrl);
    if (info) {
        const QString &fileTypeName = info->nameOf(NameInfoType::kMimeTypeName);
        if (info->isAttributes(OptInfoType::kIsFile) && ((fileTypeName == "application/zip") || (fileTypeName == "application/x-7z-compressed" && !info->nameOf(NameInfoType::kFileName).endsWith(".tar.7z")))) {
            return true;
        }
    }
    return false;
}
