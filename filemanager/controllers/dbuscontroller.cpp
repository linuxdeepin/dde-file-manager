#include "dbuscontroller.h"

#include "dbusinterface/monitormanager_interface.h"
#include "dbusinterface/clipboard_interface.h"
#include "dbusinterface/watcherinstance_interface.h"
#include "dbusinterface/filemonitorInstance_interface.h"
#include "dbusinterface/fileInfo_interface.h"
#include "dbusinterface/desktopdaemon_interface.h"
#include "dbusinterface/fileoperations_interface.h"
#include "dbusinterface/createdirjob_interface.h"
#include "dbusinterface/createfilejob_interface.h"
#include "dbusinterface/createfilefromtemplatejob_interface.h"
#include "dbusinterface/displayinterface.h"
#include "dbusinterface/dbusdocksetting.h"
#include "dbusinterface/appearancedaemon_interface.h"
#include "dbusinterface/pinyin_interface.h"
#include "widgets/themeappicon.h"

#include "../app/global.h"
#include "filemonitor/filemonitor.h"
#include "dbusinterface/dbusclientmanager.h"
#include "appcontroller.h"
#include "trashjobcontroller.h"

#include <QApplication>
#include <QClipboard>


inline bool compareByName(const QString& name1, const QString& name2)
{
    return name1 < name2;
}

inline bool appNameLessThan(const FileItemInfo &info1, const FileItemInfo &info2)
{
    return compareByName(info1.lowerPinyinName, info2.lowerPinyinName);
}


int DBusController::RequestThumailMaxCount = 100;

DBusController::DBusController(QObject *parent) : QObject(parent)
{
    init();
}

void DBusController::init(){
    QDBusConnection bus = QDBusConnection::sessionBus();
    m_fileInfoInterface = new FileInfoInterface(FileInfo_service, FileInfo_path, bus, this);
    m_desktopDaemonInterface = new DesktopDaemonInterface(DesktopDaemon_service, DesktopDaemon_path, bus, this);
    m_fileOperationsInterface = new FileOperationsInterface(FileMonitor_service, FileOperations_path, bus, this);
    m_clipboardInterface = new ClipboardInterface(FileMonitor_service, Clipboard_path, bus, this);
    m_dockSettingInterface = new DBusDockSetting(this);
    m_displayInterface = new DisplayInterface(this);
    m_pinyinInterface = new PinyinInterface(Pinyin_service, Pinyin_path, QDBusConnection::sessionBus(), this);
    m_appearanceInterface = new AppearanceDaemonInterface(Appearance_service, Appearance_path, bus, this);
    m_dockClientManagerInterface = new DBusClientManager(this);
    m_fileMonitor = new FileMonitor(this);
//    m_appController = new AppController(this);

    m_thumbnailTimer = new QTimer(this);
    m_thumbnailTimer->setInterval(1000);

    m_pinyinTimer = new QTimer(this);
    m_pinyinTimer->setSingleShot(true);
    m_pinyinTimer->setInterval(1000);

    initConnect();
}

