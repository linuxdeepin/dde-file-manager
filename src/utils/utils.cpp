// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QStandardPaths>
#include <QSvgRenderer>
#include <QPainter>
#include <QProcess>
#include <QGuiApplication>
#include <QJsonParseError>
#include <dgiovolumemanager.h>
#include <dgiomount.h>
#include <dgiofile.h>
#include "utils.h"
#include "utils.h"
#include "dmimedatabase.h"
#include "interfaces/dfmglobal.h"
#include "app/define.h"
#include "dfmapplication.h"

#include "shutil/fileutils.h"

DFM_USE_NAMESPACE
QString getThumbnailsPath(){
    QString cachePath = QStandardPaths::standardLocations(QStandardPaths::CacheLocation).at(0);
    QString thumbnailPath = joinPath(cachePath, "thumbnails");
    if (!QDir(thumbnailPath).exists()){
        QDir(thumbnailPath).mkpath(thumbnailPath);
    }
    return thumbnailPath;
}

QString decodeUrl(QString url){
    if (!url.startsWith(FilePrefix)){
        url = FilePrefix + url;
        url = QUrl(url).toLocalFile();
    }else{
        url = QUrl(url).toLocalFile();
    }
    return url;
}

QString deleteFilePrefix(QString path){
    QString ret = path.replace(FilePrefix, "");
    return decodeUrl(ret);
}

bool isDesktop(QString url){
    if (url.startsWith(FilePrefix)){
        url.replace(FilePrefix, "");
    }
    if (url == desktopLocation){
        return true;
    }
    return false;
}

bool isAppGroup(QString url){
    if (url.startsWith(FilePrefix)){
        url.replace(FilePrefix, "");
    }
    QFileInfo f(url);
    if (decodeUrl(f.path()) == desktopLocation){
        if (f.fileName().startsWith(RichDirPrefix)){
            return true;
        }
    }
    return false;
}


bool isApp(QString url){
    if (url.endsWith(AppSuffix)){
        return true;
    }
    return false;
}

bool isAllApp(QStringList urls){
    if (urls.length() > 0){
        bool ret = true;
        foreach (QString url, urls) {
            ret = ret && isApp(url);
            if (!ret){
                return false;
            }
        }
        return ret;
    }else{
        return false;
    }
}

bool isComputer(QString url){
    if (url == ComputerUrl){
        return true;
    }
    return false;
}

bool isTrash(QString url){
    if (url == TrashUrl){
        return true;
    }
    return false;
}


bool isFolder(QString url){
    url.replace(FilePrefix, "");
    if (QDir(url).exists()){
        if (decodeUrl(url).contains(desktopLocation)){
            if (QDir(url).dirName().startsWith(RichDirPrefix)){
                return false;
            }
        }
        return true;
    }
    return false;
}

bool isInDesktop(QString url){
    if (decodeUrl(QFileInfo(url).path()) == desktopLocation){
        return true;
    }else if (QFileInfo(url).path() == desktopLocation){
        return true;
    }else if (url.contains(desktopLocation)){
        return true;
    }
    return false;
}

bool isInDesktopFolder(QString url){
    if (decodeUrl(QFileInfo(url).path()) == desktopLocation){
        return true;
    }else if (QFileInfo(url).path() == desktopLocation){
        return true;
    }
    return false;
}


bool isDesktopAppFile(QString url){
    if (QFileInfo(url).fileName().endsWith(AppSuffix)){
        return true;
    }
    return false;
}

bool isRequestThumbnail(QString url){
    QString mimetypeName = getMimeTypeName(url);
    if (SupportMimeTypes.contains(mimetypeName)){
        return true;
    }else{
        return false;
    }
}

QString getMimeTypeGenericIconName(QString url){
    DMimeDatabase mimeDataBae;
    QMimeType mimeType = mimeDataBae.mimeTypeForFile(deleteFilePrefix(url));
    return mimeType.genericIconName();
}

