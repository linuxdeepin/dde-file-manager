#include <gtest/gtest.h>
#include <QDateTime>
#include <QSharedPointer>
#include <QProcess>
#include <QDateTime>
#include <QtConcurrent>
#include <QFile>

#include "disomaster.h"
#include "testhelper.h"
#include "stubext.h"
#include "shutil/fileutils.h"
#include "deviceinfo/udisklistener.h"
#include "ddiskmanager.h"
#include "dblockdevice.h"
#include "controllers/vaultcontroller.h"
#include "dfmevent.h"
#include "interfaces/dfmstandardpaths.h"
#include "ddiskdevice.h"
#include "dabstractfilewatcher.h"
#include "ddiskmanager.h"
#include "stubext.h"
#include "tag/tagmanager.h"
#define private public
#include "fileoperations/filejob.h"
#include "dialogs/dialogmanager.h"
#include "dialogs/dtaskdialog.h"
#include "fileoperations/sort.h"

using namespace testing;
using namespace stub_ext;
DFM_USE_NAMESPACE

class FileJobTest:public testing::Test{

public:
    QSharedPointer<FileJob> job;
    StubExt stl;
    virtual void SetUp() override{
        job.reset(new FileJob(FileJob::Copy));
        source.setScheme(FILE_SCHEME);
        dst.setScheme(FILE_SCHEME);
        void (*delayRemoveTasklamda)(const QMap<QString, QString> &) = []
                (const QMap<QString, QString> &){};
        stl.set(ADDR(DTaskDialog,delayRemoveTask),delayRemoveTasklamda);
        stl.set(ADDR(DTaskDialog,addTask),delayRemoveTasklamda);
        typedef int(*fptr)(QDialog*);
        fptr pQDialogExec = (fptr)(&QDialog::exec);
        int (*stub_DDialog_exec)(void) = [](void)->int{return QDialog::Rejected;};
        stl.set(pQDialogExec, stub_DDialog_exec);
        void (*showErrorDialoglamda)(const QString &, const QString &) = []
                (const QString &, const QString &){};
        stl.set(ADDR(DialogManager,showErrorDialog),showErrorDialoglamda);
        std::cout << "start FileJobTest" << std::endl;
    }

    virtual void TearDown() override{
        std::cout << "end FileJobTest" << std::endl;
    }
    DUrl source,dst;
};

TEST_F(FileJobTest,start_setDirPermissions) {
    source.setPath("./");
    dst.setPath("./dst_dir");
    EXPECT_FALSE(FileJob::setDirPermissions(source.toLocalFile(),dst.toLocalFile()));
    dst.setPath(TestHelper::createTmpDir());
    EXPECT_TRUE(FileJob::setDirPermissions(source.toLocalFile(),dst.toLocalFile()));
    TestHelper::deleteTmpFiles(QStringList() << dst.toLocalFile());
}

TEST_F(FileJobTest,get_jobType) {
    EXPECT_EQ(FileJob::Copy, job->jobType());
}

TEST_F(FileJobTest,start_setStatus) {
    EXPECT_NO_FATAL_FAILURE(job->setStatus(FileJob::Started));
}

TEST_F(FileJobTest,can_JobIdOperation) {
    QString id = QString::number(QDateTime::currentDateTime().toSecsSinceEpoch());
    EXPECT_NO_FATAL_FAILURE(job->setJobId(id));
    EXPECT_EQ(id,job->getJobId());
}

TEST_F(FileJobTest,start_checkDuplicateName) {
    QProcess::execute("mkdir " + QDir::currentPath()+"/start_checkDuplicateName");
    source.setPath(QDir::currentPath()+"/start_checkDuplicateName");
    DUrl urldir,urlfile,urlfilecopy;
    urldir = source;
    urlfile = source;
    urlfilecopy = source;
    urldir.setPath(source.toLocalFile()+"(copy)");
    QProcess::execute("mkdir "+urldir.toLocalFile());
    EXPECT_FALSE(job->checkDuplicateName(source.toLocalFile()).isEmpty());
    urlfile.setPath(QDir::currentPath()+"/start_checkDuplicateName");
    urlfilecopy.setPath(urlfile.toLocalFile()+"(copy)");
    QProcess::execute("touch "+urlfilecopy.toLocalFile());
    EXPECT_FALSE(job->checkDuplicateName(urlfile.toLocalFile()).isEmpty());
    DUrl urlfiletxt,urlfiletxtcopy;
    urlfiletxt = source;
    urlfiletxt.setPath(QDir::currentPath()+"/start_checkDuplicateName/zut_filejob_check_oo.txt");
    urlfiletxtcopy = source;
    urlfiletxtcopy.setPath(QDir::currentPath()+"/start_checkDuplicateName/zut_filejob_check_oo(copy).txt");
    QProcess::execute(QString("touch "+urlfiletxt.toLocalFile() + QString(" ") + urlfiletxtcopy.toLocalFile()));
    EXPECT_FALSE(job->checkDuplicateName(urlfiletxt.toLocalFile()).isEmpty());
    QProcess::execute("rm -rf " + QDir::currentPath()+"/start_checkDuplicateName");
}

TEST_F(FileJobTest,start_ApplyToAllOp) {
    EXPECT_NO_FATAL_FAILURE(job->setApplyToAll(true));
    EXPECT_TRUE(job->getIsApplyToAll());
}

TEST_F(FileJobTest,start_ReplaceOp) {
    EXPECT_NO_FATAL_FAILURE(job->setReplace(true));
    EXPECT_TRUE(job->getIsReplaced());
    EXPECT_NO_FATAL_FAILURE(job->resetCustomChoice());
    EXPECT_TRUE(job->getTargetDir().isEmpty());
}

TEST_F(FileJobTest,start_jobDetail) {
    EXPECT_FALSE(job->jobDetail().isEmpty());
}

TEST_F(FileJobTest,start_currentMsec) {
    EXPECT_FALSE(job->currentMsec() == 0);
}

TEST_F(FileJobTest,start_lastMsec) {
    EXPECT_FALSE(job->lastMsec() != 0);
}

TEST_F(FileJobTest,start_isJobAdded) {
    EXPECT_FALSE(job->isJobAdded());
}

TEST_F(FileJobTest,start_adjustSymlinkPath) {
    source.setPath("./");
    QString sourcepath = source.toLocalFile();
    EXPECT_NO_FATAL_FAILURE(job->adjustSymlinkPath(sourcepath,sourcepath));
}

TEST_F(FileJobTest,start_AbortedOp) {
    EXPECT_NO_FATAL_FAILURE(job->setIsAborted(true));
    EXPECT_TRUE(job->isAborted());
}

TEST_F(FileJobTest,start_IsSkipOp) {
    EXPECT_NO_FATAL_FAILURE(job->setIsSkip(true));
    EXPECT_TRUE(job->getIsSkip());
}

TEST_F(FileJobTest,start_IsCoExistedOp) {
    EXPECT_NO_FATAL_FAILURE(job->setIsCoExisted(true));
    EXPECT_TRUE(job->getIsCoExisted());
}

TEST_F(FileJobTest,start_ManualRemoveJobOp) {
    EXPECT_NO_FATAL_FAILURE(job->setManualRemoveJob(true));
    EXPECT_TRUE(job->getIsManualRemoveJob());
}

TEST_F(FileJobTest,start_WindowIdOp) {
    int winId = static_cast<int>(QDateTime::currentDateTime().toSecsSinceEpoch());
    EXPECT_NO_FATAL_FAILURE(job->setWindowId(winId));
    EXPECT_TRUE(winId == job->getWindowId());
}

TEST_F(FileJobTest,can_getIsGvfsFileOperationUsed) {
    EXPECT_FALSE(job->getIsGvfsFileOperationUsed());
}

TEST_F(FileJobTest,can_getIsFinished) {
    EXPECT_FALSE(job->getIsFinished());
}

TEST_F(FileJobTest,start_RestoreProgressOp) {
    EXPECT_NO_FATAL_FAILURE(job->setRestoreProgress(0.50));
    EXPECT_TRUE(0.50 == job->getRestoreProgress());
}

TEST_F(FileJobTest,can_getIsOpticalJob) {
    EXPECT_FALSE(job->getIsOpticalJob());
}

TEST_F(FileJobTest,can_isCanShowProgress) {
    EXPECT_TRUE(job->isCanShowProgress());
}

TEST_F(FileJobTest,start_doCopy) {
    source.setPath("~/Pictures/Wallpapers");
    dst.setPath("./");
    void (*showNoPermissionDialoglamda)(const DFMUrlListBaseEvent &) = [](const DFMUrlListBaseEvent &){};
    stl.set(ADDR(DialogManager,showNoPermissionDialog),showNoPermissionDialoglamda);

    DUrlList (*doMoveCopyJoblamda)(const DUrlList &, const DUrl &) = []
            (const DUrlList &, const DUrl &){return DUrlList();};
    stl.set(ADDR(FileJob,doMoveCopyJob),doMoveCopyJoblamda);
    void (*clearlamda)(void*) = [](void *){};
    stl.set(ADDR(QList<DUrl>,clear),clearlamda);
    job->m_noPermissonUrls.append(DUrl());

    EXPECT_TRUE(job->doCopy(DUrlList() << source, dst).isEmpty());
}