void DBusController::initConnect(){
    connect(m_desktopDaemonInterface, SIGNAL(RequestOpen(QStringList,IntList)),
            this, SLOT(openFiles(QStringList, IntList)));
    connect(signalManager, SIGNAL(openFiles(FileItemInfo,QList<FileItemInfo>)),
            this, SLOT(openFiles(FileItemInfo,QList<FileItemInfo>)));
    connect(signalManager, SIGNAL(openFiles(FileItemInfo,QStringList)),
            this, SLOT(openFiles(FileItemInfo,QStringList)));
    connect(signalManager, SIGNAL(openFile(FileItemInfo)), this, SLOT(openFile(FileItemInfo)));
    connect(m_desktopDaemonInterface, SIGNAL(RequestCreateDirectory()), this, SLOT(createDirectory()));
    connect(m_desktopDaemonInterface, SIGNAL(RequestCreateFile()), this, SLOT(createFile()));
    connect(m_desktopDaemonInterface, SIGNAL(RequestCreateFileFromTemplate(QString)),
            this, SLOT(createFileFromTemplate(QString)));
    connect(m_desktopDaemonInterface, SIGNAL(RequestSort(QString)),this, SLOT(sortByKey(QString)));

    connect(m_desktopDaemonInterface, SIGNAL(AppGroupCreated(QString,QStringList)),
            this, SLOT(createAppGroup(QString,QStringList)));

    connect(m_desktopDaemonInterface, SIGNAL(ItemCut(QStringList)),
            signalManager, SIGNAL(filesCuted(QStringList)));
    connect(m_desktopDaemonInterface, SIGNAL(RequestRename(QString)), signalManager, SIGNAL(requestRenamed(QString)));
    connect(m_desktopDaemonInterface, SIGNAL(RequestDelete(QStringList)),
            signalManager, SIGNAL(trashingAboutToExcute(QStringList)));

    connect(m_desktopDaemonInterface, SIGNAL(RequestEmptyTrash()), signalManager, SIGNAL(requestEmptyTrash()));

    connect(m_desktopDaemonInterface, SIGNAL(ItemCopied(QStringList)), signalManager, SIGNAL(filesCopied(QStringList)));
    connect(signalManager, SIGNAL(filesCopied(QStringList)), this, SLOT(copyFiles(QStringList)));
    connect(signalManager, SIGNAL(filesCuted(QStringList)), this, SLOT(cutFiles(QStringList)));
    connect(signalManager, SIGNAL(pasteFilesToDesktop()), this, SLOT(pasteFilesToDesktop()));
    connect(m_clipboardInterface, SIGNAL(RequestPaste(QString,QStringList,QString)),
            this, SLOT(pasteFiles(QString,QStringList,QString)));

    connect(signalManager, SIGNAL(requestCreatingAppGroup(QStringList)),
            this, SLOT(requestCreatingAppGroup(QStringList)));
    connect(signalManager, SIGNAL(requestMergeIntoAppGroup(QStringList,QString)),
            this, SLOT(mergeIntoAppGroup(QStringList,QString)));

    connect(m_fileMonitor, SIGNAL(fileCreated(QString)), this, SLOT(handleFileCreated(QString)));
    connect(m_fileMonitor, SIGNAL(fileDeleted(QString)), this, SLOT(handleFileDeleted(QString)));
    connect(m_fileMonitor, SIGNAL(fileMovedIn(QString)), this, SLOT(handleFileMovedIn(QString)));
    connect(m_fileMonitor, SIGNAL(fileMovedOut(QString)), this, SLOT(handleFileMovedOut(QString)));
    connect(m_fileMonitor, SIGNAL(fileRenamed(QString,QString)), this, SLOT(handleFileRenamed(QString,QString)));
    connect(m_fileMonitor, SIGNAL(fileMetaDataChanged(QString)), this, SLOT(handleFileMetaDataChanged(QString)));

    connect(m_dockSettingInterface, SIGNAL(DisplayModeChanged(int)), signalManager, SIGNAL(dockModeChanged(int)));
    connect(m_thumbnailTimer, SIGNAL(timeout()), this, SLOT(delayGetThumbnail()));
    connect(m_pinyinTimer, SIGNAL(timeout()), this, SLOT(convertNameToPinyin()));
    connect(m_displayInterface, SIGNAL(PrimaryRectChanged()), signalManager, SIGNAL(screenGeometryChanged()));
    connect(m_displayInterface, SIGNAL(PrimaryChanged()), signalManager, SIGNAL(screenGeometryChanged()));
    connect(signalManager, SIGNAL(gtkIconThemeChanged()), this, SLOT(handelIconThemeChanged()));
    connect(signalManager, SIGNAL(refreshCopyFileIcon(QString)), this, SLOT(refreshThumail(QString)));
    connect(signalManager, SIGNAL(thumbnailRequested(QString)), this, SLOT(requestThumbnail(QString)));
    connect(m_dockClientManagerInterface, SIGNAL(ActiveWindowChanged(uint)), signalManager, SIGNAL(activeWindowChanged(uint)));
}

void DBusController::loadDesktopSettings(){

}

void DBusController::loadDesktopItems(){
    asyncRequestDesktopItems();
    asyncRequestComputerIcon();
    asyncRequestTrashIcon();
    m_appController->getTrashJobController()->asyncRequestTrashCount();
    QTimer::singleShot(500, this, SLOT(handelIconThemeChanged()));
}

int DBusController::getDockMode(){
    int mode = 0;
    QDBusPendingReply<int> reply = m_dockSettingInterface->GetDisplayMode();
    reply.waitForFinished();
    if (!reply.isError()){
        mode = reply.argumentAt(0).toInt();
    }else{
        qCritical() << reply.error().message();
    }
    return mode;
}

DesktopDaemonInterface* DBusController::getDesktopDaemonInterface(){
    return m_desktopDaemonInterface;
}

DisplayInterface* DBusController::getDisplayInterface(){
    return m_displayInterface;
}

void DBusController::asyncRequestDesktopItems(){
    QDBusPendingReply<FileItemInfoMap> reply = m_desktopDaemonInterface->GetDesktopItems();
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(reply);
    connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
                        this, SLOT(asyncRequestDesktopItemsFinished(QDBusPendingCallWatcher*)));
}

