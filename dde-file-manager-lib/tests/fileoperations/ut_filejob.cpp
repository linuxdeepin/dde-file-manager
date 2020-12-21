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
    virtual void SetUp() override{
        job.reset(new FileJob(FileJob::Copy));
        source.setScheme(FILE_SCHEME);
        dst.setScheme(FILE_SCHEME);
        Stub stl;
        void (*delayRemoveTask)(const QMap<QString, QString> &) = []
                (const QMap<QString, QString> &){};
        stl.set(ADDR(DTaskDialog,delayRemoveTask),delayRemoveTask);
        stl.set(ADDR(DTaskDialog,addTask),delayRemoveTask);
        typedef int(*fptr)(QDialog*);
        fptr pQDialogExec = (fptr)(&QDialog::exec);
        int (*stub_DDialog_exec)(void) = [](void)->int{return QDialog::Rejected;};
        stl.set(pQDialogExec, stub_DDialog_exec);
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

    TestHelper::deleteTmpFiles(QStringList() << linkurl.toLocalFile() << source.toLocalFile() << dst.toLocalFile() <<
                                dstfileurl.toLocalFile() << copydir.toLocalFile() <<
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
    job->m_isInSameDisk = true;
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
    },5000);
}

TEST_F(FileJobTest, doOpticalImageBurnByChildProcess)
{
    DUrl url = DUrl::fromLocalFile("/dev/sr*");
    DUrl image;
    DISOMasterNS::BurnOptions opts;
    opts |= DISOMasterNS::BurnOption::VerifyDatas;
    EXPECT_NO_FATAL_FAILURE(job->doOpticalImageBurnByChildProcess(DUrl(), image, 10, opts));

    QStringList (*resolveDeviceNode)(QString, QVariantMap) = [](QString, QVariantMap){return QStringList();};
    Stub stl;

    stl.set(ADDR(DDiskManager,resolveDeviceNode),resolveDeviceNode);
    EXPECT_NO_FATAL_FAILURE(job->doOpticalImageBurnByChildProcess(url, image, 10, opts));
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
    EXPECT_NO_FATAL_FAILURE(job->doOpticalImageBurnByChildProcess(url, image, 10, opts));

    void (*unmount)(const QVariantMap &) = [](const QVariantMap &){};
    stl.set(ADDR(DBlockDevice,unmount),unmount);
    bool (*opticalBlank1)(void *) = [](void *){return true;};
    stl.set(ADDR(DDiskDevice,opticalBlank),opticalBlank1);
    stl.set(pipe,&pipe3);
    EXPECT_NO_FATAL_FAILURE(job->doOpticalImageBurnByChildProcess(url, image, 10, opts));

    stl.reset(pipe);
    TestHelper::runInLoop([=](){
    EXPECT_NO_FATAL_FAILURE(job->doOpticalImageBurnByChildProcess(url, image, 10, opts));
    },5000);
}

TEST_F(FileJobTest, opticalJobUpdated)
{
    DISOMasterNS::DISOMaster master;
    int status = DISOMasterNS::DISOMaster::JobStatus::Failed;
    int progress = 50;
    progress = 110;
    EXPECT_NO_FATAL_FAILURE(job->opticalJobUpdated(&master, status, progress));

    progress = 50;
    Stub stl;
    void (*handleOpticalJobFailure)(int, const QString &, const QStringList &) = []
           (int, const QString &, const QStringList &){};
    stl.set(ADDR(FileJob,handleOpticalJobFailure),handleOpticalJobFailure);
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
    Stub stl;
    void (*handleOpticalJobFailure)(int, const QString &, const QStringList &) = []
            (int, const QString &, const QStringList &){};
    stl.set(ADDR(FileJob,handleOpticalJobFailure),handleOpticalJobFailure);
    EXPECT_NO_FATAL_FAILURE(job->opticalJobUpdatedByParentProcess(status, progress, speed, msgs));
    status = DISOMasterNS::DISOMaster::JobStatus::Running;
    job->m_jobType = FileJob::OpticalImageBurn;
    job->m_opticalJobStatus = DISOMasterNS::DISOMaster::JobStatus::Finished;
    EXPECT_NO_FATAL_FAILURE(job->opticalJobUpdatedByParentProcess( status, progress, speed, msgs));

    status = DISOMasterNS::DISOMaster::JobStatus::Stalled;
    EXPECT_NO_FATAL_FAILURE(job->opticalJobUpdatedByParentProcess( status, progress, speed, msgs));
}

TEST_F(FileJobTest, start_handleOpticalJobFailure) {
    Stub stl;

    void (*showOpticalJobFailureDialog)(int, const QString &, const QStringList &) = []
            (int, const QString &, const QStringList &){};
    stl.set(ADDR(DialogManager,showOpticalJobFailureDialog),showOpticalJobFailureDialog);
    int typei = 5;
    EXPECT_NO_FATAL_FAILURE(job->handleOpticalJobFailure(typei,QString(),QStringList()));
}

TEST_F(FileJobTest, start_otherOp) {
    EXPECT_NO_FATAL_FAILURE(job->paused());
    EXPECT_NO_FATAL_FAILURE(job->started());
    EXPECT_NO_FATAL_FAILURE(job->cancelled());
    job->m_status = FileJob::Paused;
    EXPECT_NO_FATAL_FAILURE(job->handleJobFinished());
    job->m_status = FileJob::Paused;
    EXPECT_NO_FATAL_FAILURE(job->jobUpdated());
    void (*handleUpdateTaskWidget)(const QMap<QString, QString> &,const QMap<QString, QString> &) = []
            (const QMap<QString, QString> &,const QMap<QString, QString> &){};
    Stub stl;
    stl.set(ADDR(DTaskDialog,handleUpdateTaskWidget),handleUpdateTaskWidget);
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
    qint64 (*elapsed)(void *) = [](void *){return qint64(0);};
    stl.set(ADDR(QElapsedTimer,elapsed),elapsed);
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
    void (*handleTaskClose)(const QMap<QString, QString> &) = [](const QMap<QString, QString> &){};
    stl.set(ADDR(DTaskDialog,handleTaskClose),handleTaskClose);
    EXPECT_NO_FATAL_FAILURE(job->jobAborted());
    EXPECT_NO_FATAL_FAILURE(job->jobPrepared());
    job->m_applyToAll = true;
    EXPECT_NO_FATAL_FAILURE(job->jobConflicted());
    job->m_applyToAll = false;
    EXPECT_NO_FATAL_FAILURE(job->jobConflicted());
}
static int read2new = 0;
qint64 read2(char *, qint64){
    if (read2new%2 == 0) {
        read2new++;
        return 100;
    }
    read2new++;
    return 0;
}

