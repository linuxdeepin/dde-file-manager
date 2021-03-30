/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: dengkeyun<dengkeyun@uniontech.com>
 *             max-lv<lvwujun@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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

#include "mergeddesktopcontroller.h"
#include "mergeddesktopcontroller_p.h"

#include "dfilewatcher.h"
#include "dfileservices.h"
#include "appcontroller.h"

#include "interfaces/dfileservices.h"
#include "interfaces/dfmstandardpaths.h"
#include "models/mergeddesktopfileinfo.h"
#include "interfaces/private/mergeddesktop_common_p.h"
#include "private/dabstractfilewatcher_p.h"
#include "shutil/dfmfilelistfile.h"

#include <QList>
#include <QStandardPaths>

MergedDesktopWatcher::MergedDesktopWatcher(const DUrl &url, DAbstractFileWatcher *baseWatcher, QObject *parent)
    : DAbstractFileWatcher(*new MergedDesktopWatcherPrivate(this), url, parent)
{
    connect(baseWatcher, &DAbstractFileWatcher::fileAttributeChanged, this, &MergedDesktopWatcher::onFileAttributeChanged);
    connect(baseWatcher, &DAbstractFileWatcher::fileModified, this, &MergedDesktopWatcher::onFileModified);
}

void MergedDesktopWatcher::setEnabledSubfileWatcher(const DUrl &subfileUrl, bool enabled)
{
    if (subfileUrl.scheme() != DFMMD_SCHEME) {
        return;
    }

    if (enabled) {
        addWatcher(subfileUrl);
    } else {
        removeWatcher(subfileUrl);
    }
}

void MergedDesktopWatcher::addWatcher(const DUrl &url)
{
    Q_D(MergedDesktopWatcher);

    if (!url.isValid() || d->urlToWatcherMap.contains(url)) {
        return;
    }

    DUrl real_url = MergedDesktopController::convertToRealPath(url);

    DAbstractFileWatcher *watcher = DFileService::instance()->createFileWatcher(this, real_url);

    if (!watcher) {
        return;
    }

    watcher->moveToThread(this->thread());
    watcher->setParent(this);

    connect(watcher, &DAbstractFileWatcher::fileAttributeChanged, this, &MergedDesktopWatcher::onFileAttributeChanged);
    connect(watcher, &DAbstractFileWatcher::fileModified, this, &MergedDesktopWatcher::onFileModified);

    d->urlToWatcherMap[url] = watcher;

    if (d->started) {
        watcher->startWatcher();
    }
}

void MergedDesktopWatcher::removeWatcher(const DUrl &url)
{
    Q_D(MergedDesktopWatcher);

    DAbstractFileWatcher *watcher = d->urlToWatcherMap.take(url);

    if (!watcher) {
        return;
    }

    watcher->deleteLater();
}

void MergedDesktopWatcher::onFileAttributeChanged(const DUrl &url)
{
    emit fileAttributeChanged(MergedDesktopController::convertToDFMMDPath(url));
}

void MergedDesktopWatcher::onFileModified(const DUrl &url)
{
    emit fileModified(MergedDesktopController::convertToDFMMDPath(url));
}

MergedDesktopController::MergedDesktopController(QObject *parent)
    : DAbstractFileController(parent),
      m_desktopFileWatcher(new DFileWatcher(QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).first(), this))
{
    connect(m_desktopFileWatcher, &DFileWatcher::fileDeleted, this, &MergedDesktopController::desktopFilesRemoved);
    connect(m_desktopFileWatcher, &DFileWatcher::subfileCreated, this, &MergedDesktopController::desktopFilesCreated);
    connect(m_desktopFileWatcher, &DFileWatcher::fileMoved, this, &MergedDesktopController::desktopFilesRenamed);
    m_desktopFileWatcher->startWatcher();
}

const DAbstractFileInfoPointer MergedDesktopController::createFileInfo(const QSharedPointer<DFMCreateFileInfoEvent> &event) const
{
    return DAbstractFileInfoPointer(new MergedDesktopFileInfo(event->url(), currentUrl));
}

