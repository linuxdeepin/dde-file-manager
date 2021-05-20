/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
 *             hujianzhong<hujianzhong@uniontech.com>
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

#include <QDebug>

#include "app/define.h"
#include "dfileservices.h"
#include "tagcontroller.h"
#include "tag/tagmanager.h"
#include "shutil/fileutils.h"
#include "../tag/tagmanager.h"
#include "dialogs/dialogmanager.h"
#include "../models/tagfileinfo.h"
#include "../interfaces/dfileinfo.h"
#include "interfaces/dfileservices.h"
#include "controllers/appcontroller.h"
#include "private/dabstractfilewatcher_p.h"
#include "controllers/tagmanagerdaemoncontroller.h"
#include "dfileproxywatcher.h"
#include "dstorageinfo.h"
#include <dfilesystemmodel.h>

#include "controllers/vaultcontroller.h"
#include "models/vaultfileinfo.h"

template<typename Ty>
using citerator = typename QList<Ty>::const_iterator;

template<typename Ty>
using itetrator = typename QList<Ty>::iterator;

TagController::TagController(QObject* const parent)
              :DAbstractFileController{ parent }
{
       //constructor!
}

const DAbstractFileInfoPointer TagController::createFileInfo(const QSharedPointer<DFMCreateFileInfoEvent>& event) const
{
    //! 如过在标记中有保险箱的文件需要创建保险箱的fileinfo
    if (!dynamic_cast<const DFileSystemModel*>(event->sender().data())) {
        if(VaultController::isVaultFile(event->url().fragment())) {
            return DAbstractFileInfoPointer(new VaultFileInfo(event->url()));
        }
    }

    DAbstractFileInfoPointer TaggedFilesInfo{ new TagFileInfo{ event->url() } };
    return TaggedFilesInfo;
}

const QList<DAbstractFileInfoPointer> TagController::getChildren(const QSharedPointer<DFMGetChildrensEvent>& event) const
{
    DUrl currentUrl{ event->url() };
    QList<DAbstractFileInfoPointer> infoList;


    if(currentUrl.isTaggedFile()){
        QString path{ currentUrl.path() };

        if(path == QString{"/"}){
            QMap<QString, QString> tags{ TagManager::instance()->getAllTags() };
            QMap<QString, QString>::const_iterator tagBeg{ tags.cbegin() };
            QMap<QString, QString>::const_iterator tagEnd{ tags.cend() };

            for(; tagBeg != tagEnd; ++tagBeg){
                DUrl url = DUrl::fromUserTaggedFile(tagBeg.key(), QString{});
                DAbstractFileInfoPointer tagInfoPtr {
                    DFileService::instance()->createFileInfo(this, url)
                };
                infoList.push_back(tagInfoPtr);
            }

        } else if (currentUrl.taggedLocalFilePath().isEmpty()) {
            QString tagName{ currentUrl.tagName() };
            QList<QString> files{ TagManager::instance()->getFilesThroughTag(tagName) };

            for(const QString& localFilePath : files){
                DUrl url{ currentUrl };
                url.setTaggedFileUrl(localFilePath);
                DAbstractFileInfoPointer fileInfo{ new TagFileInfo(url) };

                infoList.push_back(fileInfo);
            }
        }
    }

    return infoList;
}

class TaggedFileWatcherPrivate;
class TaggedFileWatcher final : public DAbstractFileWatcher
{
public:
    explicit TaggedFileWatcher(const DUrl& url, QObject* const parent = nullptr);
    void setEnabledSubfileWatcher(const DUrl &subfileUrl, bool enabled = true);

private:
    Q_DECLARE_PRIVATE(TaggedFileWatcher)

    void addWatcher(const DUrl& url) noexcept;
    void removeWatcher(const DUrl& url) noexcept;
};


class TaggedFileWatcherPrivate final : public DAbstractFileWatcherPrivate
{
public:
    explicit TaggedFileWatcherPrivate(TaggedFileWatcher* qq)
        :DAbstractFileWatcherPrivate{qq}{}

