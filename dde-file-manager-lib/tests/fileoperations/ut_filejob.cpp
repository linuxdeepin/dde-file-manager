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
#define private public
#include "fileoperations/filejob.h"
#include "dialogs/dialogmanager.h"
#include "dialogs/dtaskdialog.h"

using namespace testing;
using namespace stub_ext;
DFM_USE_NAMESPACE

class FileJobTest:public testing::Test{

public:

    QSharedPointer<FileJob> job;
    virtual void SetUp() override{
        job.reset(new FileJob(FileJob::Copy));
        source.setScheme(FILE_SCHEME);
        dst.setScheme(FILE_SCHEME);
        Stub stl;
        void (*delayRemoveTask)(const QMap<QString, QString> &) = []
                (const QMap<QString, QString> &){};
        stl.set(ADDR(DTaskDialog,delayRemoveTask),delayRemoveTask);
        stl.set(ADDR(DTaskDialog,addTask),delayRemoveTask);
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
    source.setPath(TestHelper::createTmpDir());
    DUrl urldir,urlfile,urlfilecopy;
    urldir = source;
    urlfile = source;
    urlfilecopy = source;
    urldir.setPath(source.toLocalFile()+"(copy)");
    QProcess::execute("mkdir "+urldir.toLocalFile());
    EXPECT_FALSE(job->checkDuplicateName(source.toLocalFile()).isEmpty());
    urlfile.setPath(TestHelper::createTmpFile());
    urlfilecopy.setPath(urlfile.toLocalFile()+"(copy)");
    QProcess::execute("touch "+urlfilecopy.toLocalFile());
    EXPECT_FALSE(job->checkDuplicateName(urlfile.toLocalFile()).isEmpty());
    DUrl urlfiletxt,urlfiletxtcopy;
    urlfiletxt = source;
    urlfiletxt.setPath("/tmp/zut_filejob_check_oo.txt");
    urlfiletxtcopy = source;
    urlfiletxtcopy.setPath("/tmp/zut_filejob_check_oo(copy).txt");
    QProcess::execute(QString("touch "+urlfiletxt.toLocalFile() + QString(" ") + urlfiletxtcopy.toLocalFile()));
    EXPECT_FALSE(job->checkDuplicateName(urlfiletxt.toLocalFile()).isEmpty());
    TestHelper::deleteTmpFiles(QStringList() << source.toLocalFile() << urldir.toLocalFile()
                               << urlfile.toLocalFile() << urlfilecopy.toLocalFile()
                               << urlfiletxt.toLocalFile() << urlfiletxtcopy.toLocalFile());
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
    Stub stl;
    void (*showNoPermissionDialog)(const DFMUrlListBaseEvent &) = [](const DFMUrlListBaseEvent &){};
    stl.set(ADDR(DialogManager,showNoPermissionDialog),showNoPermissionDialog);

    DUrlList (*doMoveCopyJob)(const DUrlList &, const DUrl &) = []
            (const DUrlList &, const DUrl &){return DUrlList();};
    stl.set(ADDR(FileJob,doMoveCopyJob),doMoveCopyJob);
    void (*clear)(void*) = [](void *){};
    stl.set(ADDR(QList<DUrl>,clear),clear);
    job->m_noPermissonUrls.append(DUrl());

    EXPECT_TRUE(job->doCopy(DUrlList() << source, dst).isEmpty());
}

TEST_F(FileJobTest,start_doMove) {
    source.setPath("/bin");
    DUrl url(source);
    url.setPath(TestHelper::createTmpFile());
    dst.setPath(TestHelper::createTmpDir());
    Stub stl;
    void (*showNoPermissionDialog)(const DFMUrlListBaseEvent &) = [](const DFMUrlListBaseEvent &){};
    stl.set(ADDR(DialogManager,showNoPermissionDialog),showNoPermissionDialog);
    DUrlList (*doMoveCopyJob)(const DUrlList &, const DUrl &) = []
            (const DUrlList &, const DUrl &){return DUrlList();};
    stl.set(ADDR(FileJob,doMoveCopyJob),doMoveCopyJob);
    EXPECT_TRUE(job->doMove(DUrlList() << source << url, dst).isEmpty());
    TestHelper::deleteTmpFiles(QStringList() << url.toLocalFile() << dst.toLocalFile());
}
QString rootPath(){
    return QString("www");
}
static int getMountPointUrlnew = 0;
DUrl getMountPointUrl(){
    if (getMountPointUrlnew){
        return DUrl::fromLocalFile("~/");
    }
    getMountPointUrlnew++;
    return DUrl();
}
TEST_F(FileJobTest,start_doMoveCopyJob) {
    Stub stl;
    source.setPath(TestHelper::createTmpFile());
    dst.setPath(TestHelper::createTmpDir());
    DUrl tmpdst(dst),linkurl(source),copydir(dst);
    tmpdst.setPath("/tmp/unexits_filejob");
    bool (*checkDiskSpaceAvailable)(const DUrlList &, const DUrl &) = []
            (const DUrlList &, const DUrl &){
        return false;
    };
    linkurl.setPath(linkurl.toLocalFile()+"sys_link");
    copydir.setPath(TestHelper::createTmpDir());
    QProcess::execute("ln -s "+ source.toLocalFile() + " " + linkurl.toLocalFile());
    stl.set(ADDR(FileJob,checkDiskSpaceAvailable),checkDiskSpaceAvailable);
    void (*showDiskSpaceOutOfUsedDialogLater)(void *) = [](void *){};
    stl.set(ADDR(DialogManager,showDiskSpaceOutOfUsedDialogLater),showDiskSpaceOutOfUsedDialogLater);
    void (*removeTaskImmediately)(const QMap<QString, QString> &) = [](const QMap<QString, QString> &){};
    stl.set(ADDR(DTaskDialog,removeTaskImmediately),removeTaskImmediately);
    EXPECT_TRUE(job->doMoveCopyJob(DUrlList(), dst).isEmpty());

    stl.reset(ADDR(FileJob,checkDiskSpaceAvailable));
    EXPECT_TRUE(job->doMoveCopyJob(DUrlList() << source, tmpdst).isEmpty());


    bool (*isGvfsMountFile)(const QString &, const bool &) = []
            (const QString &, const bool &){return true;};
    stl.set(ADDR(FileUtils,isGvfsMountFile),isGvfsMountFile);
    UDiskDeviceInfoPointer (*getDeviceByFilePath)(const QString &, const bool) = []
            (const QString &, const bool){return UDiskDeviceInfoPointer(new UDiskDeviceInfo());};
    stl.set(ADDR(UDiskListener,getDeviceByFilePath),getDeviceByFilePath);
    stl.set(ADDR(UDiskDeviceInfo,getMountPointUrl),getMountPointUrl);
    stl.set(ADDR(QStorageInfo,rootPath),&rootPath);
    EXPECT_TRUE(job->doMoveCopyJob(DUrlList() << source, tmpdst).isEmpty());
    stl.reset(ADDR(QStorageInfo,rootPath));
    stl.reset(ADDR(FileUtils,isGvfsMountFile));
    job->setIsAborted(true);
    EXPECT_TRUE(job->doMoveCopyJob(DUrlList() << DUrl() << source << linkurl, dst).isEmpty());

    job->setIsAborted(false);
    EXPECT_FALSE(job->doMoveCopyJob(DUrlList() << DUrl() << source << linkurl << copydir, dst).isEmpty());

    job.reset(new FileJob(FileJob::Move));
    EXPECT_TRUE(job->doMoveCopyJob(DUrlList(), tmpdst).isEmpty());

    bool (*moveDir)(const QString &, const QString &, QString *) = []
            (const QString &, const QString &, QString *){return false;};
    stl.set(ADDR(FileJob,moveDir),moveDir);
    stl.set(ADDR(FileJob,moveFile),moveDir);
    bool (*copyFile)(const QString &, const QString &, QString *) = []
            (const QString &, const QString &, QString *){return true;};
    stl.set(ADDR(FileJob,copyFile),copyFile);
    bool (*deleteFile)(const QString &) = [](const QString &){return true;};
    stl.set(ADDR(FileJob,deleteFile),deleteFile);
    stl.set(ADDR(FileJob,deleteDir),deleteFile);
    stl.set(ADDR(FileJob,copyDir),copyFile);
    EXPECT_FALSE(job->doMoveCopyJob(DUrlList() << source << linkurl << copydir, dst).isEmpty());
    job->m_isInSameDisk = false;
    job->setIsSkip(false);
    job->jobAdded();
    EXPECT_FALSE(job->doMoveCopyJob(DUrlList() << DUrl() << source << linkurl << copydir, dst).isEmpty());

    job.reset(new FileJob(FileJob::Trash));
    bool (*isVaultFile)(QString ) = [](QString ){return true;};
    stl.set(ADDR(VaultController,isVaultFile),isVaultFile);
    EXPECT_TRUE(job->doMoveCopyJob(DUrlList() << DUrl() << source << copydir, dst).isEmpty());
    stl.reset(ADDR(VaultController,isVaultFile));

    DUrl dstfileurl(source),dstdirurl(copydir);
    dstfileurl.setPath(dst.toLocalFile() + "/ut_tst_file");
    dstdirurl.setPath(dst.toLocalFile() + "/ut_tst_dir");
    QProcess::execute("touch "+dstfileurl.toLocalFile());
    QProcess::execute("mkdir "+dstdirurl.toLocalFile());
    EXPECT_TRUE(job->doMoveCopyJob(DUrlList() << dstfileurl << dstdirurl, dst).isEmpty());

    bool (*moveDirToTrash)(const QString &, QString *) = [](const QString &, QString *){return true;};
    stl.set(ADDR(FileJob,moveDirToTrash),moveDirToTrash);
    stl.set(ADDR(FileJob,moveFileToTrash),moveDirToTrash);
    job->m_isInSameDisk = true;
    bool (*rename)(const QString &, const QString &) = [](const QString &, const QString &){return false;};
    stl.set(ADDR(QDir,rename),rename);
    bool (*renamefile)(const QString &) = [](const QString &){return false;};
    stl.set((bool (QFile::*)(const QString &))ADDR(QFile,rename),renamefile);
    int (*execute)(const QString &) = [](const QString &){return 1;};
    stl.set((int (*)(const QString &))ADDR(QProcess,execute),execute);
    EXPECT_FALSE(job->doMoveCopyJob(DUrlList() << source << copydir, dst).isEmpty());
    stl.reset((int (*)(const QString &))ADDR(QProcess,execute));

    EXPECT_FALSE(job->doMoveCopyJob(DUrlList() << DUrl() << source << copydir, dst).isEmpty());

    TestHelper::deleteTmpFiles(QStringList() << source.toLocalFile() << dst.toLocalFile() <<
                               linkurl.toLocalFile() << dstfileurl.toLocalFile() <<
                               dstdirurl.toLocalFile());
}

TEST_F(FileJobTest,start_doDelete) {
    source.setPath(TestHelper::createTmpFile());
    dst.setPath(TestHelper::createTmpDir());
    job->jobAdded();
    Stub stl;
    void (*clear)(void*) = [](void *){};
    stl.set(ADDR(QList<DUrl>,clear),clear);
    job->m_noPermissonUrls.append(DUrl());
    void (*showNoPermissionDialog)(const DFMUrlListBaseEvent &) = []
            (const DFMUrlListBaseEvent &){};
    stl.set(ADDR(DialogManager,showNoPermissionDialog),showNoPermissionDialog);
    EXPECT_NO_FATAL_FAILURE(job->doDelete(DUrlList()));
    stl.reset(ADDR(QList<DUrl>,clear));

    bool (*deleteDir)(const QString &) = [](const QString &){return false;};
    stl.set(ADDR(FileJob,deleteDir),deleteDir);
    EXPECT_NO_FATAL_FAILURE(job->doDelete(DUrlList() << DUrl() << source << dst));

    TestHelper::deleteTmpFiles(QStringList() << source.toLocalFile() << dst.toLocalFile());
}

TEST_F(FileJobTest,start_doMoveToTrash) {

    bool (*mkpath)(void *,const QString &) = [](void *,const QString &){
        return false;
    };
    Stub stl;
    stl.set(ADDR(QDir,mkpath),mkpath);

    EXPECT_TRUE(job->doMoveToTrash(DUrlList()).isEmpty());
    bool (*mkpath1)(void *,const QString &) = [](void *,const QString &str){
        if (str == DFMStandardPaths::location(DFMStandardPaths::TrashInfosPath)) {
            return false;
        }
        return true;
    };
    stl.set(ADDR(QDir,mkpath),mkpath1);
    EXPECT_TRUE(job->doMoveToTrash(DUrlList()).isEmpty());

    stl.reset(ADDR(QDir,mkpath));
    bool (*checkTrashFileOutOf1GB)(const DUrl &) = [](const DUrl &){return false;};
    stl.set(ADDR(FileJob,checkTrashFileOutOf1GB),checkTrashFileOutOf1GB);
    void (*showMoveToTrashConflictDialog)(const DUrlList &) = [](const DUrlList &){};
    stl.set(ADDR(DialogManager,showMoveToTrashConflictDialog),showMoveToTrashConflictDialog);
    job->jobAdded();
    EXPECT_TRUE(job->doMoveToTrash(DUrlList() << DUrl()).isEmpty());

    stl.reset(ADDR(FileJob,checkTrashFileOutOf1GB));
    DUrlList (*doMove)(const DUrlList &, const DUrl &) = []
            (const DUrlList &, const DUrl &){return DUrlList();};
    stl.set(ADDR(FileJob,doMove),doMove);
    EXPECT_TRUE(job->doMoveToTrash(DUrlList() << DUrl()).isEmpty());


}
TEST_F(FileJobTest, start_doTrashRestore){
    source.setPath(TestHelper::createTmpFile());
    bool (*restoreTrashFile)(const QString &, const QString &) = []
            (const QString &, const QString &){return true;};
    Stub stl;
    stl.set(ADDR(FileJob,restoreTrashFile),restoreTrashFile);
    DUrl linkurl(source),tmpdir(source);
    linkurl.setPath(linkurl.toLocalFile()+"sys_link");
    QProcess::execute("ln -s "+ source.toLocalFile() + " " + linkurl.toLocalFile());
    tmpdir.setPath(TestHelper::createTmpDir());
    DUrlList (*doMove)(const DUrlList &, const DUrl &) = []
            (const DUrlList &, const DUrl &){return DUrlList() << DUrl();};
    stl.set(ADDR(FileJob,doMove),doMove);
    job->m_isJobAdded = true;
    job->m_isManualRemoveJob = false;
    EXPECT_TRUE(job->doTrashRestore(linkurl.toLocalFile(),dst.toLocalFile()));

    bool (*copyDir)(const QString &, const QString &, QString *) = []
            (const QString &, const QString &, QString *){return true;};
    stl.set(ADDR(FileJob,copyDir),copyDir);
    EXPECT_TRUE(job->doTrashRestore(tmpdir.toLocalFile(),dst.toLocalFile()));

    stl.set(ADDR(FileJob,copyFile),copyDir);
    EXPECT_TRUE(job->doTrashRestore(source.toLocalFile(),dst.toLocalFile()));

    stl.set(ADDR(FileJob,restoreTrashFile),restoreTrashFile);
    EXPECT_FALSE(job->doTrashRestore(source.toLocalFile(),tmpdir.toLocalFile()));
    TestHelper::deleteTmpFiles(QStringList() << source.toLocalFile() << linkurl.toLocalFile()
                               << tmpdir.toLocalFile());
}

static int mediaChangeDetected1new = 0;
bool mediaChangeDetected1(void *)
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
    void (*unmount)(const QVariantMap &) = [](const QVariantMap &){};
    Stub stl;
    stl.set(ADDR(DBlockDevice,unmount),unmount);
    bool (*opticalBlank)(void *) = [](void *){return true;};
    stl.set(ADDR(DDiskDevice,opticalBlank),opticalBlank);
    bool (*ghostSignal)(const DUrl &, DAbstractFileWatcher::SignalType1, const DUrl &) = []
            (const DUrl &, DAbstractFileWatcher::SignalType1, const DUrl &){return false;};
    stl.set((bool (*)(const DUrl &, DAbstractFileWatcher::SignalType1, const DUrl &))
            ADDR(DAbstractFileWatcher,ghostSignal),ghostSignal);
    bool (*mediaChangeDetected)(void *) = [](void *){return false;};
    stl.set(ADDR(DDiskDevice,mediaChangeDetected),mediaChangeDetected);
    void (*handleOpticalJobFailure)(int, const QString &, const QStringList &) = []
            (int, const QString &, const QStringList &){};
    stl.set(ADDR(FileJob,handleOpticalJobFailure),handleOpticalJobFailure);
    job->m_isJobAdded = true;
    EXPECT_NO_FATAL_FAILURE(job->doOpticalBlank(url));