qint64 read3(char *, qint64){
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
    source.setPath(TestHelper::createTmpFile());
    dst.setPath(TestHelper::createTmpDir());
    QString dstdirpath = dst.toLocalFile();
    DUrl linkfile(source),fileurl(source);
    linkfile.setPath(source.toLocalFile() + "sys_link");
    fileurl.setPath(TestHelper::createTmpFile());
    QProcess::execute("ln -s " + source.toLocalFile() + " " + linkfile.toLocalFile());
    Stub stl;
    bool (*copyFileByGio)(void *,const QString &, const QString &, bool, QString *) = []
            (void *,const QString &, const QString &, bool, QString *targetFile){
        *targetFile = QString("kdek");
        return true;
    };
    stl.set(ADDR(FileJob,copyFileByGio),copyFileByGio);
    QString targetpaht("");
    EXPECT_NO_FATAL_FAILURE(job->copyFile(source.toLocalFile(),dst.toLocalFile(), false, &targetpaht));
    job->m_isGvfsFileOperationUsed = false;
    bool (*checkFat32FileOutof4G)(const QString &, const QString &) = []
            (const QString &, const QString &){return true;};
    stl.set(ADDR(FileJob,checkFat32FileOutof4G),checkFat32FileOutof4G);
    EXPECT_FALSE(job->copyFile(source.toLocalFile(),dst.toLocalFile()));

    stl.reset(ADDR(FileJob,checkFat32FileOutof4G));
    job->m_isAborted = true;
    EXPECT_FALSE(job->copyFile(source.toLocalFile(),dst.toLocalFile()));

    job->m_isAborted = false;
    job->m_applyToAll = true;
    job->m_status = FileJob::Cancelled;
    EXPECT_TRUE(job->copyFile(source.toLocalFile(),dst.toLocalFile(),false,&targetpaht));

    job->m_applyToAll = false;
    job->m_skipandApplyToAll = false;
    job->m_status = FileJob::Cancelled;
    void (*jobConflicted)(void *) = [](void *){};
    stl.set(ADDR(FileJob,jobConflicted),jobConflicted);
    job->m_isSkip = true;
    EXPECT_FALSE(job->copyFile(source.toLocalFile(),dst.toLocalFile()));

    job->m_isSkip = false;
    job->m_isCoExisted = true;
    job->m_isReplaced = false;
    EXPECT_FALSE(job->copyFile("",dst.toLocalFile()));

    job->m_isCoExisted = false;
    job->m_isReplaced = true;
    dst.setPath(dstdirpath);
    QString (*checkDuplicateName)(void *,const QString &) = []
            (void *,const QString &name){return name;};
    stl.set(ADDR(FileJob,checkDuplicateName),checkDuplicateName);
    EXPECT_TRUE(job->copyFile(source.toLocalFile(),dst.toLocalFile()));

    job->m_isCoExisted = false;
    job->m_isReplaced = true;
    EXPECT_FALSE(job->copyFile(source.toLocalFile(),linkfile.toLocalFile()));

    job->m_isCoExisted = false;
    job->m_isReplaced = true;
    EXPECT_FALSE(job->copyFile(source.toLocalFile(),fileurl.toLocalFile()));

    qint64 (*read1)(char *, qint64) = [](char *, qint64){return qint64(-1);};
    stl.set((qint64 (QIODevice::*)(char *, qint64))ADDR(QIODevice,read),read1);
    stl.reset(ADDR(FileJob,checkDuplicateName));
    QFileDevice::FileError (*error1)(void *) = [](void *){return QFileDevice::ReadError;};
    stl.set(ADDR(QFileDevice,error),error1);
    EXPECT_FALSE(job->copyFile(source.toLocalFile(),dst.toLocalFile(),false,&targetpaht));


    stl.set((qint64 (QIODevice::*)(char *, qint64))ADDR(QIODevice,read),&read2);
    EXPECT_TRUE(job->copyFile(source.toLocalFile(),dst.toLocalFile(),false,&targetpaht));

    stl.set((qint64 (QIODevice::*)(char *, qint64))ADDR(QIODevice,read),&read3);
    read2new = read2new%2 == 0 ? read2new : read2new + 1;
    QtConcurrent::run([=]() {
        QThread::msleep(150);
        qDebug() << "00000" << job->m_status;
        job->m_status = FileJob::Cancelled;
        job->m_isSkip = true;
    });
    EXPECT_TRUE(job->copyFile(source.toLocalFile(),dst.toLocalFile(),false,&targetpaht));
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

    TestHelper::deleteTmpFiles(QStringList() << linkfile.toLocalFile() << dst.toLocalFile()
                               << source.toLocalFile() << fileurl.toLocalFile());

}

