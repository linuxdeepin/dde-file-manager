/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuyangming<liuyangming@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             max-lv<lvwujun@uniontech.com>
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
#include "tageventreceiver.h"
#include "utils/tagmanager.h"

#include "dfm-base/dfm_event_defines.h"
#include "dfm-base/base/application/application.h"
#include "dfm-base/base/application/settings.h"
#include "dfm-base/base/schemefactory.h"

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

    if (!ok)
        return;

    for (const QUrl &url : srcUrls) {
        const auto &tags = TagManager::instance()->getTagsByUrls({ url }, true).toStringList();
        if (tags.isEmpty())
            continue;

        TagManager::instance()->removeTagsOfFiles(tags, { url });
        const QUrl &newUrl = destUrls.at(srcUrls.indexOf(url));
        if (TagManager::instance()->canTagFile(newUrl))
            TagManager::instance()->addTagsForFiles(tags, { newUrl });
    }
}

void TagEventReceiver::handleFileRemoveResult(const QList<QUrl> &srcUrls, bool ok, const QString &errMsg)
{
    Q_UNUSED(errMsg)

    if (!ok)
        return;

    for (const QUrl &url : srcUrls) {
        const auto &tags = TagManager::instance()->getTagsByUrls({ url }, true).toStringList();
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
        const auto &tags = TagManager::instance()->getTagsByUrls({ iter.key() }, true).toStringList();
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
    const auto &rec = TagManager::instance()->getTagsByUrls({ url }, true).toStringList();
    if (rec.isEmpty())
        return {};

    return rec;
}

void TagEventReceiver::handleSidebarOrderChanged(quint64 winId, const QString &group)
{
    if (group != "Tag")
        return;
    auto items = dpfSlotChannel->push("dfmplugin_sidebar", "slot_Group_UrlList", winId, group);
    auto urls = items.value<QList<QUrl>>();

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
