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

#define private public
#include "fileoperations/filejob.h"

using namespace testing;
using namespace stub_ext;


class FileJobTest:public testing::Test{

public:

    QSharedPointer<FileJob> job;
    virtual void SetUp() override{
        job.reset(new FileJob(FileJob::Copy));
        source.setScheme(FILE_SCHEME);
        dst.setScheme(FILE_SCHEME);
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
    EXPECT_NO_FATAL_FAILURE(job->started());
    EXPECT_TRUE(job->doCopy(DUrlList() << source, dst).count());
    EXPECT_NO_FATAL_FAILURE(job->jobUpdated());
    EXPECT_NO_FATAL_FAILURE(job->paused());
    EXPECT_NO_FATAL_FAILURE(job->jobAdded());
    EXPECT_NO_FATAL_FAILURE(job->jobRemoved());
    EXPECT_NO_FATAL_FAILURE(job->jobAborted());
    EXPECT_NO_FATAL_FAILURE(job->jobPrepared());
    EXPECT_NO_FATAL_FAILURE(job->jobConflicted());
    EXPECT_NO_FATAL_FAILURE(job->cancelled());
    EXPECT_NO_FATAL_FAILURE(job->handleJobFinished());

}

TEST_F(FileJobTest,start_doMove) {
    source.setPath("./Wallpapers");
    dst.setPath("../");
    EXPECT_NO_FATAL_FAILURE(job.reset(new FileJob(FileJob::Move)));
    EXPECT_NO_FATAL_FAILURE(job->started());
    EXPECT_TRUE(job->doMove(DUrlList() << source, dst).count());
}

TEST_F(FileJobTest,start_doDelete) {
    source.setPath("../Wallpapers");
    EXPECT_NO_FATAL_FAILURE(job.reset(new FileJob(FileJob::Delete)));
    EXPECT_NO_FATAL_FAILURE(job->started());
    EXPECT_NO_FATAL_FAILURE(job->doDelete(DUrlList() << source));
}

TEST_F(FileJobTest,start_doMoveToTrash) {
    source.setPath("~/Pictures/Wallpapers");
    dst.setPath("./");
    EXPECT_NO_FATAL_FAILURE(job->started());
    EXPECT_TRUE(job->doCopy(DUrlList() << source, dst).count());
    source.setPath("./Wallpapers");
    job.reset(new FileJob(FileJob::Trash));
    EXPECT_NO_FATAL_FAILURE(job->started());
    DUrlList urllist = job->doMoveToTrash(DUrlList() << source);
    EXPECT_TRUE(urllist.count());
    EXPECT_NO_FATAL_FAILURE(job->doTrashRestore(source.toLocalFile(),dst.toLocalFile()));
}

TEST_F(FileJobTest, doOpticalBurnByChildProcess)
{
    DUrl url = DUrl::fromLocalFile("/dev/sr*");
    DISOMasterNS::BurnOptions opts;
    opts |= DISOMasterNS::BurnOption::VerifyDatas;
    EXPECT_NO_FATAL_FAILURE(job->doOpticalBurnByChildProcess(url, "test", 10, opts));
}

TEST_F(FileJobTest, doOpticalBlank)
{
    DUrl url = DUrl::fromLocalFile("/dev/sr*");
//    EXPECT_NO_FATAL_FAILURE(job->doOpticalBlank(url));
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