TEST_F(FileJobTest,start_doMove) {
    QProcess::execute("mkdir " + QDir::currentPath()+"/start_doMove");
    QProcess::execute("touch " + QDir::currentPath()+"/start_doMove/11.txt");
    source.setPath("/bin");
    DUrl url(source);
    url.setPath(QDir::currentPath()+"/start_doMove/11.txt");
    dst.setPath(QDir::currentPath()+"/start_doMove/");
    void (*showNoPermissionDialoglamda)(const DFMUrlListBaseEvent &) = [](const DFMUrlListBaseEvent &){};
    stl.set(ADDR(DialogManager,showNoPermissionDialog),showNoPermissionDialoglamda);
    DUrlList (*doMoveCopyJoblamda)(const DUrlList &, const DUrl &) = []
            (const DUrlList &, const DUrl &){return DUrlList();};
    stl.set(ADDR(FileJob,doMoveCopyJob),doMoveCopyJoblamda);
    EXPECT_TRUE(job->doMove(DUrlList() << source << url, dst).isEmpty());
    QProcess::execute("rm -rf " + QDir::currentPath()+"/start_doMove");
}
QString rootPathlamda(){
    return QString("www");
}
static int getMountPointUrlnew = 0;
DUrl getMountPointUrllamda(){
    if (getMountPointUrlnew){
        return DUrl::fromLocalFile("~/");
    }
    getMountPointUrlnew++;
    return DUrl();
}
UDiskDeviceInfoPointer getDeviceByFilePathlamda(const QString &, const bool)
{
    return UDiskDeviceInfoPointer(new UDiskDeviceInfo());
};
TEST_F(FileJobTest,start_doMoveCopyJob) {
    QProcess::execute("mkdir " + QDir::currentPath()+"/start_doMoveCopyJob");
    QProcess::execute("touch " + QDir::currentPath()+"/start_doMoveCopyJob/11.txt");
    source.setPath(QDir::currentPath()+"/start_doMoveCopyJob/11.txt");
    dst.setPath(QDir::currentPath()+"/start_doMoveCopyJob");
    DUrl tmpdst(dst),linkurl(source),copydir(dst);
    tmpdst.setPath(QDir::currentPath()+"/start_doMoveCopyJob/unexits_filejob");
    bool (*checkDiskSpaceAvailablelamda)(const DUrlList &, const DUrl &) = []
            (const DUrlList &, const DUrl &){
        return false;
    };
    linkurl.setPath(linkurl.toLocalFile()+"sys_link");
    copydir.setPath(TestHelper::createTmpDir());
    QProcess::execute("ln -s "+ source.toLocalFile() + " " + linkurl.toLocalFile());
    void (*showDiskSpaceOutOfUsedDialogLaterlamda)(void *) = [](void *){};
    void (*removeTaskImmediatelylamda)(const QMap<QString, QString> &) = [](const QMap<QString, QString> &){};
    {
        Stub stl;
        stl.set(ADDR(FileJob,checkDiskSpaceAvailable),checkDiskSpaceAvailablelamda);
        stl.set(ADDR(DialogManager,showDiskSpaceOutOfUsedDialogLater),showDiskSpaceOutOfUsedDialogLaterlamda);
        stl.set(ADDR(DTaskDialog,removeTaskImmediately),removeTaskImmediatelylamda);
        EXPECT_TRUE(job->doMoveCopyJob(DUrlList(), dst).isEmpty());
    }
    {
        Stub stl;
        stl.set(ADDR(DialogManager,showDiskSpaceOutOfUsedDialogLater),showDiskSpaceOutOfUsedDialogLaterlamda);
        stl.set(ADDR(DTaskDialog,removeTaskImmediately),removeTaskImmediatelylamda);
        EXPECT_TRUE(job->doMoveCopyJob(DUrlList() << source, tmpdst).isEmpty());
    }

    bool (*isGvfsMountFilelamda)(const QString &, const bool &) = []
            (const QString &, const bool &){return true;};
    {
        Stub stl;
        stl.set(ADDR(DialogManager,showDiskSpaceOutOfUsedDialogLater),showDiskSpaceOutOfUsedDialogLaterlamda);
        stl.set(ADDR(DTaskDialog,removeTaskImmediately),removeTaskImmediatelylamda);
        stl.set(ADDR(FileUtils,isGvfsMountFile),isGvfsMountFilelamda);
        stl.set(ADDR(UDiskListener,getDeviceByFilePath),getDeviceByFilePathlamda);
        stl.set(ADDR(UDiskDeviceInfo,getMountPointUrl),getMountPointUrllamda);
        stl.set(ADDR(QStorageInfo,rootPath),&rootPathlamda);
        EXPECT_TRUE(job->doMoveCopyJob(DUrlList() << source, tmpdst).isEmpty());
    }
    stl.reset(ADDR(QStorageInfo,rootPath));
    stl.reset(ADDR(FileUtils,isGvfsMountFile));
    {
        Stub stl;
        stl.set(ADDR(DialogManager,showDiskSpaceOutOfUsedDialogLater),showDiskSpaceOutOfUsedDialogLaterlamda);
        stl.set(ADDR(DTaskDialog,removeTaskImmediately),removeTaskImmediatelylamda);
        stl.set(ADDR(UDiskListener,getDeviceByFilePath),getDeviceByFilePathlamda);
        stl.set(ADDR(UDiskDeviceInfo,getMountPointUrl),getMountPointUrllamda);
        job->setIsAborted(true);
        EXPECT_TRUE(job->doMoveCopyJob(DUrlList() << DUrl() << source << linkurl, dst).isEmpty());

        job->setIsAborted(false);
        EXPECT_FALSE(job->doMoveCopyJob(DUrlList() << DUrl() << source << linkurl << copydir, dst).isEmpty());
        job.reset(new FileJob(FileJob::Move));
        EXPECT_TRUE(job->doMoveCopyJob(DUrlList(), tmpdst).isEmpty());
    }
    bool (*moveDirlamda)(const QString &, const QString &, QString *) = []
            (const QString &, const QString &, QString *){return false;};
    bool (*copyFilelamda)(const QString &, const QString &, QString *) = []
            (const QString &, const QString &, QString *){return true;};
    bool (*deleteFilelamda)(const QString &) = [](const QString &){return true;};
    {
        Stub stl;
        stl.set(ADDR(DialogManager,showDiskSpaceOutOfUsedDialogLater),showDiskSpaceOutOfUsedDialogLaterlamda);
        stl.set(ADDR(DTaskDialog,removeTaskImmediately),removeTaskImmediatelylamda);
        stl.set(ADDR(UDiskListener,getDeviceByFilePath),getDeviceByFilePathlamda);
        stl.set(ADDR(UDiskDeviceInfo,getMountPointUrl),getMountPointUrllamda);
        stl.set(ADDR(FileJob,moveDir),moveDirlamda);
        stl.set(ADDR(FileJob,moveFile),moveDirlamda);

        stl.set(ADDR(FileJob,copyFile),copyFilelamda);

        stl.set(ADDR(FileJob,deleteFile),deleteFilelamda);
        stl.set(ADDR(FileJob,deleteDir),deleteFilelamda);
        stl.set(ADDR(FileJob,copyDir),copyFilelamda);
        EXPECT_FALSE(job->doMoveCopyJob(DUrlList() << source << linkurl << copydir, dst).isEmpty());
        job->m_isInSameDisk = false;
        job->setIsSkip(false);
        job->jobAdded();
        EXPECT_FALSE(job->doMoveCopyJob(DUrlList() << DUrl() << source << linkurl << copydir, dst).isEmpty());

        job.reset(new FileJob(FileJob::Trash));
        bool (*isVaultFilelamda)(QString ) = [](QString ){return true;};
        stl.set(ADDR(VaultController,isVaultFile),isVaultFilelamda);
        EXPECT_TRUE(job->doMoveCopyJob(DUrlList() << DUrl() << source << copydir, dst).isEmpty());
    }
    bool (*renamefilelamda)(const QString &) = [](const QString &){return false;};
    bool (*renamelamda)(const QString &, const QString &) = [](const QString &, const QString &){return false;};
    bool (*moveDirToTrashlamda)(const QString &, QString *) = [](const QString &, QString *){return true;};
    DUrl dstfileurl(source),dstdirurl(copydir);
    {
        Stub stl;
        stl.set(ADDR(DialogManager,showDiskSpaceOutOfUsedDialogLater),showDiskSpaceOutOfUsedDialogLaterlamda);
        stl.set(ADDR(DTaskDialog,removeTaskImmediately),removeTaskImmediatelylamda);
        stl.set(ADDR(UDiskListener,getDeviceByFilePath),getDeviceByFilePathlamda);
        stl.set(ADDR(UDiskDeviceInfo,getMountPointUrl),getMountPointUrllamda);
        stl.set(ADDR(FileJob,moveDir),moveDirlamda);
        stl.set(ADDR(FileJob,moveFile),moveDirlamda);

        stl.set(ADDR(FileJob,copyFile),copyFilelamda);

        stl.set(ADDR(FileJob,deleteFile),deleteFilelamda);
        stl.set(ADDR(FileJob,deleteDir),deleteFilelamda);
        stl.set(ADDR(FileJob,copyDir),copyFilelamda);


        dstfileurl.setPath(dst.toLocalFile() + "/ut_tst_file");
        dstdirurl.setPath(dst.toLocalFile() + "/ut_tst_dir");
        QProcess::execute("touch "+dstfileurl.toLocalFile());
        QProcess::execute("mkdir "+dstdirurl.toLocalFile());
        EXPECT_TRUE(job->doMoveCopyJob(DUrlList() << dstfileurl << dstdirurl, dst).isEmpty());


        stl.set(ADDR(FileJob,moveDirToTrash),moveDirToTrashlamda);
        stl.set(ADDR(FileJob,moveFileToTrash),moveDirToTrashlamda);
        job->m_isInSameDisk = true;

        stl.set(ADDR(QDir,rename),renamelamda);

        stl.set((bool (QFile::*)(const QString &))ADDR(QFile,rename),renamefilelamda);
        int (*executelamda)(const QString &) = [](const QString &){return 1;};

        EXPECT_FALSE(job->doMoveCopyJob(DUrlList() << source << copydir, dst).isEmpty());
    }
    {
        Stub stl;
        stl.set(ADDR(DialogManager,showDiskSpaceOutOfUsedDialogLater),showDiskSpaceOutOfUsedDialogLaterlamda);
        stl.set(ADDR(DTaskDialog,removeTaskImmediately),removeTaskImmediatelylamda);
        stl.set(ADDR(UDiskListener,getDeviceByFilePath),getDeviceByFilePathlamda);
        stl.set(ADDR(UDiskDeviceInfo,getMountPointUrl),getMountPointUrllamda);
        stl.set(ADDR(FileJob,moveDir),moveDirlamda);
        stl.set(ADDR(FileJob,moveFile),moveDirlamda);

        stl.set(ADDR(FileJob,copyFile),copyFilelamda);

        stl.set(ADDR(FileJob,deleteFile),deleteFilelamda);
        stl.set(ADDR(FileJob,deleteDir),deleteFilelamda);
        stl.set(ADDR(FileJob,copyDir),copyFilelamda);

        stl.set(ADDR(FileJob,moveDirToTrash),moveDirToTrashlamda);
        stl.set(ADDR(FileJob,moveFileToTrash),moveDirToTrashlamda);

        stl.set(ADDR(QDir,rename),renamelamda);
        EXPECT_FALSE(job->doMoveCopyJob(DUrlList() << DUrl() << source << copydir, dst).isEmpty());
    }

    QProcess::execute("rm -rf " + QDir::currentPath()+"/start_doMoveCopyJob");
}

TEST_F(FileJobTest,start_doDelete) {
    QProcess::execute("mkdir " + QDir::currentPath()+"/start_doDelete");
    QProcess::execute("touch " + QDir::currentPath()+"/start_doDelete/11.txt");
    source.setPath(QDir::currentPath()+"/start_doDelete/11.txt");
    dst.setPath(QDir::currentPath()+"/start_doDelete");
    job->jobAdded();
    void (*clearlamda)(void*) = [](void *){};

    job->m_noPermissonUrls.append(DUrl());
    void (*showNoPermissionDialoglamda)(const DFMUrlListBaseEvent &) = []
            (const DFMUrlListBaseEvent &){};
    stl.set(ADDR(DialogManager,showNoPermissionDialog),showNoPermissionDialoglamda);
    {
        Stub stl;
        stl.set(ADDR(QList<DUrl>,clear),clearlamda);
        EXPECT_NO_FATAL_FAILURE(job->doDelete(DUrlList()));
    }

    bool (*deleteDirlamda)(const QString &) = [](const QString &){return false;};
    stl.set(ADDR(FileJob,deleteDir),deleteDirlamda);
    EXPECT_NO_FATAL_FAILURE(job->doDelete(DUrlList() << DUrl() << source << dst));

    QProcess::execute("rm -rf " + QDir::currentPath()+"/start_doDelete");
}

TEST_F(FileJobTest,start_doMoveToTrash) {

    bool (*mkpathlamda)(void *,const QString &) = [](void *,const QString &){
        return false;
    };
    stl.set(ADDR(QDir,mkpath),mkpathlamda);

    EXPECT_TRUE(job->doMoveToTrash(DUrlList()).isEmpty());
    bool (*mkpath1lamda)(void *,const QString &) = [](void *,const QString &str){
        if (str == DFMStandardPaths::location(DFMStandardPaths::TrashInfosPath)) {
            return false;
        }
        return true;
    };
    {
        Stub stl;
        stl.set(ADDR(QDir,mkpath),mkpath1lamda);
        EXPECT_TRUE(job->doMoveToTrash(DUrlList()).isEmpty());
    }
    bool (*checkTrashFileOutOf1GBlamda)(const DUrl &) = [](const DUrl &){return false;};

    void (*showMoveToTrashConflictDialoglamda)(const DUrlList &) = [](const DUrlList &){};
    stl.set(ADDR(DialogManager,showMoveToTrashConflictDialog),showMoveToTrashConflictDialoglamda);
    job->jobAdded();
    {
        Stub stl;
        stl.set(ADDR(FileJob,checkTrashFileOutOf1GB),checkTrashFileOutOf1GBlamda);
        EXPECT_TRUE(job->doMoveToTrash(DUrlList() << DUrl()).isEmpty());
    }
    DUrlList (*doMovelamda)(const DUrlList &, const DUrl &) = []
            (const DUrlList &, const DUrl &){return DUrlList();};
    stl.set(ADDR(FileJob,doMove),doMovelamda);
    EXPECT_TRUE(job->doMoveToTrash(DUrlList() << DUrl()).isEmpty());

}
TEST_F(FileJobTest, start_doTrashRestore){
    QProcess::execute("mkdir " + QDir::currentPath()+"/start_doTrashRestore");
    QProcess::execute("touch " + QDir::currentPath()+"/start_doTrashRestore/11.txt");
    source.setPath(QDir::currentPath()+"/start_doTrashRestore/11.txt");
    bool (*restoreTrashFilelamda)(const QString &, const QString &) = []
            (const QString &, const QString &){return true;};
    stl.set(ADDR(FileJob,restoreTrashFile),restoreTrashFilelamda);
    DUrl linkurl(source),tmpdir(source);
    linkurl.setPath(linkurl.toLocalFile()+"sys_link");
    QProcess::execute("ln -s "+ source.toLocalFile() + " " + linkurl.toLocalFile());
    tmpdir.setPath(TestHelper::createTmpDir());
    DUrlList (*doMovelamda)(const DUrlList &, const DUrl &) = []
            (const DUrlList &, const DUrl &){return DUrlList() << DUrl();};
    stl.set(ADDR(FileJob,doMove),doMovelamda);
    job->m_isJobAdded = true;
    job->m_isManualRemoveJob = false;
    job->m_isInSameDisk = true;
    EXPECT_TRUE(job->doTrashRestore(linkurl.toLocalFile(),dst.toLocalFile()));

    bool (*copyDirlamda)(const QString &, const QString &, QString *) = []
            (const QString &, const QString &, QString *){return true;};
    stl.set(ADDR(FileJob,copyDir),copyDirlamda);
    EXPECT_TRUE(job->doTrashRestore(tmpdir.toLocalFile(),dst.toLocalFile()));

    stl.set(ADDR(FileJob,copyFile),copyDirlamda);
    EXPECT_TRUE(job->doTrashRestore(source.toLocalFile(),dst.toLocalFile()));

    stl.set(ADDR(FileJob,restoreTrashFile),restoreTrashFilelamda);
    EXPECT_FALSE(job->doTrashRestore(source.toLocalFile(),tmpdir.toLocalFile()));
    TestHelper::deleteTmpFiles(QStringList() << source.toLocalFile() << linkurl.toLocalFile()
                               << tmpdir.toLocalFile());
}