    stl.set(ADDR(DDiskDevice,mediaChangeDetected),&mediaChangeDetected1);
    void (*opticalJobUpdatedByParentProcess)(int, int,const QString &,const QStringList &) = []
            (int , int , const QString &,const QStringList &){};
    stl.set(ADDR(FileJob,opticalJobUpdatedByParentProcess),opticalJobUpdatedByParentProcess);
    EXPECT_NO_FATAL_FAILURE(job->doOpticalBlank(url));
}

static int pipe2new = 0;
int pipe3(int[]){
    if (0 == pipe2new) {
        pipe2new++;
        return 0;
    }
    return -1;
}
TEST_F(FileJobTest, start_doOpticalBurnByChildProcess)
{
    DUrl url = DUrl::fromLocalFile("/dev/sr*");
    DISOMasterNS::BurnOptions opts;
    opts |= DISOMasterNS::BurnOption::VerifyDatas;
    EXPECT_NO_FATAL_FAILURE(job->doOpticalBurnByChildProcess(DUrl(), "test", 10, opts));

    QStringList (*resolveDeviceNode)(QString, QVariantMap) = [](QString, QVariantMap){return QStringList();};
    Stub stl;

    stl.set(ADDR(DDiskManager,resolveDeviceNode),resolveDeviceNode);
    EXPECT_NO_FATAL_FAILURE(job->doOpticalBurnByChildProcess(url, "test", 10, opts));

    stl.reset(ADDR(DDiskManager,resolveDeviceNode));
    bool (*opticalBlank)(void *) = [](void *){return false;};
    stl.set(ADDR(DDiskDevice,opticalBlank),opticalBlank);
    bool (*ghostSignal)(const DUrl &, DAbstractFileWatcher::SignalType3, const DUrl &, const int) = []
            (const DUrl &, DAbstractFileWatcher::SignalType3, const DUrl &, const int){return true;};
    stl.set((bool (*)(const DUrl &, DAbstractFileWatcher::SignalType3, const DUrl &, const int))\
            ADDR(DAbstractFileWatcher,ghostSignal),ghostSignal);
    int (*pipe1)(int[]) = [](int[]){return -1;};
    stl.set(pipe,pipe1);
    QStringList (*resolveDeviceNode1)(QString, QVariantMap) = [](QString, QVariantMap){
        return QStringList() << QString("test");
    };
    stl.set(ADDR(DDiskManager,resolveDeviceNode),resolveDeviceNode1);
    EXPECT_NO_FATAL_FAILURE(job->doOpticalBurnByChildProcess(url, "test", 10, opts));

    void (*unmount)(const QVariantMap &) = [](const QVariantMap &){};
    stl.set(ADDR(DBlockDevice,unmount),unmount);
    bool (*opticalBlank1)(void *) = [](void *){return true;};
    stl.set(ADDR(DDiskDevice,opticalBlank),opticalBlank1);
    stl.set(pipe,&pipe3);
    EXPECT_NO_FATAL_FAILURE(job->doOpticalBurnByChildProcess(url, "test", 10, opts));

    stl.reset(pipe);
    TestHelper::runInLoop([=](){
        EXPECT_NO_FATAL_FAILURE(job->doOpticalBurnByChildProcess(url, "test", 10, opts));
    },3000);

}

