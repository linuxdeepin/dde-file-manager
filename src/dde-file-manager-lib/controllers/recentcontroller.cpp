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

#include "recentcontroller.h"
#include "dfileservices.h"
#include "dfilewatcher.h"
#include "dfmevent.h"
#include "dfmglobal.h"
#include "private/dabstractfilewatcher_p.h"
#include "dialogs/dialogmanager.h"
#include "shutil/fileutils.h"
#include "app/define.h"
#include "vaultcontroller.h"
#include "singleton.h"
#include "app/filesignalmanager.h"

#include <QFileSystemWatcher>
#include <QXmlStreamReader>
#include <QDomDocument>
#include <QtConcurrent>
#include <QQueue>
#include <QTimer>
#include <QDebug>

#include <DRecentManager>
#include <DDialog>

DCORE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

class RecentFileWatcherPrivate;
class RecentFileWatcher : public DAbstractFileWatcher
{
public:
    explicit RecentFileWatcher(const DUrl &url, DAbstractFileWatcher *proxy, QObject *parent = nullptr);

    void setEnabledSubfileWatcher(const DUrl &subfileUrl, bool enabled = true) override;

private:
    void addWatcher(const DUrl &url);
    void removeWatcher(const DUrl &url);

    void onFileDeleted(const DUrl &url);
    void onFileAttributeChanged(const DUrl &url);
    void onFileModified(const DUrl &url);
    void onFileMoved(const DUrl &from, const DUrl &to);

public slots:
    void removeRecentFile(const QString &path);

    Q_DECLARE_PRIVATE(RecentFileWatcher)
};

class RecentFileWatcherPrivate : public DAbstractFileWatcherPrivate
{
public:
    explicit RecentFileWatcherPrivate(DAbstractFileWatcher *qq)
        : DAbstractFileWatcherPrivate(qq) {}

    bool start() override
    {
        started = true;

        return proxy && proxy->startWatcher();;
    }

    bool stop() override
    {
        started = false;

        return proxy && proxy->stopWatcher();
    }

    QMap<DUrl, DAbstractFileWatcher *> urlToWatcherMap;
    QPointer<DAbstractFileWatcher> proxy;

    Q_DECLARE_PUBLIC(RecentFileWatcher)
};

RecentFileWatcher::RecentFileWatcher(const DUrl &url, DAbstractFileWatcher *proxy, QObject *parent)
    : DAbstractFileWatcher(*new RecentFileWatcherPrivate(this), url, parent)
{
    Q_ASSERT(proxy);
    Q_ASSERT(!proxy->parent());

    if (url == DUrl(RECENT_ROOT))
        connect(fileSignalManager, &FileSignalManager::requestRemoveRecentFile, this, &RecentFileWatcher::removeRecentFile);

    proxy->moveToThread(thread());
    proxy->setParent(this);

    d_func()->proxy = proxy;
    connect(proxy, &DAbstractFileWatcher::fileAttributeChanged, this, &RecentFileWatcher::onFileAttributeChanged);
    connect(proxy, &DAbstractFileWatcher::fileDeleted, this, &RecentFileWatcher::onFileDeleted);
    connect(proxy, &DAbstractFileWatcher::fileMoved, this, &RecentFileWatcher::onFileMoved);
    connect(proxy, &DAbstractFileWatcher::fileModified, this, &RecentFileWatcher::onFileModified);
}

void RecentFileWatcher::setEnabledSubfileWatcher(const DUrl &subfileUrl, bool enabled)
{
    if (!subfileUrl.isRecentFile())
        return;

    if (enabled) {
        addWatcher(subfileUrl);
    } else {
        removeWatcher(subfileUrl);
    }
}

void RecentFileWatcher::addWatcher(const DUrl &url)
{
    Q_D(RecentFileWatcher);

    if (!url.isValid() || d->urlToWatcherMap.contains(url)) {
        return;
    }

    if (DFileService::instance()->checkGvfsMountfileBusy(url,false))
        return;

    DUrl real_url = url;
    real_url.setScheme(FILE_SCHEME);

    DAbstractFileWatcher *watcher = DFileService::instance()->createFileWatcher(this, real_url);

    if (!watcher)
        return;

    watcher->moveToThread(this->thread());
    watcher->setParent(this);

    connect(watcher, &DAbstractFileWatcher::fileAttributeChanged, this, &RecentFileWatcher::onFileAttributeChanged);
    connect(watcher, &DAbstractFileWatcher::fileDeleted, this, &RecentFileWatcher::onFileDeleted);
    connect(watcher, &DAbstractFileWatcher::fileModified, this, &RecentFileWatcher::onFileModified);
    connect(watcher, &DAbstractFileWatcher::fileMoved, this, &RecentFileWatcher::onFileMoved);

    d->urlToWatcherMap[url] = watcher;

    if (d->started) {
        watcher->startWatcher();
    }
}