static int mediaChangeDetected1new = 0;
bool mediaChangeDetected1lamda(void *)
{
    if (mediaChangeDetected1new == 0){
        mediaChangeDetected1new++;
        return true;
    }
    return false;
}
TEST_F(FileJobTest, start_doOpticalBlank)
{
    DUrl url = DUrl::fromLocalFile("/dev/sr*");
    void (*unmountlamda)(const QVariantMap &) = [](const QVariantMap &){};
    stl.set(ADDR(DBlockDevice,unmount),unmountlamda);
    bool (*opticalBlanklamda)(void *) = [](void *){return true;};
    stl.set(ADDR(DDiskDevice,opticalBlank),opticalBlanklamda);
    bool (*ghostSignallamda)(const DUrl &, DAbstractFileWatcher::SignalType1, const DUrl &) = []
            (const DUrl &, DAbstractFileWatcher::SignalType1, const DUrl &){return false;};
    stl.set((bool (*)(const DUrl &, DAbstractFileWatcher::SignalType1, const DUrl &))
            ADDR(DAbstractFileWatcher,ghostSignal),ghostSignallamda);
    bool (*mediaChangeDetectedlamda)(void *) = [](void *){return false;};
    stl.set(ADDR(DDiskDevice,mediaChangeDetected),mediaChangeDetectedlamda);
    job->m_isJobAdded = true;
    EXPECT_NO_FATAL_FAILURE(job->doDiscBlank(url));


    stl.set(ADDR(DDiskDevice,mediaChangeDetected),&mediaChangeDetected1lamda);
    void (*opticalJobUpdatedByParentProcesslamda)(int, int,const QString &,const QStringList &) = []
            (int , int , const QString &,const QStringList &){};
    stl.set(ADDR(FileJob,opticalJobUpdatedByParentProcess),opticalJobUpdatedByParentProcesslamda);
    EXPECT_NO_FATAL_FAILURE(job->doDiscBlank(url));
}

static int pipe2new = 0;
int pipe3lamda(int[]){
    if (0 == pipe2new) {
        pipe2new++;
        return 0;
    }
    return -1;
}

/* 这里read会卡死 暂时注释
TEST_F(FileJobTest, start_doOpticalBurnByChildProcess)
{
    DUrl url = DUrl::fromLocalFile("/dev/sr*");
    DISOMasterNS::BurnOptions opts;
    opts |= DISOMasterNS::BurnOption::VerifyDatas;
    EXPECT_NO_FATAL_FAILURE(job->doISOBurn(DUrl(), "test", 10, opts));

    QStringList (*resolveDeviceNodelamda)(QString, QVariantMap) = [](QString, QVariantMap){return QStringList();};
    {
        Stub stl;
        stl.set(ADDR(DDiskManager,resolveDeviceNode),resolveDeviceNodelamda);
        EXPECT_NO_FATAL_FAILURE(job->doISOBurn(url, "test", 10, opts));
    }
    bool (*opticalBlanklamda)(void *) = [](void *){return false;};
    stl.set(ADDR(DDiskDevice,opticalBlank),opticalBlanklamda);
    bool (*ghostSignallamda)(const DUrl &, DAbstractFileWatcher::SignalType3, const DUrl &, const int) = []
            (const DUrl &, DAbstractFileWatcher::SignalType3, const DUrl &, const int){return true;};
    stl.set((bool (*)(const DUrl &, DAbstractFileWatcher::SignalType3, const DUrl &, const int))\
            ADDR(DAbstractFileWatcher,ghostSignal),ghostSignallamda);
    int (*pipe1)(int[]) = [](int[]){return -1;};
    stl.set(pipe,pipe1);
    QStringList (*resolveDeviceNode1lamda)(QString, QVariantMap) = [](QString, QVariantMap){
        return QStringList() << QString("test");
    };
    stl.set(ADDR(DDiskManager,resolveDeviceNode),resolveDeviceNode1lamda);
    EXPECT_NO_FATAL_FAILURE(job->doISOBurn(url, "test", 10, opts));

    void (*unmountlamda)(const QVariantMap &) = [](const QVariantMap &){};
    stl.set(ADDR(DBlockDevice,unmount),unmountlamda);
    bool (*opticalBlank1lamda)(void *) = [](void *){return true;};
    stl.set(ADDR(DDiskDevice,opticalBlank),opticalBlank1lamda);
    stl.set(pipe,&pipe3lamda);
    EXPECT_NO_FATAL_FAILURE(job->doISOBurn(url, "test", 10, opts));

    stl.reset(pipe);
    TestHelper::runInLoop([=](){
        EXPECT_NO_FATAL_FAILURE(job->doISOBurn(url, "test", 10, opts));
    },5000);
}

static ssize_t fakeRead(int, void *, size_t)
{
    return -1;
}

TEST_F(FileJobTest, doISOImageBurn)
{
    DUrl url = DUrl::fromLocalFile("/dev/sr*");
    DUrl image;
    DISOMasterNS::BurnOptions opts;
    opts |= DISOMasterNS::BurnOption::VerifyDatas;
    EXPECT_NO_FATAL_FAILURE(job->doISOImageBurn(DUrl(), image, 10, opts));
    stl.set(read, fakeRead);

    QStringList (*resolveDeviceNodelamda)(QString, QVariantMap) = [](QString, QVariantMap){return QStringList();};
    {
        Stub stl;
        stl.set(ADDR(DDiskManager,resolveDeviceNode),resolveDeviceNodelamda);
        EXPECT_NO_FATAL_FAILURE(job->doISOImageBurn(url, image, 10, opts));
    }
    bool (*opticalBlanklamda)(void *) = [](void *){return false;};
    stl.set(ADDR(DDiskDevice,opticalBlank),opticalBlanklamda);
    bool (*ghostSignallamda)(const DUrl &, DAbstractFileWatcher::SignalType3, const DUrl &, const int) = []
       (const DUrl &, DAbstractFileWatcher::SignalType3, const DUrl &, const int){return true;};
    stl.set((bool (*)(const DUrl &, DAbstractFileWatcher::SignalType3, const DUrl &, const int))\
       ADDR(DAbstractFileWatcher,ghostSignal),ghostSignallamda);
    int (*pipe1lamda)(int[]) = [](int[]){return -1;};
    stl.set(pipe,pipe1lamda);
    QStringList (*resolveDeviceNode1lamda)(QString, QVariantMap) = [](QString, QVariantMap){
    return QStringList() << QString("test");
    };
    stl.set(ADDR(DDiskManager,resolveDeviceNode),resolveDeviceNode1lamda);
    EXPECT_NO_FATAL_FAILURE(job->doISOImageBurn(url, image, 10, opts));

    void (*unmountlamda)(const QVariantMap &) = [](const QVariantMap &){};
    stl.set(ADDR(DBlockDevice,unmount),unmountlamda);
    bool (*opticalBlank1lamda)(void *) = [](void *){return true;};
    stl.set(ADDR(DDiskDevice,opticalBlank),opticalBlank1lamda);
    stl.set(pipe,&pipe3lamda);
    EXPECT_NO_FATAL_FAILURE(job->doISOImageBurn(url, image, 10, opts));

    stl.reset(pipe);
    TestHelper::runInLoop([=](){
    EXPECT_NO_FATAL_FAILURE(job->doISOImageBurn(url, image, 10, opts));
    },5000);
    stl.reset(read);
}*/

TEST_F(FileJobTest, opticalJobUpdated)
{
    DISOMasterNS::DISOMaster master;
    int status = DISOMasterNS::DISOMaster::JobStatus::Failed;
    int progress = 50;
    progress = 110;
    EXPECT_NO_FATAL_FAILURE(job->opticalJobUpdated(&master, status, progress));

    progress = 50;
    EXPECT_NO_FATAL_FAILURE(job->opticalJobUpdated(&master, status, progress));
    status = DISOMasterNS::DISOMaster::JobStatus::Running;
    job->m_jobType = FileJob::OpticalImageBurn;
    job->m_opticalJobStatus = DISOMasterNS::DISOMaster::JobStatus::Finished;
    EXPECT_NO_FATAL_FAILURE(job->opticalJobUpdated(&master, status, progress));

    status = DISOMasterNS::DISOMaster::JobStatus::Stalled;
    EXPECT_NO_FATAL_FAILURE(job->opticalJobUpdated(&master, status, progress));
}

TEST_F(FileJobTest, opticalJobUpdatedByParentProcess)
{
    int status = DISOMasterNS::DISOMaster::JobStatus::Failed;
    int progress = 50;
    QString speed("8x");
    QStringList msgs;

    progress = 110;

    EXPECT_NO_FATAL_FAILURE(job->opticalJobUpdatedByParentProcess( status, progress, speed, msgs));
    progress = 50;
    EXPECT_NO_FATAL_FAILURE(job->opticalJobUpdatedByParentProcess(status, progress, speed, msgs));
    status = DISOMasterNS::DISOMaster::JobStatus::Running;
    job->m_jobType = FileJob::OpticalImageBurn;
    job->m_opticalJobStatus = DISOMasterNS::DISOMaster::JobStatus::Finished;
    EXPECT_NO_FATAL_FAILURE(job->opticalJobUpdatedByParentProcess( status, progress, speed, msgs));

    status = DISOMasterNS::DISOMaster::JobStatus::Stalled;
    EXPECT_NO_FATAL_FAILURE(job->opticalJobUpdatedByParentProcess( status, progress, speed, msgs));
}

TEST_F(FileJobTest, start_otherOp) {
    EXPECT_NO_FATAL_FAILURE(job->paused());
    EXPECT_NO_FATAL_FAILURE(job->started());
    EXPECT_NO_FATAL_FAILURE(job->cancelled());
    job->m_status = FileJob::Paused;
    EXPECT_NO_FATAL_FAILURE(job->handleJobFinished());
    job->m_status = FileJob::Paused;
    EXPECT_NO_FATAL_FAILURE(job->jobUpdated());
    void (*handleUpdateTaskWidgetlamda)(const QMap<QString, QString> &,const QMap<QString, QString> &) = []
            (const QMap<QString, QString> &,const QMap<QString, QString> &){};
    stl.set(ADDR(DTaskDialog,handleUpdateTaskWidget),handleUpdateTaskWidgetlamda);
    job->m_status = FileJob::Run;
    job->m_isCheckingDisk = true;
    job->m_jobType = FileJob::OpticalBlank;
    EXPECT_NO_FATAL_FAILURE(job->jobUpdated());
    job->m_jobType = FileJob::Restore;
    job->m_isInSameDisk = true;
    job->m_isFinished = false;
    job->m_status = FileJob::Run;
    EXPECT_NO_FATAL_FAILURE(job->jobUpdated());
    job->m_isFinished = true;
    EXPECT_NO_FATAL_FAILURE(job->jobUpdated());
    job->m_jobType = FileJob::Trash;
    EXPECT_NO_FATAL_FAILURE(job->jobUpdated());
    job->m_isFinished = false;
    job->m_finishedCount = 4;
    job->m_allCount = 6;
    EXPECT_NO_FATAL_FAILURE(job->jobUpdated());
    job->m_isFinished = false;
    job->m_isInSameDisk = false;
    job->m_lastMsec = 10000;
    EXPECT_NO_FATAL_FAILURE(job->jobUpdated());
    qint64 (*elapsedlamda)(void *) = [](void *){return qint64(0);};
    stl.set(ADDR(QElapsedTimer,elapsed),elapsedlamda);
    job->m_lastMsec = -1000;
    job->m_bytesPerSec = 0;
    EXPECT_NO_FATAL_FAILURE(job->jobUpdated());
    job->m_bytesPerSec = 100;
    job->m_totalSize = 100;
    job->m_bytesCopied = 102;
    EXPECT_NO_FATAL_FAILURE(job->jobUpdated());
    job->m_totalSize = 150;
    job->m_lastMsec = -1000;
    job->m_bytesPerSec = 1;
    EXPECT_NO_FATAL_FAILURE(job->jobUpdated());
    job->m_totalSize = 1000;
    job->m_lastMsec = -1000;
    job->m_bytesPerSec = 1;
    EXPECT_NO_FATAL_FAILURE(job->jobUpdated());
    job->m_totalSize = 5000;
    job->m_lastMsec = -1000;
    job->m_bytesPerSec = 1;
    EXPECT_NO_FATAL_FAILURE(job->jobUpdated());
    job->m_lastMsec = -1000;
    job->m_bytesPerSec = 1;
    job->m_totalSize = 100000;
    EXPECT_NO_FATAL_FAILURE(job->jobUpdated());
    job->m_lastMsec = -1000;
    job->m_bytesPerSec = 4*ONE_MB_SIZE;
    job->m_totalSize = 100000;
    EXPECT_NO_FATAL_FAILURE(job->jobUpdated());
    job->m_isJobAdded = false;
    EXPECT_NO_FATAL_FAILURE(job->jobAdded());
    EXPECT_NO_FATAL_FAILURE(job->jobRemoved());
    EXPECT_NO_FATAL_FAILURE(job->jobAdded());
    void (*handleTaskCloselamda)(const QMap<QString, QString> &) = [](const QMap<QString, QString> &){};
    stl.set(ADDR(DTaskDialog,handleTaskClose),handleTaskCloselamda);
    EXPECT_NO_FATAL_FAILURE(job->jobAborted());
    EXPECT_NO_FATAL_FAILURE(job->jobPrepared());
    job->m_applyToAll = true;
    EXPECT_NO_FATAL_FAILURE(job->jobConflicted());
    job->m_applyToAll = false;
    EXPECT_NO_FATAL_FAILURE(job->jobConflicted());
}
static int read2new = 0;
qint64 read2lamda(char *, qint64){
    if (read2new%2 == 0) {
        read2new++;
        return 100;
    }
    read2new++;
    return 0;
}