QString getMimeTypeIconName(QString url){
    DMimeDatabase mimeDataBae;
    QMimeType mimeType = mimeDataBae.mimeTypeForFile(deleteFilePrefix(url));
    return mimeType.iconName();
}

QString getMimeTypeName(QString url){
    DMimeDatabase mimeDataBae;
    QMimeType mimeType = mimeDataBae.mimeTypeForFile(deleteFilePrefix(url));
    return mimeType.name();
}


QString getQssFromFile(QString filename)
{
    QFile f(filename);
    QString qss = "";
    if (f.open(QFile::ReadOnly))
    {
        qss = QLatin1String(f.readAll());
        f.close();
    }
    return qss;
}

QByteArray joinPath(const QByteArray& path, const QByteArray &fileName){
    return path + QDir::separator().toLatin1() + fileName;
}

QPixmap svgToPixmap(const QString &path, int w, int h)
{
    QPixmap pixmap(w, h);
    QSvgRenderer renderer(path);
    pixmap.fill(Qt::transparent);

    QPainter painter;
    painter.begin(&pixmap);

    renderer.render(&painter);

    painter.end();

    return pixmap;
}

QPixmap svgToHDPIPixmap(const QString &path, int w, int h)
{
    qreal ratio = qApp->devicePixelRatio();
    return svgToPixmap(path, w * ratio, h * ratio);
}

QPixmap svgToHDPIPixmap(const QString &path)
{
    QPixmap p(path);
    qreal ratio = qApp->devicePixelRatio();
    return svgToPixmap(path, p.width() * ratio, p.height() * ratio);
}

QString joinPath(const QString &path, const QString &fileName)
{
    return path + QDir::separator() + fileName;
}

bool isAvfsMounted()
{
    QProcess p;
    QString cmd = "/bin/bash";
    QStringList args;
    args << "-c" << "ps -ax -o 'cmd'|grep '.avfs$'";
    p.start(cmd, args);
    p.waitForFinished();
    QString result = p.readAll().trimmed();
    if(!result.isEmpty()){
        QStringList datas = result.split(" ");

        if(datas.count() == 2){
            if(datas.at(0) == "avfsd" && QFile::exists(datas.at(1)))
                return true;
        }
    }
    return false;
}

/**
 * @brief clearStageDir 清理光盘缓存路径下的所有空路径
 * @param stagingRoot
 */
void clearStageDir(const QString &stagingRoot)
{
    QFileInfo info(stagingRoot);
    if (!info.exists() || !info.isDir())
        return;

    QFile f(stagingRoot);
    f.setPermissions(f.permissions()
                     | QFile::ReadUser | QFile::WriteUser
                     | QFile::ReadGroup | QFile::ReadOther );

    const static QString stagePrefix = QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation) + "/"
            + qApp->organizationName() + "/" DISCBURN_STAGING + "/_dev_sr";

    QString absPath = info.canonicalFilePath();
    if (!absPath.startsWith(stagePrefix))
        return;

    QDir d(absPath);
    if (d.isEmpty(QDir::AllEntries | QDir::Hidden | QDir::NoDotAndDotDot | QDir::NoSymLinks)) {
        qDebug() << absPath << " is removed";
        bool removed = d.removeRecursively();
        if (!removed) {
            qDebug() << absPath << "cannot be removed recurisively...";
            return;
        }
        d.cdUp();
        clearStageDir(d.canonicalPath());
        return;
    }
    QStringList dirs = d.entryList(QDir::Dirs | QDir::Hidden | QDir::NoDotAndDotDot);
    foreach (auto dir, dirs)
        clearStageDir(absPath + "/" + dir);
}

