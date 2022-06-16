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
DPTAG_USE_NAMESPACE

TagEventReceiver *TagEventReceiver::instance()
{
    static TagEventReceiver ins;
    return &ins;
}

void TagEventReceiver::initConnect()
{
    dpfSignalDispatcher->subscribe(GlobalEventType::kCutFileResult, this, &TagEventReceiver::handleFileCutResult);
    dpfSignalDispatcher->subscribe(GlobalEventType::kMoveToTrashResult, this, &TagEventReceiver::handleFileRemoveResult);
    dpfSignalDispatcher->subscribe(GlobalEventType::kDeleteFilesResult, this, &TagEventReceiver::handleFileRemoveResult);
    dpfSignalDispatcher->subscribe(GlobalEventType::kRenameFileResult, this, &TagEventReceiver::handleFileRenameResult);
    dpfSignalDispatcher->subscribe(GlobalEventType::kRenameFileResult, this, &TagEventReceiver::handleFilesRenameResult);
    dpfSignalDispatcher->subscribe(GlobalEventType::kRestoreFromTrashResult, this, &TagEventReceiver::handleRestoreFromTrashResult);

    dpfSlotChannel->connect("dfmplugin_tag", "slot_GetTags", this, &TagEventReceiver::handleGetTags);
}

void TagEventReceiver::handleFileCutResult(const QList<QUrl> &srcUrls, const QList<QUrl> &destUrls, bool ok, const QString &errMsg)
{
    Q_UNUSED(errMsg)

    if (!ok)
        return;

    for (const QUrl &url : srcUrls) {
        QStringList tags = TagManager::instance()->getTagsByUrls({ url });
        if (!tags.isEmpty()) {
            TagManager::instance()->removeTagsOfFiles(tags, { url });

            const QUrl &newUrl = destUrls.at(srcUrls.indexOf(url));
            auto info = InfoFactory::create<AbstractFileInfo>(newUrl);

            if (TagManager::instance()->canTagFile(info))
                TagManager::instance()->addTagsForFiles(tags, { newUrl });
        }
    }
}

void TagEventReceiver::handleFileRemoveResult(const QList<QUrl> &srcUrls, bool ok, const QString &errMsg)
{
    Q_UNUSED(errMsg)

    if (!ok)
        return;

    for (const QUrl &url : srcUrls) {
        QStringList tags = TagManager::instance()->getTagsByUrls({ url });
        if (!tags.isEmpty()) {
            TagManager::instance()->removeTagsOfFiles(tags, { url });
        }
    }
}

void TagEventReceiver::handleFileRenameResult(quint64 winId, const QList<QUrl> &srcUrls, bool ok, const QString &errMsg)
{
    Q_UNUSED(winId)
    Q_UNUSED(errMsg)

    if (!ok || srcUrls.size() != 2)
        return;

    QStringList tags = TagManager::instance()->getTagsByUrls({ srcUrls.at(0) });
    if (!tags.isEmpty()) {
        TagManager::instance()->removeTagsOfFiles(tags, { srcUrls.at(0) });
        TagManager::instance()->addTagsForFiles(tags, { srcUrls.at(1) });
    }
}

void TagEventReceiver::handleFilesRenameResult(quint64 winId, const QMap<QUrl, QUrl> &renamedUrls, bool ok, const QString &errMsg)
{
    if (!ok)
        return;

    auto iter = renamedUrls.constBegin();
    for (; iter != renamedUrls.constEnd(); ++iter) {
        handleFileRenameResult(winId, QList<QUrl>() << iter.key() << iter.value(), ok, errMsg);
    }
}

void TagEventReceiver::handleWindowUrlChanged(quint64 winId, const QUrl &url)
{
    if (url.scheme() == TagManager::scheme()) {
        QTimer::singleShot(0, this, [=] {
            QDir::Filters f = QDir::AllEntries | QDir::NoDotAndDotDot | QDir::System | QDir::Hidden;
            dpfSlotChannel->push("dfmplugin_workspace", "slot_SetViewFilter", winId, f);
        });
    }
}

void TagEventReceiver::handleRestoreFromTrashResult(const QList<QUrl> &srcUrls, const QList<QUrl> &destUrls, const QVariantList &customInfos, bool ok, const QString &errMsg)
{
    Q_UNUSED(errMsg)
    Q_UNUSED(srcUrls)
    Q_UNUSED(destUrls)

    if (!ok)
        return;

    for (const auto &info : customInfos) {
        QString infoStr = info.toString();
        if (!infoStr.contains("TagNameList"))
            continue;

        QTemporaryFile file;
        if (file.open()) {
            file.write(info.toByteArray());
            file.close();

            QSettings setting(file.fileName(), QSettings::NativeFormat);
            setting.beginGroup("Trash Info");
            setting.setIniCodec("utf-8");

            const auto tagNameList = setting.value("TagNameList").toStringList();
            if (!tagNameList.isEmpty()) {
                const auto &path = QByteArray::fromPercentEncoding(setting.value("Path").toByteArray());
                TagManager::instance()->addTagsForFiles(tagNameList, { QUrl::fromLocalFile(path) });
            }

            setting.endGroup();
        }
    }
}

QStringList TagEventReceiver::handleGetTags(const QUrl &url)
{
    return TagManager::instance()->getTagsByUrls({ url });
}

void TagEventReceiver::handleSidebarOrderChanged(quint64 winId, const QString &group)
{
    if (group != "Tag")
        return;
    auto items = dpfSlotChannel->push("dfmplugin_sidebar", "slot_GetGroupItems", winId, group);
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
