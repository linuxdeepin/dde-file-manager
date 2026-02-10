// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "vaultfilehelper.h"
#include "vaulthelper.h"
#include "events/vaulteventcaller.h"

#include <dfm-base/utils/fileutils.h>
#include <dfm-base/utils/desktopfile.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/utils/windowutils.h>
#include <dfm-base/file/local/localfilehandler.h>
#include <dfm-base/dfm_global_defines.h>

#include <dfm-framework/event/event.h>

#include <QUrl>
#include <QApplication>

#define OperatorFile(type, fromUrls, toUrl) \
    dpfSignalDispatcher->publish(type, 0, fromUrls, toUrl, DFMBASE_NAMESPACE::AbstractJobHandler::JobFlag::kNoHint, nullptr)

Q_DECLARE_METATYPE(QList<QUrl> *)

using namespace dfmplugin_vault;
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
    fmDebug() << "Vault: Cut file operation requested, "
              << "source count:" << sources.size() << "target:" << target.toString();

    if (target.scheme() != scheme()) {
        fmDebug() << "Vault: Target scheme mismatch, operation cancelled";
        return false;
    }

    QList<QUrl> actualUrls;
    for (const QUrl &url : sources) {
        if (DFMBASE_NAMESPACE::FileUtils::isComputerDesktopFile(url) || DFMBASE_NAMESPACE::FileUtils::isTrashDesktopFile(url)) {
            continue;
        } else {
            actualUrls << url;
        }
    }
    auto tmpFlags = flags;
    tmpFlags |= AbstractJobHandler::JobFlag::kCountProgressCustomize;
    const QUrl url = transUrlsToLocal({ target }).first();
    dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kCutFile, windowId, actualUrls, url, tmpFlags, nullptr);

    return true;
}

bool VaultFileHelper::copyFile(const quint64 windowId, const QList<QUrl> sources, const QUrl target, const AbstractJobHandler::JobFlags flags)
{
    fmDebug() << "Vault: Copy file operation requested, "
              << "source count:" << sources.size() << "target:" << target.toString();

    if (target.scheme() != scheme()) {
        fmDebug() << "Vault: Target scheme mismatch, operation cancelled";
        return false;
    }

    QList<QUrl> actualUrls;
    for (const QUrl &url : sources) {
        if (DFMBASE_NAMESPACE::FileUtils::isComputerDesktopFile(url) || DFMBASE_NAMESPACE::FileUtils::isTrashDesktopFile(url)) {
            continue;
        } else {
            actualUrls << url;
        }
    }

    // if use &, transUrlsToLocal return value will free, and url is invalid, app crash, the same below
    auto tmpFlags = flags;
    tmpFlags |= AbstractJobHandler::JobFlag::kCountProgressCustomize;
    const QUrl url = transUrlsToLocal({ target }).first();
    dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kCopy, windowId, actualUrls, url, tmpFlags, nullptr);
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
    fmDebug() << "Vault: Delete file operation requested, "
              << "source count:" << sources.size();

    if (sources.isEmpty()) {
        fmDebug() << "Vault: Empty sources list, operation cancelled";
        return false;
    }
    if (sources.first().scheme() != scheme()) {
        fmDebug() << "Vault: Source scheme mismatch, operation cancelled";
        return false;
    }

    DFMBASE_NAMESPACE::AbstractJobHandler::OperatorCallback callback = std::bind(&VaultFileHelper::callBackFunction, this, std::placeholders::_1);
    QList<QUrl> redirectedFileUrls = transUrlsToLocal(sources);
    dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kDeleteFiles,
                                 windowId,
                                 redirectedFileUrls, flags, Q_NULLPTR, QVariant(), callback);
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
    fmDebug() << "Vault: Rename file operation requested, "
              << "old URL:" << oldUrl.toString() << "new URL:" << newUrl.toString();

    if (oldUrl.scheme() != scheme()) {
        fmDebug() << "Vault: Old URL scheme mismatch, operation cancelled";
        return false;
    }

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
    if (urls.size() < 1)
        return false;

    if (!urlTo.isValid() || !urls.first().isValid())
        return false;

    bool fromVault = VaultHelper::isVaultFile(urls.first());
    bool toVault = VaultHelper::isVaultFile(urlTo);
    if (!fromVault && !toVault)
        return false;

    if (WindowUtils::keyAltIsPressed()) {
        *action = Qt::MoveAction;
    } else if (WindowUtils::keyCtrlIsPressed()) {
        *action = Qt::CopyAction;
    } else {
        if (fromVault && toVault)
            *action = Qt::MoveAction;
        else
            *action = Qt::CopyAction;
    }

    return true;
}

bool VaultFileHelper::handleDropFiles(const QList<QUrl> &fromUrls, const QUrl &toUrl)
{
    if (fromUrls.size() < 1)
        return false;

    if (!toUrl.isValid() || !fromUrls.first().isValid())
        return false;

    bool fromVault = VaultHelper::isVaultFile(fromUrls.first());
    bool toVault = VaultHelper::isVaultFile(toUrl);
    if (!fromVault && !toVault)
        return false;

    QList<QUrl> transformedUrls;
    DFMBASE_NAMESPACE::UniversalUtils::urlsTransformToLocal(fromUrls, &transformedUrls);
    if (WindowUtils::keyAltIsPressed()) {
        OperatorFile(DFMBASE_NAMESPACE::GlobalEventType::kCutFile, transformedUrls, toUrl);
    } else if (WindowUtils::keyCtrlIsPressed()) {
        OperatorFile(DFMBASE_NAMESPACE::GlobalEventType::kCopy, transformedUrls, toUrl);
    } else {
        if (fromVault && toVault) {
            OperatorFile(DFMBASE_NAMESPACE::GlobalEventType::kCutFile, transformedUrls, toUrl);
        } else {
            OperatorFile(DFMBASE_NAMESPACE::GlobalEventType::kCopy, transformedUrls, toUrl);
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

void VaultFileHelper::callBackFunction(const AbstractJobHandler::CallbackArgus args)
{
    JobHandlePointer jobHandle = args->value(AbstractJobHandler::CallbackKey::kJobHandle).value<JobHandlePointer>();
    if (jobHandle) {
        QApplication::setOverrideCursor(Qt::WaitCursor);
        connect(jobHandle.get(), &AbstractJobHandler::finishedNotify, this, &VaultFileHelper::handleFinishedNotify);
    }
}

void VaultFileHelper::handleFinishedNotify(const JobInfoPointer &jobInfo)
{
    Q_UNUSED(jobInfo)

    disconnect(qobject_cast<AbstractJobHandler *>(sender()), &AbstractJobHandler::finishedNotify, this, &VaultFileHelper::handleFinishedNotify);
    QApplication::restoreOverrideCursor();
}

QList<QUrl> VaultFileHelper::transUrlsToLocal(const QList<QUrl> &urls)
{
    QList<QUrl> urlsTrans {};
    if (VaultHelper::instance()->urlsToLocal(urls, &urlsTrans))
        return urlsTrans;

    return urls;
}