qint64 read3lamda(char *, qint64){
    qDebug() << "read3 read2new == " << read2new;
    QThread::msleep(500);
    qDebug() << "read3 read2new2 == " << read2new;
    if (read2new%2 == 0) {
        read2new++;
        return 100;
    }
    read2new++;
    return 0;
}

TEST_F(FileJobTest, start_copyFile) {
    job->m_isGvfsFileOperationUsed = true;
    QProcess::execute("mkdir " + QDir::currentPath()+"/start_copyFile");
    QProcess::execute("touch " + QDir::currentPath()+"/start_copyFile/11.txt");
    source.setPath(QDir::currentPath()+"/start_copyFile/11.txt");
    dst.setPath(QDir::currentPath()+"/start_copyFile");
    QString dstdirpath = dst.toLocalFile();
    DUrl linkfile(source),fileurl(source),dirurl(source),targetdirurl(dst),targetfile(source);
    linkfile.setPath(source.toLocalFile() + "sys_link");
    fileurl.setPath(QDir::currentPath()+"/start_copyFile");
    QProcess::execute("ln -s " + source.toLocalFile() + " " + linkfile.toLocalFile());
    bool (*copyFileByGiolamda)(void *,const QString &, const QString &, bool, QString *) = []
            (void *,const QString &, const QString &, bool, QString *targetFile){
        *targetFile = QString("kdek");
        return true;
    };
    stl.set(ADDR(FileJob,copyFileByGio),copyFileByGiolamda);
    QString targetpaht("");
    EXPECT_NO_FATAL_FAILURE(job->copyFile(source.toLocalFile(),dst.toLocalFile(), false, &targetpaht));
    job->m_isGvfsFileOperationUsed = false;
    bool (*checkFat32FileOutof4Glamda)(const QString &, const QString &) = []
            (const QString &, const QString &){return true;};
    stl.set(ADDR(FileJob,checkFat32FileOutof4G),checkFat32FileOutof4Glamda);
    EXPECT_FALSE(job->copyFile(source.toLocalFile(),dst.toLocalFile()));
    QProcess::execute("rm -rf " + QDir::currentPath()+"/start_copyFile");
}
TEST_F(FileJobTest, start_copyFile_1) {
    job->m_isGvfsFileOperationUsed = true;
    QProcess::execute("mkdir " + QDir::currentPath()+"/start_copyFile_1");
    QProcess::execute("touch " + QDir::currentPath()+"/start_copyFile_1/11.txt");
    source.setPath(QDir::currentPath()+"/start_copyFile_1/11.txt");
    dst.setPath(QDir::currentPath()+"/start_copyFile_1");
    QString dstdirpath = dst.toLocalFile();
    DUrl linkfile(source),fileurl(source),dirurl(source),targetdirurl(dst),targetfile(source);
    linkfile.setPath(source.toLocalFile() + "sys_link");
    fileurl.setPath(QDir::currentPath()+"/start_copyFile_1");
    QProcess::execute("ln -s " + source.toLocalFile() + " " + linkfile.toLocalFile());
    bool (*copyFileByGiolamda)(void *,const QString &, const QString &, bool, QString *) = []
            (void *,const QString &, const QString &, bool, QString *targetFile){
        *targetFile = QString("kdek");
        return true;
    };
    stl.set(ADDR(FileJob,copyFileByGio),copyFileByGiolamda);
    QString targetpaht("");
    EXPECT_NO_FATAL_FAILURE(job->copyFile(source.toLocalFile(),dst.toLocalFile(), false, &targetpaht));
    job->m_isGvfsFileOperationUsed = false;
    job->m_isAborted = true;
    EXPECT_FALSE(job->copyFile(source.toLocalFile(),dst.toLocalFile()));

    job->m_isAborted = false;
    job->m_applyToAll = true;
    job->m_status = FileJob::Cancelled;
    EXPECT_TRUE(job->copyFile(source.toLocalFile(),dst.toLocalFile(),false,&targetpaht));

    job->m_applyToAll = false;
    job->m_skipandApplyToAll = false;
    job->m_status = FileJob::Cancelled;
    void (*jobConflictedlamda)(void *) = [](void *){};
    stl.set(ADDR(FileJob,jobConflicted),jobConflictedlamda);
    job->m_isSkip = true;
    EXPECT_FALSE(job->copyFile(source.toLocalFile(),dst.toLocalFile()));

    job->m_isSkip = false;
    job->m_isCoExisted = true;
    job->m_isReplaced = false;
    EXPECT_FALSE(job->copyFile("",dst.toLocalFile()));

    job->m_isCoExisted = false;
    job->m_isReplaced = true;
    dst.setPath(dstdirpath);
    QString (*checkDuplicateNamelamda)(void *,const QString &) = []
            (void *,const QString &name){return name;};
    {
        Stub stl;
        stl.set(ADDR(FileJob,checkDuplicateName),checkDuplicateNamelamda);
        EXPECT_FALSE(job->copyFile(source.toLocalFile(),dst.toLocalFile()));
    }

    job->m_isCoExisted = false;
    job->m_isReplaced = true;
    {
        Stub stl;
        stl.set(ADDR(FileJob,checkDuplicateName),checkDuplicateNamelamda);
        EXPECT_FALSE(job->copyFile(source.toLocalFile(),linkfile.toLocalFile()));
    }

    job->m_isCoExisted = false;
    job->m_isReplaced = true;
    {
        Stub stl;
        stl.set(ADDR(FileJob,checkDuplicateName),checkDuplicateNamelamda);
        EXPECT_FALSE(job->copyFile(source.toLocalFile(),fileurl.toLocalFile()));
    }

    qint64 (*read1lamda)(char *, qint64) = [](char *, qint64){return qint64(-1);};
    stl.set((qint64 (QIODevice::*)(char *, qint64))ADDR(QIODevice,read),read1lamda);
    QFileDevice::FileError (*error1lamda)(void *) = [](void *){return QFileDevice::ReadError;};
    stl.set(ADDR(QFileDevice,error),error1lamda);
    EXPECT_FALSE(job->copyFile(source.toLocalFile(),dst.toLocalFile(),false,&targetpaht));

    stl.set((qint64 (QIODevice::*)(char *, qint64))ADDR(QIODevice,read),&read2lamda);
    EXPECT_FALSE(job->copyFile(source.toLocalFile(),dst.toLocalFile(),false,&targetpaht));

    stl.set((qint64 (QIODevice::*)(char *, qint64))ADDR(QIODevice,read),&read3lamda);
    read2new = read2new%2 == 0 ? read2new : read2new + 1;
    QtConcurrent::run([=]() {
        QThread::msleep(150);
        qDebug() << "00000" << job->m_status;
        job->m_status = FileJob::Cancelled;
        job->m_isSkip = true;
    });
    EXPECT_FALSE(job->copyFile(source.toLocalFile(),dst.toLocalFile(),false,&targetpaht));
    qDebug() << "copyFile 0" << read2new;
    read2new = read2new%2 == 0 ? read2new : read2new + 1;
    job->m_isSkip = false;
    job->m_status = FileJob::Started;
    QtConcurrent::run([=]() {
        QThread::msleep(150);
        qDebug() << "000001" << job->m_status;
        job->m_status = FileJob::Cancelled;
        job->m_isSkip = false;
    });

    EXPECT_FALSE(job->copyFile(source.toLocalFile(),dst.toLocalFile(),false,&targetpaht));
    qDebug() << "copyFile 1" << read2new;
    read2new = read2new%2 == 0 ? read2new : read2new + 1;
    job->m_isSkip = false;
    job->m_status = FileJob::Started;
    QFuture<void> future = QtConcurrent::run([=]() {
        QThread::msleep(150);
        qDebug() << "000002" << job->m_status;
        job->m_status = FileJob::Paused;
        QThread::msleep(400);
        qDebug() << "000003" << job->m_status;
        job->m_status = FileJob::Conflicted;
    });
    EXPECT_FALSE(job->copyFile(source.toLocalFile(),dst.toLocalFile(),false,&targetpaht));
    qDebug() << "copyFile 1" << read2new;
    future.waitForFinished();

    QProcess::execute("rm -rf " + QDir::currentPath()+"/start_copyFile_1");

}

TEST_F(FileJobTest, start_showProgress) {
    bool (*ghostSignallamda)(const DUrl &, DAbstractFileWatcher::SignalType3, const DUrl &, const int) = []
            (const DUrl &, DAbstractFileWatcher::SignalType3, const DUrl &, const int){return true;};
    stl.set((bool (*)(const DUrl &, DAbstractFileWatcher::SignalType3, const DUrl &, const int))\
            ADDR(DAbstractFileWatcher,ghostSignal),ghostSignallamda);
    bool (*ghostSignal1lamda)(const DUrl &, DAbstractFileWatcher::SignalType1, const DUrl &) = []
            (const DUrl &, DAbstractFileWatcher::SignalType1, const DUrl &){return true;};
    bool (*ghostSignal2lamda)(const DUrl &, DAbstractFileWatcher::SignalType2 , const DUrl &, const DUrl &) = []
            (const DUrl &, DAbstractFileWatcher::SignalType2 , const DUrl &, const DUrl &){return true;};
    stl.set((bool (*)(const DUrl &, DAbstractFileWatcher::SignalType1, const DUrl &))\
            ADDR(DAbstractFileWatcher,ghostSignal),ghostSignal1lamda);
    stl.set((bool (*)(const DUrl &, DAbstractFileWatcher::SignalType2 , const DUrl &, const DUrl &))\
            ADDR(DAbstractFileWatcher,ghostSignal),ghostSignal2lamda);
    job->m_isGvfsFileOperationUsed = true;
    job->m_needGhostFileCreateSignal = true;
    EXPECT_NO_FATAL_FAILURE(job->showProgress(100,100,job.data()));
}

TEST_F(FileJobTest, start_copyFileByGio) {
    QProcess::execute("mkdir " + QDir::currentPath()+"/start_copyFileByGio");
    QProcess::execute("touch " + QDir::currentPath()+"/start_copyFileByGio/11.txt");
    source.setPath(QDir::currentPath()+"/start_copyFileByGio/11.txt");
    dst.setPath(QDir::currentPath()+"/start_copyFileByGio");
    QString dstdirpath = dst.toLocalFile();
    DUrl linkfile(source),fileurl(source),dirurl(source),targetdirurl(dst),targetfile(source);
    linkfile.setPath(source.toLocalFile() + "sys_link");
    fileurl.setPath(QDir::currentPath()+"/start_copyFileByGio");
    QProcess::execute("ln -s " + source.toLocalFile() + " " + linkfile.toLocalFile());

    bool (*ghostSignallamda)(const DUrl &, DAbstractFileWatcher::SignalType3, const DUrl &, const int) = []
            (const DUrl &, DAbstractFileWatcher::SignalType3, const DUrl &, const int){return true;};
    stl.set((bool (*)(const DUrl &, DAbstractFileWatcher::SignalType3, const DUrl &, const int))\
            ADDR(DAbstractFileWatcher,ghostSignal),ghostSignallamda);
    bool (*ghostSignal1lamda)(const DUrl &, DAbstractFileWatcher::SignalType1, const DUrl &) = []
            (const DUrl &, DAbstractFileWatcher::SignalType1, const DUrl &){return true;};
    bool (*ghostSignal2)(const DUrl &, DAbstractFileWatcher::SignalType2 , const DUrl &, const DUrl &) = []
            (const DUrl &, DAbstractFileWatcher::SignalType2 , const DUrl &, const DUrl &){return true;};
    stl.set((bool (*)(const DUrl &, DAbstractFileWatcher::SignalType1, const DUrl &))\
            ADDR(DAbstractFileWatcher,ghostSignal),ghostSignal1lamda);
    stl.set((bool (*)(const DUrl &, DAbstractFileWatcher::SignalType2 , const DUrl &, const DUrl &))\
            ADDR(DAbstractFileWatcher,ghostSignal),ghostSignal2);

    bool (*checkFat32FileOutof4Glamda)(const QString &, const QString &) = []
            (const QString &, const QString &){return true;};
    stl.set(ADDR(FileJob,checkFat32FileOutof4G),checkFat32FileOutof4Glamda);
    EXPECT_FALSE(job->copyFileByGio(source.toLocalFile(),dstdirpath));

     QProcess::execute("rm -rf " + QDir::currentPath()+"/start_copyFileByGio");
}