    virtual bool start() override;
    virtual bool stop() override;

    Q_DECLARE_PUBLIC(TaggedFileWatcher)

    DUrl m_urlBak{};
    QString m_beWatchedPath{};
    QMap<DUrl, DAbstractFileWatcher*> m_watchers{};
};


TaggedFileWatcher::TaggedFileWatcher(const DUrl& url, QObject* const parent)
    :DAbstractFileWatcher{*(new TaggedFileWatcherPrivate{this}), url, parent}
{
    TaggedFileWatcherPrivate* d{ d_func() };
    d->m_beWatchedPath =  url.path();

#ifdef QT_DEBUG
    qDebug()<< "watched url: " << url;
    qDebug()<< "watched path: " << url.path();
#endif

}

void TaggedFileWatcher::setEnabledSubfileWatcher(const DUrl& subfileUrl, bool enabled)
{
    DUrl currentWatchedDir{ this->fileUrl() };

    if (subfileUrl == currentWatchedDir)
        return;

    if (enabled) {
        this->addWatcher(subfileUrl);
    } else {
        this->removeWatcher(subfileUrl);
    }
}

void TaggedFileWatcher::addWatcher(const DUrl& url)noexcept
{
    TaggedFileWatcherPrivate* d{ d_func() };
    DUrl local_file_url{ DUrl::fromLocalFile(url.taggedLocalFilePath()) };

    if (!local_file_url.isValid() || d->m_watchers.contains(url))
        return;

    DAbstractFileWatcher* watcher{ DFileService::instance()->createFileWatcher(this, local_file_url) };

    if(!watcher)
        return;

    watcher->moveToThread(this->thread());
    watcher->setParent(this);
    d->m_watchers[url] = watcher;

    auto urlConvert = [this] (const DUrl &localUrl) {
        DUrl new_url = this->fileUrl();
        new_url.setTaggedFileUrl(localUrl.toLocalFile());

        return new_url;
    };

    connect(watcher, &DAbstractFileWatcher::fileAttributeChanged, this, [this, urlConvert] (const DUrl &url) {
        emit fileAttributeChanged(urlConvert(url));
    });

    connect(watcher, &DAbstractFileWatcher::fileModified, this, [this, urlConvert] (const DUrl &url) {
        emit fileModified(urlConvert(url));
    });

    connect(watcher, &DAbstractFileWatcher::fileDeleted, this, [this, urlConvert] (const DUrl &url) {
        emit fileDeleted(urlConvert(url));
    });

    if(d->started)
        watcher->startWatcher();
}

void TaggedFileWatcher::removeWatcher(const DUrl& url)noexcept
{
    TaggedFileWatcherPrivate* d{ d_func() };
    DAbstractFileWatcher *watcher = d->m_watchers.take(url);

    if (!watcher){
        return;
    }

    watcher->deleteLater();
}


bool TaggedFileWatcherPrivate::start()
{
//    TaggedFileWatcher* q{q_func()};

    bool ok = true;

    for (DAbstractFileWatcher *watcher : m_watchers) {
        ok = ok && watcher->startWatcher();
    }

    return ok;
}

bool TaggedFileWatcherPrivate::stop()
{
    TaggedFileWatcher* q{q_func()};
    bool value{ QObject::disconnect(TagManager::instance(), nullptr, q, nullptr) };

    for (DAbstractFileWatcher *watcher : m_watchers) {
        value = value && watcher->stopWatcher();
    }

    return value;
}