void RemoteMountsStashManager::stashRemoteMount(const QString &mpt, const QString &displayName)
{
    DFMApplication::syncGenericAttribute();

    if (!DFMApplication::genericAttribute(DFMApplication::GA_AlwaysShowOfflineRemoteConnections).toBool())
        return;
    QString key {mpt}, protocol, host, share, sharePath;
    QString mountPoint(mpt);
    if (FileUtils::isSmbPath(mountPoint)) { // parse smb mpt(/run/user/1000/gvfs/smb-share:domain=ttt,server=xx.xx.xx.xx,share=io,user=uos/path)
        protocol = "smb";
        host = FileUtils::smbAttribute(mountPoint, FileUtils::SmbAttribute::kServer);
        share = FileUtils::smbAttribute(mountPoint, FileUtils::SmbAttribute::kShareName);
        sharePath = FileUtils::smbAttribute(mountPoint, FileUtils::SmbAttribute::kSharePath);
        if(!sharePath.isEmpty())
            share.append("/").append(sharePath);
    } else if (mountPoint.contains("/ftp:")) { // parse ftp mpt(//run/user/1000/gvfs/ftp:host=4.3.2.1)
        // TODO: maybe someday we need to stash ftp too. but for now, just return.
        qInfo() << "not valid smb share, do not stash.";
        return;
    } else {
        qInfo() << "not valid smb share, do not stash.";
        return;
    }

    // stash to local config file
    QFile configFile(CONFIG_PATH);
    if (!configFile.open(QIODevice::ReadOnly))
        return;

    QJsonParseError err;
    QJsonDocument config = QJsonDocument::fromJson(configFile.readAll(), &err);
    if (err.error != QJsonParseError::NoError) {
        qWarning() << "config file is not valid json file: " << err.errorString();
        configFile.close();
        return;
    }
    configFile.close();

    QJsonObject newMount;
    QJsonObject obj = config.object();
    QJsonObject remoteMountsObj;
    ///添加到SMB聚合列表字段，无论是否配置SMB常驻，侧边栏和计算机界面的smb聚合项都以此为数据源做界面显示 - start
    bool newSmbDeviceInserted = false;
    if(protocol == SMB_SCHEME && !host.isEmpty()){//本次缓存的是smb挂载
        QString newSmbDevice = QString("%1://%2").arg(protocol).arg(host);
        if(obj.contains("StashedSmbDevices")){
            QJsonValue stashedSmbDevices = obj.value("StashedSmbDevices");
            if(stashedSmbDevices.isObject()){
                QJsonObject temObj = stashedSmbDevices.toObject();
                QJsonArray stashSbDeviceArray = temObj.value("SmbIntegrations").toArray();
                if(!stashSbDeviceArray.contains(QJsonValue(newSmbDevice))){
                    stashSbDeviceArray << QJsonValue(newSmbDevice);
                    temObj.insert("SmbIntegrations",stashSbDeviceArray);
                    obj.insert("StashedSmbDevices",temObj);
                    newSmbDeviceInserted = true;
                }
            }
        }else{
            QJsonArray stashSbDeviceArray;
            stashSbDeviceArray << QJsonValue(newSmbDevice);
            QJsonObject temObj;
            temObj.insert("SmbIntegrations",stashSbDeviceArray);
            obj.insert("StashedSmbDevices",temObj);
            newSmbDeviceInserted = true;
        }
    }
    if(!newSmbDeviceInserted){
        return ;
    }
    ///添加到SMB聚合列表字段... - end

    /* 实现smb挂载侧边栏聚合后，这里不再读取RemoteMounts字段及插入常驻挂载（请保留此部分注释）
    if (obj.contains("RemoteMounts")) {
        QJsonValue remoteMounts = obj.value("RemoteMounts");
        if (remoteMounts.isObject()) {
            remoteMountsObj = remoteMounts.toObject();
            if (remoteMountsObj.keys().contains(key)) {
                emit DFMApplication::instance()->reloadComputerModel();
                return;
            }
        }
    }
    newMount.insert(REMOTE_HOST, host);
    newMount.insert(REMOTE_SHARE, share);
    newMount.insert(REMOTE_PROTOCOL, protocol);
    newMount.insert(REMOTE_DISPLAYNAME, displayName);
    remoteMountsObj.insert(key, newMount);//smb挂载聚合后，无需写.remote缓存路径
    obj.insert("RemoteMounts", remoteMountsObj);
    */

    config.setObject(obj);
    configFile.open(QIODevice::ReadWrite | QIODevice::Truncate);
    configFile.write(config.toJson());
    configFile.close();

    qInfo() << "remote mounts: " << mpt << "is stashed.";
}