TEST_F(FileJobTest, doOpticalImageBurnByChildProcess)
{
    DUrl url = DUrl::fromLocalFile("/dev/sr*");
    DUrl image;
    DISOMasterNS::BurnOptions opts;
    opts |= DISOMasterNS::BurnOption::VerifyDatas;
    EXPECT_NO_FATAL_FAILURE(job->doOpticalImageBurnByChildProcess(url, image, 10, opts));
}

TEST_F(FileJobTest, opticalJobUpdated)
{
    DISOMasterNS::DISOMaster master;
    int status = DISOMasterNS::DISOMaster::JobStatus::Failed;
    int progress = 50;

    EXPECT_NO_FATAL_FAILURE(job->opticalJobUpdated(&master, status, progress));

    status = DISOMasterNS::DISOMaster::JobStatus::Running;
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
    EXPECT_NO_FATAL_FAILURE(job->opticalJobUpdatedByParentProcess(status, progress, speed, msgs));

    status = DISOMasterNS::DISOMaster::JobStatus::Running;
    EXPECT_NO_FATAL_FAILURE(job->opticalJobUpdatedByParentProcess(status, progress, speed, msgs));

    status = DISOMasterNS::DISOMaster::JobStatus::Stalled;
    EXPECT_NO_FATAL_FAILURE(job->opticalJobUpdatedByParentProcess(status, progress, speed, msgs));
}