DAbstractFileWatcher* TagController::createFileWatcher(const QSharedPointer<DFMCreateFileWatcherEvent>& event) const
{
#ifdef QT_DEBUG
    qDebug()<< "be watched url: " << event->url();
#endif

    if (!event->url().taggedLocalFilePath().isEmpty()) {
        DAbstractFileWatcher *base_watcher = DFileService::instance()->createFileWatcher(event->sender(), DUrl::fromLocalFile(event->url().taggedLocalFilePath()));
        const QString &tag_name = event->url().tagName();

        auto urlConvertFun = [tag_name] (const DUrl &baseUrl) {
            return DUrl::fromUserTaggedFile(tag_name, baseUrl.toLocalFile());
        };

        return new DFileProxyWatcher(event->url(), base_watcher, urlConvertFun);
    }

    return (new TaggedFileWatcher{event->url()});
}

static DUrl toLocalFile(const DUrl &url)
{
    const QString &local_file = url.taggedLocalFilePath();

    if (local_file.isEmpty())
        return DUrl();

    return DUrl::fromLocalFile(local_file);
}

static DUrlList toLocalFileList(const DUrlList &tagFiles)
{
    DUrlList list;

    for (const DUrl &url : tagFiles) {
        const DUrl &new_url = toLocalFile(url);

        if (new_url.isValid())
            list << new_url;
    }

    return list;
}

bool TagController::openFile(const QSharedPointer<DFMOpenFileEvent> &event) const
{
    const DUrl &local_file = toLocalFile(event->url());

    if (!local_file.isValid())
        return false;

    return DFileService::instance()->openFile(event->sender(), local_file);
}

bool TagController::openFiles(const QSharedPointer<DFMOpenFilesEvent> &event) const
{
    DUrlList fileUrls = event->urlList();
    DUrlList packUrl;
    QStringList pathList;
    bool result = false;

    for (DUrl fileUrl : fileUrls) {
        const DAbstractFileInfoPointer pfile = createFileInfo(dMakeEventPointer<DFMCreateFileInfoEvent>(this, fileUrl));

        if (pfile->isSymLink()) {
            const DAbstractFileInfoPointer &linkInfo = DFileService::instance()->createFileInfo(this, pfile->symLinkTarget());

            if (linkInfo && !linkInfo->exists()) {
                dialogManager->showBreakSymlinkDialog(linkInfo->fileName(), fileUrl);
                continue;
            }
            fileUrl = linkInfo->redirectedFileUrl();
        }

        if (FileUtils::isExecutableScript(fileUrl.toLocalFile())) {
            int code = dialogManager->showRunExcutableScriptDialog(fileUrl, event->windowId());
            result = FileUtils::openExcutableScriptFile(fileUrl.toLocalFile(), code) || result;
            continue;
        }

        if (FileUtils::isFileRunnable(fileUrl.toLocalFile()) && !pfile->isDesktopFile()) {
            int code = dialogManager->showRunExcutableFileDialog(fileUrl, event->windowId());
            result = FileUtils::openExcutableFile(fileUrl.toLocalFile(), code) || result;
            continue;
        }

        if (FileUtils::shouldAskUserToAddExecutableFlag(fileUrl.toLocalFile()) && !pfile->isDesktopFile()) {
            int code = dialogManager->showAskIfAddExcutableFlagAndRunDialog(fileUrl, event->windowId());
            result = FileUtils::addExecutableFlagAndExecuse(fileUrl.toLocalFile(), code) || result;
            continue;
        }

        packUrl << fileUrl;
        QString url = fileUrl.toLocalFile();
        if (FileUtils::isFileWindowsUrlShortcut(url)) {
            url = FileUtils::getInternetShortcutUrl(url);
        }
        pathList << url;
    }

    if (!pathList.empty()) {
        if (event->isEnter()) {
            result = FileUtils::openEnterFiles(pathList);
        }
        else {
            result = FileUtils::openFiles(pathList);
        }
        if (!result) {
            for (const DUrl &fileUrl : packUrl) {
                AppController::instance()->actionOpenWithCustom(dMakeEventPointer<DFMOpenFileEvent>(event->sender(), fileUrl)); // requestShowOpenWithDialog
            }
        }
    }

    return result;
}