void DBusController::asyncRequestDesktopItemsFinished(QDBusPendingCallWatcher *call){
    qDebug() << "===============";
    if (m_requestFinished){
        emit signalManager->stopRequest();
        return;
    }
    QDBusPendingReply<FileItemInfoMap> reply = *call;
    if (!reply.isError()){
        emit signalManager->stopRequest();
        FileItemInfoMap desktopItems = qdbus_cast<FileItemInfoMap>(reply.argumentAt(0));
        /*ToDo desktop daemon settings judge*/
        for(int i=0; i < desktopItems.count(); i++){
            QString key = desktopItems.keys().at(i);
            FileItemInfo info = desktopItems.values().at(i);
            bool isRequestThumbnailFlag = isRequestThumbnail(info.URI);
            qDebug() << info.URI << info.BaseName << info.Icon << info.thumbnail;
            qDebug() << "isRequestThumbnailFlag" << isRequestThumbnailFlag;
            if (info.thumbnail.length() > 0 && isRequestThumbnailFlag){
                info.Icon = info.thumbnail;
            }else if(info.thumbnail.length() > 0 && !isRequestThumbnailFlag){
                info.thumbnail = "";
            }

            desktopItems.insert(key, info);
        }
        emit signalManager->desktopItemsChanged(desktopItems);

        m_desktopItemInfoMap = desktopItems;

        m_pinyinTimer->start();

        foreach (QString url, desktopItems.keys()) {
            if (isAppGroup(decodeUrl(url))){
                getAppGroupItemsByUrl(url);
            }
        }

        m_requestFinished = true;

    }else{
        emit signalManager->stopRequest();
        qCritical() << reply.error().message();
    }

    call->deleteLater();
}


void DBusController::asyncRequestComputerIcon(){
    QDBusPendingReply<QString> reply = m_fileInfoInterface->GetThemeIcon(ComputerUrl, 48);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(reply);
    connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
                        this, SLOT(asyncRequestComputerIconFinihsed(QDBusPendingCallWatcher*)));
}

void DBusController::asyncRequestComputerIconFinihsed(QDBusPendingCallWatcher *call){
    QDBusPendingReply<QString> reply = *call;
    if (!reply.isError()){
        QString iconUrl = reply.argumentAt(0).toString();
        emit signalManager->desktoItemIconUpdated(ComputerUrl, iconUrl, 48);
    }else{
        qCritical() << reply.error().message();
    }
    call->deleteLater();
}

void DBusController::asyncRequestTrashIcon(){
    QDBusPendingReply<QString> reply = m_fileInfoInterface->GetThemeIcon(TrashUrl, 48);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(reply);
    connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
                        this, SLOT(asyncRequestTrashIconFinished(QDBusPendingCallWatcher*)));
}

void DBusController::asyncRequestTrashIconFinished(QDBusPendingCallWatcher *call){
    QDBusPendingReply<QString> reply = *call;
    if (!reply.isError()){
        QString iconUrl = reply.argumentAt(0).toString();
        emit signalManager->desktoItemIconUpdated(TrashUrl, iconUrl, 48);
    }else{
        qCritical() << reply.error().message();
    }
    call->deleteLater();
}

void DBusController::requestIconByUrl(QString url, uint size){
    qDebug() << __func__ << url;
    if (isAppGroup(url)){
        return;
    }
    QString _url(url);
    if (url == ComputerUrl || url == TrashUrl){

    }else if (!url.startsWith(FilePrefix)){
        _url = FilePrefix + url;
    }

    if (m_desktopItemInfoMap.contains(_url)){
        if (m_desktopItemInfoMap.value(_url).thumbnail.length() > 0){
            return;
        }
    }

    QDBusPendingReply<QString> reply = m_fileInfoInterface->GetThemeIcon(_url, size);
    reply.waitForFinished();
    if (!reply.isError()){
        QString iconUrl = reply.argumentAt(0).toString();
        qDebug() << url << iconUrl;
        if (iconUrl.length() == 0){
             iconUrl = ThemeAppIcon::getThemeIconPath(getMimeTypeIconName(url));
        }
        emit signalManager->desktoItemIconUpdated(url, iconUrl, size);
    }else{
        qCritical() << reply.error().message();
    }
}

void DBusController::delayGetThumbnail(){
    foreach(QString url, m_thumbnails){
        requestThumbnail(url, 48);
    }
    if (m_thumbnails.count() == 0 || m_thumbnailCount > DBusController::RequestThumailMaxCount){
        m_thumbnailTimer->stop();
        m_thumbnailCount = 0;
    }
}

void DBusController::refreshThumail(QString url, uint size){
    qDebug() << __func__ << url;
    if (isAppGroup(url)){
        return;
    }
    QString _url(url);
    if (!url.startsWith(FilePrefix)){
        _url = FilePrefix + url;
    }

    if(!isRequestThumbnail(url)){
        return;
    }

    QString mimetype = getMimeTypeName(url);
    QDBusPendingReply<QString> reply = m_fileInfoInterface->GetThumbnailWithMIME(_url, size, mimetype);
    reply.waitForFinished();
    if (!reply.isError()){
        QString iconUrl = reply.argumentAt(0).toString();
        qDebug() << iconUrl;
        if (iconUrl.length() == 0){
            QString iconUrl = ThemeAppIcon::getThemeIconPath(getMimeTypeIconName(url));
            emit signalManager->desktoItemIconUpdated(url, iconUrl, size);
        }else{
            emit signalManager->desktoItemIconUpdated(url, iconUrl, size);
        }
    }else{
        qCritical() << reply.error().message();
        if (!isAppGroup(url)){
            QString iconUrl = ThemeAppIcon::getThemeIconPath(getMimeTypeIconName(url));
            emit signalManager->desktoItemIconUpdated(url, iconUrl, size);
            requestIconByUrl(url, size);
        }
    }
}