TEST_F(FileJobTest, start_copyFileByGio_1) {
    QProcess::execute("mkdir " + QDir::currentPath()+"/start_copyFileByGio_1");
    QProcess::execute("touch " + QDir::currentPath()+"/start_copyFileByGio_1/11.txt");
    source.setPath(QDir::currentPath()+"/start_copyFileByGio_1/11.txt");
    dst.setPath(QDir::currentPath()+"/start_copyFileByGio_1");
    QString dstdirpath = dst.toLocalFile();
    DUrl linkfile(source),fileurl(source),dirurl(source),targetdirurl(dst),targetfile(source);
    linkfile.setPath(source.toLocalFile() + "sys_link");
    fileurl.setPath(QDir::currentPath()+"/start_copyFileByGio_1");
    QProcess::execute("ln -s " + source.toLocalFile() + " " + linkfile.toLocalFile());

    bool (*ghostSignallamda)(const DUrl &, DAbstractFileWatcher::SignalType3, const DUrl &, const int) = []
            (const DUrl &, DAbstractFileWatcher::SignalType3, const DUrl &, const int){return true;};
    stl.set((bool (*)(const DUrl &, DAbstractFileWatcher::SignalType3, const DUrl &, const int))\
            ADDR(DAbstractFileWatcher,ghostSignal),ghostSignallamda);
    bool (*ghostSignal1lamda)(const DUrl &, DAbstractFileWatcher::SignalType1, const DUrl &) = []
            (const DUrl &, DAbstractFileWatcher::SignalType1, const DUrl &){return true;};
    bool (*ghostSignal2lamda)(const DUrl &, DAbstractFileWatcher::SignalType2 , const DUrl &, const DUrl &) = []
            (const DUrl &, DAbstractFileWatcher::SignalType2 , const DUrl &, const DUrl &){return true;};
    stl.set((bool (*)(const DUrl &, DAbstractFileWatcher::SignalType1, const DUrl &))\
            ADDR(DAbstractFileWatcher,ghostSignal),ghostSignal1lamda);
    stl.set((bool (*)(const DUrl &, DAbstractFileWatcher::SignalType2 , const DUrl &, const DUrl &))\
            ADDR(DAbstractFileWatcher,ghostSignal),ghostSignal2lamda);

    job->m_isAborted = true;
    EXPECT_FALSE(job->copyFileByGio(source.toLocalFile(),dst.toLocalFile()));

    QString targetpaht;

    job->m_isAborted = false;
    job->m_applyToAll = true;
    job->m_status = FileJob::Cancelled;
    QProcess::execute("touch " + dst.toLocalFile() + "/" + source.fileName());
    EXPECT_TRUE(job->copyFileByGio(source.toLocalFile(),dst.toLocalFile(),false,&targetpaht));

    job->m_applyToAll = false;
    job->m_skipandApplyToAll = false;
    job->m_status = FileJob::Cancelled;
    void (*jobConflictedlamda)(void *) = [](void *){};
    stl.set(ADDR(FileJob,jobConflicted),jobConflictedlamda);
    job->m_isSkip = true;
    EXPECT_TRUE(job->copyFileByGio(source.toLocalFile(),dst.toLocalFile()));

    gboolean (*g_file_copyprelamda)(GFile *,GFile *,GFileCopyFlags ,GCancellable *,
                             GFileProgressCallback,gpointer,GError **) = []
            (GFile *,GFile *,GFileCopyFlags ,GCancellable *,
            GFileProgressCallback,gpointer,GError **error){
        *error = g_error_new(G_IO_ERROR,G_IO_ERROR_CANCELLED,"nihao");
        return gboolean(true);
    };
    stl.set(g_file_copy,g_file_copyprelamda);
    job->m_isSkip = false;
    job->m_isCoExisted = true;
    job->m_isReplaced = false;
    EXPECT_TRUE(job->copyFileByGio("",dst.toLocalFile()));

    job->m_isCoExisted = false;
    job->m_isReplaced = true;
    dst.setPath(dstdirpath);
    QString (*checkDuplicateNamelamda)(void *,const QString &) = []
            (void *,const QString &name){return name;};
    {
        Stub stl;
        stl.set(ADDR(FileJob,checkDuplicateName),checkDuplicateNamelamda);
        EXPECT_TRUE(job->copyFileByGio(source.toLocalFile(),dst.toLocalFile()));
    }

    job->m_isCoExisted = true;
    job->m_isReplaced = true;
    {
        Stub stl;
        stl.set(ADDR(FileJob,checkDuplicateName),checkDuplicateNamelamda);
        EXPECT_TRUE(job->copyFileByGio(source.toLocalFile(),linkfile.toLocalFile()));
    }

    job->m_isCoExisted = true;
    job->m_isReplaced = true;
    job->m_applyToAll = false;
    {
        Stub stl;
        stl.set(ADDR(FileJob,checkDuplicateName),checkDuplicateNamelamda);
        EXPECT_TRUE(job->copyFileByGio(source.toLocalFile(),fileurl.toLocalFile()));
    }
    gboolean (*g_file_copy1lamda)(GFile *,GFile *,GFileCopyFlags ,GCancellable *,
                             GFileProgressCallback,gpointer,GError **) = []
            (GFile *,GFile *,GFileCopyFlags ,GCancellable *,
            GFileProgressCallback,gpointer,GError **error){
        QThread::msleep(400);
        *error = g_error_new(G_IO_ERROR,G_IO_ERROR_PERMISSION_DENIED,"nihao");
        return gboolean(false);
    };
    stl.set(g_file_copy,g_file_copy1lamda);
    void (*showErrorDialog)(const QString &, const QString &) = []
            (const QString &, const QString &){};
    stl.set(ADDR(DialogManager,showErrorDialog),showErrorDialog);
    QFuture<void> future = QtConcurrent::run([=]() {
        QThread::msleep(150);
        qDebug() << "00000" << job->m_status;
        job->m_status = FileJob::Cancelled;
    });
    EXPECT_FALSE(job->copyFileByGio(source.toLocalFile(),dst.toLocalFile(),false,&targetpaht));

    gboolean (*g_file_copy2lamda)(GFile *,GFile *,GFileCopyFlags ,GCancellable *,
                             GFileProgressCallback,gpointer,GError **) = []
            (GFile *,GFile *,GFileCopyFlags ,GCancellable *,
            GFileProgressCallback,gpointer,GError **error){
        QThread::msleep(400);
        *error = g_error_new(G_IO_ERROR,G_IO_ERROR_CLOSED,"nihao");
        return gboolean(false);
    };
    stl.set(g_file_copy,g_file_copy2lamda);
    QFuture<void> future1 = QtConcurrent::run([=]() {
        QThread::msleep(150);
        qDebug() << "000001" << job->m_status;
        job->m_status = FileJob::Paused;
        QThread::msleep(300);
        qDebug() << "000002" << job->m_status;
        job->m_status = FileJob::Conflicted;
    });
    EXPECT_FALSE(job->copyFileByGio(source.toLocalFile(),dst.toLocalFile(),false,&targetpaht));

    gboolean (*g_file_copy3lamda)(GFile *,GFile *,GFileCopyFlags ,GCancellable *,
                             GFileProgressCallback,gpointer,GError **) = []
            (GFile *,GFile *,GFileCopyFlags ,GCancellable *,
            GFileProgressCallback,gpointer,GError **error){
        *error = g_error_new(G_IO_ERROR,G_IO_ERROR_CANCELLED,"nihao");
        return gboolean(true);
    };
    stl.set(g_file_copy,g_file_copy3lamda);
    EXPECT_TRUE(job->copyFileByGio(source.toLocalFile(),dst.toLocalFile(),false,&targetpaht));

    future.waitForFinished();
    future1.waitForFinished();

   QProcess::execute("rm -rf " + QDir::currentPath()+"/start_copyFileByGio_1");
}

TEST_F(FileJobTest, start_copyDir) {
    QProcess::execute("mkdir " + QDir::currentPath()+"/start_copyDir");
    QProcess::execute("mkdir " + QDir::currentPath()+"/start_copyDir/2");
    QProcess::execute("mkdir " + QDir::currentPath()+"/start_copyDir/11");
    QProcess::execute("touch " + QDir::currentPath()+"/start_copyDir/11/11.txt");
    DUrl copyurl = DUrl::fromLocalFile(QDir::currentPath()+"/start_copyDir/11");
    source.setPath(QDir::currentPath()+"/start_copyDir/11/11.txt");
    dst.setPath(QDir::currentPath()+"/start_copyDir/2");
    QString dstdirpath = dst.toLocalFile();
    DUrl linkfile(source),fileurl(source),dirurl(source),targetdirurl(dst),targetfile(source);
    linkfile.setPath(source.toLocalFile() + "sys_link");
    fileurl.setPath(QDir::currentPath()+"/start_copyDir");
    QProcess::execute("ln -s " + source.toLocalFile() + " " + linkfile.toLocalFile());

    bool (*copyFilelamda)(const QString &, const QString &,bool, QString *) = []
            (const QString &, const QString &,bool, QString *){return false;};
    stl.set(ADDR(FileJob,copyFile),copyFilelamda);
    bool (*setDirPermissionslamda)(const QString &, const QString &) = []
            (const QString &, const QString &){return false;};
    stl.set(ADDR(FileJob,setDirPermissions),setDirPermissionslamda);
    DUrlList (*childrenList)(const DUrl &) = [](const DUrl &){
        return DUrlList() << DUrl("file:///tmp");
    };

    void (*showCopyMoveToSelfDialoglamda)(const QMap<QString, QString> &) = []
            (const QMap<QString, QString> &){};
    stl.set(ADDR(DialogManager,showCopyMoveToSelfDialog),showCopyMoveToSelfDialoglamda);
    QString targetpaht;
    {
        Stub stl;
        stl.set(ADDR(DUrl,childrenList),childrenList);
        EXPECT_TRUE(job->copyDir(dst.toLocalFile(),QDir::currentPath()+"/start_copyDir",false,&targetpaht));
    }

    job->m_isAborted = true;
    EXPECT_FALSE(job->copyDir(copyurl.toLocalFile(),dst.toLocalFile(),false,&targetpaht));

    job->m_isAborted = false;
    job->m_applyToAll = true;
    job->m_status = FileJob::Cancelled;
    EXPECT_TRUE(job->copyDir(copyurl.toLocalFile(),dst.toLocalFile(),false,&targetpaht));

    job->m_applyToAll = false;
    job->m_skipandApplyToAll = false;
    job->m_status = FileJob::Cancelled;
    void (*jobConflictedlamda)(void *) = [](void *){};
    stl.set(ADDR(FileJob,jobConflicted),jobConflictedlamda);
    job->m_isSkip = true;
    EXPECT_TRUE(job->copyDir(copyurl.toLocalFile(),dst.toLocalFile()));

    job->m_isSkip = false;
    job->m_isCoExisted = true;
    job->m_isReplaced = false;
    EXPECT_FALSE(job->copyDir("",dst.toLocalFile()));

    job->m_isCoExisted = false;
    job->m_isReplaced = true;
    dst.setPath(dstdirpath);
    QString (*checkDuplicateNamelamda)(void *,const QString &) = []
            (void *,const QString &name){return name;};
    {
        Stub stl;
        stl.set(ADDR(FileJob,checkDuplicateName),checkDuplicateNamelamda);
        EXPECT_TRUE(job->copyDir(copyurl.toLocalFile(),dst.toLocalFile(),false,&targetpaht));
    }

    job->m_isCoExisted = false;
    job->m_isReplaced = true;
    {
        Stub stl;
        stl.set(ADDR(FileJob,checkDuplicateName),checkDuplicateNamelamda);
        EXPECT_FALSE(job->copyDir(copyurl.toLocalFile(),linkfile.toLocalFile(),false,&targetpaht));
    }

    job->m_isCoExisted = false;
    job->m_isReplaced = true;
    job->m_applyToAll = false;
    {
        Stub stl;
        stl.set(ADDR(FileJob,checkDuplicateName),checkDuplicateNamelamda);
        EXPECT_TRUE(job->copyDir(copyurl.toLocalFile(),fileurl.toLocalFile(),false,&targetpaht));
    }

    job->m_isCoExisted = false;
    job->m_isReplaced = true;
    job->m_applyToAll = false;
    bool (*mkdirlamda)(const QString &) = [](const QString &){return false;};
    {
        Stub stl;
        stl.set(ADDR(FileJob,checkDuplicateName),checkDuplicateNamelamda);
        stl.set(ADDR(QDir,mkdir),mkdirlamda);
        EXPECT_FALSE(job->copyDir(copyurl.toLocalFile(),QDir::currentPath()+"/start_copyDir/ut_copydir_filejob",false,&targetpaht));
    }

    char *(*savedir1lamda)(char const *) = [](char const *){
        char * ptr = nullptr;
        return ptr;};
    {
        Stub stl;
        stl.set(savedir,savedir1lamda);
        EXPECT_FALSE(job->copyDir(copyurl.toLocalFile(),dst.toLocalFile(),false,&targetpaht));
    }
    EXPECT_TRUE(job->copyDir(copyurl.toLocalFile(),dst.toLocalFile(),false,&targetpaht));

    bool (*handleSymlinkFilelamda)(const QString &, const QString &, QString *) = []
            (const QString &, const QString &, QString *){return false;};
    stl.set(ADDR(FileJob,handleSymlinkFile),handleSymlinkFilelamda);
    EXPECT_FALSE(job->copyDir(linkfile.toLocalFile(),dst.toLocalFile(),false,&targetpaht));

    QProcess::execute("rm -rf " + QDir::currentPath()+"/start_copyDir");
}