bool TagController::openFileByApp(const QSharedPointer<DFMOpenFileByAppEvent> &event) const
{
    const DUrl &local_file = toLocalFile(event->url());

    if (!local_file.isValid())
        return false;

    return DFileService::instance()->openFileByApp(event->sender(), event->appName(), local_file);
}

bool TagController::openFilesByApp(const QSharedPointer<DFMOpenFilesByAppEvent> &event) const
{
    QList<DUrl> fileUrls;

    for (DUrl url: event->urlList()) {
        const DUrl &localFile = toLocalFile(url);

        if (!localFile.isValid())
            continue;

        fileUrls << localFile;
    }

    return DFileService::instance()->openFilesByApp(event->sender(), event->appName(), fileUrls);
}

bool TagController::compressFiles(const QSharedPointer<DFMCompressEvent> &event) const
{
    const DUrlList &list = toLocalFileList(event->fileUrlList());

    if (list.isEmpty())
        return false;

    return DFileService::instance()->compressFiles(event->sender(), list);
}

bool TagController::decompressFile(const QSharedPointer<DFMDecompressEvent> &event) const
{
    const DUrlList &list = toLocalFileList(event->fileUrlList());

    if (list.isEmpty())
        return false;

    return DFileService::instance()->decompressFile(event->sender(), list);
}

bool TagController::decompressFileHere(const QSharedPointer<DFMDecompressEvent> &event) const
{
    const DUrlList &list = toLocalFileList(event->fileUrlList());

    if (list.isEmpty())
        return false;

    return DFileService::instance()->decompressFileHere(event->sender(), list);
}

bool TagController::writeFilesToClipboard(const QSharedPointer<DFMWriteUrlsToClipboardEvent> &event) const
{
    const DUrlList &list = toLocalFileList(event->fileUrlList());

    if (list.isEmpty())
        return false;

    return DFileService::instance()->writeFilesToClipboard(event->sender(), event->action(), list);
}

bool TagController::renameFile(const QSharedPointer<DFMRenameEvent> &event) const
{
    const QString &local_file = event->fromUrl().taggedLocalFilePath();

    if (local_file.isEmpty()) {
        const QString &old_name = event->fromUrl().tagName();
        const QString &new_name = event->toUrl().tagName();

        return TagManager::instance()->changeTagName(qMakePair(old_name, new_name));
    }

    return DFileService::instance()->renameFile(event->sender(), DUrl::fromLocalFile(local_file), DUrl::fromLocalFile(event->toUrl().taggedLocalFilePath()));
}

bool TagController::deleteFiles(const QSharedPointer<DFMDeleteEvent> &event) const
{
    QStringList tagNames;
    DUrlList localFiles;

    for (auto oneUrl : event->urlList()) {
        if (!oneUrl.taggedLocalFilePath().isEmpty()) {
            localFiles << DUrl::fromLocalFile(oneUrl.taggedLocalFilePath());
        } else {
            tagNames.append(oneUrl.fileName());
        }
    }

    if (!localFiles.isEmpty()) {
        return DFileService::instance()->deleteFiles(event->sender(), localFiles, false, event->silent(), event->force());
    }

    return TagManager::instance()->deleteTags(tagNames);
}

DUrlList TagController::moveToTrash(const QSharedPointer<DFMMoveToTrashEvent> &event) const
{
    const DUrlList &list = toLocalFileList(event->fileUrlList());

    if (list.isEmpty())
        return list;

    return DFileService::instance()->moveToTrash(event->sender(), list);
}

