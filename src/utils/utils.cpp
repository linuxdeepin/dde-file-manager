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
#include "dmimedatabase.h"
#include "interfaces/dfmglobal.h"
#include "app/define.h"
#include "dfmapplication.h"
#include "dfmsettings.h"

#include "shutil/fileutils.h"
#include "shutil/smbintegrationswitcher.h"

DFM_USE_NAMESPACE

static constexpr const char *kRemoteMounts {"RemoteMounts"}; //缓存的smb常驻项数据
static constexpr const char *kStashedSmbDevices = "StashedSmbDevices"; //smb聚合项组
static constexpr const char *kSmbIntegrations = "SmbIntegrations"; //smb聚合项数据列表

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
    Q_UNUSED(displayName)
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
    if (protocol == SMB_SCHEME && !host.isEmpty()){
        if(smbIntegrationSwitcher->isIntegrationMode()){ // smb聚合模式：写入配置`StashedSmbDevices`字段
            QString newSmbDevice = QString("%1://%2").arg(protocol).arg(host);
            QVariantList stashedSmbDevices = DFMApplication::genericSetting()->value(kStashedSmbDevices, kSmbIntegrations).toList();
            if(!stashedSmbDevices.contains(QVariant::fromValue(newSmbDevice))) {
                stashedSmbDevices.append(newSmbDevice);
                DFMApplication::genericSetting()->setValue(kStashedSmbDevices, kSmbIntegrations, stashedSmbDevices);
            }
        } else { // smb分离模式：写入配置`RemoteMounts`字段
            QVariantMap newMount;
            newMount.insert(REMOTE_HOST, host);
            newMount.insert(REMOTE_SHARE, share);
            newMount.insert(REMOTE_PROTOCOL, protocol);
            newMount.insert(REMOTE_DISPLAYNAME, displayName);
            DFMApplication::genericSetting()->setValue(kRemoteMounts,key, newMount);
            emit DFMApplication::instance()->reloadComputerModel();
        }
    }
}

QList<QVariantMap> RemoteMountsStashManager::remoteMounts()
{
    QList<QVariantMap> ret;
    const QSet<QString> &keys = DFMApplication::genericSetting()->keys(kRemoteMounts);
    QSetIterator<QString> it(keys);

    while (it.hasNext()) {
        const QString &key = it.next();
        QVariantMap item = DFMApplication::genericSetting()->value(kRemoteMounts, key).toMap();
        item.insert("key", key);
        ret << item;
    }

    return ret;
}
/**
 * @brief RemoteMountsStashManager::removeRemoteMountItem
 * 将缓存的smb常驻地址从配置文件的`RemoteMounts`字段中移除
 * @param key
 */
void RemoteMountsStashManager::removeRemoteMountItem(const QString &key)
{
    if (smbIntegrationSwitcher->isIntegrationMode())
        return ; //smb聚合模式不去删除smb分离模式的常驻缓存(`RemoteMounts`)
    QSet<QString> set = DFMApplication::genericSetting()->keys(kRemoteMounts);
    if(set.contains(key)) {
       DFMApplication::genericSetting()->remove(kRemoteMounts,key);
       qInfo()<<"Stashed smb url : "<<key<<" removed from RemoteMounts";
    }
}

void RemoteMountsStashManager::clearRemoteMounts()
{
    //当用户手动从设置菜单中取消勾选`sambar共享端常驻显示挂载入口`复选框时,会调用此函数将配置文件中`RemoteMounts`字段缓存的常驻入口清空；
    DFMApplication::genericSetting()->removeGroup(kRemoteMounts);
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

/**
 * @brief RemoteMountsStashManager::stashedSmbDevices
 * 该函数默认返回配置文件中`RemoteMounts`和`StashedSmbDevices`字段信息中的smb://x.x.x.x并去重
 * @param onlySmbSepration
 * 该参数值为true时，该函数只返回`RemoteMounts`字段信息中解析出的smb://x.x.x.x并去重
 * @return
 * 返回smb://x.x.x.x列表信息(或smb://domain)
 */
QStringList RemoteMountsStashManager::stashedSmbDevices(bool onlySmbSepration)
{
    QStringList smbDeviceList;
    auto getRemoteMounts = [&](QStringList &list){
        // 从配置文件`RemoteMounts`字段中解析smb://x.x.x.x字段添加到smbDeviceList中
        QSet<QString> keys = DFMApplication::genericSetting()->keys(kRemoteMounts);
        for (const QString &key : keys) {
            QVariantMap stashedSmbData = DFMApplication::genericSetting()->value(kRemoteMounts, key).toMap();
            QString protocol = stashedSmbData.value("protocol").toString();
            if(protocol == SMB_SCHEME) {
                QString server = QString("%1://%2").arg(protocol).arg(stashedSmbData.value("host").toString());
                list.append(server);
            }
        }
        list.removeDuplicates();
    };

    if (onlySmbSepration) {
        getRemoteMounts(smbDeviceList);
        return smbDeviceList;
    }

    // `StashedSmbDevices`字段是旧版文管升级为有smb聚合功能版本的新增字段,下面做首次运行的数据兼容处理
    if (!DFMApplication::genericSetting()->groups().contains(kStashedSmbDevices)) { // 若配置中没有`StashedSmbDevices`字段
        getRemoteMounts(smbDeviceList);
        // 仅在首次迁移`RemoteMounts`字段中的smb设备到`StashedSmbDevices`字段中
        DFMApplication::genericSetting()->setValue(kStashedSmbDevices, kSmbIntegrations, smbDeviceList);
        smbDeviceList.removeDuplicates();
        return smbDeviceList;
    }

    // 读取smb聚合设备
    QVariantList stashedSmbDevices = DFMApplication::genericSetting()->value(kStashedSmbDevices, kSmbIntegrations).toList();
    for (const QVariant& var : stashedSmbDevices)
        smbDeviceList << var.toString();

    smbDeviceList.removeDuplicates();

    return smbDeviceList;
}

/**
 * @brief RemoteMountsStashManager::removeStashedSmbDevice
 * 从配置中移除smb聚合ip
 * @param url
 */
void RemoteMountsStashManager::removeStashedSmbDevice(const QString &url)
{
    QVariantList stashedSmbDevices = DFMApplication::genericSetting()->value(kStashedSmbDevices, kSmbIntegrations).toList();
    if (stashedSmbDevices.isEmpty() || !stashedSmbDevices.contains(QVariant::fromValue(url)))
        return;

    if (stashedSmbDevices.contains(QVariant::fromValue(url))) {
        stashedSmbDevices.removeOne(QVariant::fromValue(url));
        DFMApplication::genericSetting()->setValue(kStashedSmbDevices, kSmbIntegrations, stashedSmbDevices);
    }
}

/**
 * @brief RemoteMountsStashManager::insertStashedSmbDevice
 * 添加smb聚合ip到配置文件
 * @param url
 */
void RemoteMountsStashManager::insertStashedSmbDevice(const QString &url)
{
    QVariantList stashedSmbDevices = DFMApplication::genericSetting()->value(kStashedSmbDevices, kSmbIntegrations).toList();
    if (stashedSmbDevices.contains(QVariant::fromValue(url)))
        return;

    stashedSmbDevices.append(url);
    DFMApplication::genericSetting()->setValue(kStashedSmbDevices, kSmbIntegrations, stashedSmbDevices);
}