TEST_F(FileJobTest, start_showProgress) {
    Stub stl;
    bool (*ghostSignal)(const DUrl &, DAbstractFileWatcher::SignalType3, const DUrl &, const int) = []
            (const DUrl &, DAbstractFileWatcher::SignalType3, const DUrl &, const int){return true;};
    stl.set((bool (*)(const DUrl &, DAbstractFileWatcher::SignalType3, const DUrl &, const int))\
            ADDR(DAbstractFileWatcher,ghostSignal),ghostSignal);
    bool (*ghostSignal1)(const DUrl &, DAbstractFileWatcher::SignalType1, const DUrl &) = []
            (const DUrl &, DAbstractFileWatcher::SignalType1, const DUrl &){return true;};
    bool (*ghostSignal2)(const DUrl &, DAbstractFileWatcher::SignalType2 , const DUrl &, const DUrl &) = []
            (const DUrl &, DAbstractFileWatcher::SignalType2 , const DUrl &, const DUrl &){return true;};
    stl.set((bool (*)(const DUrl &, DAbstractFileWatcher::SignalType1, const DUrl &))\
            ADDR(DAbstractFileWatcher,ghostSignal),ghostSignal1);
    stl.set((bool (*)(const DUrl &, DAbstractFileWatcher::SignalType2 , const DUrl &, const DUrl &))\
            ADDR(DAbstractFileWatcher,ghostSignal),ghostSignal2);
    job->m_isGvfsFileOperationUsed = true;
    job->m_needGhostFileCreateSignal = true;
    EXPECT_NO_FATAL_FAILURE(job->showProgress(100,100,job.data()));
}

TEST_F(FileJobTest, start_copyFileByGio) {
    source.setPath(TestHelper::createTmpFile());
    dst.setPath(TestHelper::createTmpDir());
    QString dstdirpath = dst.toLocalFile();
    DUrl linkfile(source),fileurl(source);
    linkfile.setPath(source.toLocalFile() + "sys_link");
    fileurl.setPath(TestHelper::createTmpFile());
    QProcess::execute("ln -s " + source.toLocalFile() + " " + linkfile.toLocalFile());

    Stub stl;

    bool (*ghostSignal)(const DUrl &, DAbstractFileWatcher::SignalType3, const DUrl &, const int) = []
            (const DUrl &, DAbstractFileWatcher::SignalType3, const DUrl &, const int){return true;};
    stl.set((bool (*)(const DUrl &, DAbstractFileWatcher::SignalType3, const DUrl &, const int))\
            ADDR(DAbstractFileWatcher,ghostSignal),ghostSignal);
    bool (*ghostSignal1)(const DUrl &, DAbstractFileWatcher::SignalType1, const DUrl &) = []
            (const DUrl &, DAbstractFileWatcher::SignalType1, const DUrl &){return true;};
    bool (*ghostSignal2)(const DUrl &, DAbstractFileWatcher::SignalType2 , const DUrl &, const DUrl &) = []
            (const DUrl &, DAbstractFileWatcher::SignalType2 , const DUrl &, const DUrl &){return true;};
    stl.set((bool (*)(const DUrl &, DAbstractFileWatcher::SignalType1, const DUrl &))\
            ADDR(DAbstractFileWatcher,ghostSignal),ghostSignal1);
    stl.set((bool (*)(const DUrl &, DAbstractFileWatcher::SignalType2 , const DUrl &, const DUrl &))\
            ADDR(DAbstractFileWatcher,ghostSignal),ghostSignal2);

    bool (*checkFat32FileOutof4G)(const QString &, const QString &) = []
            (const QString &, const QString &){return true;};
    stl.set(ADDR(FileJob,checkFat32FileOutof4G),checkFat32FileOutof4G);
    EXPECT_FALSE(job->copyFileByGio(source.toLocalFile(),dstdirpath));

    stl.reset(ADDR(FileJob,checkFat32FileOutof4G));
    job->m_isAborted = true;
    EXPECT_FALSE(job->copyFileByGio(source.toLocalFile(),dst.toLocalFile()));

    QString targetpaht;

    job->m_isAborted = false;
    job->m_applyToAll = true;
    job->m_status = FileJob::Cancelled;
    QProcess::execute("touch " + dst.toLocalFile() + "/" + source.fileName());
    EXPECT_FALSE(job->copyFileByGio(source.toLocalFile(),dst.toLocalFile(),false,&targetpaht));

    job->m_applyToAll = false;
    job->m_skipandApplyToAll = false;
    job->m_status = FileJob::Cancelled;
    void (*jobConflicted)(void *) = [](void *){};
    stl.set(ADDR(FileJob,jobConflicted),jobConflicted);
    job->m_isSkip = true;
    EXPECT_TRUE(job->copyFileByGio(source.toLocalFile(),dst.toLocalFile()));

    gboolean (*g_file_copypre)(GFile *,GFile *,GFileCopyFlags ,GCancellable *,
                             GFileProgressCallback,gpointer,GError **) = []
            (GFile *,GFile *,GFileCopyFlags ,GCancellable *,
            GFileProgressCallback,gpointer,GError **error){
        *error = g_error_new(G_IO_ERROR,G_IO_ERROR_CANCELLED,"nihao");
        return gboolean(true);
    };
    stl.set(g_file_copy,g_file_copypre);
    job->m_isSkip = false;
    job->m_isCoExisted = true;
    job->m_isReplaced = false;
    EXPECT_TRUE(job->copyFileByGio("",dst.toLocalFile()));

    job->m_isCoExisted = false;
    job->m_isReplaced = true;
    dst.setPath(dstdirpath);
    QString (*checkDuplicateName)(void *,const QString &) = []
            (void *,const QString &name){return name;};
    stl.set(ADDR(FileJob,checkDuplicateName),checkDuplicateName);
    EXPECT_TRUE(job->copyFileByGio(source.toLocalFile(),dst.toLocalFile()));

    job->m_isCoExisted = true;
    job->m_isReplaced = true;
    EXPECT_TRUE(job->copyFileByGio(source.toLocalFile(),linkfile.toLocalFile()));

    job->m_isCoExisted = true;
    job->m_isReplaced = true;
    job->m_applyToAll = false;
    EXPECT_TRUE(job->copyFileByGio(source.toLocalFile(),fileurl.toLocalFile()));


    stl.reset(ADDR(FileJob,checkDuplicateName));
    gboolean (*g_file_copy1)(GFile *,GFile *,GFileCopyFlags ,GCancellable *,
                             GFileProgressCallback,gpointer,GError **) = []
            (GFile *,GFile *,GFileCopyFlags ,GCancellable *,
            GFileProgressCallback,gpointer,GError **error){
        QThread::msleep(400);
        *error = g_error_new(G_IO_ERROR,G_IO_ERROR_PERMISSION_DENIED,"nihao");
        return gboolean(false);
    };
    stl.set(g_file_copy,g_file_copy1);
    void (*showErrorDialog)(const QString &, const QString &) = []
            (const QString &, const QString &){};
    stl.set(ADDR(DialogManager,showErrorDialog),showErrorDialog);
    QFuture<void> future = QtConcurrent::run([=]() {
        QThread::msleep(150);
        qDebug() << "00000" << job->m_status;
        job->m_status = FileJob::Cancelled;
    });
    EXPECT_FALSE(job->copyFileByGio(source.toLocalFile(),dst.toLocalFile(),false,&targetpaht));

    gboolean (*g_file_copy2)(GFile *,GFile *,GFileCopyFlags ,GCancellable *,
                             GFileProgressCallback,gpointer,GError **) = []
            (GFile *,GFile *,GFileCopyFlags ,GCancellable *,
            GFileProgressCallback,gpointer,GError **error){
        QThread::msleep(400);
        *error = g_error_new(G_IO_ERROR,G_IO_ERROR_CLOSED,"nihao");
        return gboolean(false);
    };
    stl.set(g_file_copy,g_file_copy2);
    QFuture<void> future1 = QtConcurrent::run([=]() {
        QThread::msleep(150);
        qDebug() << "000001" << job->m_status;
        job->m_status = FileJob::Paused;
        QThread::msleep(300);
        qDebug() << "000002" << job->m_status;
        job->m_status = FileJob::Conflicted;
    });
    EXPECT_FALSE(job->copyFileByGio(source.toLocalFile(),dst.toLocalFile(),false,&targetpaht));

    gboolean (*g_file_copy3)(GFile *,GFile *,GFileCopyFlags ,GCancellable *,
                             GFileProgressCallback,gpointer,GError **) = []
            (GFile *,GFile *,GFileCopyFlags ,GCancellable *,
            GFileProgressCallback,gpointer,GError **error){
        *error = g_error_new(G_IO_ERROR,G_IO_ERROR_CANCELLED,"nihao");
        return gboolean(true);
    };
    stl.set(g_file_copy,g_file_copy3);
    EXPECT_TRUE(job->copyFileByGio(source.toLocalFile(),dst.toLocalFile(),false,&targetpaht));

    future.waitForFinished();
    future1.waitForFinished();

    TestHelper::deleteTmpFiles(QStringList() << linkfile.toLocalFile() << dst.toLocalFile()
                               << source.toLocalFile() << fileurl.toLocalFile());
}