QList<QVariantMap> RemoteMountsStashManager::remoteMounts()
{
    DFMApplication::syncGenericAttribute();

    QList<QVariantMap> ret;
    QFile configFile(CONFIG_PATH);
    if (!configFile.open(QIODevice::ReadOnly)) {
        return ret;
    }
    QByteArray data = configFile.readAll();
    configFile.close();

    QJsonParseError err;
    QJsonDocument config = QJsonDocument::fromJson(data, &err);
    if (err.error != QJsonParseError::NoError) {
        qWarning() << "config file is not valid json file: " << err.errorString();
        return ret;
    }

    QJsonObject obj = config.object();
    QJsonValue remoteMounts = obj.value("RemoteMounts");
    if (remoteMounts.isObject()) {
        QJsonObject mountsObj = remoteMounts.toObject();
        const QStringList &itemKeys = mountsObj.keys();
        for (const auto &itemKey: itemKeys) {
            QJsonValue mountItem = mountsObj.value(itemKey);
            if (!mountItem.isObject())
                continue;

            QVariantMap item;
            item.insert("key", itemKey);
            QJsonObject itemObj = mountItem.toObject();
            const QStringList &mountObjKeys = itemObj.keys();
            for (const auto &mountObjKey: mountObjKeys) {
                const QVariant &value = itemObj.value(mountObjKey).toVariant();
                item.insert(mountObjKey, value);
            }
            ret << item;
        }
    }
    return ret;
}

void RemoteMountsStashManager::removeRemoteMountItem(const QString &key)
{
    DFMApplication::syncGenericAttribute();

    QFile configFile(CONFIG_PATH);
    if (!configFile.open(QIODevice::ReadOnly)) {
        return;
    }

    QByteArray data = configFile.readAll();
    configFile.close();

    QJsonParseError err;
    QJsonDocument config = QJsonDocument::fromJson(data, &err);
    if (err.error != QJsonParseError::NoError) {
        qWarning() << "config file is not valid json file: " << err.errorString();
        return;
    }

    QJsonObject obj = config.object();
    QJsonValue remoteMounts = obj.value("RemoteMounts");
    if (remoteMounts.isObject()) {
        QJsonObject mountsObj = remoteMounts.toObject();
        if (!mountsObj.contains(key)) {
            return;
        }
        mountsObj.remove(key);
        obj.insert("RemoteMounts", mountsObj);
        config.setObject(obj);
        configFile.open(QIODevice::WriteOnly | QIODevice::Truncate);
        configFile.write(config.toJson());
        configFile.close();

        qInfo() << "remote mount: " << key << "is unstashed.";
    }
}

void RemoteMountsStashManager::clearRemoteMounts()
{
    //考虑到smb聚合功能实现之后的兼容性，调用此函数移除配置中的RemoteMounts之后，将不会再添加它；
    //smb聚合项的缓存到配置文件的新字段StashedSmbDevices中。
    QFile configFile(CONFIG_PATH);
    if (!configFile.open(QIODevice::ReadOnly)) {
        return;
    }

    QByteArray data = configFile.readAll();
    configFile.close();

    QJsonParseError err;
    QJsonDocument config = QJsonDocument::fromJson(data, &err);
    if (err.error != QJsonParseError::NoError) {
        qWarning() << "config file is not valid json file: " << err.errorString();
        return;
    }

    QJsonObject obj = config.object();
    if (!obj.contains("RemoteMounts"))
        return;
    obj.remove("RemoteMounts");

    config.setObject(obj);
    configFile.open(QIODevice::WriteOnly | QIODevice::Truncate);
    configFile.write(config.toJson());
    configFile.close();

    qInfo() << "stashed remote mounts are cleared";
}

