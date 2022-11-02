// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#include "smbintegrationswitcher.h"
#include "dabstractfileinfo.h"
#include "drootfilemanager.h"
#include "dfmapplication.h"
#include "dfmsettings.h"
#include "utils.h"
#include "app/define.h"
#include "controllers/appcontroller.h"
#include "views/windowmanager.h"
#include "shutil/fileutils.h"

#include <QDebug>
#include <QMutexLocker>

DFM_USE_NAMESPACE

class SmbIntegrationSwitcher_ : public SmbIntegrationSwitcher
{
};
Q_GLOBAL_STATIC(SmbIntegrationSwitcher_, sisGlobal)

static constexpr const char *kGenericAttribute = "GenericAttribute";
static constexpr const char *kMergeTheEntriesOfSambaSharedFolders = "MergeTheEntriesOfSambaSharedFolders";
SmbIntegrationSwitcher *SmbIntegrationSwitcher::instance()
{
    return sisGlobal;
}

void SmbIntegrationSwitcher::switchIntegrationMode(bool value)
{
    qInfo()<<"Smb mode switch start";
    isModeSwitching = true;
    // 当前的smb聚合ip列表
    QStringList smbIntigrations = RemoteMountsStashManager::stashedSmbDevices();
    // 上次分离模式时的smb ip列表(`RemoteMounts`字段信息)
    QStringList remoteMounts = RemoteMountsStashManager::stashedSmbDevices(true);
    if(value) { // 切换到smb聚合模式，取聚合和分离模式时的ip并集
        smbIntigrations.append(remoteMounts);
        smbIntigrations.removeDuplicates();
        QStringList preparedSmbIntegrations;
        if (!smbIntigrations.isEmpty()) { // 若配置中此时存在缓存的smb常驻项，
            for (const QString &smbIp : smbIntigrations)
                preparedSmbIntegrations.append(smbIp);
        }
        // 检查是否存在smb挂载目录，并将挂载目录的smb ip添加到聚合目录
        QList<DAbstractFileInfoPointer> filist  = rootFileManager->getRootFile();
        foreach (DAbstractFileInfoPointer r, filist) {
            QString temIp;
            bool isSmbRelated = FileUtils::isSmbRelatedUrl(r->fileUrl(),temIp);
            if (!isSmbRelated)//!isSmbRelated:排除ftp挂载
                continue;

            if (FileUtils::isNetworkUrlMounted(r->fileUrl())) {
                // 准备将挂载目录的smb ip添加到配置中的聚合字段
                QString localPath = QUrl::fromPercentEncoding(r->fileUrl().path().toUtf8());
                QString host = FileUtils::smbAttribute(localPath,FileUtils::SmbAttribute::kServer);
                if(!host.isEmpty())
                    preparedSmbIntegrations.append((QString("%1://%2").arg(SMB_SCHEME).arg(host)));
            }
        }
        preparedSmbIntegrations.removeDuplicates();
        for (const QString & smbIp : preparedSmbIntegrations)
            RemoteMountsStashManager::insertStashedSmbDevice(smbIp);
    } else { // 切换到smb分离模式，取上次分离模式下的ip，并把当前聚合模式下不在此列的ip卸载掉
        DUrlList unmountList;
        if(!smbIntigrations.isEmpty()) // 传一个smb ip，在AppController::actionUnmountAllSmbMount中会把所有smb挂载卸载掉
            unmountList.append(DUrl(smbIntigrations.first()));
        if (!unmountList.isEmpty()) {  //没有需要移除的smbIp，则不做操作，后续通知侧边栏和计算机界面时将界面刷新为smb分离模式
            DFMApplication::setGenericAttribute(DFMApplication::GA_MergeTheEntriesOfSambaSharedFolders, value);
            smbIntegrationMode = value; //这里先切换配置状态为分离状态(false)，后续所有操作都基于此进行
            // 调用`取消记住密码并卸载`接口，把需要卸载的smbIp卸载掉，再通知侧边栏和计算机界面，将界面刷新为smb分离模式
            QSharedPointer<DFMUrlListBaseEvent> eventPtr = dMakeEventPointer<DFMUrlListBaseEvent>(DFMEvent::UnknowType, Q_NULLPTR, unmountList);
            appController->actionForgetAllSmbPassword(eventPtr);
        }
    }
    DFMApplication::setGenericAttribute(DFMApplication::GA_MergeTheEntriesOfSambaSharedFolders, value);
    smbIntegrationMode = value;
    return ;
}

bool SmbIntegrationSwitcher::isIntegrationMode()
{
    return smbIntegrationMode;
}

bool SmbIntegrationSwitcher::isSwitching()
{
    return isModeSwitching;
}

void SmbIntegrationSwitcher::switchComplate()
{
    qInfo()<<"Smb mode switch complate.";
    isModeSwitching = false;
}

SmbIntegrationSwitcher::SmbIntegrationSwitcher(QObject *parent)
    : QObject(parent), smbIntegrationMode(true), isModeSwitching(false)
{
    // 如果没有smb聚合开关配置，则默认设置为smb聚合模式，否则以用户手动设置后为准。
    bool hasSmbInteConfig = DFMApplication::genericSetting()->keys(kGenericAttribute).contains(kMergeTheEntriesOfSambaSharedFolders);
    if (hasSmbInteConfig) {
        qInfo()<<"hasSmbInteConfig = "<<hasSmbInteConfig;
        QVariant value = DFMApplication::genericAttribute(DFMApplication::GA_MergeTheEntriesOfSambaSharedFolders);
        qInfo()<<"hasSmbInteConfig value = "<<value<<","<<value.isNull();
        if(value.isNull() || !value.isValid()){
            DFMApplication::genericSetting()->setValueNoNotify(kGenericAttribute,kMergeTheEntriesOfSambaSharedFolders,true);
            smbIntegrationMode = true;
        } else {
            smbIntegrationMode = value.toBool();
        }
    }else {
        DFMApplication::genericSetting()->setValueNoNotify(kGenericAttribute,kMergeTheEntriesOfSambaSharedFolders,true);
    }
}