TEST_F(FileJobTest, start_copyFile) {
    job->m_isGvfsFileOperationUsed = true;
    source.setPath(TestHelper::createTmpFile());
    dst.setPath(TestHelper::createTmpDir());
    EXPECT_NO_FATAL_FAILURE(job->copyFile(source.toLocalFile(),dst.toLocalFile()));
    job->m_isGvfsFileOperationUsed = false;
    QtConcurrent::run([=](){
        job->setStatus(FileJob::Started);
        EXPECT_NO_FATAL_FAILURE(job->copyFile(source.toLocalFile(),dst.toLocalFile()));
    });
    QThread::msleep(100);
    job->setStatus(FileJob::Cancelled);
    QThread::msleep(100);
    QtConcurrent::run([=](){
        EXPECT_NO_FATAL_FAILURE(job->setStatus(FileJob::Run));
        EXPECT_NO_FATAL_FAILURE(job->copyFile(source.toLocalFile(),dst.toLocalFile()));
    });
    QThread::msleep(100);
    job->setStatus(FileJob::Started);
    QThread::msleep(100);
    QtConcurrent::run([=](){
        job->setStatus(FileJob::Run);
        EXPECT_NO_FATAL_FAILURE(job->copyFile(source.toLocalFile(),dst.toLocalFile()));
    });
    QThread::msleep(100);
    job->setStatus(FileJob::Cancelled);
    QThread::msleep(100);
    TestHelper::deleteTmpFiles(QStringList() << dst.toLocalFile() << source.toLocalFile());
}