TEST_F(FileJobTest, start_moveFile) {
    bool (*handleMoveJoblamda)(const QString &, const QString &, QString *) = []
            (const QString &, const QString &, QString *){return false;};
    stl.set(ADDR(FileJob,handleMoveJob),handleMoveJoblamda);
    EXPECT_FALSE(job->moveFile("",""));
}
TEST_F(FileJobTest, start_moveFileByGio) {
    QProcess::execute("mkdir " + QDir::currentPath()+"/start_moveFileByGio");
    QProcess::execute("touch " + QDir::currentPath()+"/start_moveFileByGio/11.txt");
    source.setPath(QDir::currentPath()+"/start_moveFileByGio/11.txt");
    dst.setPath(QDir::currentPath()+"/start_moveFileByGio");
    QString dstdirpath = dst.toLocalFile();
    DUrl linkfile(source),fileurl(source),dirurl(source),targetdirurl(dst),targetfile(source);
    linkfile.setPath(source.toLocalFile() + "sys_link");
    fileurl.setPath(QDir::currentPath()+"/start_moveFileByGio");
    dirurl.setPath(QDir::currentPath()+"/ut_jobfile_moveFileByGio");
    targetdirurl.setPath(dst.toLocalFile() + "/" + dst.fileName());
    QProcess::execute("mkdir " + dirurl.toLocalFile() + " " + targetdirurl.toLocalFile());
    QProcess::execute("ln -s " + source.toLocalFile() + " " + linkfile.toLocalFile());
    DUrlList (*childrenListlamda)(const DUrl &) = [](const DUrl &){
        return DUrlList() << DUrl("file://"+QDir::currentPath()+"/ut_jobfile_moveFileByGio");
    };
    stl.set(ADDR(DUrl,childrenList),childrenListlamda);
    bool (*ghostSignallamda)(const DUrl &, DAbstractFileWatcher::SignalType3, const DUrl &, const int) = []
            (const DUrl &, DAbstractFileWatcher::SignalType3, const DUrl &, const int){return true;};
    stl.set((bool (*)(const DUrl &, DAbstractFileWatcher::SignalType3, const DUrl &, const int))\
            ADDR(DAbstractFileWatcher,ghostSignal),ghostSignallamda);
    bool (*ghostSignal1lamda)(const DUrl &, DAbstractFileWatcher::SignalType1, const DUrl &) = []
            (const DUrl &, DAbstractFileWatcher::SignalType1, const DUrl &){return true;};
    bool (*ghostSignal2lamda)(const DUrl &, DAbstractFileWatcher::SignalType2 , const DUrl &, const DUrl &) = []
            (const DUrl &, DAbstractFileWatcher::SignalType2 , const DUrl &, const DUrl &){return true;};
    stl.set((bool (*)(const DUrl &, DAbstractFileWatcher::SignalType1, const DUrl &))\
            ADDR(DAbstractFileWatcher,ghostSignal),ghostSignal1lamda);
    stl.set((bool (*)(const DUrl &, DAbstractFileWatcher::SignalType2 , const DUrl &, const DUrl &))\
            ADDR(DAbstractFileWatcher,ghostSignal),ghostSignal2lamda);

    typedef int(*fptr)(QDialog*);
    fptr pQDialogExec = (fptr)(&QDialog::exec);
    int (*stub_DDialog_exec)(void) = [](void)->int{return QDialog::Rejected;};
    stl.set(pQDialogExec, stub_DDialog_exec);
    void (*showErrorDialoglamda)(const QString &, const QString &) = []
            (const QString &, const QString &){};
    stl.set(ADDR(DialogManager,showErrorDialog),showErrorDialoglamda);
    QString targetpath;
    void (*showCopyMoveToSelfDialoglamda)(const QMap<QString, QString> &) = []
            (const QMap<QString, QString> &){};
    stl.set(ADDR(DialogManager,showCopyMoveToSelfDialog),showCopyMoveToSelfDialoglamda);
    EXPECT_TRUE(job->moveFileByGio(dst.toLocalFile(),"file://"+QDir::currentPath()+"/ut_jobfile_moveFileByGio"));

    job->m_isAborted = true;
    EXPECT_FALSE(job->moveFileByGio(targetdirurl.toLocalFile(),dst.toLocalFile(),&targetpath));

    job->m_isAborted = false;
    job->m_applyToAll = true;
    job->m_status = FileJob::Cancelled;
    EXPECT_FALSE(job->moveFileByGio(dst.toLocalFile(),dst.toLocalFile(),&targetpath));

    job->m_isAborted = false;
    job->m_applyToAll = true;
    job->m_status = FileJob::Cancelled;
    EXPECT_TRUE(job->moveFileByGio(targetdirurl.toLocalFile(),dst.toLocalFile(),&targetpath));

    job->m_applyToAll = false;
    void (*jobConflictedlamda)(void *) = [](void *){};
    stl.set(ADDR(FileJob,jobConflicted),jobConflictedlamda);
    job->m_status = FileJob::Cancelled;
    EXPECT_TRUE(job->moveFileByGio(targetdirurl.toLocalFile(),dst.toLocalFile(),&targetpath));

    job->m_applyToAll = false;
    job->m_status = FileJob::Cancelled;
    job->m_isCoExisted = true;
    job->m_isReplaced = false;
    EXPECT_TRUE(job->moveFileByGio(targetdirurl.toLocalFile(),dst.toLocalFile(),&targetpath));

    job->m_applyToAll = false;
    job->m_status = FileJob::Cancelled;
    job->m_isCoExisted = false;
    job->m_isReplaced = true;
    EXPECT_TRUE(job->moveFileByGio(targetdirurl.toLocalFile(),dst.toLocalFile(),&targetpath));

    gboolean (*g_file_move1lamda)(GFile *,GFile *,GFileCopyFlags ,GCancellable *,
                             GFileProgressCallback,gpointer,GError **) = []
            (GFile *,GFile *,GFileCopyFlags ,GCancellable *,
            GFileProgressCallback,gpointer,GError **error){
        QThread::msleep(400);
        *error = g_error_new(G_IO_ERROR,G_IO_ERROR_PERMISSION_DENIED,"nihao");
        return gboolean(false);
    };
    stl.set(g_file_move,g_file_move1lamda);
    QFuture<void> future = QtConcurrent::run([=]() {
        QThread::msleep(150);
        qDebug() << "00000" << job->m_status;
        job->m_status = FileJob::Cancelled;
    });
    EXPECT_TRUE(job->moveFileByGio(source.toLocalFile(),dst.toLocalFile(),&targetpath));

    gboolean (*g_file_move2lamda)(GFile *,GFile *,GFileCopyFlags ,GCancellable *,
                             GFileProgressCallback,gpointer,GError **) = []
            (GFile *,GFile *,GFileCopyFlags ,GCancellable *,
            GFileProgressCallback,gpointer,GError **error){
        QThread::msleep(400);
        *error = g_error_new(G_IO_ERROR,G_IO_ERROR_CLOSED,"nihao");
        return gboolean(false);
    };
    stl.set(g_file_move,g_file_move2lamda);
    QFuture<void> future1 = QtConcurrent::run([=]() {
        QThread::msleep(150);
        qDebug() << "000001" << job->m_status;
        job->m_status = FileJob::Paused;
        QThread::msleep(300);
        qDebug() << "000002" << job->m_status;
        job->m_status = FileJob::Conflicted;
    });
    EXPECT_TRUE(job->moveFileByGio(source.toLocalFile(),dst.toLocalFile(),&targetpath));
    future.waitForFinished();

    gboolean (*g_file_move3lamda)(GFile *,GFile *,GFileCopyFlags ,GCancellable *,
                             GFileProgressCallback,gpointer,GError **) = []
            (GFile *,GFile *,GFileCopyFlags ,GCancellable *,
            GFileProgressCallback,gpointer,GError **error){
        *error = g_error_new(G_IO_ERROR,G_IO_ERROR_CANCELLED,"nihao");
        return gboolean(true);
    };
    stl.set(g_file_move,g_file_move3lamda);
    EXPECT_TRUE(job->moveFileByGio(source.toLocalFile(),dst.toLocalFile(),&targetpath));
    future1.waitForFinished();
    QProcess::execute("rm -rf " + QDir::currentPath()+"/start_moveFileByGio");
}

TEST_F(FileJobTest, start_moveDir) {
    bool (*handleMoveJoblamda)(const QString &, const QString &, QString *) = []
            (const QString &, const QString &, QString *){return true;};
    stl.set(ADDR(FileJob,handleMoveJob),handleMoveJoblamda);
    EXPECT_TRUE(job->moveDir("",""));
}

TEST_F(FileJobTest, start_handleMoveJob) {
    QProcess::execute("mkdir " + QDir::currentPath()+"/start_handleMoveJob");
    QProcess::execute("touch " + QDir::currentPath()+"/start_handleMoveJob/11.txt");
    source.setPath(QDir::currentPath()+"/start_handleMoveJob/11.txt");
    dst.setPath(QDir::currentPath()+"/start_handleMoveJob");
    QString dstdirpath = dst.toLocalFile();
    DUrl linkfile(source),fileurl(source),dirurl(source),targetdirurl(dst),targetfile(source);
    linkfile.setPath(source.toLocalFile() + "sys_link");
    fileurl.setPath(QDir::currentPath()+"/start_handleMoveJob");
    dirurl.setPath( QDir::currentPath()+"/start_handleMoveJob/ut_jobfile_moveFileByGio");
    targetfile.setPath(dst.toLocalFile() + "/jfieit.txt");
    targetdirurl.setPath(dst.toLocalFile() + "/" + dst.fileName());
    QProcess::execute("touch " + targetfile.toLocalFile());
    QProcess::execute("mkdir " + dirurl.toLocalFile() + " " + targetdirurl.toLocalFile());
    QProcess::execute("ln -s " + source.toLocalFile() + " " + linkfile.toLocalFile());
    DUrlList (*childrenListlamda)(const DUrl &) = [](const DUrl &){
        return DUrlList() << DUrl("file://"+QDir::currentPath()+"/start_handleMoveJob/ut_jobfile_moveFileByGio");
    };

    bool (*ghostSignallamda)(const DUrl &, DAbstractFileWatcher::SignalType3, const DUrl &, const int) = []
            (const DUrl &, DAbstractFileWatcher::SignalType3, const DUrl &, const int){return true;};
    stl.set((bool (*)(const DUrl &, DAbstractFileWatcher::SignalType3, const DUrl &, const int))\
            ADDR(DAbstractFileWatcher,ghostSignal),ghostSignallamda);
    bool (*ghostSignal1lamda)(const DUrl &, DAbstractFileWatcher::SignalType1, const DUrl &) = []
            (const DUrl &, DAbstractFileWatcher::SignalType1, const DUrl &){return true;};
    bool (*ghostSignal2lamda)(const DUrl &, DAbstractFileWatcher::SignalType2 , const DUrl &, const DUrl &) = []
            (const DUrl &, DAbstractFileWatcher::SignalType2 , const DUrl &, const DUrl &){return true;};
    stl.set((bool (*)(const DUrl &, DAbstractFileWatcher::SignalType1, const DUrl &))\
            ADDR(DAbstractFileWatcher,ghostSignal),ghostSignal1lamda);
    stl.set((bool (*)(const DUrl &, DAbstractFileWatcher::SignalType2 , const DUrl &, const DUrl &))\
            ADDR(DAbstractFileWatcher,ghostSignal),ghostSignal2lamda);

    bool (*handleSymlinkFilelamda)(const QString &, const QString &, QString *) = []
            (const QString &, const QString &, QString *){return false;};
    stl.set(ADDR(FileJob,handleSymlinkFile),handleSymlinkFilelamda);
    stl.set(ADDR(FileJob,moveDir),handleSymlinkFilelamda);
    stl.set(ADDR(FileJob,moveFile),handleSymlinkFilelamda);

    QString targetpath;
    void (*showCopyMoveToSelfDialoglamda)(const QMap<QString, QString> &) = []
            (const QMap<QString, QString> &){};
    stl.set(ADDR(DialogManager,showCopyMoveToSelfDialog),showCopyMoveToSelfDialoglamda);
    {
        Stub stl;
        stl.set(ADDR(DUrl,childrenList),childrenListlamda);
        EXPECT_FALSE(job->handleMoveJob(dst.toLocalFile(),QDir::currentPath()+"/start_handleMoveJob/ut_jobfile_moveFileByGio"));
    }

    job->m_isAborted = true;
    EXPECT_FALSE(job->handleMoveJob(targetdirurl.toLocalFile(),dst.toLocalFile(),&targetpath));

    job->m_isAborted = false;
    job->m_applyToAll = true;
    job->m_status = FileJob::Cancelled;
    EXPECT_TRUE(job->handleMoveJob(dst.toLocalFile(),dst.toLocalFile(),&targetpath));

    job->m_isAborted = false;
    job->m_applyToAll = true;
    job->m_status = FileJob::Cancelled;
    EXPECT_TRUE(job->handleMoveJob(targetdirurl.toLocalFile(),dst.toLocalFile(),&targetpath));

    job->m_applyToAll = false;
    void (*jobConflictedlamda)(void *) = [](void *){};
    stl.set(ADDR(FileJob,jobConflicted),jobConflictedlamda);
    job->m_status = FileJob::Cancelled;
    EXPECT_TRUE(job->handleMoveJob(targetdirurl.toLocalFile(),dst.toLocalFile(),&targetpath));

    job->m_applyToAll = false;
    job->m_status = FileJob::Cancelled;
    job->m_isCoExisted = true;
    job->m_isReplaced = false;
    EXPECT_TRUE(job->handleMoveJob(targetdirurl.toLocalFile(),dst.toLocalFile(),&targetpath));

    job->m_applyToAll = false;
    job->m_status = FileJob::Cancelled;
    job->m_isCoExisted = false;
    job->m_isReplaced = true;
    EXPECT_TRUE(job->handleMoveJob(targetdirurl.toLocalFile(),dst.toLocalFile(),&targetpath));

    job->m_applyToAll = false;
    job->m_status = FileJob::Cancelled;
    job->m_isCoExisted = false;
    job->m_isReplaced = true;
    EXPECT_FALSE(job->handleMoveJob(targetdirurl.toLocalFile(),source.toLocalFile(),&targetpath));

    DUrl tmpfile(source);
    tmpfile.setPath(TestHelper::createTmpFile());
    EXPECT_TRUE(job->handleMoveJob(source.toLocalFile(),dst.toLocalFile(),&targetpath));

    EXPECT_TRUE(job->handleMoveJob(linkfile.toLocalFile(),dst.toLocalFile(),&targetpath));
    bool (*handleSymlinkFile1lamda)(const QString &, const QString &, QString *) = []
            (const QString &, const QString &, QString *){
        QThread::msleep(400);
        return false;
    };
    stl.set(ADDR(FileJob,handleSymlinkFile),handleSymlinkFile1lamda);
    stl.set(ADDR(FileJob,moveDir),handleSymlinkFile1lamda);
    stl.set(ADDR(FileJob,moveFile),handleSymlinkFile1lamda);
    QFuture<void> future1 = QtConcurrent::run([=]() {
        QThread::msleep(150);
        qDebug() << "000001" << job->m_status;
        job->m_status = FileJob::Paused;
        QThread::msleep(300);
        qDebug() << "000002" << job->m_status;
        job->m_status = FileJob::Conflicted;
    });
    EXPECT_TRUE(job->handleMoveJob(linkfile.toLocalFile(),dst.toLocalFile(),&targetpath));
    future1.waitForFinished();

    QFuture<void> future = QtConcurrent::run([=]() {
        QThread::msleep(150);
        qDebug() << "000000" << job->m_status;
        job->m_status = FileJob::Cancelled;
    });
    EXPECT_TRUE(job->handleMoveJob(linkfile.toLocalFile(),dst.toLocalFile(),&targetpath));
    future.waitForFinished();

    EXPECT_FALSE(job->handleMoveJob(linkfile.toLocalFile(),dst.toLocalFile() + "/ut_dir",&targetpath));

    EXPECT_TRUE(job->handleMoveJob(dst.toLocalFile(),dst.toLocalFile(),&targetpath));

    QProcess::execute("rm -rf " + QDir::currentPath()+"/start_handleMoveJob");
}

