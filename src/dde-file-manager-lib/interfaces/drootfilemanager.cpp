// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "drootfilemanager.h"
#include "dfileservices.h"
#include "dabstractfileinfo.h"
#include "dabstractfilewatcher.h"
#include "dfiledevice.h"
#include "dfmapplication.h"

#include "app/filesignalmanager.h"
#include "app/define.h"
#include "controllers/jobcontroller.h"
#include "dfileinfo.h"

#include "shutil/fileutils.h"
#include "shutil/filebatchprocess.h"
#include "interfaces/dfmglobal.h"
#include "singleton.h"
#include "models/dfmrootfileinfo.h"
#include "utils.h"
#include "dfmapplication.h"
#include "plugins/schemepluginmanager.h"
#include "utils/grouppolicy.h"
#include "controllers/dfmrootcontroller.h"

#include <ddiskmanager.h>
#include <dblockdevice.h>

#include <QUrl>
#include <QDebug>
#include <QtConcurrent/QtConcurrentRun>


class DRootFileManagerPrivate
{
public:

    static QMap<DUrl, DAbstractFileInfoPointer> rootfilelist;
    static QMutex rootfileMtx;
    QAtomicInteger<bool> m_bRootFileInited = false;
    QAtomicInteger<bool>  bstartonce = false;
    DAbstractFileWatcher *m_rootFileWatcher = nullptr;

    QHash<DUrl, bool> m_rootsmbftpurllist;
    JobController *m_jobcontroller = nullptr;

    volatile bool m_rootChanged = true; //用于判断是否需要发送查询完毕信号，通知外部刷新。
};

QMap<DUrl, DAbstractFileInfoPointer> DRootFileManagerPrivate::rootfilelist; //本地跟踪root目录，本地磁盘，外部磁盘挂载，网络文件挂载
QMutex DRootFileManagerPrivate::rootfileMtx;

DRootFileManager::DRootFileManager(QObject *parent)
    : QObject(parent)
    , d_ptr(new DRootFileManagerPrivate())
{
    if (DTK_POLICY_SUPPORT) {
        connect(GroupPolicy::instance(), &GroupPolicy::valueChanged, this, &DRootFileManager::policyHideSystemPartition);
        connect(fileSignalManager, &FileSignalManager::requestHideSystemPartition, this, &DRootFileManager::settingHideSystemPartition);

        {
            auto syncSambaPermanent = [](const QVariant &var) {
                DFMApplication::setGenericAttribute(DFMApplication::GA_AlwaysShowOfflineRemoteConnections, var);
            };

            static constexpr char SambaPermanentKey[] { "dfm.samba.permanent" };
            GroupPolicy::instance()->addSyncFunc(SambaPermanentKey, syncSambaPermanent);
//            QVariant var;
//            bool confSetted = GroupPolicy::instance()->isConfigSetted(SambaPermanentKey, &var);
//            bool showOffline = DFMApplication::genericAttribute(DFMApplication::GA_AlwaysShowOfflineRemoteConnections).toBool();
//            if (!confSetted) {
//                GroupPolicy::instance()->setValue(SambaPermanentKey, showOffline);
//            } else {
//                auto currDConf = GroupPolicy::instance()->getValue(SambaPermanentKey).toBool();
//                if (currDConf != showOffline) // 以组策略的值优先
//                    syncSambaPermanent(currDConf);
//            }
        }
    }

    connect(fileSignalManager, &FileSignalManager::requestHideSystemPartition, this, &DRootFileManager::hideSystemPartition);
    connect(DFMApplication::instance(), &DFMApplication::reloadComputerModel, this, &DRootFileManager::hideSystemPartition);
}

DRootFileManager::~DRootFileManager()
{
    d_ptr->m_rootsmbftpurllist.clear();
}