TEST_F(FileJobTest, start_) {
    source.setPath("/run/user/1000/gvfs/smb-share:server=10.8.11.190,share=test");
    dst.setPath("~/Desktop");
    EXPECT_TRUE(job->checkUseGvfsFileOperation(DUrlList() << source << DUrl(), dst));
    dst.setPath("/run/user/1000/gvfs/smb-share:server=10.8.11.190,share=test");
    EXPECT_TRUE(job->checkUseGvfsFileOperation(DUrlList() << source << DUrl(), dst));
}

TEST_F(FileJobTest, start_checkDiskSpaceAvailable) {
    source.setPath(TestHelper::createTmpFile());
    dst.setPath(TestHelper::createTmpDir());
    Stub stl;
    bool (*isGvfsMountFile)(void *,const QString &, const bool &) = [](void *,const QString &, const bool &){return true;};
    stl.set(ADDR(FileUtils,isGvfsMountFile),isGvfsMountFile);
    EXPECT_TRUE(job->checkDiskSpaceAvailable(DUrlList() << source,dst));
    stl.reset(ADDR(FileUtils,isGvfsMountFile));
    qint64 (*totalSize)(void *,const DUrlList &, const qint64 &, bool &)= []\
            (void *,const DUrlList &, const qint64 &, bool &){
        qint64 oo = 10000;
        return oo;
    };
    stl.set(static_cast<qint64 (*)(const DUrlList &, const qint64 &, bool &)>(&FileUtils::totalSize),totalSize);
    EXPECT_TRUE(job->checkDiskSpaceAvailable(DUrlList() << source,dst));
    TestHelper::deleteTmpFiles(QStringList() << source.toLocalFile() << dst.toLocalFile());
}