const QList<DAbstractFileInfoPointer> MergedDesktopController::getChildren(const QSharedPointer<DFMGetChildrensEvent> &event) const
{
//    if (!dataInitialized) {
//        initData();
//        dataInitialized = true;
//    }
    // blumia: 文件监听占用完了的时候有可能桌面会监听不到文件变动,此时即便 F5 也不会刷新该 Controller 存储的整理桌面数据,故改为每次都重新初始化整理数据

    //加锁失败，说明有线程在跑后面的算法
    if (!m_runMtx.tryLock(0)){
        m_childrenLock.lock();
        m_cv.wait(&m_childrenLock);
        QList<DAbstractFileInfoPointer> infoList = m_childrenList;
        m_childrenLock.unlock();
        return infoList;
    }

    QMutexLocker aful(&m_arrangedFileUrlsMtx); //禁止其他线程修改arrangedFileUrls

    arrangedFileUrls = initData(event->filters());
    currentUrl = event->url();
    QString path { currentUrl.path() };
    QList<DAbstractFileInfoPointer> infoList;

    auto appendEntryFiles = [this](QList<DAbstractFileInfoPointer> &infoList, const DMD_TYPES &entryType){
        for (const DUrl & url : arrangedFileUrls[entryType]) {
            DAbstractFileInfoPointer info {
                new MergedDesktopFileInfo(convertToDFMMDPath(url), currentUrl)
            };
            infoList.append(info);
        }
    };

    auto appendVirtualEntries = [this, &infoList,appendEntryFiles](bool displayEmptyEntry = false, const QStringList & expandedEntries = {}) {
        for (unsigned int i = DMD_FIRST_TYPE; i <= DMD_ALL_ENTRY; i++) {
            DMD_TYPES oneType = static_cast<DMD_TYPES>(i);
            if (!displayEmptyEntry && arrangedFileUrls[oneType].isEmpty()) {
                continue;
            }
            //屏蔽掉2个以下文件的分类图标
            if(arrangedFileUrls[oneType].size() < 2){
                appendEntryFiles(infoList, oneType);
                continue;
            }
            QString entryName = entryNameByEnum(oneType);
            DUrl url(DFMMD_ROOT VIRTUALENTRY_FOLDER + entryNameByEnum(oneType));
            DAbstractFileInfoPointer infoPtr {
                new MergedDesktopFileInfo(url, currentUrl)
            };
            infoList.push_back(infoPtr);
            if (expandedEntries.contains(entryName)) {
                appendEntryFiles(infoList, oneType);
            }
        }
    };

    auto appendFolders = [this, &infoList]() {
        for (const DUrl & url : arrangedFileUrls[DMD_FOLDER]) {
            DAbstractFileInfoPointer info {
                new MergedDesktopFileInfo(convertToDFMMDPath(url), currentUrl)
            };
            infoList.append(info);
        }
    };

    auto makeAndInsertInfo = [this, &infoList](QString urlStr) {
        DUrl entryUrl(urlStr);
        DAbstractFileInfoPointer adeEntryInfoPtr {
            new MergedDesktopFileInfo(convertToDFMMDPath(entryUrl), currentUrl)
        };
        infoList.push_back(adeEntryInfoPtr);
    };

    if(currentUrl.scheme() == DFMMD_SCHEME) {
        if (path == QStringLiteral("/")) {
            makeAndInsertInfo(DFMMD_ROOT VIRTUALENTRY_FOLDER);
            makeAndInsertInfo(DFMMD_ROOT VIRTUALFOLDER_FOLDER);
            makeAndInsertInfo(DFMMD_ROOT MERGEDDESKTOP_FOLDER);
        } else if (path.startsWith(QStringLiteral(VIRTUALENTRY_PATH))) {
            if (path == QStringLiteral(VIRTUALENTRY_PATH)) {
                appendVirtualEntries();
            } else {
                QString entryName = path.split('/', QString::SkipEmptyParts).last();
                DMD_TYPES entryType = entryTypeByName(entryName);
                appendEntryFiles(infoList, entryType);
            }
        } else if (path == QStringLiteral(VIRTUALFOLDER_PATH)) {
            appendFolders();
        } else if (path == QStringLiteral(MERGEDDESKTOP_PATH)) {
            QString expandedFolder = currentUrl.fragment();
            QStringList expandedFolders;
            if (!expandedFolder.isEmpty()) {
                expandedFolders = expandedFolder.split(',', QString::SkipEmptyParts);
            }
            appendVirtualEntries(false, expandedFolders);
            appendFolders();
        }
    }

    m_runMtx.unlock();
    m_childrenLock.lock();
    m_childrenList = infoList;
    m_childrenLock.unlock();
    m_cv.wakeAll();
    return infoList;
}

