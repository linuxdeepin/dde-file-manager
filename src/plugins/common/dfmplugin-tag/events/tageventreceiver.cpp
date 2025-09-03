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
    Q_UNUSED(destUrls)
    Q_UNUSED(customInfos)

    // TODO(zhangs): save or restore taginfo

    if (!ok)
        return;
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