void RecentFileWatcher::removeWatcher(const DUrl &url)
{
    Q_D(RecentFileWatcher);

    DAbstractFileWatcher *watcher = d->urlToWatcherMap.take(url);

    if (!watcher) {
        return;
    }

    watcher->deleteLater();
}

void RecentFileWatcher::onFileDeleted(const DUrl &url)
{    
    DUrl newUrl = url;
    newUrl.setScheme(RECENT_SCHEME);
    removeWatcher(newUrl);

    emit fileDeleted(newUrl);
}

void RecentFileWatcher::onFileAttributeChanged(const DUrl &url)
{
    DUrl newUrl = url;
    newUrl.setScheme(RECENT_SCHEME);

    emit fileAttributeChanged(newUrl);
}

void RecentFileWatcher::onFileModified(const DUrl &url)
{
    DUrl newUrl = url;
    newUrl.setScheme(RECENT_SCHEME);

    emit fileModified(newUrl);
}

void RecentFileWatcher::onFileMoved(const DUrl &from, const DUrl &to)
{
    DUrl newFromUrl = from;
    newFromUrl.setScheme(RECENT_SCHEME);
    removeWatcher(newFromUrl);

    emit fileMoved(newFromUrl, to);
}
//fix bug 63922 移除父目录是path的url
void RecentFileWatcher::removeRecentFile(const QString &path)
{
    Q_D(RecentFileWatcher);

    DUrlList removeUrls;
    for (auto url : d->urlToWatcherMap.keys()) {
        if (url.path().startsWith(path))
            removeUrls << url;
    }

    for (auto url : removeUrls) {
        onFileDeleted(url);
    }
}

class RecentDirIterator : public DDirIterator
{
public:
    RecentDirIterator(const DUrl &url, const QStringList &nameFilters, QDir::Filters filter,
                      QDirIterator::IteratorFlags flags, RecentController *parent);

    DUrl next() override;
    bool hasNext() const override;

    QString fileName() const override;
    DUrl fileUrl() const override;
    const DAbstractFileInfoPointer fileInfo() const override;
    DUrl url() const override;

private:
    DAbstractFileInfoPointer m_currentFileInfo;
    RecentController *parent;

    mutable QQueue<DUrl> urlList;
    DUrl m_url;
};

RecentDirIterator::RecentDirIterator(const DUrl &url, const QStringList &nameFilters,
                                     QDir::Filters filter, QDirIterator::IteratorFlags flags,
                                     RecentController *parent)
    : DDirIterator(),
      parent(parent)
{
    Q_UNUSED(url)
    Q_UNUSED(nameFilters)
    Q_UNUSED(filter)
    Q_UNUSED(flags)

    for (DUrl url : parent->recentNodes.keys()) {
        if (!DFileService::instance()->checkGvfsMountfileBusy(url,false))
            urlList << url;
    }
}

DUrl RecentDirIterator::next()
{
    if (!urlList.isEmpty()) {
        m_url = urlList.dequeue();
        return m_url;
    }

    return DUrl();
}

bool RecentDirIterator::hasNext() const
{
    if (!urlList.isEmpty()) {
        return true;
    }

    return false;
}

QString RecentDirIterator::fileName() const
{
    DAbstractFileInfoPointer currentInfo = parent->recentNodes.value(m_url);

    return currentInfo ? currentInfo->fileName() : QString();
}

DUrl RecentDirIterator::fileUrl() const
{
    DAbstractFileInfoPointer currentInfo = parent->recentNodes.value(m_url);

    return currentInfo ? currentInfo->fileUrl() : DUrl();
}

const DAbstractFileInfoPointer RecentDirIterator::fileInfo() const
{
    return parent->recentNodes.value(m_url);
}

DUrl RecentDirIterator::url() const
{
    return DUrl(RECENT_ROOT);
}

RecentController::RecentController(QObject *parent)
    : DAbstractFileController(parent),
      m_xbelPath(QDir::homePath() + "/.local/share/recently-used.xbel"),
      m_watcher(new DFileWatcher(m_xbelPath, this))
{
    asyncHandleFileChanged();

    connect(m_watcher, &DFileWatcher::subfileCreated, this, &RecentController::asyncHandleFileChanged);
    connect(m_watcher, &DFileWatcher::fileModified, this, &RecentController::asyncHandleFileChanged);

    m_watcher->startWatcher();
}

