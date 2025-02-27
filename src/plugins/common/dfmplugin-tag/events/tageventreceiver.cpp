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

    for (const QUrl &url : srcUrls) {
        const auto &tags = TagManager::instance()->getTagsByUrls({ url });
        if (tags.isEmpty())
            continue;

        TagManager::instance()->removeTagsOfFiles(tags, { url });
        const QUrl &newUrl = destUrls.at(srcUrls.indexOf(url));
        if (TagManager::instance()->canTagFile(newUrl))
            TagManager::instance()->addTagsForFiles(tags, { newUrl });
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

    for (const QUrl &url : srcUrls) {
        const auto &tags = TagManager::instance()->getTagsByUrls({ url });
        if (tags.isEmpty())
            continue;

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
        const auto &tags = TagManager::instance()->getTagsByUrls({ iter.key() });
        if (tags.isEmpty())
            continue;

        TagManager::instance()->removeTagsOfFiles(tags, { iter.key() });
        TagManager::instance()->addTagsForFiles(tags, { iter.value() });
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