void RemoteMountsStashManager::stashCurrentMounts()
{
    for (auto gvfsmp : DGioVolumeManager::getMounts()) {
        auto rootFile = gvfsmp->getRootFile();
        if (!rootFile)
            continue;

        qDebug() << rootFile->path() << rootFile->basename() << rootFile->uri() << gvfsmp->name();
        auto mpt = rootFile->path();
        stashRemoteMount(rootFile->path(), gvfsmp->name());
    }
}

QString RemoteMountsStashManager::getDisplayNameByConnUrl(const QString &url)
{
    QString displayName;
    const auto &&mounts = remoteMounts();
    for (const auto &mount: mounts) {
        auto protocol = mount.value(REMOTE_PROTOCOL).toString();
        auto host = mount.value(REMOTE_HOST).toString();
        auto share = mount.value(REMOTE_SHARE).toString();
        if (protocol.isEmpty() || host.isEmpty()) {
            qWarning() << "protocol or host is empty: " << mount;
            continue;
        }

        QString connPath = QString("%1://%2/%3").arg(protocol).arg(host).arg(share);
        if (connPath == url) {
            displayName = mount.value(REMOTE_DISPLAYNAME).toString();
            break;
        }
    }

    if (displayName.isEmpty())
        displayName = QObject::tr("Unknown");
    return displayName;
}

/*!
 * \brief RemoteMountsStashManager::normalizeConnUrl
 * \param url like "/smb://1.2.3.4/share-folder.remote" which is from dfmrootfileinfo
 * \return
 */
QString RemoteMountsStashManager::normalizeConnUrl(const QString &url)
{
    auto path = url;
    while (path.startsWith("/"))
        path = path.mid(1);
    path.remove(QString(".%1").arg(SUFFIX_STASHED_REMOTE));
    return path;
}

/*!
 * \brief RemoteMountsStashManager::stashedSmbDevices 获取当前缓存的smb聚合设备列表
 * \return
 */
QStringList RemoteMountsStashManager::stashedSmbDevices()
{
    DFMApplication::syncGenericAttribute();

    QFile configFile(CONFIG_PATH);
    if (!configFile.open(QIODevice::ReadOnly))
        return QStringList();

    QJsonParseError err;
    QJsonDocument config = QJsonDocument::fromJson(configFile.readAll(), &err);
    if (err.error != QJsonParseError::NoError) {
        qWarning() << "config file is not valid json file: " << err.errorString();
        configFile.close();
        return QStringList();
    }
    configFile.close();

    QJsonObject obj = config.object();
    QStringList smbDevices;
    if(obj.contains("StashedSmbDevices")){
        QJsonValue stashedSmbDevices = obj.value("StashedSmbDevices");
        if(stashedSmbDevices.isObject()){
            QJsonObject temObj = stashedSmbDevices.toObject();
            QJsonArray jsonArray = temObj.value("SmbIntegrations").toArray();
            foreach (QJsonValue va, jsonArray) {
                QString tem = va.toString();
                if(!tem.isEmpty())
                    smbDevices << tem;
            }
        }
    }
    ///为了和SMB挂载项聚合功能实现前的配置兼容，这里读出原有缓存的smb挂载信息并提取smb://x.x.x.x保存到
    ///StashedSmbDevices字段中
    if(obj.contains("RemoteMounts")){
        QJsonValue remoteMounts = obj.value("RemoteMounts");
        if (remoteMounts.isObject()) {
            QJsonObject mountsObj = remoteMounts.toObject();
            const QStringList &itemKeys = mountsObj.keys();
            for (const auto &itemKey: itemKeys) {
                QJsonValue mountItem = mountsObj.value(itemKey);
                if (!mountItem.isObject())
                    continue;
                QJsonObject mountObj = mountItem.toObject();
                if(mountObj.value("protocol").toString() == SMB_SCHEME){
                    smbDevices << QString("%1://%2").arg(SMB_SCHEME).arg(mountObj.value("host").toString());
                }
            }
            config.setObject(obj);
            configFile.open(QIODevice::WriteOnly | QIODevice::Truncate);
            configFile.write(config.toJson());
            configFile.close();
        }
    }
    smbDevices.removeDuplicates();
    return smbDevices;
}