bool RecentController::openFileLocation(const QSharedPointer<DFMOpenFileLocation> &event) const
{
    return DFileService::instance()->openFileLocation(event->sender(), DUrl::fromLocalFile(event->url().path()));
}

bool RecentController::openFile(const QSharedPointer<DFMOpenFileEvent> &event) const
{
    //在smb/ftp更改文件名称后最近使用里面打开文件，使用createFileInfo不能判断源文件是否存在为真
    //这里直接获取真实路径进行判断源文件是否存在
    if (!QFile(event->url().path()).exists()) {
        DDialog d(QObject::tr("Failed to open the file, cause: %1")
                  .arg(QObject::tr("Original file does not exist")),
                  QObject::tr("Do you want to delete %1?")
                  .arg(event->url().fileName()));

        Qt::WindowFlags flags = d.windowFlags();
        // dialog show top
        d.setWindowFlags(flags | Qt::CustomizeWindowHint | Qt::WindowStaysOnTopHint);
        d.setIcon(QIcon::fromTheme("dialog-error"));
        d.addButton(QObject::tr("Confirm","button"), true, DDialog::ButtonRecommend);
        d.addButton(QObject::tr("Cancel","button"), false, DDialog::ButtonRecommend);
        d.setMaximumWidth(640);
        if(d.exec() == 0)
            DRecentManager::removeItem(event->url().path()); //删除当前最近使用项
        return true;
    }

    return DFileService::instance()->openFile(event->sender(), DUrl::fromLocalFile(event->url().path()));
}

bool RecentController::openFiles(const QSharedPointer<DFMOpenFilesEvent> &event) const
{
    DUrlList fileUrls = event->urlList();
    DUrlList packUrl;
    QStringList pathList;
    bool result = false;

    for (DUrl fileUrl : fileUrls) {
        const DAbstractFileInfoPointer pfile = createFileInfo(dMakeEventPointer<DFMCreateFileInfoEvent>(this, fileUrl));
        if (!pfile) {
            continue;
        }
        if (pfile->isSymLink()) {
            const DAbstractFileInfoPointer &linkInfo = DFileService::instance()->createFileInfo(this, pfile->symLinkTarget());

            if (linkInfo && !linkInfo->exists()) {
                dialogManager->showBreakSymlinkDialog(linkInfo->fileName(), DUrl(fileUrl.path().remove(RECENT_ROOT)));
                continue;
            }
            fileUrl = linkInfo->redirectedFileUrl();
        }

        if (FileUtils::isExecutableScript(fileUrl.path().remove(RECENT_ROOT))) {
            int code = dialogManager->showRunExcutableScriptDialog(DUrl(fileUrl.path().remove(RECENT_ROOT)), event->windowId());
            result = FileUtils::openExcutableScriptFile(fileUrl.path().remove(RECENT_ROOT), code) || result;
            continue;
        }

        if (FileUtils::isFileRunnable(fileUrl.path().remove(RECENT_ROOT)) && !pfile->isDesktopFile()) {
            int code = dialogManager->showRunExcutableFileDialog(DUrl(fileUrl.path().remove(RECENT_ROOT)), event->windowId());
            result = FileUtils::openExcutableFile(fileUrl.path().remove(RECENT_ROOT), code) || result;
            continue;
        }

        if (FileUtils::shouldAskUserToAddExecutableFlag(fileUrl.path().remove(RECENT_ROOT)) && !pfile->isDesktopFile()) {
            int code = dialogManager->showAskIfAddExcutableFlagAndRunDialog(DUrl(fileUrl.path().remove(RECENT_ROOT)), event->windowId());
            result = FileUtils::addExecutableFlagAndExecuse(fileUrl.path().remove(RECENT_ROOT), code) || result;
            continue;
        }

        packUrl << fileUrl;
        QString url = fileUrl.path().remove(RECENT_ROOT);
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
                DFileService::instance()->openFile(event->sender(), fileUrl);
            }
        }
    }

    return result;
}
bool RecentController::openFileByApp(const QSharedPointer<DFMOpenFileByAppEvent> &event) const
{
    return DFileService::instance()->openFileByApp(event->sender(), event->appName(), DUrl::fromLocalFile(event->url().path()));
}

bool RecentController::openFilesByApp(const QSharedPointer<DFMOpenFilesByAppEvent> &event) const
{
    //RecentController::openFilesByApp负责将所有recentUrl转成fileUrl然后走filecontroller::openFilesByApp的逻辑流程
    const DUrlList recentUrls = event.data()->urlList();
    DUrlList fileUrls;
    for (const DUrl &url : recentUrls) {
        fileUrls.append(DUrl::fromLocalFile(url.path()));
    }
    return DFileService::instance()->openFilesByApp(event->sender(), event->appName(), fileUrls);
}