TEST_F(FileJobTest, start_checkTrashFileOutOf1GB) {
    source.setPath(TestHelper::createTmpFile());
    dst.setPath(source.parentUrl().toLocalFile()+"/syslink_tmp");
    QProcess::execute("ln -s " + source.toLocalFile() + " " + dst.toLocalFile());
    EXPECT_TRUE(job->checkTrashFileOutOf1GB(dst));
    EXPECT_TRUE(job->checkTrashFileOutOf1GB(source));
    TestHelper::deleteTmpFiles(QStringList() << dst.toLocalFile() << source.toLocalFile());
}

TEST_F(FileJobTest, start_checkFat32FileOutof4G) {
    source.setPath(TestHelper::createTmpFile());
    dst.setPath(TestHelper::createTmpDir());

    Stub stl;
    void (*show4gFat32Dialog)(void *) = [](void *){};
    stl.set(ADDR(DialogManager,show4gFat32Dialog),show4gFat32Dialog);
    qint64 (*size)(void*) = [](void *){
        qint64 tmpsize = 6l*1024*1024*1024;
        return tmpsize;
    };
    stl.set(ADDR(QFileInfo,size),size);
    EXPECT_FALSE(job->checkFat32FileOutof4G(source.toLocalFile(),dst.toLocalFile()));
    UDiskDeviceInfoPointer (*getDeviceByPath)(void *,const QString &) = [](void *,const QString &){
        return UDiskDeviceInfoPointer(new UDiskDeviceInfo());
    };
    stl.set(ADDR(UDiskListener,getDeviceByPath),getDeviceByPath);
    EXPECT_FALSE(job->checkFat32FileOutof4G(source.toLocalFile(),dst.toLocalFile()));
    stl.reset(ADDR(UDiskListener,getDeviceByPath));
    stl.set(ADDR(UDiskListener,getDeviceByFilePath),getDeviceByPath);
    QStringList (*resolveDeviceNode)(QString , QVariantMap ) = [](QString , QVariantMap){
        return QStringList() << "vfat";
    };
    stl.set(ADDR(DDiskManager,resolveDeviceNode),resolveDeviceNode);
    QString (*idType)(void *) = [](void *){return QString("vfat");};
    stl.set(ADDR(DBlockDevice,idType),idType);
    EXPECT_TRUE(job->checkFat32FileOutof4G(source.toLocalFile(),dst.toLocalFile()));
    TestHelper::deleteTmpFiles(QStringList() << dst.toLocalFile() << source.toLocalFile());
}

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
    source.setPath(TestHelper::createTmpFile());
    Stub stl;
    bool (*isVaultFile)(QString) = [](QString){return true;};
    stl.set(ADDR(VaultController,isVaultFile),isVaultFile);
    QFileDevice::Permissions (*getPermissions)(QString) = [](QString){
        QFileDevice::Permissions ooo;
        return ooo;
    };
    stl.set(ADDR(VaultController,getPermissions),getPermissions);
    EXPECT_FALSE(job->canMove(source.toLocalFile()));
    stl.reset(ADDR(VaultController,getPermissions));
    stl.reset(ADDR(VaultController,getPermissions));
    __uid_t (*getuidtmp) (void*) = [](void*){
        __uid_t tid = 0;
        return tid;
    };
    stl.set(getuid,getuidtmp);
    EXPECT_TRUE(job->canMove(source.toLocalFile()));
    stl.reset(getuid);
    uint (*ownerId)(void *) = [](void *){
        uint tmpid = 0;
        return tmpid;
    };
    stl.set(ADDR(QFileInfo,ownerId),ownerId);
    EXPECT_FALSE(job->canMove(source.toLocalFile()));
    TestHelper::deleteTmpFiles(QStringList() << dst.toLocalFile() << source.toLocalFile());
}

