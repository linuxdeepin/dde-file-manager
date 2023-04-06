// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "vaultfilehelper.h"
#include "vaulthelper.h"
#include "events/vaulteventcaller.h"

#include <dfm-base/utils/fileutils.h>
#include <dfm-base/utils/desktopfile.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/file/local/localfilehandler.h>
#include <dfm-base/dfm_global_defines.h>

#include <dfm-framework/event/event.h>
#include <QUrl>

Q_DECLARE_METATYPE(QList<QUrl> *)

DPVAULT_USE_NAMESPACE
DFMGLOBAL_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

VaultFileHelper *VaultFileHelper::instance()
{
    static VaultFileHelper ins;
    return &ins;
}

VaultFileHelper::VaultFileHelper(QObject *parent)
    : QObject(parent)
{
}

bool VaultFileHelper::cutFile(const quint64 windowId, const QList<QUrl> sources, const QUrl target, const AbstractJobHandler::JobFlags flags)
{
    if (target.scheme() != scheme())
        return false;

    QList<QUrl> actualUrls;
    for (const QUrl &url : sources) {
        if (DFMBASE_NAMESPACE::FileUtils::isComputerDesktopFile(url) || DFMBASE_NAMESPACE::FileUtils::isTrashDesktopFile(url)) {
            continue;
        } else {
            actualUrls << url;
        }
    }
    const QUrl url = transUrlsToLocal({ target }).first();
    dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kCutFile, windowId, actualUrls, url, flags, nullptr);

    return true;
}

bool VaultFileHelper::copyFile(const quint64 windowId, const QList<QUrl> sources, const QUrl target, const AbstractJobHandler::JobFlags flags)
{
    if (target.scheme() != scheme())
        return false;

    QList<QUrl> actualUrls;
    for (const QUrl &url : sources) {
        if (DFMBASE_NAMESPACE::FileUtils::isComputerDesktopFile(url) || DFMBASE_NAMESPACE::FileUtils::isTrashDesktopFile(url)) {
            continue;
        } else {
            actualUrls << url;
        }
    }

    // if use &, transUrlsToLocal return value will free, and url is invalid, app crash, the same below
    const QUrl url = transUrlsToLocal({ target }).first();
    dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kCopy, windowId, actualUrls, url, flags, nullptr);
    return true;
}

bool VaultFileHelper::moveToTrash(const quint64 windowId, const QList<QUrl> sources, const AbstractJobHandler::JobFlags flags)
{
    if (sources.isEmpty())
        return false;
    if (!VaultHelper::isVaultFile(sources.first()))
        return false;

    QList<QUrl> redirectedFileUrls = transUrlsToLocal(sources);
    dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kDeleteFiles,
                                 windowId,
                                 redirectedFileUrls, flags, nullptr);
    return true;
}

bool VaultFileHelper::deleteFile(const quint64 windowId, const QList<QUrl> sources, const AbstractJobHandler::JobFlags flags)
{
    if (sources.isEmpty())
        return false;
    if (sources.first().scheme() != scheme())
        return false;

    QList<QUrl> redirectedFileUrls = transUrlsToLocal(sources);

    dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kDeleteFiles,
                                 windowId,
                                 redirectedFileUrls, flags, nullptr);
    return true;
}

bool VaultFileHelper::openFileInPlugin(quint64 windowId, const QList<QUrl> urls)
{
    if (urls.isEmpty())
        return false;
    if (urls.first().scheme() != scheme())
        return false;

    QList<QUrl> redirectedFileUrls = transUrlsToLocal(urls);

    if (!redirectedFileUrls.isEmpty())
        VaultEventCaller::sendOpenFiles(windowId, redirectedFileUrls);

    return true;
}

bool VaultFileHelper::renameFile(const quint64 windowId, const QUrl oldUrl, const QUrl newUrl, const AbstractJobHandler::JobFlags flags)
{
    if (oldUrl.scheme() != scheme())
        return false;

    const QUrl ourl = transUrlsToLocal({ oldUrl }).first();
    const QUrl nurl = transUrlsToLocal({ newUrl }).first();
    dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kRenameFile, windowId, ourl, nurl, flags);

    return true;
}