bool RecentController::writeFilesToClipboard(const QSharedPointer<DFMWriteUrlsToClipboardEvent> &event) const
{
    //最近使用文件不支持剪切，这里主要是屏蔽从ctrl+x快捷键操作过来的剪切事件。
    if (event->action() == DFMGlobal::CutAction)
        return false;
    return DFileService::instance()->writeFilesToClipboard(event->sender(), event->action(),
                                                           realUrlList(event->urlList()));
}

bool RecentController::renameFile(const QSharedPointer<DFMRenameEvent> &event) const
{
    DUrl oldUrl = DUrl::fromLocalFile(event->fromUrl().path());
    DUrl newUrl = DUrl::fromLocalFile(event->toUrl().path());

    return DFileService::instance()->renameFile(event->sender(), oldUrl, newUrl);
}

bool RecentController::compressFiles(const QSharedPointer<DFMCompressEvent> &event) const
{
    return DFileService::instance()->compressFiles(event->sender(), realUrlList(event->urlList()));
}

bool RecentController::decompressFile(const QSharedPointer<DFMDecompressEvent> &event) const
{
    return DFileService::instance()->decompressFile(event->sender(), realUrlList(event->urlList()));
}

bool RecentController::createSymlink(const QSharedPointer<DFMCreateSymlinkEvent> &event) const
{
    return DFileService::instance()->createSymlink(event->sender(), DUrl::fromLocalFile(event->fileUrl().path()), event->toUrl(), event->force());
}

bool RecentController::deleteFiles(const QSharedPointer<DFMDeleteEvent> &event) const
{
    DThreadUtil::runInMainThread([event] {
        DDialog dlg;
        dlg.setIcon(QIcon::fromTheme("dialog-warning"));
        dlg.addButton(tr("Cancel", "button"));
        dlg.addButton(tr("Remove", "button"), true, DDialog::ButtonRecommend);

        if (event->urlList().size() == 1)
            dlg.setTitle(tr("Do you want to remove this item?"));
        else
            dlg.setTitle(tr("Do yout want to remove %1 items?").arg(event->urlList().size()));
        dlg.setMessage(tr("It does not delete the original files"));

        int code = dlg.exec();
        if (code == 1) {
            QStringList list;
            for (const DUrl &url : event->urlList()) {
                //list << DUrl::fromLocalFile(url.path()).toString();
                //通过durl转换path会出现编码问题，这里直接用字符串拼出正确的path;
                QString urlPath = url.path();
                list << "file://" + urlPath;
            }

            DRecentManager::removeItems(list);
        }
    });

    return true;
}

DUrlList RecentController::moveToTrash(const QSharedPointer<DFMMoveToTrashEvent> &event) const
{
    deleteFiles(dMakeEventPointer<DFMDeleteEvent>(event->sender(),
                                                  event->urlList(),
                                                  event->silent()));

    return DUrlList();
}

bool RecentController::setFileTags(const QSharedPointer<DFMSetFileTagsEvent> &event) const
{
    if (!event->url().isValid()) {
        return false;
    }

    QList<QString> tags = event->tags();
    return DFileService::instance()->setFileTags(this, DUrl::fromLocalFile(event->fileUrl().path()), tags);
}

bool RecentController::removeTagsOfFile(const QSharedPointer<DFMRemoveTagsOfFileEvent> &event) const
{
    if (!event->url().isValid()) {
        return false;
    }

    return DFileService::instance()->removeTagsOfFile(this, DUrl::fromLocalFile(event->fileUrl().path()), event->tags());
}

QList<QString> RecentController::getTagsThroughFiles(const QSharedPointer<DFMGetTagsThroughFilesEvent> &event) const
{
    QList<DUrl> list = event->urlList();

    for (DUrl &item : list) {
        item = DUrl::fromLocalFile(item.path());
    }

    return DFileService::instance()->getTagsThroughFiles(this, list);
}

const DDirIteratorPointer RecentController::createDirIterator(const QSharedPointer<DFMCreateDiriterator> &event) const
{
    RecentDirIterator *iterator = new RecentDirIterator(event->url(), event->nameFilters(),
                                                        event->filters(), event->flags(),
                                                        const_cast<RecentController *>(this));
    return DDirIteratorPointer(iterator);
}

const DAbstractFileInfoPointer RecentController::createFileInfo(const QSharedPointer<DFMCreateFileInfoEvent> &event) const
{
    if (event->url().path() == "/") {
        return DAbstractFileInfoPointer(new RecentFileInfo(DUrl(RECENT_ROOT)));
    }

    return DAbstractFileInfoPointer(recentNodes.value(event->url()));
}