DAbstractFileWatcher *MergedDesktopController::createFileWatcher(const QSharedPointer<DFMCreateFileWatcherEvent> &) const
{
    return new MergedDesktopWatcher(DUrl(DFMMD_ROOT MERGEDDESKTOP_FOLDER), m_desktopFileWatcher, nullptr);
}

bool MergedDesktopController::openFile(const QSharedPointer<DFMOpenFileEvent> &event) const
{
    return DFileService::instance()->openFile(event->sender(), convertToRealPath(event->url()));
}

bool MergedDesktopController::openFiles(const QSharedPointer<DFMOpenFilesEvent> &event) const
{
    return DFileService::instance()->openFiles(event->sender(), convertToRealPaths(event->urlList()));
}

bool MergedDesktopController::openFileByApp(const QSharedPointer<DFMOpenFileByAppEvent> &event) const
{
    return DFileService::instance()->openFileByApp(event->sender(), event->appName(), convertToRealPath(event->url()));
}

DUrlList MergedDesktopController::moveToTrash(const QSharedPointer<DFMMoveToTrashEvent> &event) const
{
    DUrlList urlList = convertToRealPaths(event->urlList());
    return DFileService::instance()->moveToTrash(event->sender(), urlList);
}

bool MergedDesktopController::writeFilesToClipboard(const QSharedPointer<DFMWriteUrlsToClipboardEvent> &event) const
{
    DUrlList urlList = convertToRealPaths(event->urlList());
    return DFileService::instance()->writeFilesToClipboard(event->sender(), event->action(), urlList);
}

DUrlList MergedDesktopController::pasteFile(const QSharedPointer<DFMPasteEvent> &event) const
{
//    return DUrlList(); // disabled for now.

    return DFileService::instance()->pasteFile(event->sender(), event->action(),
                                               DUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::DesktopLocation)),
                                               event->fileUrlList());
}

bool MergedDesktopController::deleteFiles(const QSharedPointer<DFMDeleteEvent> &event) const
{
    DUrlList urlList = convertToRealPaths(event->urlList());
    // 注意这里第三个参数 confirmationDialog, 在函数体内并未使用，但需要后面的两个参数所以给第三个参数赋值true
    return DFileService::instance()->deleteFiles(event->sender(), urlList, true, event->silent(), event->force());
}

bool MergedDesktopController::renameFile(const QSharedPointer<DFMRenameEvent> &event) const
{
    return DFileService::instance()->renameFile(event->sender(),
                                                convertToRealPath(event->fromUrl()),
                                                convertToRealPath(event->toUrl()));
}

bool MergedDesktopController::openInTerminal(const QSharedPointer<DFMOpenInTerminalEvent> &event) const
{
    return DFileService::instance()->openInTerminal(event->sender(),
                                                    DUrl::fromLocalFile(DFMStandardPaths::location(DFMStandardPaths::DesktopPath)));
}

// fixme: AppController::actionNewFolder 调了 FileUtils::newDocumentUrl 调了 getUrlByChildFileName ,可能需要重写
//        否则在自动整理视图下新建文件后不会被选中
bool MergedDesktopController::mkdir(const QSharedPointer<DFMMkdirEvent> &event) const
{
    bool result = DFileService::instance()->mkdir(event->sender(),
                                                  DUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::DesktopLocation)));

    AppController::selectionAndRenameFile = qMakePair(event->url(), event->windowId());

    return result;
}

bool MergedDesktopController::touch(const QSharedPointer<DFMTouchFileEvent> &event) const
{
    bool result = DFileService::instance()->touchFile(event->sender(),
                                               DUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::DesktopLocation)));

    AppController::selectionAndRenameFile = qMakePair(event->url(), event->windowId());

    return result;
}

bool MergedDesktopController::setPermissions(const QSharedPointer<DFMSetPermissionEvent> &event) const
{
    return DFileService::instance()->setPermissions(event->sender(), convertToRealPath(event->url()), event->permissions());
}

bool MergedDesktopController::compressFiles(const QSharedPointer<DFMCompressEvent> &event) const
{
    DUrlList urlList = convertToRealPaths(event->urlList());
    return DFileService::instance()->compressFiles(event->sender(), urlList);
}

bool MergedDesktopController::decompressFile(const QSharedPointer<DFMDecompressEvent> &event) const
{
    DUrlList urlList = convertToRealPaths(event->urlList());
    return DFileService::instance()->decompressFile(event->sender(), urlList);
}