bool VaultFileHelper::makeDir(const quint64 windowId, const QUrl url,
                              const QUrl &targetUrl,
                              const QVariant custom,
                              AbstractJobHandler::OperatorCallback callback)
{
    if (url.scheme() != scheme())
        return false;

    const QUrl dirUrl = transUrlsToLocal({ url }).first();
    if (dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kMkdir, windowId, dirUrl)) {
        if (callback) {
            AbstractJobHandler::CallbackArgus args(new QMap<AbstractJobHandler::CallbackKey, QVariant>);
            args->insert(AbstractJobHandler::CallbackKey::kWindowId, QVariant::fromValue(windowId));
            args->insert(AbstractJobHandler::CallbackKey::kSourceUrls, QVariant::fromValue(QList<QUrl>() << url));
            args->insert(AbstractJobHandler::CallbackKey::kTargets, QVariant::fromValue(QList<QUrl>() << targetUrl));
            args->insert(AbstractJobHandler::CallbackKey::kSuccessed, QVariant::fromValue(true));
            args->insert(AbstractJobHandler::CallbackKey::kCustom, custom);
            callback(args);
        }
    }

    return true;
}

bool VaultFileHelper::touchFile(const quint64 windowId,
                                const QUrl url, const QUrl &targetUrl,
                                const DFMGLOBAL_NAMESPACE::CreateFileType type,
                                const QString &suffix,
                                const QVariant &custom, AbstractJobHandler::OperatorCallback callback,
                                QString *error)
{
    if (url.scheme() != scheme())
        return false;

    const QUrl dirUrl = transUrlsToLocal({ url }).first();
    dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kTouchFile, windowId, dirUrl, type, suffix);

    if (callback) {
        AbstractJobHandler::CallbackArgus args(new QMap<AbstractJobHandler::CallbackKey, QVariant>);
        args->insert(AbstractJobHandler::CallbackKey::kWindowId, QVariant::fromValue(windowId));
        args->insert(AbstractJobHandler::CallbackKey::kSourceUrls, QVariant::fromValue(QList<QUrl>() << url));
        args->insert(AbstractJobHandler::CallbackKey::kTargets, QVariant::fromValue(QList<QUrl>() << targetUrl));
        args->insert(AbstractJobHandler::CallbackKey::kCustom, custom);
        callback(args);
    }

    return true;
}

bool VaultFileHelper::touchCustomFile(const quint64 windowId, const QUrl url, const QUrl &targetUrl,
                                      const QUrl tempUrl, const QString &suffix,
                                      const QVariant &custom, AbstractJobHandler::OperatorCallback callback,
                                      QString *error)
{
    if (url.scheme() != scheme())
        return false;

    const QUrl dirUrl = transUrlsToLocal({ url }).first();
    dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kTouchFile,
                                 windowId, dirUrl, tempUrl, suffix);

    if (callback) {
        AbstractJobHandler::CallbackArgus args(new QMap<AbstractJobHandler::CallbackKey, QVariant>);
        args->insert(AbstractJobHandler::CallbackKey::kWindowId, QVariant::fromValue(windowId));
        args->insert(AbstractJobHandler::CallbackKey::kSourceUrls, QVariant::fromValue(QList<QUrl>() << url));
        args->insert(AbstractJobHandler::CallbackKey::kTargets, QVariant::fromValue(QList<QUrl>() << targetUrl));
        args->insert(AbstractJobHandler::CallbackKey::kCustom, custom);
        callback(args);
    }

    return true;
}

bool VaultFileHelper::writeUrlsToClipboard(const quint64 windowId, const DFMBASE_NAMESPACE::ClipBoard::ClipboardAction action, const QList<QUrl> urls)
{
    if (urls.isEmpty())
        return false;
    if (urls.first().scheme() != scheme())
        return false;

    QList<QUrl> redirectedFileUrls = transUrlsToLocal(urls);

    dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kWriteUrlsToClipboard, windowId, action, redirectedFileUrls);

    return true;
}