TEST_F(FileJobTest, start_copyDir) {
    Stub stl;
    source.setPath(TestHelper::createTmpFile());
    dst.setPath(TestHelper::createTmpDir());
    QString dstdirpath = dst.toLocalFile();
    DUrl linkfile(source),fileurl(source);
    linkfile.setPath(source.toLocalFile() + "sys_link");
    fileurl.setPath(TestHelper::createTmpFile());
    QProcess::execute("ln -s " + source.toLocalFile() + " " + linkfile.toLocalFile());

    bool (*copyFile)(const QString &, const QString &,bool, QString *) = []
            (const QString &, const QString &,bool, QString *){return false;};
    stl.set(ADDR(FileJob,copyFile),copyFile);
    bool (*setDirPermissions)(const QString &, const QString &) = []
            (const QString &, const QString &){return false;};
    stl.set(ADDR(FileJob,setDirPermissions),setDirPermissions);
    DUrlList (*childrenList)(const DUrl &) = [](const DUrl &){
        return DUrlList() << DUrl("file:///tmp");
    };
    stl.set(ADDR(DUrl,childrenList),childrenList);
    void (*showCopyMoveToSelfDialog)(const QMap<QString, QString> &) = []
            (const QMap<QString, QString> &){};
    stl.set(ADDR(DialogManager,showCopyMoveToSelfDialog),showCopyMoveToSelfDialog);
    QString targetpaht;
    EXPECT_FALSE(job->copyDir(source.toLocalFile(),"/tmp",false,&targetpaht));

    stl.reset(ADDR(DUrl,childrenList));
    job->m_isAborted = true;
    EXPECT_FALSE(job->copyDir(source.toLocalFile(),dst.toLocalFile(),false,&targetpaht));

    job->m_isAborted = false;
    job->m_applyToAll = true;
    job->m_status = FileJob::Cancelled;
    EXPECT_FALSE(job->copyDir(source.toLocalFile(),dst.toLocalFile(),false,&targetpaht));

    job->m_applyToAll = false;
    job->m_skipandApplyToAll = false;
    job->m_status = FileJob::Cancelled;
    void (*jobConflicted)(void *) = [](void *){};
    stl.set(ADDR(FileJob,jobConflicted),jobConflicted);
    job->m_isSkip = true;
    EXPECT_TRUE(job->copyDir(source.toLocalFile(),dst.toLocalFile()));

    job->m_isSkip = false;
    job->m_isCoExisted = true;
    job->m_isReplaced = false;
    EXPECT_FALSE(job->copyDir("",dst.toLocalFile()));

    job->m_isCoExisted = false;
    job->m_isReplaced = true;
    dst.setPath(dstdirpath);
    QString (*checkDuplicateName)(void *,const QString &) = []
            (void *,const QString &name){return name;};
    stl.set(ADDR(FileJob,checkDuplicateName),checkDuplicateName);
    EXPECT_FALSE(job->copyDir(source.toLocalFile(),dst.toLocalFile(),false,&targetpaht));

    job->m_isCoExisted = false;
    job->m_isReplaced = true;
    EXPECT_FALSE(job->copyDir(source.toLocalFile(),linkfile.toLocalFile(),false,&targetpaht));

    job->m_isCoExisted = false;
    job->m_isReplaced = true;
    job->m_applyToAll = false;
    EXPECT_FALSE(job->copyDir(source.toLocalFile(),fileurl.toLocalFile(),false,&targetpaht));

    job->m_isCoExisted = false;
    job->m_isReplaced = true;
    job->m_applyToAll = false;
    bool (*mkdir)(const QString &) = [](const QString &){return false;};
    stl.set(ADDR(QDir,mkdir),mkdir);
    EXPECT_FALSE(job->copyDir(source.toLocalFile(),"/tmp/ut_copydir_filejob",false,&targetpaht));
    stl.reset(ADDR(QDir,mkdir));
    stl.reset(ADDR(FileJob,checkDuplicateName));

    char *(*savedir1)(char const *) = [](char const *){
        char * ptr = nullptr;
        return ptr;};
    stl.set(savedir,savedir1);
    EXPECT_FALSE(job->copyDir(source.toLocalFile(),dst.toLocalFile(),false,&targetpaht));
    stl.reset(savedir);
    EXPECT_FALSE(job->copyDir(source.toLocalFile(),dst.toLocalFile(),false,&targetpaht));

    bool (*handleSymlinkFile)(const QString &, const QString &, QString *) = []
            (const QString &, const QString &, QString *){return false;};
    stl.set(ADDR(FileJob,handleSymlinkFile),handleSymlinkFile);
    EXPECT_FALSE(job->copyDir(linkfile.toLocalFile(),dst.toLocalFile(),false,&targetpaht));

    TestHelper::deleteTmpFiles(QStringList() << linkfile.toLocalFile() << dst.toLocalFile()
                               << source.toLocalFile() << fileurl.toLocalFile() <<
                               dst.toLocalFile()+"(copy)" << source.toLocalFile()+"(copy)");
}