bool MergedDesktopController::createSymlink(const QSharedPointer<DFMCreateSymlinkEvent> &event) const
{
    return DFileService::instance()->createSymlink(event->sender(),
                                                   convertToRealPath(event->fileUrl()),
                                                   convertToRealPath(event->toUrl()));
}

bool MergedDesktopController::setFileTags(const QSharedPointer<DFMSetFileTagsEvent> &event) const
{
    return DFileService::instance()->setFileTags(event->sender(), convertToRealPath(event->url()), event->tags());
}

bool MergedDesktopController::removeTagsOfFile(const QSharedPointer<DFMRemoveTagsOfFileEvent> &event) const
{
    return DFileService::instance()->removeTagsOfFile(event->sender(), convertToRealPath(event->url()), event->tags());
}

QList<QString> MergedDesktopController::getTagsThroughFiles(const QSharedPointer<DFMGetTagsThroughFilesEvent> &event) const
{
    DUrlList urlList = convertToRealPaths(event->urlList());
    return DFileService::instance()->getTagsThroughFiles(event->sender(), urlList);
}

DFileDevice *MergedDesktopController::createFileDevice(const QSharedPointer<DFMUrlBaseEvent> &event) const
{
    return DFileService::instance()->createFileDevice(event->sender(), convertToRealPath(event->url()));
}

DFileHandler *MergedDesktopController::createFileHandler(const QSharedPointer<DFMUrlBaseEvent> &event) const
{
    return DFileService::instance()->createFileHandler(event->sender(), convertToRealPath(event->url()));
}

DStorageInfo *MergedDesktopController::createStorageInfo(const QSharedPointer<DFMUrlBaseEvent> &event) const
{
    return DFileService::instance()->createStorageInfo(event->sender(), convertToRealPath(event->url()));
}

bool MergedDesktopController::setExtraProperties(const QSharedPointer<DFMSetFileExtraProperties> &event) const
{
    return DFileService::instance()->setExtraProperties(event->sender(), convertToRealPath(event->url()),
                                                           event->extraProperties());
}

const QString MergedDesktopController::entryNameByEnum(DMD_TYPES singleType)
{
    switch (singleType) {
    case DMD_PICTURE:
        return tr("Pictures");
    case DMD_MUSIC:
        return tr("Music");
    case DMD_APPLICATION:
        return tr("Applications");
    case DMD_VIDEO:
        return tr("Videos");
    case DMD_DOCUMENT:
        return tr("Documents");
    case DMD_OTHER:
        return tr("Others");
    case DMD_FOLDER:
        return "Folders";
    default:
        return "Bug";
    }
}

DMD_TYPES MergedDesktopController::entryTypeByName(QString entryName)
{
    if (entryName == tr("Pictures")) {
        return DMD_PICTURE;
    } else if (entryName == tr("Music")) {
        return DMD_MUSIC;
    } else if (entryName == tr("Applications")) {
        return DMD_APPLICATION;
    } else if (entryName == tr("Videos")) {
        return DMD_VIDEO;
    } else if (entryName == tr("Documents")) {
        return DMD_DOCUMENT;
    } else if (entryName == tr("Others")) {
        return DMD_OTHER;
    }

    qWarning() << "MergedDesktopController::entryTypeByName() cannot match a reasonable result, that can be a bug." << qPrintable(entryName);

    return DMD_OTHER;
}

DUrl MergedDesktopController::getVirtualEntryPath(DMD_TYPES oneType)
{
    if (oneType == DMD_FOLDER) {
        return DUrl(DFMMD_ROOT VIRTUALFOLDER_FOLDER "/");
    }

    return DUrl(DFMMD_ROOT VIRTUALENTRY_FOLDER + entryNameByEnum(oneType) + "/");
}