void DBusController::requestThumbnail(QString url, uint size){
    qDebug() << __func__<< url << m_thumbnailCount;

    if (m_thumbnailCount > DBusController::RequestThumailMaxCount){
        m_thumbnails.clear();
        m_thumbnailTimer->stop();
        m_thumbnailCount = 0;
    }
    if (isAppGroup(url)){
        return;
    }
    QString _url(url);
    if (!url.startsWith(FilePrefix)){
        _url = FilePrefix + url;
    }

    QString mimetype = getMimeTypeName(url);
    if(!isRequestThumbnail(url)){
        if (m_thumbnails.contains(url)){
            m_thumbnails.removeOne(url);
        }
        qDebug() << "Unsupport request thumbail" << url << mimetype;
    }else{
        qDebug() << _url << size << mimetype;
        QDBusPendingReply<QString> reply = m_fileInfoInterface->GetThumbnailWithMIME(_url, size, mimetype);
        reply.waitForFinished();
        if (!reply.isError()){
            QString iconUrl = reply.argumentAt(0).toString();
            qDebug() << iconUrl;
            if (m_thumbnails.contains(url)){
                m_thumbnails.removeOne(url);
            }
            emit signalManager->desktoItemIconUpdated(url, iconUrl, size);
        }else{
            qCritical() << reply.error().message();
            if (m_thumbnails.contains(url)){
                m_thumbnails.removeOne(url);
            }
            requestIconByUrl(url, size);
            QString iconUrl = ThemeAppIcon::getThemeIconPath(getMimeTypeIconName(url));
            emit signalManager->desktoItemIconUpdated(url, iconUrl, size);
        }
    }
    m_thumbnailCount += 1;
}

void DBusController::convertNameToPinyin(){
    QStringList names;
    for(int i=0; i< m_desktopItemInfoMap.count(); i++){
        names.append(m_desktopItemInfoMap.values()[i].DisplayName);
    }

    QDBusPendingReply<QString> reply = m_pinyinInterface->QueryList(names);
    reply.waitForFinished();
    if (!reply.isError()){
        QString result = reply.argumentAt(0).toString();
        QJsonObject obj = QJsonObject::fromVariantMap(QJsonDocument::fromJson(result.toLocal8Bit()).toVariant().toMap());
        foreach(QString key, m_desktopItemInfoMap.keys()){
            QString displayName = m_desktopItemInfoMap.value(key).DisplayName;
            QString uri = m_desktopItemInfoMap.value(key).URI;
            if (obj.contains(displayName)){
                  QList<QVariant> pinyins  = obj.value(displayName).toVariant().toList();
//                  qDebug() << displayName << pinyins;
                  if (pinyins.length() > 0){
                        m_desktopItemInfoMap[key].pinyinName = pinyins.at(0).toString();
                        m_desktopItemInfoMap[key].lowerPinyinName = m_desktopItemInfoMap[key].pinyinName.toLower();
                  }
            }
        }
        sortPingyinEnglish();
    }else{
        qCritical() << reply.error().message();
    }
}


void DBusController::sortPingyinEnglish(){
    QList<FileItemInfo> pinyinInfos;
    QList<FileItemInfo> englishInfos;
    foreach (FileItemInfo info, m_desktopItemInfoMap.values()) {
        if (info.pinyinName.length() > 0 && info.DisplayName.length() > 0){
            if (info.lowerPinyinName.at(0) == info.DisplayName.toLower().at(0)){
                englishInfos.append(info);
            }else{
                pinyinInfos.append(info);
            }
        }
    }
    std::sort(pinyinInfos.begin(), pinyinInfos.end(), appNameLessThan);
    std::sort(englishInfos.begin(), englishInfos.end(), appNameLessThan);
    m_sortedPinyinInfos.clear();
    m_sortedPinyinInfos = pinyinInfos + englishInfos;
    emit signalManager->pinyinResultChanged(m_sortedPinyinInfos);
}

QMap<QString, FileItemInfoMap> DBusController::getAppGroups(){
    return m_appGroups;
}

FileOperationsInterface* DBusController::getFileOperationsInterface(){
    return m_fileOperationsInterface;
}

FileInfoInterface* DBusController::getFileInfoInterface(){
    return m_fileInfoInterface;
}