TEST_F(FileJobTest, start_handleSymlinkFile) {
    job->m_isAborted = true;
    EXPECT_FALSE(job->handleSymlinkFile("",""));
    QProcess::execute("mkdir " + QDir::currentPath()+"/start_handleSymlinkFile");
    QProcess::execute("touch " + QDir::currentPath()+"/start_handleSymlinkFile/11.txt");
    source.setPath(QDir::currentPath()+"/start_handleSymlinkFile");
    job->m_isAborted = false;
    dst.setPath(source.toLocalFile()+"_sys_link");
    job->m_applyToAll = false;
    QString target;
    EXPECT_FALSE(job->handleSymlinkFile(source.toLocalFile(),dst.toLocalFile(),&target));

    job.reset(new FileJob(FileJob::Trash));
    bool (*moveFileToTrashlamda)(const QString &, QString *) = []
            (const QString &, QString *){return true;};
    Stub stl;
    stl.set(ADDR(FileJob,moveFileToTrash), moveFileToTrashlamda);
    EXPECT_FALSE(job->handleSymlinkFile(source.toLocalFile(),dst.toLocalFile(),&target));

    DUrl tmplink(dst);
    tmplink.setPath(source.toLocalFile() + "oo_ut_link");
    EXPECT_FALSE(job->handleSymlinkFile(source.toLocalFile(),tmplink.toLocalFile(),&target));

    bool (*moveFileToTrash1lamda)(const QString &, QString *) = []
            (const QString &, QString *){
        QThread::msleep(400);
        return true;
    };
    stl.set(ADDR(FileJob,moveFileToTrash),moveFileToTrash1lamda);
    QFuture<void> future1 = QtConcurrent::run([=]() {
        QThread::msleep(150);
        qDebug() << "000001" << job->m_status;
        job->m_status = FileJob::Paused;
        QThread::msleep(300);
        qDebug() << "000002" << job->m_status;
        job->m_status = FileJob::Conflicted;
    });
    EXPECT_FALSE(job->handleSymlinkFile(source.toLocalFile(),tmplink.toLocalFile(),&target));
    future1.waitForFinished();

    QFuture<void> future = QtConcurrent::run([=]() {
        QThread::msleep(150);
        qDebug() << "000000" << job->m_status;
        job->m_status = FileJob::Cancelled;
    });
    EXPECT_FALSE(job->handleSymlinkFile(source.toLocalFile(),tmplink.toLocalFile(),&target));
    future.waitForFinished();

    QProcess::execute("rm -rf " + QDir::currentPath()+"/start_handleSymlinkFile");
}

TEST_F(FileJobTest, start_restoreTrashFile) {
    void (*jobConflictedlamda)(void *) = [](void *){};
    stl.set(ADDR(FileJob,jobConflicted),jobConflictedlamda);
    QProcess::execute("mkdir " + QDir::currentPath()+"/start_restoreTrashFile");
    QProcess::execute("touch " + QDir::currentPath()+"/start_restoreTrashFile/11.txt");
    source.setPath(QDir::currentPath()+"/start_restoreTrashFile/11.txt");
    dst.setPath(QDir::currentPath()+"/start_restoreTrashFile/11.txt");

    job->m_isAborted = true;
    job->m_isSkip = true;
    EXPECT_FALSE(job->restoreTrashFile(source.toLocalFile(),dst.toLocalFile()));

    job->m_isAborted = false;
    job->m_isCoExisted = true;
    job->m_isReplaced = false;
    EXPECT_FALSE(job->restoreTrashFile(source.toLocalFile(),"/tmp/ut_testtttt_rest"));

    job->m_isCoExisted = true;
    job->m_isReplaced = true;
    EXPECT_FALSE(job->restoreTrashFile("",dst.toLocalFile()));

    job->m_isReplaced = true;
    EXPECT_FALSE(job->restoreTrashFile(source.toLocalFile(),dst.toLocalFile()));

    QString tmpdir = TestHelper::createTmpDir();
    job->m_isReplaced = true;
    EXPECT_FALSE(job->restoreTrashFile(tmpdir,dst.toLocalFile()));

    EXPECT_FALSE(job->restoreTrashFile("/tmp/ut_testtttt_rest",dst.toLocalFile()));

    QProcess::execute("rm -rf " + QDir::currentPath()+"/start_restoreTrashFile");
}
TEST_F(FileJobTest, start_deleteFile) {
    QProcess::execute("mkdir " + QDir::currentPath()+"/start_deleteFile");
    QProcess::execute("touch " + QDir::currentPath()+"/start_deleteFile/11.txt");
    source.setPath(QDir::currentPath()+"/start_deleteFile/11.txt");
    TestHelper::deleteTmpFiles(QStringList() << source.toLocalFile() );
    EXPECT_FALSE(job->deleteFile(source.toLocalFile()));
    EXPECT_FALSE(job->deleteFile(source.toLocalFile()));
    QProcess::execute("rm -rf " + QDir::currentPath()+"/start_deleteFile");
}

TEST_F(FileJobTest, start_deleteFileByGio) {
    QProcess::execute("mkdir " + QDir::currentPath()+"/start_deleteFileByGio");
    QProcess::execute("touch " + QDir::currentPath()+"/start_deleteFileByGio/11.txt");
    source.setPath(QDir::currentPath()+"/start_deleteFileByGio/11.txt");
    EXPECT_TRUE(job->deleteFileByGio(source.toLocalFile()));
    EXPECT_FALSE(job->deleteFileByGio(source.toLocalFile()));
    QProcess::execute("rm -rf " + QDir::currentPath()+"/start_deleteFileByGio");
}

TEST_F(FileJobTest, start_) {
    source.setPath("/run/user/1000/gvfs/smb-share:server=10.8.11.190,share=test");
    dst.setPath("~/Desktop");
    EXPECT_FALSE(job->checkUseGvfsFileOperation(dst.toLocalFile()));
    EXPECT_TRUE(job->checkUseGvfsFileOperation(DUrlList() << source << DUrl(), dst));
    dst.setPath("/run/user/1000/gvfs/smb-share:server=10.8.11.190,share=test");
    EXPECT_TRUE(job->checkUseGvfsFileOperation(DUrlList() << source << DUrl(), dst));

}
qint64 totalSizelamda(const DUrlList &, const qint64 &, bool &isInLimit){
    qint64 oo = 10000;
    isInLimit = false;
    return oo;
};
TEST_F(FileJobTest, start_checkDiskSpaceAvailable) {
    QProcess::execute("mkdir " + QDir::currentPath()+"/start_checkDiskSpaceAvailable");
    QProcess::execute("touch " + QDir::currentPath()+"/start_checkDiskSpaceAvailable/11.txt");
    source.setPath(QDir::currentPath()+"/start_checkDiskSpaceAvailable/11.txt");
    dst.setPath(QDir::currentPath()+"/start_checkDiskSpaceAvailable");
    bool (*isGvfsMountFilelamda)(void *,const QString &, const bool &) =
            [](void *,const QString &, const bool &){return true;};
    bool (*isGvfsMountFilelamda1)(void *,const QString &, const bool &) =
            [](void *,const QString &, const bool &){return false;};
    {
        StubExt stl;
        stl.set(ADDR(FileUtils,isGvfsMountFile),isGvfsMountFilelamda);
        EXPECT_TRUE(job->checkDiskSpaceAvailable(DUrlList() << source,dst));
    }
    {
        Stub stl;
        stl.set(ADDR(FileUtils,isGvfsMountFile),isGvfsMountFilelamda1);
        stl.set((qint64 (*)(const DUrlList &, const qint64 &, bool &))(&FileUtils::totalSize),totalSizelamda);
        EXPECT_FALSE(job->checkDiskSpaceAvailable(DUrlList() << source,dst));
    }
    QProcess::execute("rm -rf " + QDir::currentPath()+"/start_checkDiskSpaceAvailable");
}

TEST_F(FileJobTest, start_checkTrashFileOutOf1GB) {
    QProcess::execute("mkdir " + QDir::currentPath()+"/start_checkTrashFileOutOf1GB");
    QProcess::execute("touch " + QDir::currentPath()+"/start_checkTrashFileOutOf1GB/11.txt");
    source.setPath(QDir::currentPath()+"/start_checkTrashFileOutOf1GB/11.txt");
    dst.setPath(source.parentUrl().toLocalFile()+"/syslink_tmp");
    QProcess::execute("ln -s " + source.toLocalFile() + " " + dst.toLocalFile());
    EXPECT_TRUE(job->checkTrashFileOutOf1GB(dst));
    EXPECT_TRUE(job->checkTrashFileOutOf1GB(source));
    QProcess::execute("rm -rf " + QDir::currentPath()+"/start_checkTrashFileOutOf1GB");
}

UDiskDeviceInfoPointer getDeviceByPathlamda(void *,const QString &){
    return UDiskDeviceInfoPointer(new UDiskDeviceInfo());
};

