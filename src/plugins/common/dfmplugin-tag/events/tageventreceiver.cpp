// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "tageventreceiver.h"
#include "utils/tagmanager.h"

#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/base/application/settings.h>
#include <dfm-base/base/schemefactory.h>

#include <dfm-framework/dpf.h>

#include <QTimer>
#include <QDir>
#include <QSettings>
#include <QTemporaryFile>
#include <QRegularExpression>

Q_DECLARE_METATYPE(QDir::Filters);

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_tag;

TagEventReceiver *TagEventReceiver::instance()
{
    static TagEventReceiver ins;
    return &ins;
}

void TagEventReceiver::handleFileCutResult(const QList<QUrl> &srcUrls, const QList<QUrl> &destUrls, bool ok, const QString &errMsg)
{
    Q_UNUSED(errMsg)

    if (!ok || destUrls.isEmpty())
        return;

    if (srcUrls.size() != destUrls.size())
        return;

    for (int i = 0; i < srcUrls.size(); ++i) {
        const QUrl &srcUrl = srcUrls.at(i);
        const QUrl &destUrl = destUrls.at(i);
        processFileTags(srcUrl, destUrl, true);   // true means remove source tags
    }
}

void TagEventReceiver::handleFileCopyResult(const QList<QUrl> &srcUrls, const QList<QUrl> &destUrls, bool ok, const QString &errMsg)
{
    Q_UNUSED(errMsg)

    if (!ok || destUrls.isEmpty())
        return;

    if (srcUrls.size() != destUrls.size())
        return;

    for (int i = 0; i < srcUrls.size(); ++i) {
        const QUrl &srcUrl = srcUrls.at(i);
        const QUrl &destUrl = destUrls.at(i);
        processFileTags(srcUrl, destUrl, false);   // false means don't remove source tags
    }
}

void TagEventReceiver::handleHideFilesResult(const quint64 &winId, const QList<QUrl> &urls, bool ok)
{
    Q_UNUSED(winId)
    if (ok && !urls.isEmpty()) {
        for (const QUrl &url : urls) {
            const auto &tags = TagManager::instance()->getTagsByUrls({ url });
            if (tags.isEmpty())
                continue;

            TagManager::instance()->hideFiles(tags, { url });
        }
    }
}

void TagEventReceiver::handleFileRemoveResult(const QList<QUrl> &srcUrls, bool ok, const QString &errMsg)
{
    Q_UNUSED(errMsg)

    if (!ok)
        return;

    for (const auto &url : srcUrls) {
        auto info = InfoFactory::create<FileInfo>(url);
        if (!info)
            continue;

        const auto &path = info->pathOf(FileInfo::FilePathInfoType::kAbsoluteFilePath);
        TagManager::instance()->removeChildren(path);

        const auto &tags = TagManager::instance()->getTagsByUrls({ url });
        if (!tags.isEmpty())
            TagManager::instance()->removeTagsOfFiles(tags, { url });
    }
}

void TagEventReceiver::handleFileTrashedResult(const QList<QUrl> &srcUrls, bool ok, const QString &errMsg)
{
    Q_UNUSED(errMsg)

    if (!ok || srcUrls.isEmpty())
        return;

    for (const auto &url : srcUrls) {
        // Get file tags
        const auto &tags = TagManager::instance()->getTagsByUrls({ url });
        if (tags.isEmpty())
            continue;

        // Get file info
        auto info = InfoFactory::create<FileInfo>(url);
        if (!info)
            continue;

        // Get original path and inode
        QString originalPath = info->pathOf(FileInfo::FilePathInfoType::kAbsoluteFilePath);
        qint64 inode = info->extendAttributes(FileInfo::FileExtendedInfoType::kInode).toLongLong();

        if (originalPath.isEmpty() || inode <= 0) {
            fmWarning() << "Failed to get file info for trash operation - path:" << originalPath << "inode:" << inode;
            continue;
        }

        if (!tags.isEmpty())
            TagManager::instance()->removeTagsOfFiles(tags, { url });

        // Save to trash file tags table via TagManager
        TagManager::instance()->saveTrashFileTags(originalPath, static_cast<quint64>(inode), tags);
    }
}

void TagEventReceiver::handleTrashCleanedResult(const QList<QUrl> &destUrls, bool ok, const QString &errMsg)
{
    Q_UNUSED(errMsg)
    Q_UNUSED(destUrls)

    if (!ok)
        return;

    // Clear all trash file tags via TagManager
    TagManager::instance()->clearAllTrashTags();
}

void TagEventReceiver::handleFileRenameResult(quint64 winId, const QMap<QUrl, QUrl> &renamedUrls, bool ok, const QString &errMsg)
{
    Q_UNUSED(winId)
    Q_UNUSED(errMsg)

    if (!ok || renamedUrls.isEmpty())
        return;

    auto iter = renamedUrls.constBegin();
    for (; iter != renamedUrls.constEnd(); ++iter) {
        processFileTags(iter.key(), iter.value(), true);
    }
}

void TagEventReceiver::handleWindowUrlChanged(quint64 winId, const QUrl &url)
{
    if (url.scheme() == TagManager::scheme()) {
        QTimer::singleShot(0, this, [=] {
            QDir::Filters f = QDir::AllEntries | QDir::NoDotAndDotDot | QDir::System | QDir::Hidden;
            dpfSlotChannel->push("dfmplugin_workspace", "slot_View_SetFilter", winId, f);
        });
    }
}