void DBusController::getAppGroupItemsByUrl(QString group_url){
    if (!group_url.startsWith(FilePrefix)){
         group_url =  group_url;
    }else{
         group_url = decodeUrl(group_url);
    }
    QString group_dir = group_url;
    QDir groupDir(group_dir);
    qDebug() << group_url << groupDir << groupDir.exists();
    QFileInfoList fileInfoList  = groupDir.entryInfoList(QDir::NoDotAndDotDot | QDir::Files);
    if (groupDir.exists()){
        if (fileInfoList.count() == 0){
            bool flag = groupDir.removeRecursively();
            qDebug() << decodeUrl(group_url) << "delete removeRecursively" << flag;
        }else if (fileInfoList.count() == 1){
            qDebug() << fileInfoList.at(0).filePath() << "only one .desktop file in app group";
            QFile f(fileInfoList.at(0).filePath());
            QString newFileName = desktopLocation + QString(QDir::separator()) + fileInfoList.at(0).fileName();
            f.rename(newFileName);
            QDir(fileInfoList.at(0).absoluteDir()).removeRecursively();
//            emit signalManager->appGounpDetailClosed();
        }else{
             qDebug() << "update app group icon==============1";
            QDBusPendingReply<FileItemInfoMap> reply = m_desktopDaemonInterface->GetAppGroupItems(group_url);
            reply.waitForFinished();
            if (!reply.isError()){
                FileItemInfoMap desktopItemInfos = qdbus_cast<FileItemInfoMap>(reply.argumentAt(0));

                if (desktopItemInfos.count() > 1){
                    qDebug() << "update app group icon==============2" << desktopItemInfos.keys();
                    emit signalManager->appGounpItemsChanged(group_url, desktopItemInfos);
                    m_appGroups.insert(group_url, desktopItemInfos);
                }else{
                    m_appGroups.remove(group_url);
                }
            }else{
                qCritical() << reply.error().message();
            }
        }
    }else{
        qCritical() << "App Group Dir isn't exists" << groupDir;
    }
}

FileItemInfoMap DBusController::getDesktopItemInfoMap(){
    return m_desktopItemInfoMap;
}


void DBusController::asyncRenameDesktopItemByUrl(QString url){
    QDBusPendingReply<FileItemInfo> reply = m_desktopDaemonInterface->GetItemInfo(url);

    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(reply, this);
    connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
                        this, SLOT(asyncRenameDesktopItemByUrlFinished(QDBusPendingCallWatcher*)));
}

void DBusController::asyncRenameDesktopItemByUrlFinished(QDBusPendingCallWatcher *call){
    QDBusPendingReply<FileItemInfo> reply = *call;
    if (!reply.isError()) {
        FileItemInfo desktopItemInfo = qdbus_cast<FileItemInfo>(reply.argumentAt(0));
        emit signalManager->itemMoved(desktopItemInfo);
        updateDesktopItemInfoMap(desktopItemInfo);

        if (isAppGroup(desktopItemInfo.URI)){
            qDebug() << "renamed file move in app group" << desktopItemInfo.URI;
            getAppGroupItemsByUrl(desktopItemInfo.URI);
        }
        requestIconByUrl(desktopItemInfo.URI, 48);
    } else {
        qCritical() << reply.error().message();
    }
    call->deleteLater();
}


void DBusController::asyncCreateDesktopItemByUrl(QString url){
    QDBusPendingReply<FileItemInfo> reply = m_desktopDaemonInterface->GetItemInfo(url);

    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(reply, this);
    connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
                        this, SLOT(asyncCreateDesktopItemByUrlFinished(QDBusPendingCallWatcher*)));
}


void DBusController::asyncCreateDesktopItemByUrlFinished(QDBusPendingCallWatcher *call){
    QDBusPendingReply<FileItemInfo> reply = *call;
    if (!reply.isError()) {
        FileItemInfo desktopItemInfo = qdbus_cast<FileItemInfo>(reply.argumentAt(0));
        qDebug() << desktopItemInfo.URI << desktopItemInfo.Icon << desktopItemInfo.thumbnail << desktopItemInfo.MIME << getMimeTypeName(desktopItemInfo.URI);
        /*ToDo desktop daemon settings judge*/
        bool isRequestThumbnailFlag = isRequestThumbnail(desktopItemInfo.URI);
        if (desktopItemInfo.thumbnail.length() > 0 && isRequestThumbnailFlag){
            desktopItemInfo.Icon = desktopItemInfo.thumbnail;
        }
        m_thumbnails.append(desktopItemInfo.URI);
        qDebug() << desktopItemInfo.URI << isRequestThumbnailFlag;
        if (isRequestThumbnailFlag){
            m_thumbnailTimer->start();
        }
        emit signalManager->itemCreated(desktopItemInfo);
        qDebug() << "asyncCreateDesktopItemByUrlFinished" << desktopItemInfo.thumbnail <<"11111111111";
        updateDesktopItemInfoMap(desktopItemInfo);
        qDebug() << "isAppGroup(desktopItemInfo.URI)" << isAppGroup(desktopItemInfo.URI);
        if (isAppGroup(desktopItemInfo.URI)){
            getAppGroupItemsByUrl(desktopItemInfo.URI);
        }

        if (desktopItemInfo.MIME != getMimeTypeName(desktopItemInfo.URI)){
            qDebug() << "update icon because of mimetype" << getMimeTypeName(desktopItemInfo.URI);
            QString url = desktopItemInfo.URI;
            QString iconUrl = ThemeAppIcon::getThemeIconPath(getMimeTypeIconName(url));
            emit signalManager->desktoItemIconUpdated(url, iconUrl, 48);
        }

    } else {
        qCritical() << reply.error().message();
    }
    call->deleteLater();
}