TEST_F(FileJobTest, start_writeTrashInfo){
//    Stub stl;
//    bool (*opentmp)(QIODevice::OpenMode) = [](QIODevice::OpenMode) {return false;};
//    typedef bool (*fptr)(void *,QIODevice::OpenMode);
//    fptr A_foo = (fptr)(&QFile::open);   //obtaining an address
//    stl.set(A_foo,opentmp);
    EXPECT_TRUE(job->writeTrashInfo("ifue_98jfei_8e8f","jfaskjdf","kfek"));
}

TEST_F(FileJobTest, start_moveFileToTrash){
    EXPECT_NO_FATAL_FAILURE(job->setStatus(FileJob::Cancelled));
    QString targetPath;
    EXPECT_FALSE(job->moveFileToTrash(QString("jhfenfd"),&targetPath));
    EXPECT_NO_FATAL_FAILURE(job->setStatus(FileJob::Run));
    bool (*writeTrashInfo)(void *,const QString &, const QString &, const QString &) =[]
            (void *,const QString &, const QString &, const QString &)
    {
        return false;
    };
    EXPECT_TRUE(job->moveFileToTrash(QString("mnfeingienfjdj"),&targetPath));
    Stub stl;
    stl.set(ADDR(FileJob,writeTrashInfo),writeTrashInfo);
    EXPECT_FALSE(job->moveFileToTrash(QString("mnfeingienfjdj"),&targetPath));
}