bool VaultFileHelper::renameFiles(const quint64 windowId, const QList<QUrl> urls, const QPair<QString, QString> replacePair, bool flg)
{
    if (urls.isEmpty())
        return false;
    if (urls.first().scheme() != scheme())
        return false;

    QList<QUrl> actualUrls = transUrlsToLocal(urls);

    dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kRenameFiles, windowId, actualUrls, replacePair, flg);

    return true;
}

bool VaultFileHelper::renameFilesAddText(const quint64 windowId, const QList<QUrl> urls, const QPair<QString, DFMBASE_NAMESPACE::AbstractJobHandler::FileNameAddFlag> replacePair)
{
    if (urls.isEmpty())
        return false;
    if (urls.first().scheme() != scheme())
        return false;

    QList<QUrl> actualUrls = transUrlsToLocal(urls);

    dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kRenameFiles, windowId, actualUrls, replacePair);

    return true;
}

bool VaultFileHelper::checkDragDropAction(const QList<QUrl> &urls, const QUrl &urlTo, Qt::DropAction *action)
{
    Q_UNUSED(urls);

    if (!urlTo.isValid() || urlTo.scheme() != scheme())
        return false;

    if (*action == Qt::MoveAction) {
        *action = Qt::CopyAction;
        return true;
    }

    return false;
}

bool VaultFileHelper::handleDropFiles(const QList<QUrl> &fromUrls, const QUrl &toUrl)
{
    QList<QUrl> transformedUrls;
    DFMBASE_NAMESPACE::UniversalUtils::urlsTransform(fromUrls, &transformedUrls);
    if (toUrl.scheme() == scheme()) {
        dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kCopy,
                                     0,
                                     transformedUrls,
                                     toUrl,
                                     DFMBASE_NAMESPACE::AbstractJobHandler::JobFlag::kNoHint, nullptr);
        return true;
    }

    return false;
}

bool VaultFileHelper::openFileByApp(const quint64 windowId, const QList<QUrl> urls, const QList<QString> apps)
{
    Q_UNUSED(windowId)

    if (urls.isEmpty())
        return false;

    const QUrl &url = urls.at(0);
    if (!VaultHelper::isVaultFile(url))
        return false;

    if (apps.isEmpty())
        return false;

    const QString &desktopFile = apps.at(0);
    if (desktopFile.isEmpty())
        return false;

    DFMBASE_NAMESPACE::DesktopFile d(desktopFile);
    if (d.desktopExec().contains("dde-file-manager") || d.desktopExec().contains("file-manager.sh")) {
        int count = urls.size();
        if (count > 1) {
            for (int i = 0; i < count; ++i)
                dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kOpenNewWindow, urls.at(i));
        } else {
            if (DFMBASE_NAMESPACE::UniversalUtils::checkLaunchAppInterface()) {
                QStringList filePathsStr {};
                for (const auto &url : urls)
                    filePathsStr << url.toString();
                DFMBASE_NAMESPACE::UniversalUtils::launchAppByDBus(desktopFile, filePathsStr);
            } else {
                dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kOpenNewWindow, urls.at(0));
            }
        }
    } else {
        QList<QUrl> localUrls;
        if (VaultHelper::instance()->urlsToLocal(urls, &localUrls)) {
            dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kOpenFilesByApp, 0, localUrls, apps);
        } else {
            qCritical() << "the list has no vault url!";
        }
    }

    return true;
}

bool VaultFileHelper::setPermision(const quint64 windowId,
                                   const QUrl url,
                                   const QFileDevice::Permissions permissions,
                                   bool *ok,
                                   QString *error)
{
    if (VaultHelper::instance()->scheme() != url.scheme())
        return false;

    const QList<QUrl> &localUrls = transUrlsToLocal(QList<QUrl>() << url);
    if (localUrls.isEmpty())
        return false;

    DFMBASE_NAMESPACE::LocalFileHandler fileHandler;
    bool succ = fileHandler.setPermissions(localUrls.at(0), permissions);
    if (!succ && error)
        *error = fileHandler.errorString();

    if (ok)
        *ok = succ;

    return true;
}

QList<QUrl> VaultFileHelper::transUrlsToLocal(const QList<QUrl> &urls)
{
    QList<QUrl> urlsTrans {};
    if (VaultHelper::instance()->urlsToLocal(urls, &urlsTrans))
        return urlsTrans;

    return urls;
}