void DBusController::handleFileCreated(const QString &path){
    qDebug() << "handleFileCreated" << path;
    QFileInfo f(path);
    if (f.fileName().startsWith(".") && !isAppGroup(path)){
        return;
    }
    if (isDesktop(f.path())){
        qDebug() << "create file in desktop" << path;
        asyncCreateDesktopItemByUrl(path);
        QTimer* refreshTimer = new QTimer;
        refreshTimer->setSingleShot(true);
        refreshTimer->setInterval(500);
        connect(refreshTimer, &QTimer::timeout, [=](){
            this->refreshThumail("file://" + path);
        });
        connect(refreshTimer, SIGNAL(timeout()), refreshTimer, SLOT(deleteLater()));
        refreshTimer->start();
    }else{
        if(isApp(path) && isAppGroup(f.path())){
            qDebug() << "create file in app group" << path;
            getAppGroupItemsByUrl(f.path());
        }else{
            qDebug() << "*******" << "created invalid file(not .desktop file)"<< "*********";
        }
    }
    m_pinyinTimer->start();
}

void DBusController::handleFileDeleted(const QString &path){
    qDebug() << "handleFileDeleted" << path;
    QFileInfo f(path);
    if (isDesktop(f.path())){
        qDebug() << "delete desktop file:" << path;
        removeDesktopItemInfoByUrl(path);
        emit signalManager->itemDeleted(path);
        if (isAppGroup(path)){
            getAppGroupItemsByUrl(path);
//            emit signalManager->appGounpDetailClosed();
        }
    }else if (isAppGroup(f.path())){
        removeDesktopItemInfoByUrl(path);
        emit signalManager->itemDeleted(path);
        qDebug() << "delete desktop app group folder:" << path;
        getAppGroupItemsByUrl(f.path());
    }
    m_pinyinTimer->start();
}


void DBusController::handleFileMovedIn(const QString &path){
    qDebug() << "handleFileMovedIn";
    QFileInfo f(path);
    if (isDesktop(f.path())){
        qDebug() << "move file in desktop" << path;
        asyncCreateDesktopItemByUrl(path);
    }else if (isAppGroup(f.path())){
        qDebug() << "move file in App Group" << path;
       getAppGroupItemsByUrl(f.path());
   }
   m_pinyinTimer->start();
}


void DBusController::handleFileMovedOut(const QString &path){
    qDebug() << "handleFileMovedOut" << path;
    QFileInfo f(path);
    if (isDesktop(f.path())){
        qDebug() << "move file out desktop" << path;
        removeDesktopItemInfoByUrl(path);
        emit signalManager->itemDeleted(path);
    }else if (isAppGroup(f.path())){
         qDebug() << "move file out App Group" << path;
        getAppGroupItemsByUrl(f.path());
    }
    m_pinyinTimer->start();
}


void DBusController::handleFileRenamed(const QString &oldPath, const QString &newPath){
    QFileInfo oldFileInfo(oldPath);
    QFileInfo newFileInfo(newPath);
    qDebug() << oldPath << newPath;
    qDebug() <<oldFileInfo.filePath() << isInDesktopFolder(oldFileInfo.filePath()) << newFileInfo.filePath()<<isInDesktopFolder(newFileInfo.path());

    bool isAppGroupFolder = isAppGroup(oldFileInfo.filePath()) && isAppGroup(newFileInfo.filePath());
    bool isDesktopFile = isInDesktopFolder(oldFileInfo.filePath()) && isInDesktopFolder(newFileInfo.filePath());
    qDebug() << "isAppGroupFolder" << isAppGroupFolder<< "isDesktopFile" << isDesktopFile;

    if (newFileInfo.fileName().startsWith(".") && !isAppGroupFolder){
        emit signalManager->itemDeleted(oldPath);
        return;
    }
    if (isDesktopFile){
        qDebug() << "desktop file renamed";
        m_itemShoudBeMoved = oldPath;
        qDebug() << m_desktopItemInfoMap.keys() << decodeUrl(oldPath);
        bool flag(false);
        foreach (QString key, m_desktopItemInfoMap.keys()) {
            QString uri = decodeUrl(key);
            if (uri == oldPath){
                flag = true;
            }
        }
        if (flag){
            emit signalManager->itemShoudBeMoved(oldPath);
            asyncRenameDesktopItemByUrl(newPath);
        }else{
            m_renameOldPath  = oldPath;
            m_renameNewPath = newPath;
            QTimer* delayTimer = new QTimer;
            delayTimer->setSingleShot(true);
            delayTimer->setInterval(200);
            connect(delayTimer, SIGNAL(timeout()), this, SLOT(delayHandleRenameEvent()));
            connect(delayTimer, SIGNAL(timeout()), delayTimer, SLOT(deleteLater()));
            delayTimer->start();
        }
    }
    m_pinyinTimer->start();
}