TEST_F(FileJobTest, start_moveFile) {
    Stub stl;
    bool (*handleMoveJob)(const QString &, const QString &, QString *) = []
            (const QString &, const QString &, QString *){return false;};
    stl.set(ADDR(FileJob,handleMoveJob),handleMoveJob);
    EXPECT_FALSE(job->moveFile("",""));
}
TEST_F(FileJobTest, start_moveFileByGio) {
    Stub stl;
    source.setPath(TestHelper::createTmpFile());
    dst.setPath(TestHelper::createTmpDir());
    QString dstdirpath = dst.toLocalFile();
    DUrl linkfile(source),fileurl(source),dirurl(source),targetdirurl(dst);
    linkfile.setPath(source.toLocalFile() + "sys_link");
    fileurl.setPath(TestHelper::createTmpFile());
    dirurl.setPath("/tmp/ut_jobfile_moveFileByGio");
    targetdirurl.setPath(dst.toLocalFile() + "/" + dst.fileName());
    QProcess::execute("mkdir " + dirurl.toLocalFile() + " " + targetdirurl.toLocalFile());
    QProcess::execute("ln -s " + source.toLocalFile() + " " + linkfile.toLocalFile());
    DUrlList (*childrenList)(const DUrl &) = [](const DUrl &){
        return DUrlList() << DUrl("file:///tmp/ut_jobfile_moveFileByGio");
    };
    stl.set(ADDR(DUrl,childrenList),childrenList);
    bool (*ghostSignal)(const DUrl &, DAbstractFileWatcher::SignalType3, const DUrl &, const int) = []
            (const DUrl &, DAbstractFileWatcher::SignalType3, const DUrl &, const int){return true;};
    stl.set((bool (*)(const DUrl &, DAbstractFileWatcher::SignalType3, const DUrl &, const int))\
            ADDR(DAbstractFileWatcher,ghostSignal),ghostSignal);
    bool (*ghostSignal1)(const DUrl &, DAbstractFileWatcher::SignalType1, const DUrl &) = []
            (const DUrl &, DAbstractFileWatcher::SignalType1, const DUrl &){return true;};
    bool (*ghostSignal2)(const DUrl &, DAbstractFileWatcher::SignalType2 , const DUrl &, const DUrl &) = []
            (const DUrl &, DAbstractFileWatcher::SignalType2 , const DUrl &, const DUrl &){return true;};
    stl.set((bool (*)(const DUrl &, DAbstractFileWatcher::SignalType1, const DUrl &))\
            ADDR(DAbstractFileWatcher,ghostSignal),ghostSignal1);
    stl.set((bool (*)(const DUrl &, DAbstractFileWatcher::SignalType2 , const DUrl &, const DUrl &))\
            ADDR(DAbstractFileWatcher,ghostSignal),ghostSignal2);
    QString targetpath;
    void (*showCopyMoveToSelfDialog)(const QMap<QString, QString> &) = []
            (const QMap<QString, QString> &){};
    stl.set(ADDR(DialogManager,showCopyMoveToSelfDialog),showCopyMoveToSelfDialog);
    EXPECT_TRUE(job->moveFileByGio(dst.toLocalFile(),"/tmp/ut_jobfile_moveFileByGio"));

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
    void (*jobConflicted)(void *) = [](void *){};
    stl.set(ADDR(FileJob,jobConflicted),jobConflicted);
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

    stl.reset(ADDR(FileJob,checkDuplicateName));
    gboolean (*g_file_move1)(GFile *,GFile *,GFileCopyFlags ,GCancellable *,
                             GFileProgressCallback,gpointer,GError **) = []
            (GFile *,GFile *,GFileCopyFlags ,GCancellable *,
            GFileProgressCallback,gpointer,GError **error){
        QThread::msleep(400);
        *error = g_error_new(G_IO_ERROR,G_IO_ERROR_PERMISSION_DENIED,"nihao");
        return gboolean(false);
    };
    stl.set(g_file_move,g_file_move1);
    void (*showErrorDialog)(const QString &, const QString &) = []
            (const QString &, const QString &){};
    stl.set(ADDR(DialogManager,showErrorDialog),showErrorDialog);
    QFuture<void> future = QtConcurrent::run([=]() {
        QThread::msleep(150);
        qDebug() << "00000" << job->m_status;
        job->m_status = FileJob::Cancelled;
    });
    EXPECT_TRUE(job->moveFileByGio(source.toLocalFile(),dst.toLocalFile(),&targetpath));

    gboolean (*g_file_move2)(GFile *,GFile *,GFileCopyFlags ,GCancellable *,
                             GFileProgressCallback,gpointer,GError **) = []
            (GFile *,GFile *,GFileCopyFlags ,GCancellable *,
            GFileProgressCallback,gpointer,GError **error){
        QThread::msleep(400);
        *error = g_error_new(G_IO_ERROR,G_IO_ERROR_CLOSED,"nihao");
        return gboolean(false);
    };
    stl.set(g_file_move,g_file_move2);
    QFuture<void> future1 = QtConcurrent::run([=]() {
        QThread::msleep(150);
        qDebug() << "000001" << job->m_status;
        job->m_status = FileJob::Paused;
        QThread::msleep(300);
        qDebug() << "000002" << job->m_status;
        job->m_status = FileJob::Conflicted;
    });
    EXPECT_TRUE(job->moveFileByGio(source.toLocalFile(),dst.toLocalFile(),&targetpath));

    gboolean (*g_file_move3)(GFile *,GFile *,GFileCopyFlags ,GCancellable *,
                             GFileProgressCallback,gpointer,GError **) = []
            (GFile *,GFile *,GFileCopyFlags ,GCancellable *,
            GFileProgressCallback,gpointer,GError **error){
        *error = g_error_new(G_IO_ERROR,G_IO_ERROR_CANCELLED,"nihao");
        return gboolean(true);
    };
    stl.set(g_file_move,g_file_move3);
    EXPECT_TRUE(job->moveFileByGio(source.toLocalFile(),dst.toLocalFile(),&targetpath));

    TestHelper::deleteTmpFiles(QStringList() << linkfile.toLocalFile() << dst.toLocalFile()
                               << source.toLocalFile() << fileurl.toLocalFile() <<
                               dirurl.toLocalFile());
}