QList<DAbstractFileInfoPointer> DRootFileManager::getRootFile()
{
    QMutexLocker lk(&d_ptr->rootfileMtx);
    QList<DAbstractFileInfoPointer> ret = d_ptr->rootfilelist.values();
    lk.unlock();
    // fix bug 63220 排序算法中，个数低于20个时第一次调用比对函数时，传入a,b分别时是QList的第二个和第一个元素
    // 当传入元素大于20,采用了二分法，第一次调用比对函数时，传入a,b分别时是QList的第二个和元素个数的一半的元素，
    // 视频和桌面是同一级别，所以桌面就在最后了。先排一次序。
    std::sort(ret.begin(), ret.end(), &DFMRootFileInfo::typeCompareByUrl);

    if (!ret.isEmpty()) {
        // fix 25778 每次打开文管，"我的目录" 顺序随机排列
        static const QList<QString> udir = {"desktop", "videos", "music", "pictures", "documents", "downloads"};
        for (int i = 0; i < udir.count(); i++) {
            for (int j = 0; j < ret.count(); j++) {
                if (ret[j]->fileUrl().path().contains(udir[i]) && ret[j]->suffix() == SUFFIX_USRDIR && i != j) {
                    ret.move(j, i);
                    break;
                }
            }
        }
    }

    return ret;
}

DRootFileManager *DRootFileManager::instance()
{
    static DRootFileManager services;

    return &services;
}

bool DRootFileManager::isRootFileInited() const
{
    return d_ptr->m_bRootFileInited.load();
}

void DRootFileManager::changeRootFile(const DUrl &fileurl, const bool bcreate)
{
    QMutexLocker lk(&d_ptr->rootfileMtx);
    if (bcreate) {
        if (!d_ptr->rootfilelist.contains(fileurl)) {
            DAbstractFileInfoPointer info = DFileService::instance()->createFileInfo(nullptr, fileurl);
            if (info && info->exists()) {
                d_ptr->rootfilelist.insert(fileurl, info);
                qInfo() << "  insert   " << fileurl;
            }
        }
    } else {
        qDebug() << "  remove   " << d_ptr->rootfilelist;
        bool isRemoded = false;
        if (d_ptr->rootfilelist.contains(fileurl)) {
            isRemoded = d_ptr->rootfilelist.remove(fileurl)>=1;
        }
        if(!isRemoded){//当配置成smb常驻时，d_ptr->rootfilelist中保存的是：dfmroot:///smb://host/share_folder.remote
            QString path = QUrl::fromPercentEncoding(fileurl.path().toUtf8());
            if (path.contains(QString(".%1").arg(SUFFIX_GVFSMP)) && path.contains("gvfs/smb-share:server=") && path.contains(",share=")) {
                QString host = path.section("server=",-1).section(",",0,0);
                QString shardFolder = path.section("share=",-1).section(QString(".%1").arg(SUFFIX_GVFSMP),0,0);
                DUrl tem(QString("dfmroot:///smb://%1/%2.remote").arg(host).arg(shardFolder));//tem like: dfmroot:///smb://host/share_folder.remote
                if (d_ptr->rootfilelist.contains(tem)) {
                    d_ptr->rootfilelist.remove(tem);
                }
            }
        }
    }
}