void DBusController::delayHandleRenameEvent()
{
    qDebug() << "Delay handleFileRenamed action";
    emit signalManager->itemShoudBeMoved(m_renameOldPath);
    asyncRenameDesktopItemByUrl(m_renameNewPath);
}


void DBusController::handleFileMetaDataChanged(const QString &path)
{
    emit signalManager->fileMetaDataChanged(path);
}

void DBusController::updateDesktopItemInfoMap(FileItemInfo desktopItemInfo){
    m_desktopItemInfoMap.insert(desktopItemInfo.URI, desktopItemInfo);
}


void DBusController::updateDesktopItemInfoMap_moved(FileItemInfo desktopItemInfo){
    QString oldKey = m_itemShoudBeMoved;
    QString newKey = desktopItemInfo.URI;

    FileItemInfoMap::Iterator iterator = m_desktopItemInfoMap.find(oldKey);
    if (iterator!=m_desktopItemInfoMap.end()){
        m_desktopItemInfoMap.insert(iterator, newKey, desktopItemInfo);
    }
}

void DBusController::removeDesktopItemInfoByUrl(QString url){
    if (m_desktopItemInfoMap.contains(url)){
        m_desktopItemInfoMap.remove(url);
    }
}

void DBusController::openFiles(QStringList files, IntList intFlags){
    qDebug() << files << intFlags;

    foreach (QString file, files) {
        int index = files.indexOf(file);
        if (intFlags.at(index) == 0){ //RequestOpenPolicyOpen = 0
            QString key = QString(QUrl(file.toLocal8Bit()).toEncoded());
            if (m_desktopItemInfoMap.contains(key)){
                FileItemInfo desktopItemInfo = m_desktopItemInfoMap.value(key);
                qDebug() << desktopItemInfo.URI << "open";
                QDBusPendingReply<> reply = m_desktopDaemonInterface->ActivateFile(desktopItemInfo.URI, QStringList(), desktopItemInfo.CanExecute, 0);
                reply.waitForFinished();
                if (!reply.isError()){

                }else{
                    qCritical() << reply.error().message();
                }
            }
        }else{ //RequestOpenPolicyOpen = 1

        }
    }
}

void DBusController::openFiles(FileItemInfo destinationDesktopItemInfo, QList<FileItemInfo> desktopItemInfos){
    QStringList urls;
    foreach (FileItemInfo info, desktopItemInfos){
        urls.append(info.URI);
    }
    openFiles(destinationDesktopItemInfo, urls);
}


void DBusController::openFiles(FileItemInfo destinationDesktopItemInfo, QStringList urls){
    QDBusPendingReply<> reply = m_desktopDaemonInterface->ActivateFile(destinationDesktopItemInfo.URI, urls, destinationDesktopItemInfo.CanExecute, 0);
    reply.waitForFinished();
    if (!reply.isError()){

    }else{
        qCritical() << reply.error().message();
    }
}

void DBusController::openFile(FileItemInfo desktopItemInfo){
    //TODO query RequestOpenPolicyOpen or RequestOpenPolicyOpen
    QDBusPendingReply<> reply = m_desktopDaemonInterface->ActivateFileWithTimestamp(desktopItemInfo.URI, QStringList(), desktopItemInfo.CanExecute, 0, 0);
    reply.waitForFinished();
    if (!reply.isError()){

    }else{
        qCritical() << reply.error().message() << desktopItemInfo.URI;
    }
//    emit signalManager->appGounpDetailClosed();
}


void DBusController::createDirectory(){
    QDBusPendingReply<QString, QDBusObjectPath, QString> reply = m_fileOperationsInterface->NewCreateDirectoryJob(desktopLocation, "", "", "", "");
    reply.waitForFinished();
    if (!reply.isError()){
        QString service = reply.argumentAt(0).toString();
        QString path = qdbus_cast<QDBusObjectPath>(reply.argumentAt(1)).path();
        m_createDirJobInterface = new CreateDirJobInterface(service, path, QDBusConnection::sessionBus(), this);
        connect(m_createDirJobInterface, SIGNAL(Done(QString,QString)),
                this, SLOT(createDirectoryFinished(QString,QString)));
        m_createDirJobInterface->Execute();
    }else{
        qCritical() << reply.error().message();
    }
}


void DBusController::createDirectoryFinished(QString dirName, QString error){
    Q_UNUSED(dirName)
    Q_UNUSED(error)
    disconnect(m_createDirJobInterface, SIGNAL(Done(QString, QString)), this, SLOT(createDirectoryFinished(QString, QString)));
    m_createDirJobInterface = NULL;
    emit signalManager->fileCreated(dirName);
}

void DBusController::createFile(){
    QDBusPendingReply<QString, QDBusObjectPath, QString> reply = m_fileOperationsInterface->NewCreateFileJob(desktopLocation, "", "", "", "", "");
    reply.waitForFinished();
    if (!reply.isError()){
        QString service = reply.argumentAt(0).toString();
        QString path = qdbus_cast<QDBusObjectPath>(reply.argumentAt(1)).path();
        m_createFileJobInterface = new CreateFileJobInterface(service, path, QDBusConnection::sessionBus(), this);
        connect(m_createFileJobInterface, SIGNAL(Done(QString,QString)),
                this, SLOT(createFileFinished(QString, QString)));
        m_createFileJobInterface->Execute();
    }else{
        qCritical() << reply.error().message();
    }
}

