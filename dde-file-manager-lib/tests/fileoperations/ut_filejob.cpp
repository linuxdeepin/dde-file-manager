#include <gtest/gtest.h>
#include <QDateTime>
#include <QSharedPointer>
#include <QProcess>
#include <QDateTime>

#include "fileoperations/filejob.h"
#include "disomaster.h"


using namespace testing;


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
    QProcess::execute("mkdir" + dst.toLocalFile());
    EXPECT_FALSE(FileJob::setDirPermissions(source.toLocalFile(),dst.toLocalFile()));
    QProcess::execute("r" + dst.toLocalFile());
}

TEST_F(FileJobTest,get_jobType) {
    EXPECT_EQ(FileJob::Copy, job->jobType());
}

TEST_F(FileJobTest,start_setStatus) {
    job->setStatus(FileJob::Started);
}

TEST_F(FileJobTest,can_JobIdOperation) {
    QString id = QString::number(QDateTime::currentDateTime().toSecsSinceEpoch());
    job->setJobId(id);
    EXPECT_EQ(id,job->getJobId());
}

TEST_F(FileJobTest,start_checkDuplicateName) {
    source.setPath("./");
    EXPECT_FALSE(job->checkDuplicateName(source.toLocalFile()).isEmpty());
    source.setPath("../LICENSE");
    EXPECT_FALSE(job->checkDuplicateName(source.toLocalFile()).isEmpty());
}

TEST_F(FileJobTest,start_ApplyToAllOp) {
    job->setApplyToAll(true);
    EXPECT_TRUE(job->getIsApplyToAll());
}

TEST_F(FileJobTest,start_ReplaceOp) {
    job->setReplace(true);
    EXPECT_TRUE(job->getIsReplaced());
    job->resetCustomChoice();
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
    job->adjustSymlinkPath(sourcepath,sourcepath);
}

TEST_F(FileJobTest,start_AbortedOp) {
    job->setIsAborted(true);
    EXPECT_TRUE(job->isAborted());
}

TEST_F(FileJobTest,start_IsSkipOp) {
    job->setIsSkip(true);
    EXPECT_TRUE(job->getIsSkip());
}

TEST_F(FileJobTest,start_IsCoExistedOp) {
    job->setIsCoExisted(true);
    EXPECT_TRUE(job->getIsCoExisted());
}

TEST_F(FileJobTest,start_ManualRemoveJobOp) {
    job->setManualRemoveJob(true);
    EXPECT_TRUE(job->getIsManualRemoveJob());
}

TEST_F(FileJobTest,start_WindowIdOp) {
    int winId = static_cast<int>(QDateTime::currentDateTime().toSecsSinceEpoch());
    job->setWindowId(winId);
    EXPECT_TRUE(winId == job->getWindowId());
}

TEST_F(FileJobTest,can_getIsGvfsFileOperationUsed) {
    EXPECT_FALSE(job->getIsGvfsFileOperationUsed());
}

TEST_F(FileJobTest,can_getIsFinished) {
    EXPECT_FALSE(job->getIsFinished());
}

TEST_F(FileJobTest,start_RestoreProgressOp) {
    job->setRestoreProgress(0.50);
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
    job->started();
    EXPECT_TRUE(job->doCopy(DUrlList() << source, dst).count());
    job->jobUpdated();
    job->paused();
    job->jobAdded();
    job->jobRemoved();
    job->jobAborted();
    job->jobPrepared();
    job->jobConflicted();
    job->cancelled();
    job->handleJobFinished();

}

TEST_F(FileJobTest,start_doMove) {
    source.setPath("./Wallpapers");
    dst.setPath("../");
    job.reset(new FileJob(FileJob::Move));
    job->started();
    EXPECT_TRUE(job->doMove(DUrlList() << source, dst).count());
}

TEST_F(FileJobTest,start_doDelete) {
    source.setPath("../Wallpapers");
    job.reset(new FileJob(FileJob::Delete));
    job->started();
    job->doDelete(DUrlList() << source);
}

TEST_F(FileJobTest,start_doMoveToTrash) {
    source.setPath("~/Pictures/Wallpapers");
    dst.setPath("./");
    job->started();
    EXPECT_TRUE(job->doCopy(DUrlList() << source, dst).count());
    source.setPath("./Wallpapers");
    job.reset(new FileJob(FileJob::Trash));
    job->started();
    DUrlList urllist = job->doMoveToTrash(DUrlList() << source);
    EXPECT_TRUE(urllist.count());
    job->doTrashRestore(source.toLocalFile(),dst.toLocalFile());
}

TEST_F(FileJobTest, doOpticalBurnByChildProcess)
{
    DUrl url = DUrl::fromLocalFile("/dev/sr*");
    EXPECT_NO_FATAL_FAILURE(job->doOpticalBurnByChildProcess(url, "test", 10, 4));
}

TEST_F(FileJobTest, doOpticalBlank)
{
    DUrl url = DUrl::fromLocalFile("/dev/sr*");
    EXPECT_NO_FATAL_FAILURE(job->doOpticalBlank(url));
}

TEST_F(FileJobTest, doOpticalImageBurnByChildProcess)
{
    DUrl url = DUrl::fromLocalFile("/dev/sr*");
    DUrl image;
    EXPECT_NO_FATAL_FAILURE(job->doOpticalImageBurnByChildProcess(url, image, 10, 4));
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