void RemoteMountsStashManager::removeStashedSmbDevice(const QString &url)
{
    DFMApplication::syncGenericAttribute();

    QFile configFile(CONFIG_PATH);
    if (!configFile.open(QIODevice::ReadOnly)) {
        return;
    }

    QByteArray data = configFile.readAll();
    configFile.close();

    QJsonParseError err;
    QJsonDocument config = QJsonDocument::fromJson(data, &err);
    if (err.error != QJsonParseError::NoError) {
        qWarning() << "config file is not valid json file: " << err.errorString();
        return;
    }

    QJsonObject obj = config.object();
        if(obj.contains("StashedSmbDevices")){
            QJsonValue stashedSmbDevices = obj.value("StashedSmbDevices");
            if(stashedSmbDevices.isObject()){
                QJsonObject temObj = stashedSmbDevices.toObject();
                QJsonArray jsonArray = temObj.value("SmbIntegrations").toArray();
                int count = jsonArray.count();
                for(int i = 0;i<count;i++){
                    QString tem = jsonArray.at(i).toString();
                    if(tem == url){
                       jsonArray.removeAt(i);
                       break;
                    }
                }
                if(jsonArray.count() >0 ){
                    temObj.insert("SmbIntegrations",jsonArray);
                    obj.insert("StashedSmbDevices", temObj);
                }
                else
                    obj.remove("StashedSmbDevices");
            }
        }
        config.setObject(obj);
        configFile.open(QIODevice::WriteOnly | QIODevice::Truncate);
        configFile.write(config.toJson());
        configFile.close();

        qInfo() << " smb merged remote mount: " << url << "is unstashed.";

}

void RemoteMountsStashManager::insertStashedSmbDevice(const QString &url)
{
    DFMApplication::syncGenericAttribute();

    QMutex mutex;
    QMutexLocker locker(&mutex);
    QFile configFile(CONFIG_PATH);
    if (!configFile.open(QIODevice::ReadOnly)) {
        return;
    }

    QByteArray data = configFile.readAll();
    configFile.close();

    QJsonParseError err;
    QJsonDocument config = QJsonDocument::fromJson(data, &err);
    if (err.error != QJsonParseError::NoError) {
        qWarning() << "config file is not valid json file: " << err.errorString();
        return;
    }
    QJsonObject obj = config.object();
    if(obj.contains("StashedSmbDevices")){
        QJsonValue stashedSmbDevices = obj.value("StashedSmbDevices");
        if(stashedSmbDevices.isObject()){
            QJsonObject temObj = stashedSmbDevices.toObject();
            QJsonArray stashSbDeviceArray = temObj.value("SmbIntegrations").toArray();
            if(!stashSbDeviceArray.contains(QJsonValue(url))){
                stashSbDeviceArray << QJsonValue(url);
                temObj.insert("SmbIntegrations",stashSbDeviceArray);
                obj.insert("StashedSmbDevices",temObj);
            }
        }
    }else{
        QJsonArray stashSbDeviceArray;
        stashSbDeviceArray << QJsonValue(url);
        QJsonObject temObj;
        temObj.insert("SmbIntegrations",stashSbDeviceArray);
        obj.insert("StashedSmbDevices",temObj);
    }
    config.setObject(obj);
    configFile.open(QIODevice::WriteOnly | QIODevice::Truncate);
    configFile.write(config.toJson());
    configFile.close();
}