TEST_F(FileJobTest, start_moveDir) {
    Stub stl;
    bool (*handleMoveJob)(const QString &, const QString &, QString *) = []
            (const QString &, const QString &, QString *){return true;};
    stl.set(ADDR(FileJob,handleMoveJob),handleMoveJob);
    EXPECT_TRUE(job->moveDir("",""));
}

TEST_F(FileJobTest, start_handleMoveJob) {
    Stub stl;
    source.setPath(TestHelper::createTmpFile());
    dst.setPath(TestHelper::createTmpDir());
    QString dstdirpath = dst.toLocalFile();
    DUrl linkfile(source),fileurl(source),dirurl(source),targetdirurl(dst),targetfile(source);
    linkfile.setPath(source.toLocalFile() + "sys_link");
    fileurl.setPath(TestHelper::createTmpFile());
    dirurl.setPath("/tmp/ut_jobfile_moveFileByGio");
    targetfile.setPath(dst.toLocalFile() + "/jfieit.txt");
    targetdirurl.setPath(dst.toLocalFile() + "/" + dst.fileName());
    QProcess::execute("touch " + targetfile.toLocalFile());
    QProcess::execute("mkdir " + dirurl.toLocalFile() + " " + targetdirurl.toLocalFile());
    QProcess::execute("ln -s " + source.toLocalFile() + " " + linkfile.toLocalFile());
    DUrlList (*childrenList)(const DUrl &) = [](const DUrl &){
        return DUrlList() << DUrl("file:///tmp/ut_jobfile_moveFileByGio");
    };
    stl.set(ADDR(DUrl,childrenList),childrenList);
    bool (*ghostSignal)(const DUrl &, DAbstractFileWatcher::SignalType3, const DUrl &, const int) = []
            (const DUrl &, DAbstractFileWatcher::SignalType3, const DUrl &, const int){return true;};
    stl.set((bool (*)(const DUrl &, DAbstractFileWatcher::SignalType3, const DUrl &, const int))\
            ADDR(DAbstractFileWatcher,ghostSignal),ghostSignal);
    bool (*ghostSignal1)(const DUrl &, DAbstractFileWatcher::SignalType1, const DUrl &) = []
            (const DUrl &, DAbstractFileWatcher::SignalType1, const DUrl &){return true;};
    bool (*ghostSignal2)(const DUrl &, DAbstractFileWatcher::SignalType2 , const DUrl &, const DUrl &) = []
            (const DUrl &, DAbstractFileWatcher::SignalType2 , const DUrl &, const DUrl &){return true;};
    stl.set((bool (*)(const DUrl &, DAbstractFileWatcher::SignalType1, const DUrl &))\
            ADDR(DAbstractFileWatcher,ghostSignal),ghostSignal1);
    stl.set((bool (*)(const DUrl &, DAbstractFileWatcher::SignalType2 , const DUrl &, const DUrl &))\
            ADDR(DAbstractFileWatcher,ghostSignal),ghostSignal2);

    bool (*handleSymlinkFile)(const QString &, const QString &, QString *) = []
            (const QString &, const QString &, QString *){return false;};
    stl.set(ADDR(FileJob,handleSymlinkFile),handleSymlinkFile);
    stl.set(ADDR(FileJob,moveDir),handleSymlinkFile);
    stl.set(ADDR(FileJob,moveFile),handleSymlinkFile);

    QString targetpath;
    void (*showCopyMoveToSelfDialog)(const QMap<QString, QString> &) = []
            (const QMap<QString, QString> &){};
    stl.set(ADDR(DialogManager,showCopyMoveToSelfDialog),showCopyMoveToSelfDialog);
    EXPECT_TRUE(job->handleMoveJob(dst.toLocalFile(),"/tmp/ut_jobfile_moveFileByGio"));

    stl.reset(ADDR(DUrl,childrenList));

    job->m_isAborted = true;
    EXPECT_FALSE(job->handleMoveJob(targetdirurl.toLocalFile(),dst.toLocalFile(),&targetpath));

    job->m_isAborted = false;
    job->m_applyToAll = true;
    job->m_status = FileJob::Cancelled;
    EXPECT_FALSE(job->handleMoveJob(dst.toLocalFile(),dst.toLocalFile(),&targetpath));

    job->m_isAborted = false;
    job->m_applyToAll = true;
    job->m_status = FileJob::Cancelled;
    EXPECT_FALSE(job->handleMoveJob(targetdirurl.toLocalFile(),dst.toLocalFile(),&targetpath));

    job->m_applyToAll = false;
    void (*jobConflicted)(void *) = [](void *){};
    stl.set(ADDR(FileJob,jobConflicted),jobConflicted);
    job->m_status = FileJob::Cancelled;
    EXPECT_FALSE(job->handleMoveJob(targetdirurl.toLocalFile(),dst.toLocalFile(),&targetpath));

    job->m_applyToAll = false;
    job->m_status = FileJob::Cancelled;
    job->m_isCoExisted = true;
    job->m_isReplaced = false;
    EXPECT_FALSE(job->handleMoveJob(targetdirurl.toLocalFile(),dst.toLocalFile(),&targetpath));

    job->m_applyToAll = false;
    job->m_status = FileJob::Cancelled;
    job->m_isCoExisted = false;
    job->m_isReplaced = true;
    EXPECT_FALSE(job->handleMoveJob(targetdirurl.toLocalFile(),dst.toLocalFile(),&targetpath));

    job->m_applyToAll = false;
    job->m_status = FileJob::Cancelled;
    job->m_isCoExisted = false;
    job->m_isReplaced = true;
    EXPECT_FALSE(job->handleMoveJob(targetdirurl.toLocalFile(),source.toLocalFile(),&targetpath));

    DUrl tmpfile(source);
    tmpfile.setPath(TestHelper::createTmpFile());
    EXPECT_FALSE(job->handleMoveJob(source.toLocalFile(),dst.toLocalFile(),&targetpath));

    EXPECT_FALSE(job->handleMoveJob(linkfile.toLocalFile(),dst.toLocalFile(),&targetpath));
    bool (*handleSymlinkFile1)(const QString &, const QString &, QString *) = []
            (const QString &, const QString &, QString *){
        QThread::msleep(400);
        return false;
    };
    stl.set(ADDR(FileJob,handleSymlinkFile),handleSymlinkFile1);
    stl.set(ADDR(FileJob,moveDir),handleSymlinkFile1);
    stl.set(ADDR(FileJob,moveFile),handleSymlinkFile1);
    QFuture<void> future1 = QtConcurrent::run([=]() {
        QThread::msleep(150);
        qDebug() << "000001" << job->m_status;
        job->m_status = FileJob::Paused;
        QThread::msleep(300);
        qDebug() << "000002" << job->m_status;
        job->m_status = FileJob::Conflicted;
    });
    EXPECT_FALSE(job->handleMoveJob(linkfile.toLocalFile(),dst.toLocalFile(),&targetpath));
    future1.waitForFinished();

    QFuture<void> future = QtConcurrent::run([=]() {
        QThread::msleep(150);
        qDebug() << "000000" << job->m_status;
        job->m_status = FileJob::Cancelled;
    });
    EXPECT_FALSE(job->handleMoveJob(linkfile.toLocalFile(),dst.toLocalFile(),&targetpath));
    future.waitForFinished();

    EXPECT_FALSE(job->handleMoveJob(linkfile.toLocalFile(),dst.toLocalFile() + "/ut_dir",&targetpath));

    EXPECT_FALSE(job->handleMoveJob(dst.toLocalFile(),dst.toLocalFile(),&targetpath));

    TestHelper::deleteTmpFiles(QStringList() << linkfile.toLocalFile() << dst.toLocalFile()
                               << source.toLocalFile() << fileurl.toLocalFile() <<
                               dirurl.toLocalFile() << tmpfile.toLocalFile());
}

