// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#include "filereplacer.h"
#include <dfm-base/utils/protocolutils.h>
#include <dfm-io/dfileinfo.h>

#include <QFileInfo>
#include <QRandomGenerator>

#include <unistd.h>
#include <errno.h>

DFMBASE_USE_NAMESPACE
USING_IO_NAMESPACE

DPFILEOPERATIONS_BEGIN_NAMESPACE

namespace FileReplacer {

bool shouldUseTemporaryFile(const QString &targetPath)
{
    QUrl targetUrl = QUrl::fromLocalFile(targetPath);

    // Only local filesystems
    if (ProtocolUtils::isRemoteFile(targetUrl)) {
        return false;
    }

    DFMIO::DFileInfo fileInfo(targetUrl);
    fileInfo.initQuerier();
    // Only if target exists
    if (!fileInfo.exists()) {
        return false;
    }

    // Symlinks are deleted directly, not replaced
    if (fileInfo.attribute(DFMIO::DFileInfo::AttributeID::kStandardIsSymlink).toBool()) {
        return false;
    }

    return true;
}

FileReplacementContext createReplacementContext(const QString &targetPath)
{
    FileReplacementContext ctx;

    if (!shouldUseTemporaryFile(targetPath)) {
        return ctx;   // Empty context
    }

    ctx.originalTargetPath = targetPath;
    ctx.temporaryFilePath = generateTemporaryPath(targetPath);
    return ctx;
}

QString generateTemporaryPath(const QString &targetPath)
{
    QFileInfo fileInfo(targetPath);
    QString dirPath = fileInfo.absolutePath();

    // 使用随机字符串作为临时文件名，避免与原始文件名长度相关的问题
    // 格式: .ddefileop-xxxxxxxx
    quint32 random = QRandomGenerator::global()->generate();
    QString suffix = QString::number(random, 16).mid(0, 8).rightJustified(8, '0');

    return QString("%1/.ddefileop-%2").arg(dirPath, suffix);
}

bool applyReplacement(const FileReplacementContext &context)
{
    if (!context.isReplacement()) {
        return true;   // No replacement needed
    }

    // Step 1: Remove original target file
    if (::unlink(context.originalTargetPath.toLocal8Bit().constData()) != 0 && errno != ENOENT) {
        fmWarning() << "Failed to unlink:" << context.originalTargetPath << "error:" << strerror(errno);
        ::unlink(context.temporaryFilePath.toLocal8Bit().constData());
        return false;
    }

    // Step 2: Atomic rename
    if (::rename(context.temporaryFilePath.toLocal8Bit().constData(),
                 context.originalTargetPath.toLocal8Bit().constData())
        != 0) {
        fmWarning() << "Failed to rename" << context.temporaryFilePath << "to"
                    << context.originalTargetPath << "error:" << strerror(errno);
        ::unlink(context.temporaryFilePath.toLocal8Bit().constData());
        return false;
    }

    fmDebug() << "Successfully replaced" << context.originalTargetPath;
    return true;
}

}   // namespace FileReplacer

DPFILEOPERATIONS_END_NAMESPACE