DUrlList TagController::pasteFile(const QSharedPointer<DFMPasteEvent> &event) const
{
    DUrlList list;

    const DUrl &to_url = event->targetUrl();

    if (!to_url.taggedLocalFilePath().isEmpty()) {
        return DFileService::instance()->pasteFile(event->sender(), event->action(), DUrl::fromLocalFile(to_url.taggedLocalFilePath()), event->urlList());
    }

    if (to_url.tagName().isEmpty())
        return list;

    if (event->action() != DFMGlobal::CopyAction) {
        return list;
    }

    for (const DUrl &url : event->urlList()) {
        if (DFileService::instance()->makeTagsOfFiles(event->sender(), {url}, {to_url.tagName()}, {"just for not clear exist tags...."}))  {
            list << url;
        }
    }

    return list;
}

bool TagController::setPermissions(const QSharedPointer<DFMSetPermissionEvent> &event) const
{
    DUrl url = event->url();

    if (!url.taggedLocalFilePath().isEmpty()) {
        return DFileService::instance()->setPermissions(event->sender(), DUrl::fromLocalFile(url.taggedLocalFilePath()), event->permissions());
    }

    return false;
}

bool TagController::openFileLocation(const QSharedPointer<DFMOpenFileLocation> &event) const
{
    const DUrl &local_file = toLocalFile(event->url());

    if (!local_file.isValid())
        return false;

    return DFileService::instance()->openFileLocation(event->sender(), local_file);
}

bool TagController::addToBookmark(const QSharedPointer<DFMAddToBookmarkEvent> &event) const
{
    return DFileService::instance()->addToBookmark(event->sender(), DUrl::fromLocalFile(event->url().taggedLocalFilePath()));
}

bool TagController::removeBookmark(const QSharedPointer<DFMRemoveBookmarkEvent> &event) const
{
    DUrl destUrl = DUrl::fromLocalFile(event->url().taggedLocalFilePath());
    return DFileService::instance()->removeBookmark(nullptr, destUrl);
}

bool TagController::createSymlink(const QSharedPointer<DFMCreateSymlinkEvent> &event) const
{
    const DUrl &local_file = toLocalFile(event->fileUrl());

    if (!local_file.isValid())
        return false;

    return DFileService::instance()->createSymlink(event->sender(), local_file, event->toUrl());
}

bool TagController::shareFolder(const QSharedPointer<DFMFileShareEvent> &event) const
{
    const DUrl &local_file = toLocalFile(event->url());

    if (!local_file.isValid())
        return false;

    return DFileService::instance()->shareFolder(event->sender(), local_file, event->name(), event->isWritable(), event->allowGuest());
}

bool TagController::unShareFolder(const QSharedPointer<DFMCancelFileShareEvent> &event) const
{
    const DUrl &local_file = toLocalFile(event->url());

    if (!local_file.isValid())
        return false;

    return DFileService::instance()->unShareFolder(event->sender(), local_file);
}

bool TagController::openInTerminal(const QSharedPointer<DFMOpenInTerminalEvent> &event) const
{
    const DUrl &local_file = toLocalFile(event->url());

    if (!local_file.isValid())
        return false;

    return DFileService::instance()->openInTerminal(event->sender(), local_file);
}

bool TagController::setFileTags(const QSharedPointer<DFMSetFileTagsEvent> &event) const
{
    QList<QString> tags = event->tags();
    return DFileService::instance()->setFileTags(this, DUrl::fromLocalFile(event->url().taggedLocalFilePath()), tags);
}

bool TagController::removeTagsOfFile(const QSharedPointer<DFMRemoveTagsOfFileEvent> &event) const
{
    QList<QString> tags = event->tags();
    return DFileService::instance()->removeTagsOfFile(this, DUrl::fromLocalFile(event->url().taggedLocalFilePath()), tags);
}

QList<QString> TagController::getTagsThroughFiles(const QSharedPointer<DFMGetTagsThroughFilesEvent> &event) const
{
    DUrlList new_list;

    for (const DUrl &tag_url : event->urlList()) {
        const QString &file = tag_url.taggedLocalFilePath();

        if (!file.isEmpty())
            new_list << DUrl::fromLocalFile(file);
    }

    return DFileService::instance()->getTagsThroughFiles(this, new_list);
}