TEST_F(FileJobTest, start_handleSymlinkFile) {
    Stub stl;
    job->m_isAborted = true;
    EXPECT_FALSE(job->handleSymlinkFile("",""));
    source.setPath(TestHelper::createTmpFile());
    job->m_isAborted = false;
    dst.setPath(source.toLocalFile()+"_sys_link");
    job->m_applyToAll = false;
    QString target;
    EXPECT_FALSE(job->handleSymlinkFile(source.toLocalFile(),dst.toLocalFile(),&target));

    job.reset(new FileJob(FileJob::Trash));
    bool (*moveFileToTrash)(const QString &, QString *) = []
            (const QString &, QString *){return true;};
    stl.set(ADDR(FileJob,moveFileToTrash),moveFileToTrash);
    EXPECT_FALSE(job->handleSymlinkFile(source.toLocalFile(),dst.toLocalFile(),&target));

    DUrl tmplink(dst);
    tmplink.setPath(source.toLocalFile() + "oo_ut_link");
    EXPECT_FALSE(job->handleSymlinkFile(source.toLocalFile(),tmplink.toLocalFile(),&target));

    bool (*moveFileToTrash1)(const QString &, QString *) = []
            (const QString &, QString *){
        QThread::msleep(400);
        return true;
    };
    stl.set(ADDR(FileJob,moveFileToTrash),moveFileToTrash1);
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

    TestHelper::deleteTmpFiles(QStringList() << dst.toLocalFile() << tmplink.toLocalFile()
                               << source.toLocalFile());
}