DMD_TYPES MergedDesktopController::checkUrlArrangedType(const DUrl url)
{
    DAbstractFileInfoPointer info = DFileService::instance()->createFileInfo(nullptr, url);
    if (info) {
        QString mimetypeDisplayName = info->mimeTypeDisplayName();
        if (mimetypeDisplayName.startsWith(qApp->translate("MimeTypeDisplayManager", "Application"))) {
            return DMD_APPLICATION;
        } else if (mimetypeDisplayName.startsWith(qApp->translate("MimeTypeDisplayManager", "Image"))) {
            return DMD_PICTURE;
        } else if (mimetypeDisplayName.startsWith(qApp->translate("MimeTypeDisplayManager", "Audio"))) {
            return DMD_MUSIC;
        } else if (mimetypeDisplayName.startsWith(qApp->translate("MimeTypeDisplayManager", "Video"))) {
            return DMD_VIDEO;
        } else if (mimetypeDisplayName.startsWith(qApp->translate("MimeTypeDisplayManager", "Text"))) {
            return DMD_DOCUMENT;
        } else if (mimetypeDisplayName.startsWith(qApp->translate("MimeTypeDisplayManager", "Directory"))) {
            return DMD_FOLDER;
        } else {
            return DMD_OTHER;
        }
    }

    return DMD_OTHER;
}

void MergedDesktopController::desktopFilesCreated(const DUrl &url)
{
    QMutexLocker aful(&m_arrangedFileUrlsMtx); //禁止其他线程修改arrangedFileUrls

    DMD_TYPES typeInfo = checkUrlArrangedType(url);
    if (arrangedFileUrls[typeInfo].contains(url)) {
        qWarning() << url << "existed, it must be a bug!!!!!!!!";
        return; //不return会崩溃，不知道为什么 todo
//        arrangedFileUrls[typeInfo].removeAll(url);//return后不执行，导致警告屏蔽之
    }
    arrangedFileUrls[typeInfo].append(url);
    aful.unlock();

    DUrl vUrl = convertToDFMMDPath(url, typeInfo);
    DUrl parentUrl = getVirtualEntryPath(typeInfo);
    DAbstractFileWatcher::ghostSignal(parentUrl, &DAbstractFileWatcher::subfileCreated, vUrl);
    DUrl parentUrl2(DFMMD_ROOT MERGEDDESKTOP_FOLDER);
    DAbstractFileWatcher::ghostSignal(parentUrl2, &DAbstractFileWatcher::subfileCreated, vUrl);
}

void MergedDesktopController::desktopFilesRemoved(const DUrl &url)
{
    QMutexLocker aful(&m_arrangedFileUrlsMtx); //禁止其他线程修改arrangedFileUrls

    for (unsigned int i = DMD_FIRST_TYPE; i <= DMD_ALL_TYPE; i++) {
        DMD_TYPES typeInfo = static_cast<DMD_TYPES>(i);
        if (arrangedFileUrls[typeInfo].removeOne(url)) {
            aful.unlock();

            DUrl vUrl = convertToDFMMDPath(url, typeInfo);

            DUrl parentUrl = getVirtualEntryPath(typeInfo);
            DAbstractFileWatcher::ghostSignal(parentUrl, &DAbstractFileWatcher::fileDeleted, vUrl);
            DUrl parentUrl2(DFMMD_ROOT MERGEDDESKTOP_FOLDER);
            DAbstractFileWatcher::ghostSignal(parentUrl2, &DAbstractFileWatcher::fileDeleted, vUrl);
            return;
        }
    }
}

void
MergedDesktopController::desktopFilesRenamed(const DUrl &oriUrl, const DUrl &dstUrl)
{
    QMutexLocker aful(&m_arrangedFileUrlsMtx); //禁止其他线程修改arrangedFileUrls

    //获取原文件的类型
    DMD_TYPES orgTypeInfo = checkUrlArrangedType(oriUrl);
    for (unsigned int i = DMD_FIRST_TYPE; i <= DMD_ALL_TYPE; i++) {
        DMD_TYPES typeInfo = static_cast<DMD_TYPES>(i);
        if (arrangedFileUrls[typeInfo].removeOne(oriUrl)) {
            orgTypeInfo = typeInfo;
            break;
        }
    }

    DMD_TYPES typeInfo = checkUrlArrangedType(dstUrl);
    arrangedFileUrls[typeInfo].append(dstUrl);
    aful.unlock();

    DUrl vOriUrl = convertToDFMMDPath(oriUrl, orgTypeInfo);
    DUrl vDstUrl = convertToDFMMDPath(dstUrl, typeInfo);

    DUrl parentUrl = getVirtualEntryPath(typeInfo);
    DAbstractFileWatcher::ghostSignal(parentUrl, &DAbstractFileWatcher::fileMoved, vOriUrl, vDstUrl);
    DUrl parentUrl2(DFMMD_ROOT MERGEDDESKTOP_FOLDER);
    DAbstractFileWatcher::ghostSignal(parentUrl2, &DAbstractFileWatcher::fileMoved, vOriUrl, vDstUrl);
}