DAbstractFileWatcher *RecentController::createFileWatcher(const QSharedPointer<DFMCreateFileWatcherEvent> &event) const
{
    DAbstractFileWatcher *base_watcher = DFileService::instance()->createFileWatcher(event->sender(), DUrl::fromLocalFile(event->url().path()));
    return new RecentFileWatcher(event->url(), base_watcher);
}

DUrlList RecentController::realUrlList(const DUrlList &recentUrls)
{
    DUrlList list;

    for (const DUrl &url : recentUrls) {
        list << DUrl::fromLocalFile(url.path());
    }

    return list;
}

void RecentController::handleFileChanged()
{
    // read xbel file.
    QFile file(m_xbelPath);
    QList<DUrl> urlList;

    // try interrupting any other running parsers, then acquire the lock
    m_condition.wakeAll();
    if (!m_xbelFileLock.tryLock(100)) {
        return;
    }

    QPointer<RecentController> dp = this;

    if (m_condition.wait(&m_xbelFileLock, 1000)) {
        // if the parser is interrupted.
        // we need a timeout long enough so that
        // virtually all execution time is spent here.
        m_xbelFileLock.unlock();
        return;
    }

    if (file.open(QIODevice::ReadOnly)) {
        QXmlStreamReader reader(&file);

        while (!reader.atEnd()) {

            if (!reader.readNextStartElement() ||
                    reader.name() != "bookmark") {
                continue;
            }

            const QStringRef &location = reader.attributes().value("href");
            const QStringRef &readTime = reader.attributes().value("modified");

            if (!location.isEmpty()) {
                DUrl url = DUrl(location.toString());
                QFileInfo info(url.toLocalFile());
                DUrl recentUrl = url;
                recentUrl.setScheme(RECENT_SCHEME);

                if (info.exists() && info.isFile()) {
                    urlList << recentUrl;

                    DThreadUtil::runInMainThread([ = ]() {
                        if (dp.isNull())
                            return;
                        // 保险箱内文件不显示到最近使用页面
                        if(!VaultController::isVaultFile(location.toString())) {
                            if (!recentNodes.contains(recentUrl)) {
                                recentNodes[recentUrl] = new RecentFileInfo(recentUrl);
                                DAbstractFileWatcher::ghostSignal(DUrl(RECENT_ROOT),
                                                                  &DAbstractFileWatcher::subfileCreated,
                                                                  recentUrl);
                            }
                            //如果readtime变更了，需要通知filesystemmodel重新排序
                            else if (recentNodes[recentUrl]->readDateTime().toSecsSinceEpoch() != QDateTime::fromString(readTime.toString(), Qt::ISODate).toSecsSinceEpoch()) {
                                //先更新info数据
                                recentNodes[recentUrl]->updateInfo();
                                DAbstractFileWatcher::ghostSignal(DUrl(RECENT_ROOT),
                                                                  &DAbstractFileWatcher::fileModified,
                                                                  recentUrl);
                            }
                        }
                    });
                }
            }
        }
    }

    if (dp.isNull())
        return;
    // delete does not exist url.
    for (auto iter = recentNodes.begin(); iter != recentNodes.end();) {
        if (dp.isNull())
            return;
        const DUrl &url = iter.key();
        if (!urlList.contains(url)) {
            DThreadUtil::runInMainThread([this, &iter, url]() {
                iter = recentNodes.erase(iter);

                DAbstractFileWatcher::ghostSignal(DUrl(RECENT_ROOT),
                                                  &DAbstractFileWatcher::fileDeleted,
                                                  url);
            });
        } else {
            auto info = iter.value();
            if (info) {
                iter.value()->updateInfo();
                ++iter;
            } else {
                iter = recentNodes.erase(iter);
            }
        }
        if (dp.isNull())
            return;
    }

    m_xbelFileLock.unlock();
}

void RecentController::asyncHandleFileChanged()
{
    /* remark 190410:
     * This slot always gets triggered multiple times consecutively by the
     * file closed/modified events. The correct fix should be applying a
     * lock to 'recently-used.xbel' whenever writing.
     * The current solution is setting a timeout, during which no matter
     * how many handleFileChanged()'s are triggered, only the last one
     * gets executed.
     * The issue mentioned above is specific to applications that use DTK.
     * Applications using GTK does not trigger file closed/modified events
     * at all for some obscure reasons.
     */
    QtConcurrent::run(this, &RecentController::handleFileChanged);
}