void DRootFileManager::startQuryRootFile()
{
    if (!d_ptr->bstartonce) {
        d_ptr->bstartonce = true;

        //NOTE [XIAO] 加载插件
        SchemePluginManager::instance()->loadSchemePlugin();

        DAbstractFileWatcher *devicesWatcher = DFileService::instance()->createFileWatcher(nullptr, DUrl(DFMROOT_ROOT), this);
        Q_CHECK_PTR(devicesWatcher);
        if (d_ptr->m_rootFileWatcher) {
            delete d_ptr->m_rootFileWatcher;
            d_ptr->m_rootFileWatcher = nullptr;
        }
        d_ptr->m_rootFileWatcher = devicesWatcher;
        if (qApp->thread() != devicesWatcher->thread()) {
            devicesWatcher->moveToThread(qApp->thread());
            devicesWatcher->setParent(this);
        }

        QTimer::singleShot(1000, devicesWatcher, [devicesWatcher]() {
            devicesWatcher->startWatcher();
        });

        connect(devicesWatcher, &DAbstractFileWatcher::subfileCreated, this, [this](const DUrl & url) {
            changeRootFile(url);
        });

        connect(devicesWatcher, &DAbstractFileWatcher::fileDeleted, this, [this](const DUrl & url) {
            changeRootFile(url, false);
        });
    }

    bool openAsAdmin = DFMGlobal::isOpenAsAdmin();
    QMutexLocker lk(&d_ptr->rootfileMtx);

    if (openAsAdmin && d_ptr->m_jobcontroller && d_ptr->m_jobcontroller->isRunning()) {
        qInfo() << "startQuryRootFile thread is running" << d_ptr->m_jobcontroller->currentThread();
        return;
    }

    if (!openAsAdmin && d_ptr->m_jobcontroller) {
        qInfo() << "startQuryRootFile thread is running" << d_ptr->m_jobcontroller->currentThread();
        return;
    }

    //启用异步线程去读取
    d_ptr->m_jobcontroller = fileService->getChildrenJob(this, DUrl(DFMROOT_ROOT), QStringList(), QDir::AllEntries);

    lk.unlock();

    connect(d_ptr->m_jobcontroller, &JobController::addChildren, this, [this](const DAbstractFileInfoPointer & chi) {
        if (!chi)
            return;
        QMutexLocker locker(&d_ptr->rootfileMtx);
        if (!d_ptr->rootfilelist.contains(chi->fileUrl()) && chi->exists()) {
            d_ptr->rootfilelist.insert(chi->fileUrl(), chi);
            d_ptr->m_rootChanged = true;
            locker.unlock();
            emit rootFileChange(chi); // 其实中间结果没有必要,直接拿最终结果就行了,但保留接口，以后便于扩展
        }
    }, Qt::DirectConnection);

    connect(d_ptr->m_jobcontroller, &JobController::addChildrenList, this, [this](QList<DAbstractFileInfoPointer> ch) {
        for (auto chi : ch) {
            QMutexLocker locker(&d_ptr->rootfileMtx);
            if (!d_ptr->rootfilelist.contains(chi->fileUrl()) && chi->exists()) {
                d_ptr->rootfilelist.insert(chi->fileUrl(), chi);
                d_ptr->m_rootChanged = true;
                locker.unlock();
                emit rootFileChange(chi); // 其实中间结果没有必要,直接拿最终结果就行了,但保留接口，以后便于扩展
            }
        }
    }, Qt::DirectConnection);
    connect(d_ptr->m_jobcontroller, &JobController::finished, this, [this]() {
        QMutexLocker locker(&d_ptr->rootfileMtx);
        d_ptr->m_jobcontroller->deleteLater();
        qInfo() << "get root file info thread jobcontroller finished " << QThread::currentThreadId() << d_ptr->rootfilelist.size();
        d_ptr->m_jobcontroller = nullptr;
        d_ptr->m_bRootFileInited.store(true);
        locker.unlock();

        d_ptr->m_bRootFileInited.store(true);
        if (d_ptr->m_rootChanged)
            emit queryRootFileFinsh();

        d_ptr->m_rootChanged = false;

        emit serviceHideSystemPartition(); // 刷新计算机页面，去掉无用块设备
    }, openAsAdmin ? Qt::DirectConnection : Qt::AutoConnection);
    d_ptr->m_jobcontroller->start();
}


DAbstractFileWatcher *DRootFileManager::rootFileWather() const
{
    return d_ptr->m_rootFileWatcher;
}

void DRootFileManager::clearThread()
{
    if (d_ptr->m_jobcontroller && !d_ptr->m_jobcontroller->isFinished()) {
        d_ptr->m_jobcontroller->wait();
    }
}

void DRootFileManager::changRootFile(const QList<DAbstractFileInfoPointer> &rootinfo)
{
    QMutexLocker lk(&d_ptr->rootfileMtx);
    for (const DAbstractFileInfoPointer &fi : rootinfo) {
        DUrl durl = fi->fileUrl();
        if (!d_ptr->rootfilelist.contains(durl) && fi->exists()) {
            d_ptr->rootfilelist.insert(durl, fi);
        }
    }
}

bool DRootFileManager::isRootFileContain(const DUrl &url)
{
    QMutexLocker lk(&d_ptr->rootfileMtx);
    return d_ptr->rootfilelist.contains(url);
}

const DAbstractFileInfoPointer DRootFileManager::getFileInfo(const DUrl &fileUrl)
{
    QMutexLocker lk(&DRootFileManagerPrivate::rootfileMtx);
    return DRootFileManagerPrivate::rootfilelist.value(fileUrl);
}