// arm 下这里会卡死 原因不明 暂时注释
#ifndef __arm__
TEST_F(FileJobTest, start_checkFat32FileOutof4G) {
    QProcess processMkdir;
    processMkdir.start("mkdir " + QDir::currentPath()+"/start_checkFat32FileOutof4G");
    if (!processMkdir.waitForFinished())
        return;

    QProcess processTouch;
    processTouch.start("touch " + QDir::currentPath()+"/start_checkFat32FileOutof4G/11.txt");
    if (!processTouch.waitForFinished())
        return;

    //QProcess::execute("mkdir " + QDir::currentPath()+"/start_checkFat32FileOutof4G");
    //QProcess::execute("touch " + QDir::currentPath()+"/start_checkFat32FileOutof4G/11.txt");

    source.setPath(QDir::currentPath()+"/start_checkFat32FileOutof4G/11.txt");
    dst.setPath(QDir::currentPath()+"/start_checkFat32FileOutof4G");

    void (*show4gFat32Dialoglamda)(void *) = [](void *){};
    qint64 (*sizelamda)(void*) = [](void *){
        qint64 tmpsize = 6l*1024*1024*1024;
        return tmpsize;
    };
    {
        StubExt stl;
        stl.set(ADDR(DialogManager,show4gFat32Dialog),show4gFat32Dialoglamda);

        stl.set(ADDR(QFileInfo,size),sizelamda);
        EXPECT_FALSE(job->checkFat32FileOutof4G(source.toLocalFile(),dst.toLocalFile()));

        stl.set(ADDR(UDiskListener,getDeviceByPath),getDeviceByPathlamda);
        EXPECT_FALSE(job->checkFat32FileOutof4G(source.toLocalFile(),dst.toLocalFile()));
    }

    QStringList (*resolveDeviceNodelamda)(QString , QVariantMap ) = [](QString , QVariantMap){
        return QStringList() << "vfat";
    };
    {
        StubExt stl;
        stl.set(ADDR(DialogManager,show4gFat32Dialog),show4gFat32Dialoglamda);

        stl.set(ADDR(QFileInfo,size),sizelamda);
        stl.set(ADDR(UDiskListener,getDeviceByFilePath),getDeviceByPathlamda);
        stl.set(ADDR(DDiskManager,resolveDeviceNode),resolveDeviceNodelamda);
        QString (*idTypelamda)(void *) = [](void *){return QString("vfat");};
        stl.set(ADDR(DBlockDevice,idType),idTypelamda);
        EXPECT_TRUE(job->checkFat32FileOutof4G(source.toLocalFile(),dst.toLocalFile()));
    }

    QProcess processRm;
    processRm.start("rm -rf " + QDir::currentPath()+"/start_checkFat32FileOutof4G");
    processRm.waitForFinished();
    //QProcess::execute("rm -rf " + QDir::currentPath()+"/start_checkFat32FileOutof4G");
}
#endif

TEST_F(FileJobTest, start_getXorrisoErrorMsg){
    QString error;
    error = "While grafting '(.kk)' file object exists and may not be overwritten";
    EXPECT_TRUE(job->getXorrisoErrorMsg(QStringList() << error).contains("is a duplicate file"));
    error = "Image size 2651, exceeds free space on media 41654.";
    EXPECT_FALSE(job->getXorrisoErrorMsg(QStringList() << error).contains("Insufficient disc space."));
    error = "Lost connection to drive";
    EXPECT_TRUE(job->getXorrisoErrorMsg(QStringList() << error).contains("Lost connection to drive."));
    error = "servo failure";
    EXPECT_TRUE(job->getXorrisoErrorMsg(QStringList() << error).contains("The CD/DVD drive is not ready. Try another disc."));
    error = "Device or resource busy";
    EXPECT_TRUE(job->getXorrisoErrorMsg(QStringList() << error).contains("Exit the program using the drive"));
    error = "-volid: Text too long";
    EXPECT_TRUE(job->getXorrisoErrorMsg(QStringList() << error).contains("invalid volume name"));
    error = "Unknown error";
    EXPECT_TRUE(job->getXorrisoErrorMsg(QStringList() << error).contains("Unknown error"));
}

TEST_F(FileJobTest, start_canMove){
    QProcess::execute("mkdir " + QDir::currentPath()+"/start_canMove");
    QProcess::execute("touch " + QDir::currentPath()+"/start_canMove/11.txt");
    source.setPath(QDir::currentPath()+"/start_canMove/11.txt");
    bool (*isVaultFilelamda)(QString) = [](QString){return true;};
    stl.set(ADDR(VaultController,isVaultFile),isVaultFilelamda);
    QFileDevice::Permissions (*getPermissionslamda)(QString) = [](QString){
        QFileDevice::Permissions ooo;
        return ooo;
    };
    stl.set(ADDR(VaultController,getPermissions),getPermissionslamda);
    EXPECT_FALSE(job->canMove(source.toLocalFile()));
    QProcess::execute("rm -rf " + QDir::currentPath()+"/start_canMove");
}
TEST_F(FileJobTest, start_canMove_one){
    QProcess::execute("mkdir " + QDir::currentPath()+"/start_canMove_one");
    QProcess::execute("touch " + QDir::currentPath()+"/start_canMove_one/11.txt");
    source.setPath(QDir::currentPath()+"/start_canMove_one/11.txt");
    __uid_t (*getuidtmp) (void*) = [](void*){
        __uid_t tid = 0;
        return tid;
    };
    __uid_t (*getuidtmpnomal) (void*) = [](void*){
        __uid_t tid = 10;
        return tid;
    };
    {
        Stub stl;
        stl.set(getuid,getuidtmp);
        EXPECT_TRUE(job->canMove(source.toLocalFile()));
    }
    uint (*ownerIdlamda)(void *) = [](void *){
        uint tmpid = 0;
        return tmpid;
    };
    {
        Stub stl;
        stl.set(getuid,getuidtmpnomal);
        stl.set(ADDR(QFileInfo,ownerId),ownerIdlamda);
        EXPECT_TRUE(job->canMove(source.toLocalFile()));
    }
    QProcess::execute("rm -rf " + QDir::currentPath()+"/start_canMove_one");
}

TEST_F(FileJobTest, start_writeTrashInfo){
    {
        StubExt stl;
        typedef bool (*openfile)(QFile *,QFile::OpenMode);
        stl.set_lamda((openfile)((bool (QFile::*)(QFile::OpenMode))(&QFile::open)),[](){return false;});
        EXPECT_FALSE(job->writeTrashInfo("ifue_98jfei_8e8f","jfaskjdf","kfek"));
    }

    StubExt stl;
    QList<QString> (*getTagsThroughFileslamda)(const QList<DUrl>&) = [](const QList<DUrl>&){
        QList<QString> tt;
        return tt << "ut_oireoirsfsdj";
    };
    stl.set(ADDR(TagManager,getTagsThroughFiles),getTagsThroughFileslamda);
    qint64 (*write1lamda)(const QByteArray &) = [](const QByteArray &){return qint64(-1);};
    stl.set((qint64 (QIODevice::*)(const QByteArray &))ADDR(QIODevice,write),write1lamda);
    EXPECT_FALSE(job->writeTrashInfo("ifue_98jfei_8e8f","jfaskjdf","kfek"));
}
TEST_F(FileJobTest, start_moveFileToTrash)
{
    EXPECT_NO_FATAL_FAILURE(job->setStatus(FileJob::Cancelled));
    QString targetPath;
    EXPECT_FALSE(job->moveFileToTrash(QString("jhfenfd"), &targetPath));
    EXPECT_NO_FATAL_FAILURE(job->setStatus(FileJob::Run));
    bool (*writeTrashInfolamda1)(void *, const QString &, const QString &, const QString &) = []
    (void *, const QString &, const QString &, const QString &) {
        return true;
    };
    stl.set(ADDR(FileJob, writeTrashInfo), writeTrashInfolamda1);
    EXPECT_TRUE(job->moveFileToTrash(QString("mnfeingienfjdj"), &targetPath));
    bool (*writeTrashInfolamda)(void *, const QString &, const QString &, const QString &) = []
    (void *, const QString &, const QString &, const QString &) {
        return false;
    };
    stl.reset(ADDR(FileJob, writeTrashInfo));
    stl.set(ADDR(FileJob, writeTrashInfo), writeTrashInfolamda);
    EXPECT_FALSE(job->moveFileToTrash(QString("mnfeingienfjdj"), &targetPath));
}

TEST_F(FileJobTest, start_moveDirToTrash)
{
    EXPECT_NO_FATAL_FAILURE(job->setStatus(FileJob::Cancelled));
    QString targetPath;
    source.setPath(TestHelper::createTmpDir());
    EXPECT_FALSE(job->moveDirToTrash(source.toLocalFile(), &targetPath));
    EXPECT_NO_FATAL_FAILURE(job->setStatus(FileJob::Run));
    Stub stl;
    bool (*writeTrashInfolamda)(void *, const QString &, const QString &, const QString &) = []
    (void *, const QString &, const QString &, const QString &) {
        return true;
    };
    stl.set(ADDR(FileJob, writeTrashInfo), writeTrashInfolamda);
    EXPECT_TRUE(job->moveDirToTrash(source.toLocalFile(), &targetPath));
    bool (*writeTrashInfolamda1)(void *, const QString &, const QString &, const QString &) = []
    (void *, const QString &, const QString &, const QString &) {
        return false;
    };
    stl.reset(ADDR(FileJob, writeTrashInfo));
    stl.set(ADDR(FileJob, writeTrashInfo), writeTrashInfolamda1);
    EXPECT_FALSE(job->moveDirToTrash(source.toLocalFile(), &targetPath));
    TestHelper::deleteTmpFile(source.toLocalFile());
}

TEST_F(FileJobTest, start_deleteEmptyDir){
    QProcess::execute("mkdir " + QDir::currentPath()+"/start_deleteEmptyDir");
    source.setPath(QDir::currentPath()+"/start_deleteEmptyDir");
    QProcess::execute("mkdir "+ source.toLocalFile() + "/temp1");
    QProcess::execute("mkdir "+ source.toLocalFile() + "/temp2");
    QProcess::execute("touch "+ source.toLocalFile() + "/temp1/teste");
    EXPECT_NO_FATAL_FAILURE(job->deleteEmptyDir(source.toLocalFile()));
    QProcess::execute("rm -rf " + QDir::currentPath()+"/start_deleteEmptyDir");
}


TEST_F(FileJobTest, start_deleteDir){
    QProcess::execute("mkdir " + QDir::currentPath()+"/start_deleteDir");
    source.setPath(QDir::currentPath()+"/start_deleteDir");
    QProcess::execute("mkdir "+ source.toLocalFile() + "/temp1");
    QProcess::execute("mkdir "+ source.toLocalFile() + "/temp2");
    QProcess::execute("touch "+ source.toLocalFile() + "/temp1/teste");

    bool (*deleteFilelamda)(const QString &) = [](const QString &){return false;};
    {
        Stub stl;
        stl.set(ADDR(FileJob,deleteFile),deleteFilelamda);

        EXPECT_NO_FATAL_FAILURE(job->setStatus(FileJob::Cancelled));
        EXPECT_FALSE(job->deleteDir(source.toLocalFile()));
        EXPECT_NO_FATAL_FAILURE(job->setStatus(FileJob::Run));
        EXPECT_FALSE(job->deleteDir(source.toLocalFile()));
    }
    QProcess::execute("mkdir "+ source.toLocalFile() + "/temp1");
    QProcess::execute("mkdir "+ source.toLocalFile() + "/temp2");
    QProcess::execute("touch "+ source.toLocalFile() + "/temp1/teste");
    {
        Stub stl;
        stl.set(ADDR(FileJob,deleteDir),deleteFilelamda);
        EXPECT_FALSE(job->deleteDir(source.toLocalFile()));
    }
    EXPECT_TRUE(job->deleteDir(source.toLocalFile()));

    bool (*rmdirlamda)(const QString &) = [](const QString &){return false;};
    {
        Stub stl;
        stl.set(ADDR(QDir,rmdir),rmdirlamda);
        QProcess::execute("mkdir "+source.toLocalFile());
        EXPECT_FALSE(job->deleteDir(source.toLocalFile()));
    }
    QProcess::execute("rm -rf " + QDir::currentPath()+"/start_deleteDir");
}

TEST_F(FileJobTest, start_getNotExistsTrashFileName){
    QProcess::execute("mkdir " + QDir::currentPath()+"/start_getNotExistsTrashFileName");
    QProcess::execute("touch " + QDir::currentPath()+"/start_getNotExistsTrashFileName/11.txt");
    source.setPath(QDir::currentPath()+"/start_getNotExistsTrashFileName/11.txt");
    EXPECT_FALSE(job->getNotExistsTrashFileName(source.toLocalFile()).isEmpty());
    QProcess::execute("rm -rf " + QDir::currentPath()+"/start_getNotExistsTrashFileName");
}

TEST_F(FileJobTest, start_getStorageInfo){
    QProcess::execute("mkdir " + QDir::currentPath()+"/start_getStorageInfo");
    QProcess::execute("touch " + QDir::currentPath()+"/start_getStorageInfo/11.txt");
    source.setPath(QDir::currentPath()+"/start_getStorageInfo/11.txt");
    EXPECT_TRUE(job->getStorageInfo(source.toLocalFile()).isValid());
    QProcess::execute("rm -rf " + QDir::currentPath()+"/start_getStorageInfo");
}