TEST_F(FileJobTest, start_restoreTrashFile) {
    Stub stl;
    void (*jobConflicted)(void *) = [](void *){};
    stl.set(ADDR(FileJob,jobConflicted),jobConflicted);
    source.setPath(TestHelper::createTmpFile());
    dst.setPath(TestHelper::createTmpFile());

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

    TestHelper::deleteTmpFiles(QStringList() << "/tmp/ut_testtttt_rest" << dst.toLocalFile()
                               << source.toLocalFile() << tmpdir );
}
TEST_F(FileJobTest, start_deleteFile) {
    source.setPath(TestHelper::createTmpFile());
    TestHelper::deleteTmpFiles(QStringList() << source.toLocalFile() );
    EXPECT_FALSE(job->deleteFile(source.toLocalFile()));
    EXPECT_FALSE(job->deleteFile(source.toLocalFile()));
    TestHelper::deleteTmpFile(source.toLocalFile());
}

TEST_F(FileJobTest, start_deleteFileByGio) {
    source.setPath(TestHelper::createTmpFile());
    EXPECT_TRUE(job->deleteFileByGio(source.toLocalFile()));
    EXPECT_FALSE(job->deleteFileByGio(source.toLocalFile()));
    TestHelper::deleteTmpFile(source.toLocalFile());
}

TEST_F(FileJobTest, start_) {
    source.setPath("/run/user/1000/gvfs/smb-share:server=10.8.11.190,share=test");
    dst.setPath("~/Desktop");
    EXPECT_FALSE(job->checkUseGvfsFileOperation(dst.toLocalFile()));
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
            (void *,const DUrlList &, const qint64 &, bool &isInLimit){
        qint64 oo = 10000;
        isInLimit = false;
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
    StubExt stl;
    typedef bool (*openfile)(QFile *,QFile::OpenMode);
    stl.set_lamda((openfile)((bool (QFile::*)(QFile::OpenMode))(&QFile::open)),[](){return false;});
    EXPECT_FALSE(job->writeTrashInfo("ifue_98jfei_8e8f","jfaskjdf","kfek"));

    stl.reset((openfile)((bool (QFile::*)(QFile::OpenMode))(&QFile::open)));
    QList<QString> (*getTagsThroughFiles)(const QList<DUrl>&) = [](const QList<DUrl>&){
        QList<QString> tt;
        return tt << "ut_oireoirsfsdj";
    };
    stl.set(ADDR(TagManager,getTagsThroughFiles),getTagsThroughFiles);
    qint64 (*write1)(const QByteArray &) = [](const QByteArray &){return qint64(-1);};
    stl.set((qint64 (QIODevice::*)(const QByteArray &))ADDR(QIODevice,write),write1);
    EXPECT_FALSE(job->writeTrashInfo("ifue_98jfei_8e8f","jfaskjdf","kfek"));
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
    EXPECT_FALSE(job->moveDirToTrash(source.toLocalFile(),&targetPath));
    EXPECT_NO_FATAL_FAILURE(job->setStatus(FileJob::Run));
    bool (*writeTrashInfo)(void *,const QString &, const QString &, const QString &) =[]
            (void *,const QString &, const QString &, const QString &)
    {
        return false;
    };
    EXPECT_TRUE(job->moveDirToTrash(source.toLocalFile(),&targetPath));
    Stub stl;
    stl.set(ADDR(FileJob,writeTrashInfo),writeTrashInfo);
    EXPECT_FALSE(job->moveDirToTrash(source.toLocalFile(),&targetPath));
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

    Stub stl;
    bool (*deleteFile)(const QString &) = [](const QString &){return false;};
    stl.set(ADDR(FileJob,deleteFile),deleteFile);

    EXPECT_NO_FATAL_FAILURE(job->setStatus(FileJob::Cancelled));
    EXPECT_FALSE(job->deleteDir(source.toLocalFile()));
    EXPECT_NO_FATAL_FAILURE(job->setStatus(FileJob::Run));
    EXPECT_FALSE(job->deleteDir(source.toLocalFile()));
    stl.reset(ADDR(FileJob,deleteFile));
    QProcess::execute("mkdir "+ source.toLocalFile() + "/temp1");
    QProcess::execute("mkdir "+ source.toLocalFile() + "/temp2");
    QProcess::execute("touch "+ source.toLocalFile() + "/temp1/teste");
    stl.set(ADDR(FileJob,deleteDir),deleteFile);
    EXPECT_FALSE(job->deleteDir(source.toLocalFile()));
    stl.reset(ADDR(FileJob,deleteDir));
    EXPECT_TRUE(job->deleteDir(source.toLocalFile()));

    bool (*rmdir)(const QString &) = [](const QString &){return false;};
    stl.set(ADDR(QDir,rmdir),rmdir);
    QProcess::execute("mkdir "+source.toLocalFile());
    EXPECT_FALSE(job->deleteDir(source.toLocalFile()));
    TestHelper::deleteTmpFile(source.toLocalFile());
}

TEST_F(FileJobTest, start_getNotExistsTrashFileName){
    source.setPath(TestHelper::createTmpFile(".txt"));
    EXPECT_FALSE(job->getNotExistsTrashFileName(source.toLocalFile()).isEmpty());
    TestHelper::deleteTmpFile(source.toLocalFile());
}

TEST_F(FileJobTest, start_getStorageInfo){
    source.setPath(TestHelper::createTmpFile(".txt"));
    EXPECT_TRUE(job->getStorageInfo(source.toLocalFile()).isValid());
    TestHelper::deleteTmpFile(source.toLocalFile());
}