bool DRootFileManager::isRootFileContainSmb(const DUrl &smburl)
{
    DUrl temUrl = smburl;
    if(smburl.scheme() == "smb")
        temUrl = DUrl(QString("%1%2").arg(DFMROOT_ROOT).arg(smburl.toString()));
    const QString &smburlPath = temUrl.path();//smburl.path();
    if(!FileUtils::isSmbPath(smburlPath))
        return false;

    QMutexLocker lock(&d_ptr->rootfileMtx);
    for (auto info : d_ptr->rootfilelist) {
        QString strUrl = info->extraProperties().value("rooturi").toString();
        if(strUrl.isEmpty())
            continue;
        if(strUrl.endsWith("/"))
            strUrl.chop(1);
        QString strUrlDecode = QByteArray::fromPercentEncoding(strUrl.toLocal8Bit().data());
        if(smburl.toString().startsWith(strUrlDecode))
            return true;
    }
    return false;
}

// sometimes we can use this function to relaod computer model.
// and i think this function called reloadComputerModel could be better.
void DRootFileManager::hideSystemPartition()
{
    QList<DAbstractFileInfoPointer> fileist = DFileService::instance()->\
            getChildren(this, DUrl(DFMROOT_ROOT), QStringList(), QDir::AllEntries, QDirIterator::NoIteratorFlags, false);
    d_ptr->rootfileMtx.lock();
    d_ptr->rootfilelist.clear();
    d_ptr->rootfileMtx.unlock();
    changRootFile(fileist);

    emit serviceHideSystemPartition();
}

void DRootFileManager::policyHideSystemPartition(const QString &key)
{
    if (DTK_POLICY_SUPPORT) {
        if (key != DISK_HIDDEN)
            return;
        QStringList policyList = GroupPolicy::instance()->getValue(DISK_HIDDEN).toStringList();
        bool isHidden = DFMApplication::genericAttribute(DFMApplication::GA_HiddenSystemPartition).toBool();

        if (0 == policyList.count() && !isHidden)
            return;

        QStringList systemDisks = DFMRootController::systemDiskList();
        bool allHidden = true;
        for (auto dk : systemDisks){
            if (!policyList.contains(dk)) {
                allHidden = false;
                break;
            }
        }

        if (allHidden && isHidden)
            return;

        if (allHidden && !isHidden)
            DFMApplication::instance()->setGenericAttribute(DFMApplication::GA_HiddenSystemPartition, true);

        if (!allHidden && isHidden)
            DFMApplication::instance()->setGenericAttribute(DFMApplication::GA_HiddenSystemPartition, false);


        QList<DAbstractFileInfoPointer> fileist = DFileService::instance()->\
                getChildren(this, DUrl(DFMROOT_ROOT), QStringList(), QDir::AllEntries, QDirIterator::NoIteratorFlags, false);
        d_ptr->rootfileMtx.lock();
        d_ptr->rootfilelist.clear();
        d_ptr->rootfileMtx.unlock();
        changRootFile(fileist);

        emit serviceHideSystemPartition();
    }
}

void DRootFileManager::settingHideSystemPartition(bool isHide)
{
    Q_UNUSED(isHide)

    if (DTK_POLICY_SUPPORT) {
        bool isSetPolicy = GroupPolicy::instance()->containKey(DISK_HIDDEN);
        if (isSetPolicy) {
            bool isHidden = DFMApplication::genericAttribute(DFMApplication::GA_HiddenSystemPartition).toBool();
            auto policyList = GroupPolicy::instance()->getValue(DISK_HIDDEN).toList();

            if (isHidden) {
                QStringList systemDisks = DFMRootController::systemDiskList();
                bool allHidden = true;
                for (auto dk : systemDisks){
                    if (!policyList.contains(dk)) {
                        allHidden = false;
                        policyList << dk;
                    }
                }

                if (!allHidden)
                    GroupPolicy::instance()->setValue(DISK_HIDDEN, policyList);
            } else {
                if (!policyList.isEmpty())
                    GroupPolicy::instance()->setValue(DISK_HIDDEN, QVariantList());
            }
        }
    }
}