void TagEventReceiver::handleRestoreFromTrashResult(const QList<QUrl> &srcUrls, const QList<QUrl> &destUrls, const QVariantList &customInfos, bool ok, const QString &errMsg)
{
    Q_UNUSED(errMsg)
    Q_UNUSED(srcUrls)

    if (!ok || destUrls.isEmpty() || customInfos.isEmpty())
        return;

    if (destUrls.size() != customInfos.size()) {
        fmWarning() << "Restore from trash: destination URL and customInfos count mismatch";
        return;
    }

    for (int i = 0; i < destUrls.size(); ++i) {
        const QUrl &destUrl = destUrls.at(i);
        QString customInfo = customInfos.at(i).toString();

        // Parse original path from trash info using regex
        static QRegularExpression pathRegex("^Path=(.+)$", QRegularExpression::MultilineOption);
        QRegularExpressionMatch match = pathRegex.match(customInfo);
        if (!match.hasMatch()) {
            fmWarning() << "Failed to parse original path from trash info";
            continue;
        }

        QString encodedPath = match.captured(1);
        QString originalPath = QUrl::fromPercentEncoding(encodedPath.toUtf8());
        if (originalPath.isEmpty()) {
            fmWarning() << "Original path is empty after decoding";
            continue;
        }

        // Get inode from destination file
        auto info = InfoFactory::create<FileInfo>(destUrl);
        if (!info)
            continue;

        qint64 inode = info->extendAttributes(FileInfo::FileExtendedInfoType::kInode).toLongLong();
        if (inode <= 0) {
            fmWarning() << "Failed to get inode for destination file:" << destUrl;
            continue;
        }

        // Query trash file tags via TagManager using original path and inode
        QStringList tags = TagManager::instance()->getTrashFileTags(originalPath, static_cast<quint64>(inode));
        if (tags.isEmpty())
            continue;

        // Restore tags to destination file via TagManager
        if (TagManager::instance()->addTagsForFiles(tags, { destUrl })) {
            fmInfo() << "Restored tags from trash - original path:" << originalPath << "destination:" << destUrl << "tags count:" << tags.size();

            // Remove trash record via TagManager
            TagManager::instance()->removeTrashFileTags(originalPath, static_cast<quint64>(inode));
        } else {
            fmWarning() << "Failed to restore tags for file:" << destUrl;
        }
    }
}

QStringList TagEventReceiver::handleGetTags(const QUrl &url)
{
    const auto &rec = TagManager::instance()->getTagsByUrls({ url });
    if (rec.isEmpty())
        return {};

    return rec;
}

void TagEventReceiver::handleSidebarOrderChanged(quint64 winId, const QString &group, QList<QUrl> urls)
{
    if (group != "Group_Tag")
        return;

    QVariantList lst;
    for (auto &url : urls) {
        url.setQuery(QString("tagname=%1").arg(url.path().remove("/")));
        lst << url.toString();
    }
    if (!lst.isEmpty())
        Application::genericSetting()->setValue(kSidebarOrder, kTagOrderKey, lst);
}

TagEventReceiver::TagEventReceiver(QObject *parent)
    : QObject(parent)
{
}

void TagEventReceiver::addTagsToUrl(const QUrl &url, const QStringList &tags)
{
    if (tags.isEmpty() || !TagManager::instance()->canTagFile(url))
        return;

    TagManager::instance()->addTagsForFiles(tags, { url });
}

void TagEventReceiver::processDirectoryTags(const QUrl &srcUrl, const QUrl &destUrl, bool shouldRemoveSource)
{
    auto srcInfo = InfoFactory::create<FileInfo>(srcUrl);
    auto destInfo = InfoFactory::create<FileInfo>(destUrl);
    if (!srcInfo || !destInfo)
        return;

    QString srcPath = srcInfo->pathOf(FileInfo::FilePathInfoType::kAbsoluteFilePath);
    QString destPath = destInfo->pathOf(FileInfo::FilePathInfoType::kAbsoluteFilePath);

    const auto &children = TagManager::instance()->findChildren(srcPath);
    for (auto it = children.cbegin(); it != children.cend(); ++it) {
        QString srcTagFile = it.key();
        QString destFile = srcTagFile.replace(0, srcPath.length(), destPath);

        if (std::filesystem::exists(destFile.toLocal8Bit().data()))
            addTagsToUrl(QUrl::fromLocalFile(destFile), it.value());

        if (shouldRemoveSource)
            TagManager::instance()->removeTagsOfFiles(it.value(), { QUrl::fromLocalFile(it.key()) });
    }
}

void TagEventReceiver::processFileTags(const QUrl &srcUrl, const QUrl &destUrl, bool shouldRemoveSource)
{
    auto info = InfoFactory::create<FileInfo>(srcUrl);
    if (!info) {
        fmWarning() << "Failed to create FileInfo for:" << srcUrl.toString();
        return;
    }

    QStringList tags = TagManager::instance()->getTagsByUrls({ srcUrl });
    if (!tags.isEmpty()) {
        if (shouldRemoveSource)
            TagManager::instance()->removeTagsOfFiles(tags, { srcUrl });
        addTagsToUrl(destUrl, tags);
    }

    // Process directory if applicable
    if (shouldRemoveSource || info->isAttributes(FileInfo::FileIsType::kIsDir))
        processDirectoryTags(srcUrl, destUrl, shouldRemoveSource);
}