void DBusController::createFileFinished(QString filename, QString error){
    Q_UNUSED(filename)
    Q_UNUSED(error)
    disconnect(m_createFileJobInterface, SIGNAL(Done(QString, QString)), this, SLOT(createFileFinished(QString, QString)));
    m_createFileJobInterface = NULL;
    emit signalManager->fileCreated(filename);
}


void DBusController::createFileFromTemplate(QString templatefile){
    QDBusPendingReply<QString, QDBusObjectPath, QString> reply = m_fileOperationsInterface->NewCreateFileFromTemplateJob(desktopLocation, templatefile, "", "", "");
    reply.waitForFinished();
    if (!reply.isError()){
        QString service = reply.argumentAt(0).toString();
        QString path = qdbus_cast<QDBusObjectPath>(reply.argumentAt(1)).path();
        m_createFileFromTemplateJobInterface = new CreateFileFromTemplateJobInterface(service, path, QDBusConnection::sessionBus(), this);
        connect(m_createFileFromTemplateJobInterface, SIGNAL(Done(QString,QString)),
                this, SLOT(createFileFromTemplateFinished(QString, QString)));
        m_createFileFromTemplateJobInterface->Execute();
    }else{
        qCritical() << reply.error().message();
    }
}


void DBusController::createFileFromTemplateFinished(QString filename, QString error){
    Q_UNUSED(filename)
    Q_UNUSED(error)
    disconnect(m_createFileFromTemplateJobInterface, SIGNAL(Done(QString, QString)), this, SLOT(createFileFromTemplateFinished(QString, QString)));
    m_createFileFromTemplateJobInterface = NULL;
    emit signalManager->fileCreated(filename);
}


void DBusController::sortByKey(QString key){
    qDebug() << key;
    emit signalManager->sortByKey(key);
}


void DBusController::requestCreatingAppGroup(QStringList urls){
    QDBusPendingReply<> reply = m_desktopDaemonInterface->RequestCreatingAppGroup(urls);
    reply.waitForFinished();
    if (!reply.isError()){

    }else{
        qCritical() << reply.error().message();
    }
}

void DBusController::createAppGroup(QString group_url, QStringList urls){
    Q_UNUSED(group_url)
    Q_UNUSED(urls)
//    qDebug() << group_url << urls;
//    if (urls.count() >= 2){
//        emit signalManager->appGounpCreated(group_url);
//    }
}

void DBusController::mergeIntoAppGroup(QStringList urls, QString group_url){
    qDebug() << urls << "merge into" << group_url;
    QDBusPendingReply<> reply = m_desktopDaemonInterface->RequestMergeIntoAppGroup(urls, group_url);
    reply.waitForFinished();
    if (!reply.isError()){
        getAppGroupItemsByUrl(group_url);
    }else{
        qCritical() << reply.error().message();
    }
}

void DBusController::copyFiles(QStringList urls){
    emit signalManager->cancleFilesCuted();
    m_clipboardInterface->CopyToClipboard(urls);
}

void DBusController::cutFiles(QStringList urls){
    m_clipboardInterface->CutToClipboard(urls);
}

void DBusController::pasteFilesToDesktop(){
    m_clipboardInterface->EmitPaste(desktopLocation);
}

void DBusController::pasteFiles(QString action, QStringList files, QString destination){
    qDebug() << action << files << destination;
    if (action == "cut"){
        bool isFilesFromDesktop = true;
        foreach (QString fpath, files) {
            QString url = decodeUrl(fpath);
            QFileInfo f(url);
            bool flag = (f.absolutePath() == destination);
            isFilesFromDesktop = isFilesFromDesktop && flag;
        }
        if (isFilesFromDesktop){
            emit signalManager->cancelFilesCuted(files);
        }else{
            emit signalManager->moveFilesExcuted(files, destination);
        }
        qApp->clipboard()->clear();
    }else if (action == "copy"){
        emit signalManager->copyFilesExcuted(files, destination);
    }
}


void DBusController::handelIconThemeChanged(){
//    bool rmFlag = QDir(getThumbnailsPath()).removeRecursively();
//    qDebug() << "Remove cache" << rmFlag;
    requestIconByUrl(ComputerUrl, 48);
    requestIconByUrl(TrashUrl, 48);
    foreach(QString url, m_desktopItemInfoMap.keys()){
        qDebug() << getMimeTypeName(url);
        if (m_desktopItemInfoMap.value(url).thumbnail.length() == 0){
            requestIconByUrl(url, 48);
        }/*else if (m_desktopItemInfoMap.value(url).thumbnail.length() > 0 && getMimeTypeName(url) == "text/plain"){
            requestIconByUrl(url, 48);
        }*/
    }
}

DBusController::~DBusController()
{

}