QMap<DMD_TYPES, QList<DUrl> > MergedDesktopController::initData(QDir::Filters ftrs)
{
    QMap<DMD_TYPES, QList<DUrl> > tArrangedFileUrls;

    QDir desktopDir(QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).first());
#if 0
    const QStringList &fileList = desktopDir.entryList(QDir::AllEntries | QDir::NoDotAndDotDot);
#else
    //自动整理下需要显示隐藏文件,故新加QDir::Filters ftrs
    QStringList fileList = desktopDir.entryList(ftrs, QDir::Name);
#endif
    //文件名排序
    auto compateFunc = [](const QString &str1, const QString &str2) -> bool{
        return FileSortFunction::compareByString(str1,str2);
    };
    qSort(fileList.begin(),fileList.end(),compateFunc);
    //end

    //解决自动整理时的文件隐藏显示问题
    bool showHidden = ftrs.testFlag(QDir::Hidden);
    DFMFileListFile hiddenFiles(QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).first());

    for (const QString &oneFile : fileList) {
        if (!showHidden && hiddenFiles.contains(oneFile)){
            continue;
        }
        DUrl oneUrl = DUrl::fromLocalFile(desktopDir.filePath(oneFile));
        DMD_TYPES typeInfo = checkUrlArrangedType(oneUrl);
        tArrangedFileUrls[typeInfo].append(oneUrl);
    }

    return tArrangedFileUrls;
}

//为了防止自动整理下剪切与分类名相同的文件夹，这里创建虚拟的分类路径做对比
bool MergedDesktopController::isVirtualEntryPaths(const DUrl &oneUrl)
{
    auto tgPath = oneUrl.toString();
    for (unsigned int i = DMD_FIRST_TYPE; i <= DMD_ALL_ENTRY; i++) {
        DMD_TYPES oneType = static_cast<DMD_TYPES>(i);
        QString temp = DFMMD_ROOT VIRTUALENTRY_FOLDER + entryNameByEnum(oneType);
        if(tgPath == temp)
            return true;
    }

    return false;
}

DUrl MergedDesktopController::convertToDFMMDPath(const DUrl &oriUrl)
{
    DMD_TYPES typeInfo = checkUrlArrangedType(oriUrl);
    QDir desktopDir(QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).first());

    return convertToDFMMDPath(oriUrl, typeInfo);
}

DUrl MergedDesktopController::convertToDFMMDPath(const DUrl &oriUrl, DMD_TYPES oneType)
{
    DUrl vUrl;


#if 0
    //    if (oneType == DMD_FOLDER) {
    //        vUrl = DUrl(DFMMD_ROOT VIRTUALFOLDER_FOLDER + oriUrl.fileName());
    //    } else {
    //        vUrl = DUrl(DFMMD_ROOT VIRTUALENTRY_FOLDER + entryNameByEnum(oneType) + QDir::separator() + oriUrl.fileName());
    //    }
#else
    //oriUrl.fileName()自动整理时在获取带有特殊不规范字符的名字时不准确，改为‘/’获取
    auto str = oriUrl.toString();
    auto idxPos = str.indexOf("/");
    auto fileNameStartPos = str.length() - idxPos - 1;
    auto fileName = str.right(fileNameStartPos);
    if (oneType == DMD_FOLDER) {
        vUrl = DUrl(DFMMD_ROOT VIRTUALFOLDER_FOLDER + fileName);
    } else {
        vUrl = DUrl(DFMMD_ROOT VIRTUALENTRY_FOLDER + entryNameByEnum(oneType) + QDir::separator() + fileName);
    }
#endif

    return vUrl;
}

DUrl MergedDesktopController::convertToRealPath(const DUrl &oneUrl)
{
    if (oneUrl.scheme() != DFMMD_SCHEME) return oneUrl;

    QString desktopPath = QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).first() + QDir::separator();

    return DUrl::fromLocalFile(desktopPath + oneUrl.fileName());
}

DUrlList MergedDesktopController::convertToRealPaths(DUrlList urlList)
{
    for (DUrl &url : urlList) {
        DAbstractFileInfoPointer info = DFileService::instance()->createFileInfo(nullptr, url);
        if (info && info->isVirtualEntry()) {
            urlList.removeOne(url);
            continue;
        }
        url = convertToRealPath(url);
    }

    return urlList;
}