TEST_F(FileJobTest, start_moveDirToTrash){
    EXPECT_NO_FATAL_FAILURE(job->setStatus(FileJob::Cancelled));
    QString targetPath;
    source.setPath(TestHelper::createTmpDir());
    EXPECT_FALSE(job->moveFileToTrash(source.toLocalFile(),&targetPath));
    EXPECT_NO_FATAL_FAILURE(job->setStatus(FileJob::Run));
    bool (*writeTrashInfo)(void *,const QString &, const QString &, const QString &) =[]
            (void *,const QString &, const QString &, const QString &)
    {
        return false;
    };
    EXPECT_TRUE(job->moveFileToTrash(source.toLocalFile(),&targetPath));
    Stub stl;
    stl.set(ADDR(FileJob,writeTrashInfo),writeTrashInfo);
    EXPECT_FALSE(job->moveFileToTrash(source.toLocalFile(),&targetPath));
    TestHelper::deleteTmpFile(source.toLocalFile());
}

TEST_F(FileJobTest, start_deleteEmptyDir){
    source.setPath(TestHelper::createTmpDir());
    QProcess::execute("mkdir "+ source.toLocalFile() + "/temp1");
    QProcess::execute("mkdir "+ source.toLocalFile() + "/temp2");
    QProcess::execute("touch "+ source.toLocalFile() + "/temp1/teste");
    EXPECT_NO_FATAL_FAILURE(job->deleteEmptyDir(source.toLocalFile()));
    TestHelper::deleteTmpFile(source.toLocalFile());
}


TEST_F(FileJobTest, start_deleteDir){
    source.setPath(TestHelper::createTmpDir());
    QProcess::execute("mkdir "+ source.toLocalFile() + "/temp1");
    QProcess::execute("mkdir "+ source.toLocalFile() + "/temp2");
    QProcess::execute("touch "+ source.toLocalFile() + "/temp1/teste");
    EXPECT_NO_FATAL_FAILURE(job->setStatus(FileJob::Cancelled));
    EXPECT_FALSE(job->deleteDir(source.toLocalFile()));
    EXPECT_NO_FATAL_FAILURE(job->setStatus(FileJob::Run));
    EXPECT_TRUE(job->deleteDir(source.toLocalFile()));
    TestHelper::deleteTmpFile(source.toLocalFile());
}
