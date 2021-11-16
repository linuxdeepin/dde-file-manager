#include <gtest/gtest.h>

#include "fcntl.h"
#include "sys/mman.h"
#define private public
#define protected public
#include "deviceinfo/udisklistener.h"
#include "dfilecopymovejob.h"
#include "private/dfilecopymovejob_p.h"
#include "interfaces/dfileservices.h"
#include "interfaces/dfileinfo.h"
#include "testhelper.h"
#include "dfmglobal.h"
#include "stubext.h"
#include "controllers/vaultcontroller.h"
#include "dgiofiledevice.h"
#include "dabstractfilewatcher.h"
#include "dlocalfiledevice.h"
#include "testhelper.h"

#include <QDateTime>
#include <QThread>
#include <QProcess>
#include <QtConcurrent>
#include <QByteArray>
#include <QtDebug>
#include <QVariant>
#include <QDialog>
#include <QtConcurrent>

using namespace testing;
using namespace stub_ext;
DFM_USE_NAMESPACE
bool stub_ghostSignal(const DUrl &, DAbstractFileWatcher::SignalType3, const DUrl &, const int)
{
    return true;
};
bool stub_ghostSignal1(const DUrl &, DAbstractFileWatcher::SignalType1, const DUrl &)
{
    return true;
};
bool stub_ghostSignal2(const DUrl &, DAbstractFileWatcher::SignalType2, const DUrl &, const DUrl &)
{
    return true;
};
int stub_DDialog_exec(void)
{
    return QDialog::Rejected;
};
class DFileCopyMoveJobTest: public testing::Test
{

public:

    QSharedPointer<DFileCopyMoveJob> job = nullptr;
    virtual void SetUp() override
    {
        job.reset(new DFileCopyMoveJob());
        Stub stl;

        stl.set((bool (*)(const DUrl &, DAbstractFileWatcher::SignalType3, const DUrl &, const int))\
                ADDR(DAbstractFileWatcher, ghostSignal), stub_ghostSignal);
        stl.set((bool (*)(const DUrl &, DAbstractFileWatcher::SignalType1, const DUrl &))\
                ADDR(DAbstractFileWatcher, ghostSignal), stub_ghostSignal1);
        stl.set((bool (*)(const DUrl &, DAbstractFileWatcher::SignalType2, const DUrl &, const DUrl &))\
                ADDR(DAbstractFileWatcher, ghostSignal), stub_ghostSignal2);

        typedef int(*fptr)(QDialog *);
        fptr pQDialogExec = (fptr)(&QDialog::exec);
        stl.set(pQDialogExec, stub_DDialog_exec);
        std::cout << "start DFileCopyMoveJobTest" << std::endl;
    }

    virtual void TearDown() override
    {
        std::cout << "end DFileCopyMoveJobTest" << std::endl;
        // CI 进度较快，这里休眠 500 ms 来等待线程退出，如果超时了还未退出线程，job 内部自有机制保证安全退出，只不过那样的话有些测试可能达不到效果。
        QThread::msleep(500);
        job.reset();
    }
};

class ErrorHandleMe : public QObject, public DFileCopyMoveJob::Handle
{
public:
    ErrorHandleMe(QSharedPointer<DFileCopyMoveJob> job)
        : QObject(nullptr)
        , fileJob(job)
    {
    }

    ~ErrorHandleMe() override
    {
        fileJob->disconnect();
        fileJob.reset();
        qDebug() << " ErrorHandle() ";
    }

    // 处理任务对话框显示之前的错误, 无法处理的错误将立即弹出对话框处理
    DFileCopyMoveJob::Action handleError(DFileCopyMoveJob *job, DFileCopyMoveJob::Error error,
                                         const DAbstractFileInfoPointer sourceInfo,
                                         const DAbstractFileInfoPointer targetInfo) override
    {
        Q_UNUSED(job);
        Q_UNUSED(sourceInfo);
        Q_UNUSED(targetInfo);
        if (error == DFileCopyMoveJob::DirectoryExistsError) {
            return DFileCopyMoveJob::CoexistAction;
        }
        if (error == DFileCopyMoveJob::FileExistsError) {
            return DFileCopyMoveJob::CancelAction;
        }
        if (error == DFileCopyMoveJob::UnknowError) {
            return DFileCopyMoveJob::CancelAction;
        }
        return DFileCopyMoveJob::SkipAction;
    }
    QSharedPointer<DFileCopyMoveJob> fileJob;
};

class ErrorHandleTest : public QObject, public DFileCopyMoveJob::Handle
{
public:
    ErrorHandleTest(QSharedPointer<DFileCopyMoveJob> job)
        : QObject(nullptr)
        , fileJob(job)
    {
    }

    ~ErrorHandleTest() override
    {
        fileJob->disconnect();
        fileJob.reset();
        qDebug() << " ErrorHandle() ";
    }

    // 处理任务对话框显示之前的错误, 无法处理的错误将立即弹出对话框处理
    DFileCopyMoveJob::Action handleError(DFileCopyMoveJob *job, DFileCopyMoveJob::Error error,
                                         const DAbstractFileInfoPointer sourceInfo,
                                         const DAbstractFileInfoPointer targetInfo) override
    {
        Q_UNUSED(job);
        Q_UNUSED(sourceInfo);
        Q_UNUSED(targetInfo);
        if (error == DFileCopyMoveJob::DirectoryExistsError) {
            return DFileCopyMoveJob::CoexistAction;
        }
        if (error == DFileCopyMoveJob::FileExistsError) {
            return DFileCopyMoveJob::CancelAction;
        }
        return DFileCopyMoveJob::NoAction;
    }
    QSharedPointer<DFileCopyMoveJob> fileJob;
};

#ifndef __arm__
TEST_F(DFileCopyMoveJobTest, can_job_running)
{
    TestHelper::runInLoop([]() {});
    job->setMode(DFileCopyMoveJob::CopyMode);
    job->setRefine(DFileCopyMoveJob::NoRefine);
    DUrl urlsour, target;
    urlsour.setScheme(FILE_SCHEME);
    target.setScheme(FILE_SCHEME);
    urlsour.setPath("~/Pictures/Wallpapers");
    target.setPath(QDir::currentPath() + "/test_copy_all");

    if (QThread::currentThread()->loopLevel() <= 0) {
        // 确保对象所在线程有事件循环
        job->moveToThread(qApp->thread());
    }
    job->start(DUrlList() << urlsour, target);
    while (!job->isFinished()) {
        QThread::msleep(100);
    }
    QProcess::execute("mkdir " + target.toLocalFile());
    job->start(DUrlList() << urlsour, target);
    while (!job->isFinished()) {
        QThread::msleep(100);
    }
    urlsour.setPath("/etc/apt");
    job->setFileHints(DFileCopyMoveJob::FollowSymlink);
    job->setFileHints(job->fileHints() | DFileCopyMoveJob::Attributes);
    job->setFileHints(job->fileHints() | DFileCopyMoveJob::ResizeDestinationFile);
    job->setFileHints(job->fileHints() | DFileCopyMoveJob::DontFormatFileName);
    job->setFileHints(job->fileHints() | DFileCopyMoveJob::DontSortInode);
    job->start(DUrlList() << urlsour, target);
    job->togglePause();
    job->togglePause();
    while (!job->isFinished()) {
        QThread::msleep(100);
    }
    job->togglePause();
    job->stop();
}
#endif
TEST_F(DFileCopyMoveJobTest, can_job_running_cut)
{
    job->setMode(DFileCopyMoveJob::CutMode);
    DUrl urlsour, target;
    urlsour.setScheme(FILE_SCHEME);
    target.setScheme(FILE_SCHEME);
    urlsour.setPath("/etc/apt");
    target.setPath(QDir::currentPath() + "/test_copy_all/etc");
    QProcess::execute("mkdir " + target.parentUrl().toLocalFile());
    QProcess::execute("mkdir " + target.toLocalFile());
    if (QThread::currentThread()->loopLevel() <= 0) {
        // 确保对象所在线程有事件循环
        job->moveToThread(qApp->thread());
    }
    job->start(DUrlList() << urlsour, target);

    EXPECT_EQ(target.toString(), job->targetUrl().toString());
    EXPECT_EQ(-1, job->totalDataSize());
    EXPECT_EQ(0, job->totalFilesCount());

    QThread::msleep(300);
    EXPECT_EQ(true, job->totalDataSize() != -1);
    EXPECT_EQ(true, job->totalFilesCount() != -1);
    EXPECT_NO_FATAL_FAILURE(job->error());
    EXPECT_EQ(true, job->isCanShowProgress());

    job->stop();

    while (!job->isFinished()) {
        QThread::msleep(100);
    }
    job->stop();
}

TEST_F(DFileCopyMoveJobTest, can_job_running_refine)
{
    job->setMode(DFileCopyMoveJob::CutMode);
    DUrl urlsour, target;
    urlsour.setScheme(FILE_SCHEME);
    target.setScheme(FILE_SCHEME);
    urlsour.setPath(QDir::currentPath() + "/test_copy_all/etc");
    target.setPath(QDir::currentPath() + "/test_copy_all/etc1");
    if (QThread::currentThread()->loopLevel() <= 0) {
        // 确保对象所在线程有事件循环
        job->moveToThread(qApp->thread());
    }
    job->start(DUrlList() << urlsour, target);


    while (!job->isFinished()) {
        QThread::msleep(100);
    }

    target.setPath(QDir::currentPath() + "/test_copy_all");
    job->stop();
}

TEST_F(DFileCopyMoveJobTest, can_job_running_remove)
{
    job->setMode(DFileCopyMoveJob::CutMode);
    job->setFileHints(DFileCopyMoveJob::ForceDeleteFile);
    DUrl urlsour, target;
    urlsour.setScheme(FILE_SCHEME);
    target.setScheme(FILE_SCHEME);
    EXPECT_EQ(DFileCopyMoveJob::ForceDeleteFile, job->fileHints());
    urlsour.setPath(QDir::currentPath() + "/test_copy_all");
    if (QThread::currentThread()->loopLevel() <= 0) {
        // 确保对象所在线程有事件循环
        job->moveToThread(qApp->thread());
    }
    job->start(DUrlList() << urlsour, DUrl());
    while (!job->isFinished()) {
        QThread::msleep(100);
    }
    // repeat?
    /*job->start(DUrlList() << urlsour, DUrl());
    while (!job->isFinished()) {
        QThread::msleep(100);
    }*/
    job->stop();
}

TEST_F(DFileCopyMoveJobTest, can_job_running_error)
{
    job->setMode(DFileCopyMoveJob::CopyMode);
    DUrl urlsour, target;
    urlsour.setScheme(FILE_SCHEME);
    target.setScheme(FILE_SCHEME);
    urlsour.setPath("~/Wallpapers");
    target.setPath("/usr/bin");
    if (QThread::currentThread()->loopLevel() <= 0) {
        // 确保对象所在线程有事件循环
        job->moveToThread(qApp->thread());
    }
    job->start(DUrlList() << urlsour, target);
    while (!job->isFinished()) {
        QThread::msleep(100);
    }
    job->stop();
}

TEST_F(DFileCopyMoveJobTest, get_errorToString)
{
    DFileCopyMoveJobPrivate *jobd = job->d_func();
    ASSERT_TRUE(jobd);
    EXPECT_FALSE(jobd->errorToString(DFileCopyMoveJob::PermissionError).isEmpty());
    EXPECT_FALSE(jobd->errorToString(DFileCopyMoveJob::SpecialFileError).isEmpty());
    EXPECT_FALSE(jobd->errorToString(DFileCopyMoveJob::FileExistsError).isEmpty());
    EXPECT_FALSE(jobd->errorToString(DFileCopyMoveJob::DirectoryExistsError).isEmpty());
    EXPECT_FALSE(jobd->errorToString(DFileCopyMoveJob::OpenError).isEmpty());
    EXPECT_FALSE(jobd->errorToString(DFileCopyMoveJob::ReadError).isEmpty());
    EXPECT_FALSE(jobd->errorToString(DFileCopyMoveJob::WriteError).isEmpty());
    EXPECT_FALSE(jobd->errorToString(DFileCopyMoveJob::MkdirError).isEmpty());
    EXPECT_FALSE(jobd->errorToString(DFileCopyMoveJob::RemoveError).isEmpty());
    EXPECT_FALSE(jobd->errorToString(DFileCopyMoveJob::RenameError).isEmpty());
    EXPECT_FALSE(jobd->errorToString(DFileCopyMoveJob::FileSizeTooBigError).isEmpty());
    EXPECT_FALSE(jobd->errorToString(DFileCopyMoveJob::NotEnoughSpaceError).isEmpty());
    EXPECT_FALSE(jobd->errorToString(DFileCopyMoveJob::IntegrityCheckingError).isEmpty());
    EXPECT_FALSE(jobd->errorToString(DFileCopyMoveJob::TargetReadOnlyError).isEmpty());
    EXPECT_FALSE(jobd->errorToString(DFileCopyMoveJob::TargetIsSelfError).isEmpty());
    EXPECT_TRUE(jobd->errorToString(DFileCopyMoveJob::UnknowError).isEmpty());
    job->stop();
}

TEST_F(DFileCopyMoveJobTest, start_setState)
{
    DFileCopyMoveJobPrivate *jobd = job->d_func();
    ASSERT_TRUE(jobd);
    jobd->setState(DFileCopyMoveJob::SleepState);
    jobd->setState(DFileCopyMoveJob::RunningState);
    EXPECT_TRUE(jobd->stateCheck());
    QThread::msleep(100);
    jobd->setState(DFileCopyMoveJob::SleepState);
    jobd->setState(DFileCopyMoveJob::RunningState);
    jobd->setState(DFileCopyMoveJob::PausedState);
    // 概率造成 ut 崩溃
    /*QtConcurrent::run([ = ]() {
        EXPECT_FALSE(jobd->stateCheck());
    });*/
    QThread::msleep(300);

    job->stop();
    jobd->setState(DFileCopyMoveJob::StoppedState);
    EXPECT_FALSE(jobd->stateCheck());
    job->stop();
}

TEST_F(DFileCopyMoveJobTest, start_setError)
{
    DFileCopyMoveJobPrivate *jobd = job->d_func();
    ASSERT_TRUE(jobd);
    jobd->setError(DFileCopyMoveJob::NoError);
    jobd->setError(DFileCopyMoveJob::NoError);
    jobd->setError(DFileCopyMoveJob::PermissionError);
    jobd->setError(DFileCopyMoveJob::PermissionError);
    job->stop();
}

TEST_F(DFileCopyMoveJobTest, start_handleError)
{
    DFileCopyMoveJobPrivate *jobd = job->d_func();
    ASSERT_TRUE(jobd);
    DUrl url;
    url.setScheme(FILE_SCHEME);
    url.setPath("~/Videos");
    DAbstractFileInfoPointer source = DFileService::instance()->createFileInfo(nullptr, url);
    jobd->setError(DFileCopyMoveJob::NoError);
    EXPECT_EQ(DFileCopyMoveJob::NoAction, jobd->handleError(source, source));
    jobd->setError(DFileCopyMoveJob::SpecialFileError);
    job->setActionOfErrorType(DFileCopyMoveJob::SpecialFileError, DFileCopyMoveJob::SkipAction);
    EXPECT_EQ(DFileCopyMoveJob::SkipAction, jobd->handleError(source, source));
    jobd->setError(DFileCopyMoveJob::FileExistsError);
    EXPECT_EQ(DFileCopyMoveJob::CoexistAction, jobd->handleError(source, source));
    jobd->setError(DFileCopyMoveJob::DirectoryExistsError);
    EXPECT_EQ(DFileCopyMoveJob::CoexistAction, jobd->handleError(source, source));
    ErrorHandleMe hanle(job);
    jobd->setState(DFileCopyMoveJob::StoppedState);
    job->setErrorHandle(&hanle, hanle.thread());
    jobd->setState(DFileCopyMoveJob::RunningState);
    TestHelper::runInLoop([ = ]() {
        jobd->setError(DFileCopyMoveJob::DirectoryExistsError);
        EXPECT_EQ(DFileCopyMoveJob::CoexistAction, jobd->handleError(source, source));
    }, 200);

    job->setErrorHandle(&hanle, hanle.thread());
    hanle.moveToThread(qApp->thread());
    TestHelper::runInLoop([ = ]() {
        jobd->setError(DFileCopyMoveJob::SpecialFileError);
        EXPECT_EQ(DFileCopyMoveJob::SkipAction, jobd->handleError(source, source));
    }, 200);

    job->setErrorHandle(&hanle, hanle.thread());
    hanle.moveToThread(qApp->thread());
    TestHelper::runInLoop([ = ]() {
        jobd->setError(DFileCopyMoveJob::FileExistsError);
        EXPECT_EQ(DFileCopyMoveJob::CancelAction, jobd->handleError(source, source));
    }, 200);

    ErrorHandleTest *hanlee = new ErrorHandleTest(job);
    job->setErrorHandle(hanlee, hanlee->thread());
    TestHelper::runInLoop([ = ]() {
        jobd->setError(DFileCopyMoveJob::DirectoryExistsError);
        jobd->handleError(source, source);
    }, 200);

    job->taskDailogClose();
    jobd->setError(DFileCopyMoveJob::SpecialFileError);
    EXPECT_EQ(DFileCopyMoveJob::CancelAction, jobd->handleError(source, source));
    job->stop();
    delete hanlee;
}

TEST_F(DFileCopyMoveJobTest, start_setAndhandleError)
{
    DFileCopyMoveJobPrivate *jobd = job->d_func();
    ASSERT_TRUE(jobd);
    DUrl url;
    url.setScheme(FILE_SCHEME);
    url.setPath("~/Videos");
    DAbstractFileInfoPointer source = DFileService::instance()->createFileInfo(nullptr, url);
    EXPECT_EQ(DFileCopyMoveJob::CancelAction, jobd->setAndhandleError(
                  DFileCopyMoveJob::FileExistsError, source, source));
    EXPECT_EQ(DFileCopyMoveJob::CancelAction, jobd->setAndhandleError(
                  DFileCopyMoveJob::NoError, DAbstractFileInfoPointer(nullptr), DAbstractFileInfoPointer(nullptr)));
    EXPECT_EQ(DFileCopyMoveJob::CancelAction, jobd->setAndhandleError(
                  DFileCopyMoveJob::CancelError, DAbstractFileInfoPointer(nullptr), DAbstractFileInfoPointer(nullptr)));
    job->stop();
}

TEST_F(DFileCopyMoveJobTest, start_isRunning)
{
    DFileCopyMoveJobPrivate *jobd = job->d_func();
    ASSERT_TRUE(jobd);
    EXPECT_FALSE(jobd->isRunning());
    job->stop();
}

QByteArray stub_fileSystemType(void *)
{
    return QByteArray("vfat");
};

TEST_F(DFileCopyMoveJobTest, start_formatFileName)
{
    DFileCopyMoveJobPrivate *jobd = job->d_func();
    ASSERT_TRUE(jobd);
    job->setFileHints(DFileCopyMoveJob::DontFormatFileName);
    EXPECT_EQ(jobd->formatFileName("hello"), QString("hello"));
    job->setFileHints(DFileCopyMoveJob::NoHint);
    EXPECT_EQ(jobd->formatFileName("hello"), QString("hello"));
    DUrl url;
    url.setScheme(FILE_SCHEME);
    url.setPath("~/Videos");
    jobd->enterDirectory(DUrl(), DUrl());
    EXPECT_EQ(jobd->formatFileName("hello"), QString("hello"));
    jobd->leaveDirectory();
    jobd->enterDirectory(url, url);
    EXPECT_EQ(jobd->formatFileName("hello"), QString("hello"));
    Stub st;

    st.set(ADDR(DStorageInfo, fileSystemType), stub_fileSystemType);
    EXPECT_EQ(jobd->formatFileName("hello"), QString("hello"));
    job->stop();
}

TEST_F(DFileCopyMoveJobTest, start_checkFileSize)
{
    DFileCopyMoveJobPrivate *jobd = job->d_func();
    ASSERT_TRUE(jobd);
    EXPECT_TRUE(jobd->checkFreeSpace(16));
    jobd->enterDirectory(DUrl(), DUrl());
    EXPECT_TRUE(jobd->checkFileSize(16));
    EXPECT_TRUE(jobd->checkFreeSpace(16));
    DUrl url;
    url.setScheme(FILE_SCHEME);
    url.setPath("~/Videos");
    jobd->leaveDirectory();
    jobd->enterDirectory(url, url);
    Stub st;
    st.set(ADDR(DStorageInfo, fileSystemType), stub_fileSystemType);
    qint64 size = 5l * 1024 * 1024 * 1024;
    EXPECT_FALSE(jobd->checkFileSize(size));
    job->stop();
}

TEST_F(DFileCopyMoveJobTest, start_getNewFileName)
{
    DFileCopyMoveJobPrivate *jobd = job->d_func();
    ASSERT_TRUE(jobd);
    DUrl from, to;
    from.setScheme(FILE_SCHEME);
    from.setPath("./zut_7ztest.7z.2");
    DAbstractFileInfoPointer source = DFileService::instance()->createFileInfo(nullptr, from);
    to = from;
    to.setPath("./");
    DAbstractFileInfoPointer toinfo = DFileService::instance()->createFileInfo(nullptr, to);
    std::cout << from.toLocalFile().toStdString() << std::endl;
    std::cout << jobd->getNewFileName(source, toinfo).toStdString() << std::endl;
    EXPECT_FALSE(jobd->getNewFileName(source, toinfo).isEmpty());
    job->stop();
}
DAbstractFileInfoPointer stub_createFileInfo(const QObject *, const DUrl &)
{
    DUrl tagurl("file:///tmp/zut_test_dir/zut_test_dir");
    return DAbstractFileInfoPointer(new DFileInfo(tagurl));
};
DAbstractFileInfoPointer stub_createFileInfo1(const QObject *, const DUrl &)
{
    return DAbstractFileInfoPointer(nullptr);
};
DFileCopyMoveJob::Action stub_setAndhandleErrorSkip(DFileCopyMoveJob::Error, const DAbstractFileInfoPointer,
                                                    const DAbstractFileInfoPointer, const QString &)
{
    return DFileCopyMoveJob::SkipAction;
};
DFileCopyMoveJob::Action stub_setAndhandleErrorEnforce(DFileCopyMoveJob::Error, const DAbstractFileInfoPointer,
                                                       const DAbstractFileInfoPointer, const QString &)
{
    return DFileCopyMoveJob::EnforceAction;
};
#ifndef __arm__
TEST_F(DFileCopyMoveJobTest, start_doProcess)
{
    DFileCopyMoveJobPrivate *jobd = job->d_func();
    ASSERT_TRUE(jobd);
    ASSERT_FALSE(jobd->doProcess(DUrl(), DAbstractFileInfoPointer(nullptr), DAbstractFileInfoPointer(nullptr)));
    DUrl from, from1, to;
    from.setScheme(FILE_SCHEME);
    from.setPath("./zut_7ztest.7z.2");
    DAbstractFileInfoPointer source = DFileService::instance()->createFileInfo(nullptr, from);
    ErrorHandleMe hanle(job);
    job->setErrorHandle(&hanle, hanle.thread());
    EXPECT_FALSE(jobd->doProcess(DUrl(), source, DAbstractFileInfoPointer(nullptr)));

    from.setPath(TestHelper::createTmpFile());
    source = DFileService::instance()->createFileInfo(nullptr, from);
    EXPECT_FALSE(jobd->doProcess(DUrl(), source, DAbstractFileInfoPointer(nullptr)));

    EXPECT_TRUE(jobd->doProcess(from, source, DAbstractFileInfoPointer(nullptr)));
    from1 = from;
    from1.setPath(TestHelper::createTmpDir());
    source = DFileService::instance()->createFileInfo(nullptr, from1);
    job->setMode(DFileCopyMoveJob::CutMode);
    jobd->setState(DFileCopyMoveJob::IOWaitState);
    job->setFileHints(DFileCopyMoveJob::ForceDeleteFile);
    jobd->setState(DFileCopyMoveJob::RunningState);
    EXPECT_TRUE(jobd->doProcess(from1, source, DAbstractFileInfoPointer(nullptr)));

    from.setPath("./zut_7ztest.7z.2");
    QProcess::execute("touch " + from.toLocalFile());
    to = from;
    to.setPath("./");
    DAbstractFileInfoPointer toinfo = DFileService::instance()->createFileInfo(nullptr, to);
    source = DFileService::instance()->createFileInfo(nullptr, from);
    jobd->enterDirectory(from1, to);
    job->setMode(DFileCopyMoveJob::CopyMode);
    EXPECT_TRUE(jobd->doProcess(from1, source, toinfo));
    jobd->leaveDirectory();
    QProcess::execute("rm " + to.toLocalFile() + "zut_7ztest.7z.2");
    QProcess::execute("rm " + to.toLocalFile() + "zut_7ztest.7z(copy).2");

    DUrl trashurl, desktopurl;
    trashurl.setScheme(FILE_SCHEME);
    trashurl.setPath("~/.local/share/Trash/files/zut_7ztest.7z.2");
    job->setMode(DFileCopyMoveJob::CutMode);
    desktopurl.setScheme(FILE_SCHEME);
    desktopurl.setPath("~/Desktop");
    source = DFileService::instance()->createFileInfo(nullptr, trashurl);
    DAbstractFileInfoPointer taginfo = DFileService::instance()->createFileInfo(nullptr, desktopurl);
    jobd->enterDirectory(trashurl, desktopurl);
    EXPECT_TRUE(jobd->doProcess(trashurl, source, taginfo));
    jobd->leaveDirectory();
    QProcess::execute("rm " + desktopurl.toLocalFile() + "/zut_7ztest.7z.2");

    from.setPath(TestHelper::createTmpFile());
    job->setMode(DFileCopyMoveJob::CutMode);
    jobd->enterDirectory(from, from);
    source = DFileService::instance()->createFileInfo(nullptr, from);
    EXPECT_FALSE(jobd->doProcess(from, source, source));
    jobd->leaveDirectory();
    TestHelper::deleteTmpFile(from.toLocalFile());

    from.setPath(TestHelper::createTmpFile());
    to = from;
    to.setPath("./" + from.fileName());
    QProcess::execute("mkdir " + to.toLocalFile());
    source = DFileService::instance()->createFileInfo(nullptr, from);
    taginfo = DFileService::instance()->createFileInfo(nullptr, to.parentUrl());
    job->setMode(DFileCopyMoveJob::CopyMode);
    jobd->enterDirectory(from, to);
    EXPECT_TRUE(jobd->doProcess(from, source, taginfo));
    jobd->leaveDirectory();
    TestHelper::deleteTmpFiles(QStringList() << from.toLocalFile() << to.toLocalFile()
                               << to.toLocalFile() + "(copy)");

    from.setPath(TestHelper::createTmpFile());
    to = from;
    DUrl linkurl;
    linkurl = from;
    linkurl.setPath("./zut_linksys");
    QProcess::execute("ln -s "  + from.toLocalFile() + " " + linkurl.toLocalFile());
    to.setPath("./");
    source = DFileService::instance()->createFileInfo(nullptr, linkurl);
    taginfo = DFileService::instance()->createFileInfo(nullptr, to);
    job->setMode(DFileCopyMoveJob::CopyMode);
    jobd->setState(DFileCopyMoveJob::IOWaitState);
    job->setFileHints(DFileCopyMoveJob::FollowSymlink);
    jobd->setState(DFileCopyMoveJob::RunningState);
    jobd->enterDirectory(from, to);
    EXPECT_TRUE(jobd->doProcess(from, source, taginfo));
    jobd->setState(DFileCopyMoveJob::IOWaitState);
    job->setFileHints(DFileCopyMoveJob::NoHint);
    jobd->setState(DFileCopyMoveJob::RunningState);
    EXPECT_TRUE(jobd->doProcess(from, source, taginfo));
    jobd->leaveDirectory();
    TestHelper::deleteTmpFiles(QStringList() << from.toLocalFile() << linkurl.toLocalFile()
                               << to.toLocalFile() + linkurl.fileName() << to.toLocalFile() + linkurl.fileName() + "(copy)"
                               << QString("\"" + to.toLocalFile() + linkurl.fileName() + "(copy 1)\""));

    Stub st;
    from.setPath("/tmp/zut_test_dir");
    QProcess::execute("mkdir " + from.toLocalFile());
    job->setMode(DFileCopyMoveJob::CutMode);
    DUrl tagurl("file:///tmp/zut_test_dir/zut_test_dir");
    QProcess::execute("mkdir " + tagurl.toLocalFile());
    jobd->enterDirectory(from, tagurl);
    source = DFileService::instance()->createFileInfo(nullptr, from);
    taginfo = DFileService::instance()->createFileInfo(nullptr, tagurl);

    st.set(ADDR(DFileService, createFileInfo), stub_createFileInfo);
    EXPECT_TRUE(jobd->doProcess(from, source, taginfo));
    st.set(ADDR(DFileCopyMoveJobPrivate, setAndhandleError), stub_setAndhandleErrorSkip);
    EXPECT_TRUE(jobd->doProcess(from, source, taginfo));
    TestHelper::deleteTmpFile(tagurl.toLocalFile());

    st.set(ADDR(DFileCopyMoveJobPrivate, setAndhandleError), stub_setAndhandleErrorEnforce);
    EXPECT_FALSE(jobd->doProcess(from, source, taginfo));
    TestHelper::deleteTmpFile(tagurl.toLocalFile());

    st.set(ADDR(DFileCopyMoveJobPrivate, setAndhandleError), stub_setAndhandleErrorSkip);
    EXPECT_TRUE(jobd->doProcess(from, source, taginfo));

    st.reset(ADDR(DFileCopyMoveJobPrivate, setAndhandleError));
    st.set(ADDR(DFileService, createFileInfo), stub_createFileInfo1);
    EXPECT_FALSE(jobd->doProcess(from, source, taginfo));
    jobd->leaveDirectory();
    TestHelper::deleteTmpFiles(QStringList() << from.toLocalFile() << tagurl.toLocalFile());

    st.reset(ADDR(DFileService, createFileInfo));

    from.setPath(TestHelper::createTmpFile());
    to = from;
    to.setPath("./" + from.fileName());
    QProcess::execute("touch " + to.toLocalFile());
    source = DFileService::instance()->createFileInfo(nullptr, from);
    taginfo = DFileService::instance()->createFileInfo(nullptr, to.parentUrl());
    job->setMode(DFileCopyMoveJob::CopyMode);
    jobd->enterDirectory(from, to);
    DFileCopyMoveJob::Action(*sss2)(DFileCopyMoveJob::Error, const DAbstractFileInfoPointer,
                                    const DAbstractFileInfoPointer, const QString &) = [](DFileCopyMoveJob::Error, const DAbstractFileInfoPointer,
    const DAbstractFileInfoPointer, const QString &) {
        return DFileCopyMoveJob::ReplaceAction;
    };
    st.set(ADDR(DFileCopyMoveJobPrivate, setAndhandleError), sss2);
    EXPECT_NO_FATAL_FAILURE(jobd->doProcess(from, source, taginfo));
    EXPECT_TRUE(jobd->doProcess(to, source, taginfo));
    DFileCopyMoveJob::Action(*sss3)(DFileCopyMoveJob::Error, const DAbstractFileInfoPointer,
                                    const DAbstractFileInfoPointer, const QString &) = [](DFileCopyMoveJob::Error, const DAbstractFileInfoPointer,
    const DAbstractFileInfoPointer, const QString &) {
        return DFileCopyMoveJob::MergeAction;
    };
    st.set(ADDR(DFileCopyMoveJobPrivate, setAndhandleError), sss3);
    EXPECT_FALSE(jobd->doProcess(from, source, taginfo));
    TestHelper::deleteTmpFiles(QStringList() << from.toLocalFile() << to.toLocalFile());
    from.setPath(TestHelper::createTmpDir());
    to = from;
    to.setPath("./" + from.fileName());
    QProcess::execute("mkdir " + to.toLocalFile());
    source = DFileService::instance()->createFileInfo(nullptr, from);
    taginfo = DFileService::instance()->createFileInfo(nullptr, to.parentUrl());
    st.set(ADDR(DFileCopyMoveJobPrivate, setAndhandleError), sss2);
    EXPECT_FALSE(jobd->doProcess(from, source, taginfo));
    st.set(ADDR(DFileCopyMoveJobPrivate, setAndhandleError), sss3);
    EXPECT_TRUE(jobd->doProcess(from, source, taginfo));

    DFileCopyMoveJob::Action(*sss4)(DFileCopyMoveJob::Error, const DAbstractFileInfoPointer,
                                    const DAbstractFileInfoPointer, const QString &) = [](DFileCopyMoveJob::Error, const DAbstractFileInfoPointer,
    const DAbstractFileInfoPointer, const QString &) {
        return DFileCopyMoveJob::CoexistAction;
    };
    st.set(ADDR(DFileCopyMoveJobPrivate, setAndhandleError), sss4);
    EXPECT_TRUE(jobd->doProcess(from, source, taginfo));

    DFileCopyMoveJob::Action(*sss5)(DFileCopyMoveJob::Error, const DAbstractFileInfoPointer,
                                    const DAbstractFileInfoPointer, const QString &) = [](DFileCopyMoveJob::Error, const DAbstractFileInfoPointer,
    const DAbstractFileInfoPointer, const QString &) {
        return DFileCopyMoveJob::SkipAction;
    };
    st.set(ADDR(DFileCopyMoveJobPrivate, setAndhandleError), sss5);
    EXPECT_TRUE(jobd->doProcess(from, source, taginfo));
    jobd->leaveDirectory();
    TestHelper::deleteTmpFiles(QStringList() << from.toLocalFile() << to.toLocalFile());
    QProcess::execute("rm -rf " + to.toLocalFile() + "(copy)");

    bool (*sss6)(qint64) = [](qint64) {return false;};
    st.set(ADDR(DFileCopyMoveJobPrivate, checkFreeSpace), sss6);
    job->setMode(DFileCopyMoveJob::CopyMode);
    jobd->setState(DFileCopyMoveJob::IOWaitState);
    job->setFileHints(DFileCopyMoveJob::NoHint);
    jobd->setState(DFileCopyMoveJob::RunningState);
    from.setPath(TestHelper::createTmpFile());
    to.setPath("./");
    source = DFileService::instance()->createFileInfo(nullptr, from);
    taginfo = DFileService::instance()->createFileInfo(nullptr, to);
    jobd->enterDirectory(from, to);
    st.set(ADDR(DFileCopyMoveJobPrivate, setAndhandleError), sss5);
    EXPECT_TRUE(jobd->doProcess(from, source, taginfo));
    DFileCopyMoveJob::Action(*sss7)(DFileCopyMoveJob::Error, const DAbstractFileInfoPointer,
                                    const DAbstractFileInfoPointer, const QString &) = [](DFileCopyMoveJob::Error, const DAbstractFileInfoPointer,
    const DAbstractFileInfoPointer, const QString &) {
        return DFileCopyMoveJob::EnforceAction;
    };
    st.set(ADDR(DFileCopyMoveJobPrivate, setAndhandleError), sss7);
    EXPECT_TRUE(jobd->doProcess(from, source, taginfo));
    DFileCopyMoveJob::Action(*sss8)(DFileCopyMoveJob::Error, const DAbstractFileInfoPointer,
                                    const DAbstractFileInfoPointer, const QString &) = [](DFileCopyMoveJob::Error, const DAbstractFileInfoPointer,
    const DAbstractFileInfoPointer, const QString &) {
        return DFileCopyMoveJob::NoAction;
    };
    st.set(ADDR(DFileCopyMoveJobPrivate, setAndhandleError), sss8);
    EXPECT_FALSE(jobd->doProcess(from, source, taginfo));

    st.reset(ADDR(DFileCopyMoveJobPrivate, checkFreeSpace));
    bool (*sss9)(qint64) = [](qint64) {return false;};
    st.set(ADDR(DFileCopyMoveJobPrivate, checkFileSize), sss9);
    st.set(ADDR(DFileCopyMoveJobPrivate, setAndhandleError), sss5);
    TestHelper::deleteTmpFiles(QStringList() << to.toLocalFile() + from.fileName());
    EXPECT_TRUE(jobd->doProcess(from, source, taginfo));
    st.set(ADDR(DFileCopyMoveJobPrivate, setAndhandleError), sss7);
    EXPECT_FALSE(jobd->doProcess(from, source, taginfo));
    TestHelper::deleteTmpFiles(QStringList() << to.toLocalFile() + from.fileName());
    st.set(ADDR(DFileCopyMoveJobPrivate, setAndhandleError), sss8);
    EXPECT_FALSE(jobd->doProcess(from, source, taginfo));


    jobd->leaveDirectory();
    TestHelper::deleteTmpFiles(QStringList() << from.toLocalFile() <<
                               to.toLocalFile() + from.fileName());
    job->stop();
}
#endif
TEST_F(DFileCopyMoveJobTest, start_mergeDirectory)
{
    DFileCopyMoveJobPrivate *jobd = job->d_func();
    ASSERT_TRUE(jobd);
    Stub st;
    bool (*isVaultFile)(QString) = [](QString) {return true;};
    DUrl from, to;
    from.setScheme(FILE_SCHEME);
    from.setPath(TestHelper::createTmpDir());
    to = from;
    to.setPath("./fsjkaaaghfgggggsdfgdsfgsdfgsdfgsdfgaajksdhgajskdhgjkashdjkfasfsdsgsdfgsdfgsdfglah\
               gdrgdfgsdfgsdfgsdsgddddddddddddddddddddddddddddddddgsdggsdsdfffffggggggggggsdfgsdgsdfgg\
               grgsdfgdfjklasndjkasvvskncnkz.hnjxvjkrshgkejrhguierhgkjashdfgjlkashdfgjkahsgjhasjghakghja");
    DAbstractFileInfoPointer frominfo = DFileService::instance()->createFileInfo(nullptr, from);
    DAbstractFileInfoPointer toinfo = DFileService::instance()->createFileInfo(nullptr, to);
    st.set(ADDR(VaultController, isVaultFile), isVaultFile);
    EXPECT_FALSE(jobd->mergeDirectory(nullptr, frominfo, toinfo));
    QSharedPointer<DFileHandler>  handler(DFileService::instance()->createFileHandler(nullptr, from));
    to.setPath("/zut_mergeDirectory_kk");
    toinfo = DFileService::instance()->createFileInfo(nullptr, to);
    EXPECT_FALSE(jobd->mergeDirectory(handler, frominfo, toinfo));
    to.setPath("./zut_mergeDirectory_kk");
    toinfo = DFileService::instance()->createFileInfo(nullptr, to);
    EXPECT_TRUE(jobd->mergeDirectory(handler, frominfo, toinfo));
    TestHelper::deleteTmpFiles(QStringList() << to.toLocalFile() << from.toLocalFile());
    DDirIteratorPointer(*createDirIterator)(const QObject, const DUrl, const QStringList, QDir::Filters,
                                            QDirIterator::IteratorFlags, bool silent, bool isgvfs) = \
                                                                                                     [](const QObject, const DUrl, const QStringList, QDir::Filters,
    QDirIterator::IteratorFlags, bool silent, bool isgvfs) {
        return DDirIteratorPointer(nullptr);
    };
    st.set(ADDR(DFileService, createDirIterator), createDirIterator);
    from.setPath(TestHelper::createTmpDir());
    frominfo = DFileService::instance()->createFileInfo(nullptr, from);
    QProcess::execute("touch " + from.toLocalFile() + "/z_ut_iuuuuuu");
    EXPECT_FALSE(jobd->mergeDirectory(handler, frominfo, toinfo));
    st.reset(ADDR(DFileService, createDirIterator));
    bool (*stateCheck)(void *) = [](void *) {return false;};
    st.set(ADDR(DFileCopyMoveJobPrivate, stateCheck), stateCheck);
    EXPECT_FALSE(jobd->mergeDirectory(handler, frominfo, toinfo));
    TestHelper::deleteTmpFiles(QStringList() << from.toLocalFile() << to.toLocalFile());
    job->stop();
}

TEST_F(DFileCopyMoveJobTest, start_run_moveMOde)
{
    DUrl from;
    DFileCopyMoveJobPrivate *jobd = job->d_func();
    ASSERT_TRUE(jobd);
    from.setScheme(FILE_SCHEME);
    from.setPath(TestHelper::createTmpDir());
    QProcess::execute("touch " + from.toLocalFile() + "/z_ut_iuuuuuu");
    job->setMode(DFileCopyMoveJob::MoveMode);
    job->start(DUrlList() << from, DUrl());
    while (!job->isFinished()) {
        QThread::msleep(100);
    }

    job->setCurTrashData(QVariant::fromValue(std::forward<DUrlList>(DUrlList() << from)));
    job->setActionOfErrorType(DFileCopyMoveJob::NoError, DFileCopyMoveJob::NoAction);
    EXPECT_EQ(DFileCopyMoveJob::NoError, job->error());
    EXPECT_EQ(DFileCopyMoveJob::NoHint, job->fileHints());
    EXPECT_FALSE(job->targetUrl().isValid());
    EXPECT_FALSE(job->fileStatisticsIsFinished());
    EXPECT_FALSE(job->completedDirectorys().empty());
    EXPECT_TRUE(job->isCanShowProgress());
    job->stop();
}

TEST_F(DFileCopyMoveJobTest, start_getWriteBytes)
{
    DFileCopyMoveJobPrivate *jobd = job->d_func();
    ASSERT_TRUE(jobd);
    EXPECT_TRUE(0 == jobd->getWriteBytes(-1));
    DUrl from;
    from.setScheme(FILE_SCHEME);
    from.setPath("/proc/self/task/9018765463");
    QProcess::execute("mkdir " + from.toLocalFile());
    from.setPath("/proc/self/task/9018765463/io");
    QProcess::execute("touch " + from.toLocalFile());
    EXPECT_TRUE(0 == jobd->getWriteBytes(9018765463));
    QFile file(from.toLocalFile());
    if (file.open(QIODevice::WriteOnly)) {
        file.write(QByteArray("write_bytes: ioio"));
        file.close();
    }
    EXPECT_TRUE(0 == jobd->getWriteBytes(9018765463));
    TestHelper::deleteTmpFile("/proc/self/task/9018765463");
    job->stop();
}

TEST_F(DFileCopyMoveJobTest, start_supportActions)
{
    EXPECT_EQ(DFileCopyMoveJob::SkipAction | DFileCopyMoveJob::RetryAction | DFileCopyMoveJob::CancelAction,
              job->supportActions(DFileCopyMoveJob::PermissionError));
    EXPECT_EQ(DFileCopyMoveJob::SkipAction | DFileCopyMoveJob::RetryAction | DFileCopyMoveJob::CancelAction,
              job->supportActions(DFileCopyMoveJob::OpenError));
    EXPECT_EQ(DFileCopyMoveJob::SkipAction | DFileCopyMoveJob::RetryAction | DFileCopyMoveJob::CancelAction,
              job->supportActions(DFileCopyMoveJob::ReadError));
    EXPECT_EQ(DFileCopyMoveJob::SkipAction | DFileCopyMoveJob::RetryAction | DFileCopyMoveJob::CancelAction,
              job->supportActions(DFileCopyMoveJob::WriteError));
    EXPECT_EQ(DFileCopyMoveJob::SkipAction | DFileCopyMoveJob::RetryAction | DFileCopyMoveJob::CancelAction,
              job->supportActions(DFileCopyMoveJob::SymlinkError));
    EXPECT_EQ(DFileCopyMoveJob::SkipAction | DFileCopyMoveJob::RetryAction | DFileCopyMoveJob::CancelAction,
              job->supportActions(DFileCopyMoveJob::MkdirError));
    EXPECT_EQ(DFileCopyMoveJob::SkipAction | DFileCopyMoveJob::RetryAction | DFileCopyMoveJob::CancelAction,
              job->supportActions(DFileCopyMoveJob::ResizeError));
    EXPECT_EQ(DFileCopyMoveJob::SkipAction | DFileCopyMoveJob::RetryAction | DFileCopyMoveJob::CancelAction,
              job->supportActions(DFileCopyMoveJob::RemoveError));
    EXPECT_EQ(DFileCopyMoveJob::SkipAction | DFileCopyMoveJob::RetryAction | DFileCopyMoveJob::CancelAction,
              job->supportActions(DFileCopyMoveJob::RenameError));
    EXPECT_EQ(DFileCopyMoveJob::SkipAction | DFileCopyMoveJob::RetryAction | DFileCopyMoveJob::CancelAction,
              job->supportActions(DFileCopyMoveJob::IntegrityCheckingError));
    EXPECT_EQ(DFileCopyMoveJob::SkipAction | DFileCopyMoveJob::CancelAction,
              job->supportActions(DFileCopyMoveJob::SpecialFileError));
    EXPECT_EQ(DFileCopyMoveJob::SkipAction | DFileCopyMoveJob::EnforceAction | DFileCopyMoveJob::CancelAction,
              job->supportActions(DFileCopyMoveJob::FileSizeTooBigError));
    EXPECT_EQ(DFileCopyMoveJob::SkipAction | DFileCopyMoveJob::EnforceAction | DFileCopyMoveJob::CancelAction | DFileCopyMoveJob::RetryAction,
              job->supportActions(DFileCopyMoveJob::NotEnoughSpaceError));

    EXPECT_EQ(DFileCopyMoveJob::SkipAction | DFileCopyMoveJob::CoexistAction | DFileCopyMoveJob::CancelAction | DFileCopyMoveJob::ReplaceAction,
              job->supportActions(DFileCopyMoveJob::FileExistsError));
    EXPECT_EQ(DFileCopyMoveJob::SkipAction | DFileCopyMoveJob::CoexistAction | DFileCopyMoveJob::CancelAction | DFileCopyMoveJob::MergeAction,
              job->supportActions(DFileCopyMoveJob::DirectoryExistsError));
    EXPECT_EQ(DFileCopyMoveJob::SkipAction | DFileCopyMoveJob::EnforceAction,
              job->supportActions(DFileCopyMoveJob::TargetReadOnlyError));
    EXPECT_EQ(DFileCopyMoveJob::SkipAction | DFileCopyMoveJob::EnforceAction,
              job->supportActions(DFileCopyMoveJob::TargetIsSelfError));
    EXPECT_EQ(DFileCopyMoveJob::SkipAction | DFileCopyMoveJob::CancelAction,
              job->supportActions(DFileCopyMoveJob::UnknowError));
    job->stop();
}

TEST_F(DFileCopyMoveJobTest, start_process)
{
    DFileCopyMoveJobPrivate *jobd = job->d_func();
    ASSERT_TRUE(jobd);
    DUrl from, to;
    from.setScheme(FILE_SCHEME);
    from.setPath(TestHelper::createTmpFile());
    to = from;
    to.setPath("./");
    DAbstractFileInfoPointer toinfo = DFileService::instance()->createFileInfo(nullptr, to);
    EXPECT_FALSE(jobd->process(from, toinfo));
    TestHelper::deleteTmpFiles(QStringList() << from.toLocalFile() << to.toLocalFile() + from.fileName());
    job->stop();
}
TEST_F(DFileCopyMoveJobTest, start_doRemoveFile)
{
    DFileCopyMoveJobPrivate *jobd = job->d_func();
    ASSERT_TRUE(jobd);
    DUrl from, to;
    from.setScheme(FILE_SCHEME);
    QString path = TestHelper::createTmpFile();
    from.setPath(path + "_ooo");
    QSharedPointer<DFileHandler>  handler(DFileService::instance()->createFileHandler(nullptr, from));
    DAbstractFileInfoPointer frominfo = DFileService::instance()->createFileInfo(nullptr, from);
    EXPECT_TRUE(jobd->doRemoveFile(handler, frominfo));

    Stub st;
    bool (*isVaultFile)(QString) = [](QString) {return true;};
    st.set(ADDR(VaultController, isVaultFile), isVaultFile);
    from.setPath(path);
    handler.reset(DFileService::instance()->createFileHandler(nullptr, from));
    frominfo = DFileService::instance()->createFileInfo(nullptr, from);
    VaultController::FileBaseInfo(*getFileInfo)(const DUrl &) = [](const DUrl &) {
        VaultController::FileBaseInfo stl;
        stl.isWritable = false;
        return stl;
    };
    st.set(ADDR(VaultController, getFileInfo), getFileInfo);
    EXPECT_FALSE(jobd->doRemoveFile(handler, frominfo));
    TestHelper::deleteTmpFiles(QStringList() << path);
    job->stop();
}

TEST_F(DFileCopyMoveJobTest, start_doRenameFile)
{
    DFileCopyMoveJobPrivate *jobd = job->d_func();
    ASSERT_TRUE(jobd);
    DUrl from, to;
    from.setScheme(FILE_SCHEME);
    from.setPath(TestHelper::createTmpFile());
    DUrl linkurl, linkurlnew;
    linkurl = from;
    to = from;
    QThread::msleep(10);
    to.setPath(TestHelper::createTmpFile());
    linkurl.setPath("./zut_linksys");
    linkurlnew = from;
    linkurlnew.setPath("./zut_linksys_new_o");
    QProcess::execute("ln -s " + from.toLocalFile() + linkurl.toLocalFile());
    QProcess::execute("ln -s " + to.toLocalFile() + linkurlnew.toLocalFile());
    QProcess::execute("chmod 0000 " + to.toLocalFile());
    QSharedPointer<DFileHandler>  handler(DFileService::instance()->createFileHandler(nullptr, linkurl));
    DAbstractFileInfoPointer oldinfo = DFileService::instance()->createFileInfo(nullptr, linkurl);
    DAbstractFileInfoPointer newinfo = DFileService::instance()->createFileInfo(nullptr, linkurlnew);
    jobd->enterDirectory(linkurl, linkurlnew);
    EXPECT_FALSE(jobd->doRenameFile(handler, oldinfo, newinfo));

    jobd->setState(DFileCopyMoveJob::RunningState);
    QProcess::execute("chmod 0777 " + to.toLocalFile());
    EXPECT_TRUE(jobd->doRenameFile(handler, oldinfo, newinfo));
    jobd->leaveDirectory();
    TestHelper::deleteTmpFiles(QStringList() << linkurlnew.toLocalFile() << linkurl.toLocalFile() << from.toLocalFile() << to.toLocalFile());
    job->stop();
}

TEST_F(DFileCopyMoveJobTest, start_doLinkFile)
{
    DFileCopyMoveJobPrivate *jobd = job->d_func();
    ASSERT_TRUE(jobd);
    DUrl from, to;
    from.setScheme(FILE_SCHEME);
    from.setPath("./bin");
    QString fromPath = from.path();
    QProcess::execute("mkdir " + from.path());
    QString linkpath = QDir::currentPath() + "/zut_syslink_tset";
    QSharedPointer<DFileHandler>  handler(DFileService::instance()->createFileHandler(nullptr, from));
    DAbstractFileInfoPointer fileinfo = DFileService::instance()->createFileInfo(nullptr, from);
    EXPECT_TRUE(jobd->doLinkFile(handler, fileinfo, linkpath));
    from.setPath(QDir::currentPath() + "/zut_syslink_tset");
    fileinfo->refresh();
    EXPECT_TRUE(jobd->doLinkFile(handler, fileinfo, linkpath));
    TestHelper::deleteTmpFiles(QStringList() << from.toLocalFile() << linkpath << fromPath);
    job->stop();
}
class devicetest : public  DFileDevice
{
    bool open(OpenMode mode) override
    {
        Q_UNUSED(mode);
        return false;
    }
    bool resize(qint64 size) override
    {
        Q_UNUSED(size);
        return false;
    }
public:
    ~devicetest() override {}
protected:
    qint64 readData(char *data, qint64 maxlen) override
    {
        Q_UNUSED(data);
        Q_UNUSED(maxlen);
        return 0;
    }
    qint64 writeData(const char *data, qint64 len) override
    {
        Q_UNUSED(data);
        Q_UNUSED(len);
        return 0;
    }
};
class devicetest1 : public  DFileDevice
{
    bool open(OpenMode mode) override
    {
        Q_UNUSED(mode);
        return true;
    }
    bool resize(qint64 size) override
    {
        Q_UNUSED(size);
        return false;
    }
public:
    ~devicetest1() override {}
protected:
    qint64 readData(char *data, qint64 maxlen) override
    {
        Q_UNUSED(data);
        Q_UNUSED(maxlen);
        return 0;
    }
    qint64 writeData(const char *data, qint64 len) override
    {
        Q_UNUSED(data);
        Q_UNUSED(len);
        return 0;
    }
};
class devicetest2 : public  DFileDevice
{
public:
    ~devicetest2() override {}
protected:
    qint64 readData(char *data, qint64 maxlen) override
    {
        Q_UNUSED(data);
        Q_UNUSED(maxlen);
        QThread::msleep(200);
        return 0;
    }
    qint64 writeData(const char *data, qint64 len) override
    {
        Q_UNUSED(data);
        Q_UNUSED(len);
        return 0;
    }
};

class devicetest3 : public  DFileDevice
{
    bool seek(qint64 pos) override
    {
        return false;
    }
public:
    ~devicetest3() override {}
protected:
    qint64 readData(char *data, qint64 maxlen) override
    {
        Q_UNUSED(data);
        Q_UNUSED(maxlen);
        return -1;
    }
    qint64 writeData(const char *data, qint64 len) override
    {
        Q_UNUSED(data);
        Q_UNUSED(len);
        return 0;
    }
};
class devicetest4 : public  DFileDevice
{
protected:
    qint64 readData(char *data, qint64 maxlen) override
    {
        Q_UNUSED(data);
        Q_UNUSED(maxlen);
        return 0;
    }
    qint64 writeData(const char *data, qint64 len) override
    {
        Q_UNUSED(data);
        Q_UNUSED(len);
        QThread::msleep(200);
        return 0;
    }
public:
    ~devicetest4() override {}
};
class devicetest5 : public  DFileDevice
{
public:
    ~devicetest5() override {}
protected:
    qint64 readData(char *data, qint64 maxlen) override
    {
        Q_UNUSED(data);
        Q_UNUSED(maxlen);
        return -1;
    }
    qint64 writeData(const char *data, qint64 len) override
    {
        Q_UNUSED(data);
        Q_UNUSED(len);
        return -1;
    }
};

TEST_F(DFileCopyMoveJobTest, start_doCopyFile)
{
    DFileCopyMoveJobPrivate *jobd = job->d_func();
    ASSERT_TRUE(jobd);
    DUrl from, to;
    from.setScheme(FILE_SCHEME);
    from.setPath(TestHelper::createTmpFile());
    QFile filefrom(from.toLocalFile());
    if (filefrom.open(QIODevice::WriteOnly)) {
        QByteArray data(64, 'f');
        filefrom.write(data);
        filefrom.close();
    }
    Stub stl;
    DFileDevice* (*createFileDevice)(const QObject *, const DUrl &) = [](const QObject *, const DUrl &) {
        DFileDevice *device = nullptr;
        return device;
    };
    stl.set(ADDR(DFileService, createFileDevice), createFileDevice);
    QSharedPointer<DFileHandler>  handler(DFileService::instance()->createFileHandler(nullptr, from));
    DAbstractFileInfoPointer frominfo = DFileService::instance()->createFileInfo(nullptr, from);
    to = from;
    to.setPath(QDir::currentPath() + "/zut_test_file_device");
    DAbstractFileInfoPointer toinfo = DFileService::instance()->createFileInfo(nullptr, to);
    EXPECT_FALSE(jobd->doCopyFile(frominfo, toinfo, handler));

    DFileDevice* (*createFileDevice1)(void *, const QObject *, const DUrl & url) = [](void *, const QObject *, const DUrl & url) {
        DFileDevice *device = nullptr;
        if (url.toLocalFile() == QDir::currentPath() + "/zut_test_file_device") {
            return device;
        }
        device = new DLocalFileDevice();
        device->setFileUrl(url);
        return device;
    };

    stl.set(ADDR(DFileService, createFileDevice), createFileDevice1);
    EXPECT_FALSE(jobd->doCopyFile(frominfo, toinfo, handler));

    stl.reset(ADDR(DFileService, createFileDevice));
    bool (*isVaultFile)(QString) = [](QString) {return true;};
    stl.set(ADDR(VaultController, isVaultFile), isVaultFile);
    to.setPath("./fsjkaaaghfgggggsdfgdsfgsdfgsdfgsdfgaajksdhgajskdhgjkashdjkfasfsdsgsdfgsdfgsdfglah\
               gdrgdfgsdfgsdfgsdsgddddddddddddddddddddddddddddddddgsdggsdsdfffffggggggggggsdfgsdgsdfgg\
               grgsdfgdfjklasndjkasvvskncnkz.hnjxvjkrshgkejrhguierhgkjashdfgjlkashdfgjkahsgjhasjghakghja");
    toinfo = DFileService::instance()->createFileInfo(nullptr, to);
    EXPECT_FALSE(jobd->doCopyFile(frominfo, toinfo, handler));
    to.setPath(QDir::currentPath() + "/zut_test_file_device");
    toinfo = DFileService::instance()->createFileInfo(nullptr, to);
    stl.reset(ADDR(VaultController, isVaultFile));

    DFileDevice* (*createFileDevice2)(void *, const QObject *, const DUrl &) = [](void *, const QObject *, const DUrl & url) {
        DFileDevice *device = nullptr;
        if (url.toLocalFile() != QDir::currentPath() + "/zut_test_file_device") {
            device = new DLocalFileDevice();
            device->setFileUrl(url);
        } else {
            device = new devicetest();
        }
        return device;
    };
    stl.set(ADDR(DFileService, createFileDevice), createFileDevice2);
    EXPECT_FALSE(jobd->doCopyFile(frominfo, toinfo, handler));
    DFileDevice* (*createFileDevice3)(void *, const QObject *, const DUrl &) = [](void *, const QObject *, const DUrl & url) {
        DFileDevice *device = nullptr;
        if (url.toLocalFile() == QDir::currentPath() + "/zut_test_file_device") {
            device = new DLocalFileDevice();
            device->setFileUrl(url);
        } else {
            device = new devicetest();
        }
        return device;
    };
    stl.set(ADDR(DFileService, createFileDevice), createFileDevice3);
    EXPECT_FALSE(jobd->doCopyFile(frominfo, toinfo, handler));
    DFileDevice* (*createFileDevice4)(void *, const QObject *, const DUrl &) = [](void *, const QObject *, const DUrl & url) {
        DFileDevice *device = nullptr;
        if (url.toLocalFile() == QDir::currentPath() + "/zut_test_file_device") {
            device = new DLocalFileDevice();
            device->setFileUrl(url);
        } else {
            device = new devicetest();
            device->setErrorString("Permission denied");
        }
        return device;
    };
    stl.set(ADDR(DFileService, createFileDevice), createFileDevice4);
    EXPECT_FALSE(jobd->doCopyFile(frominfo, toinfo, handler));

    DFileDevice* (*createFileDevice5)(void *, const QObject *, const DUrl &) = [](void *, const QObject *, const DUrl & url) {
        DFileDevice *device = nullptr;
        if (url.toLocalFile() != QDir::currentPath() + "/zut_test_file_device") {
            device = new DLocalFileDevice();
            device->setFileUrl(url);
        } else {
            device = new devicetest1();
        }
        return device;
    };
    job->setFileHints(DFileCopyMoveJob::ResizeDestinationFile);
    stl.set(ADDR(DFileService, createFileDevice), createFileDevice5);
    EXPECT_FALSE(jobd->doCopyFile(frominfo, toinfo, handler));
    DFileCopyMoveJob::Action(*ssse3)(DFileCopyMoveJob::Error, const DAbstractFileInfoPointer,
                                     const DAbstractFileInfoPointer, const QString &) = [](DFileCopyMoveJob::Error, const DAbstractFileInfoPointer,
    const DAbstractFileInfoPointer, const QString &) {
        return DFileCopyMoveJob::SkipAction;
    };
    stl.set(ADDR(DFileCopyMoveJobPrivate, setAndhandleError), ssse3);
    EXPECT_FALSE(jobd->doCopyFile(frominfo, toinfo, handler));
    stl.reset(ADDR(DFileCopyMoveJobPrivate, setAndhandleError));
    job->setFileHints(DFileCopyMoveJob::NoHint);


    DFileDevice* (*createFileDevice6)(void *, const QObject *, const DUrl &) = [](void *, const QObject *, const DUrl & url) {
        DFileDevice *device = nullptr;
        if (url.toLocalFile() == QDir::currentPath() + "/zut_test_file_device") {
            device = new DLocalFileDevice();
            device->setFileUrl(url);
        } else {
            device = new devicetest2();
        }
        return device;
    };
    stl.set(ADDR(DFileService, createFileDevice), createFileDevice6);
    QFuture<void> future = QtConcurrent::run([ = ]() {
        EXPECT_FALSE(jobd->doCopyFile(frominfo, toinfo, handler));
    });
    QThread::msleep(100);
    job->stop();
    while (future.isRunning()) {
        QThread::msleep(100);
    }
    jobd->setState(DFileCopyMoveJob::RunningState);


    DFileDevice* (*createFileDevice7)(void *, const QObject *, const DUrl &) = [](void *, const QObject *, const DUrl & url) {
        DFileDevice *device = nullptr;
        if (url.toLocalFile() == QDir::currentPath() + "/zut_test_file_device") {
            device = new DLocalFileDevice();
            device->setFileUrl(url);
        } else {
            device = new devicetest3();
        }
        return device;
    };
    stl.set(ADDR(DFileService, createFileDevice), createFileDevice7);
    EXPECT_TRUE(jobd->doCopyFile(frominfo, toinfo, handler));

    DFileDevice* (*createFileDevice8)(void *, const QObject *, const DUrl &) = [](void *, const QObject *, const DUrl & url) {
        DFileDevice *device = nullptr;
        if (url.toLocalFile() == QDir::currentPath() + "/zut_test_file_device") {
            device = new DLocalFileDevice();
            device->setFileUrl(url);
        } else {
            device = new devicetest3();
        }
        return device;
    };
    stl.set(ADDR(DFileService, createFileDevice), createFileDevice8);
    stl.set(ADDR(DFileCopyMoveJobPrivate, setAndhandleError), ssse3);
    EXPECT_TRUE(jobd->doCopyFile(frominfo, toinfo, handler));
    DFileCopyMoveJob::Action(*ssse4)(DFileCopyMoveJob::Error, const DAbstractFileInfoPointer,
                                     const DAbstractFileInfoPointer, const QString &) = [](DFileCopyMoveJob::Error, const DAbstractFileInfoPointer,
    const DAbstractFileInfoPointer, const QString &) {
        return DFileCopyMoveJob::RetryAction;
    };
    stl.set(ADDR(DFileCopyMoveJobPrivate, setAndhandleError), ssse4);
    EXPECT_TRUE(jobd->doCopyFile(frominfo, toinfo, handler));


    stl.reset(ADDR(DFileCopyMoveJobPrivate, setAndhandleError));


    DFileDevice* (*createFileDevice9)(void *, const QObject *, const DUrl &) = [](void *, const QObject *, const DUrl & url) {
        DFileDevice *device = nullptr;
        if (url.toLocalFile() != QDir::currentPath() + "/zut_test_file_device") {
            device = new DLocalFileDevice();
            device->setFileUrl(url);
        } else {
            device = new devicetest4();
        }
        return device;
    };
    stl.set(ADDR(DFileService, createFileDevice), createFileDevice9);
    QFuture<void> future1 = QtConcurrent::run([ = ]() {
        EXPECT_TRUE(jobd->doCopyFile(frominfo, toinfo, handler));
    });
    QThread::msleep(100);
    job->stop();
    while (future1.isRunning()) {
        QThread::msleep(100);
    }
    jobd->setState(DFileCopyMoveJob::RunningState);

    DFileDevice* (*createFileDevice10)(void *, const QObject *, const DUrl &) = [](void *, const QObject *, const DUrl & url) {
        DFileDevice *device = nullptr;
        if (url.toLocalFile() != QDir::currentPath() + "/zut_test_file_device") {
            device = new DLocalFileDevice();
            device->setFileUrl(url);
        } else {
            device = new devicetest5();
        }
        return device;
    };
    stl.set(ADDR(DFileService, createFileDevice), createFileDevice10);
    EXPECT_TRUE(jobd->doCopyFile(frominfo, toinfo, handler));

    DFileDevice* (*createFileDevice11)(void *, const QObject *, const DUrl &) = [](void *, const QObject *, const DUrl & url) {
        DFileDevice *device = nullptr;
        if (url.toLocalFile() != QDir::currentPath() + "/zut_test_file_device") {
            device = new DLocalFileDevice();
            device->setFileUrl(url);
        } else {
            device = new devicetest2();
        }
        return device;
    };
    stl.set(ADDR(DFileService, createFileDevice), createFileDevice11);
    stl.set(ADDR(DFileCopyMoveJobPrivate, setAndhandleError), ssse3);
    EXPECT_TRUE(jobd->doCopyFile(frominfo, toinfo, handler));
    job->stop();

    TestHelper::deleteTmpFiles(QStringList() << from.toLocalFile() << to.toLocalFile());
}

static int setAndhandleErrorExNew = 0;
DFileCopyMoveJob::Action setAndhandleErrorEx(DFileCopyMoveJob::Error, const DAbstractFileInfoPointer,
                                             const DAbstractFileInfoPointer, const QString &)
{
    std::cout << "setAndhandleErrorExNew  =========   " << setAndhandleErrorExNew << std::endl;
    if (setAndhandleErrorExNew % 2 == 0) {
        setAndhandleErrorExNew++;
        return DFileCopyMoveJob::RetryAction;
    }
    setAndhandleErrorExNew++;
    return DFileCopyMoveJob::SkipAction;
};

int openExTest(const char *, int, ...)
{
    if (setAndhandleErrorExNew % 2 == 0) {
        setAndhandleErrorExNew++;
        return 1;
    }
    setAndhandleErrorExNew++;
    return -1;
}

int openTest(const char *, int, ...)
{
    return -1;
}

void *mmapExTest(void *, size_t, int, int, int, __off_t)
{
    if (setAndhandleErrorExNew % 2 == 0) {
        setAndhandleErrorExNew++;
        return ((void *) 1);
    }
    setAndhandleErrorExNew++;
    return MAP_FAILED;
}

class devicetest6 : public  DFileDevice
{
    bool open(OpenMode mode) override
    {
        Q_UNUSED(mode);
        return false;
    }
    bool resize(qint64 size) override
    {
        Q_UNUSED(size);
        return false;
    }
public:
    ~devicetest6() override {}
protected:
    qint64 readData(char *data, qint64 maxlen) override
    {
        Q_UNUSED(data);
        Q_UNUSED(maxlen);
        return 0;
    }
    qint64 writeData(const char *data, qint64 len) override
    {
        Q_UNUSED(data);
        Q_UNUSED(len);
        return 0;
    }
};

class devicetest7 : public  DFileDevice
{
    bool open(OpenMode mode) override
    {
        Q_UNUSED(mode);
        return true;
    }
    bool resize(qint64 size) override
    {
        Q_UNUSED(size);
        return false;
    }
    bool seek(qint64 pos) override
    {
        Q_UNUSED(pos);
        return false;
    }
public:
    ~devicetest7() override {}
protected:
    qint64 readData(char *data, qint64 maxlen) override
    {
        Q_UNUSED(data);
        Q_UNUSED(maxlen);
        return -1;
    }
    qint64 writeData(const char *data, qint64 len) override
    {
        Q_UNUSED(data);
        Q_UNUSED(len);
        return 0;
    }
};

DFileDevice *createFileDevicelamda(const QObject *, const DUrl &)
{
    DFileDevice *device = nullptr;
    return device;
};
DFileDevice *createFileDevicelamda1(void *, const QObject *, const DUrl &url)
{
    DFileDevice *device = nullptr;
    if (url.toLocalFile() == QDir::currentPath() + "/zut_test_file_device") {
        return device;
    }
    device = new DLocalFileDevice();
    device->setFileUrl(url);
    return device;
};

TEST_F(DFileCopyMoveJobTest, start_doCopyFileU)
{
    DFileCopyMoveJobPrivate *jobd = job->d_func();
    ASSERT_TRUE(jobd);
    DUrl from(DUrl::fromLocalFile(TestHelper::createTmpFile())), to;
    QFile filefrom(from.toLocalFile());
    if (filefrom.open(QIODevice::WriteOnly)) {
        QByteArray data(10240, 'f');
        filefrom.write(data);
        filefrom.close();
    }
    StubExt stl;
    to = from;
    QString longname = QString("_jksfjdflsadjflasdjfalsdjflksdjkflasdjkflsjdkflsdjflksdjflskdjfjj\
        gjdlkfgjsldfjgklsdjfgklsjdfklgjsdkfjgklsdfjgklsdjfglkdsjfgjksdfg\
        gjskdfjgsdfjglsdmvskdlfjgkdsjfgksdjfgksdfjgkdjfgkdkfgsldgjkdfjgk\
        gkdfslgjdfkkgjskldfjgklsdjfgklsdfjgklsdjfgklsdjfgklsdjfkgsjdfkgjdk\
        jsdfkljgssssssssssgsjdklfjgklsdjfgklsjdfgklsjdfkgjsdlfjgklsdfjkdfjg");
    to.setPath(QDir::currentPath() + "/zut_test_file_device" + longname);
    QSharedPointer<DFileHandler>  handler(DFileService::instance()->createFileHandler(nullptr, from));
    DAbstractFileInfoPointer frominfo = DFileService::instance()->createFileInfo(nullptr, from);
    DAbstractFileInfoPointer toinfo = DFileService::instance()->createFileInfo(nullptr, to);

    stl.set_lamda(&VaultController::isVaultFile, []() {return true;});
    stl.set_lamda(&DFileCopyMoveJobPrivate::setAndhandleError,
    []() {return DFileCopyMoveJob::SkipAction;});
    EXPECT_TRUE(jobd->doCopyFileOnBlock(frominfo, toinfo, handler));

    stl.reset(&VaultController::isVaultFile);
    to.setPath(QDir::currentPath() + "/zut_test_file_device");
    toinfo = DFileService::instance()->createFileInfo(nullptr, to);
    stl.set(open, openTest);
    stl.reset(&DFileCopyMoveJobPrivate::setAndhandleError);
    stl.set_lamda(&DFileCopyMoveJobPrivate::setAndhandleError,
    []() {return DFileCopyMoveJob::RetryAction;});
    EXPECT_FALSE(jobd->doCopyFileOnBlock(frominfo, toinfo, handler));

    stl.reset(&DFileCopyMoveJobPrivate::setAndhandleError);
    stl.set_lamda(&DFileCopyMoveJobPrivate::setAndhandleError,
    []() {return DFileCopyMoveJob::SkipAction;});
    EXPECT_TRUE(jobd->doCopyFileOnBlock(frominfo, toinfo, handler));
    TestHelper::deleteTmpFile(to.toLocalFile());

    setAndhandleErrorExNew = setAndhandleErrorExNew % 2 == 0 ?
                             setAndhandleErrorExNew : setAndhandleErrorExNew + 1;
    stl.set(open, openExTest);
    stl.set_lamda(::close, []() {return 0;});
    stl.reset(&DFileCopyMoveJobPrivate::setAndhandleError);
    stl.set_lamda(&DFileCopyMoveJobPrivate::setAndhandleError,
    []() {return DFileCopyMoveJob::RetryAction;});
    EXPECT_FALSE(jobd->doCopyFileOnBlock(frominfo, toinfo, handler));

    setAndhandleErrorExNew = setAndhandleErrorExNew % 2 == 0 ?
                             setAndhandleErrorExNew : setAndhandleErrorExNew + 1;
    stl.reset(&DFileCopyMoveJobPrivate::setAndhandleError);
    stl.set_lamda(&DFileCopyMoveJobPrivate::setAndhandleError,
    []() {return DFileCopyMoveJob::SkipAction;});
    EXPECT_FALSE(jobd->doCopyFileOnBlock(frominfo, toinfo, handler));

    stl.reset(open);
    stl.reset(close);
    stl.reset(&DFileCopyMoveJobPrivate::setAndhandleError);
    EXPECT_FALSE(jobd->doCopyFileOnBlock(frominfo, toinfo, handler));

    stl.set_lamda(&DFileCopyMoveJobPrivate::stateCheck,
    []() {return true;});
    stl.set_lamda(&DFileCopyMoveJobPrivate::setAndhandleError,
    []() {return DFileCopyMoveJob::RetryAction;});
    stl.set_lamda(read, []() {return -1;});
    stl.reset(open);
    stl.set_lamda(lseek, []() {return false;});
    EXPECT_FALSE(jobd->doCopyFileOnBlock(frominfo, toinfo, handler));
    QProcess::execute("chmod 0777 " + to.toLocalFile());
    TestHelper::deleteTmpFile(to.toLocalFile());

    stl.reset(&DFileCopyMoveJobPrivate::setAndhandleError);
    stl.set_lamda(&DFileCopyMoveJobPrivate::setAndhandleError,
    []() {return DFileCopyMoveJob::SkipAction;});
    EXPECT_TRUE(jobd->doCopyFileOnBlock(frominfo, toinfo, handler));
    QProcess::execute("chmod 0777 " + to.toLocalFile());

    stl.reset(&DFileCopyMoveJobPrivate::setAndhandleError);
    stl.set_lamda(&DFileCopyMoveJobPrivate::setAndhandleError,
    []() {return DFileCopyMoveJob::CancelAction;});
    EXPECT_TRUE(jobd->doCopyFileOnBlock(frominfo, toinfo, handler));
    QProcess::execute("chmod 0777 " + to.toLocalFile());

    stl.reset(read);
    stl.reset(&DFileCopyMoveJobPrivate::setAndhandleError);
    stl.set_lamda(&DFileCopyMoveJobPrivate::writeRefineThread, []() {return false;});
    EXPECT_TRUE(jobd->doCopyFileOnBlock(frominfo, toinfo, handler));
    jobd->m_writeResult.waitForFinished();
    EXPECT_NO_FATAL_FAILURE(jobd->cancelReadFileDealWriteThread());
    TestHelper::deleteTmpFile(to.toLocalFile());

    DUrl urltmp(from);
    urltmp.setPath(TestHelper::createTmpFile());
    handler.reset(DFileService::instance()->createFileHandler(nullptr, from));
    frominfo = DFileService::instance()->createFileInfo(nullptr, urltmp);
    EXPECT_TRUE(jobd->doCopyFileOnBlock(frominfo, toinfo, handler));
    jobd->m_writeResult.waitForFinished();
    EXPECT_NO_FATAL_FAILURE(jobd->cancelReadFileDealWriteThread());
    TestHelper::deleteTmpFile(to.toLocalFile());
    job->stop();

    TestHelper::deleteTmpFiles(QStringList() << from.toLocalFile() << to.toLocalFile()
                               << urltmp.toLocalFile());
}

TEST_F(DFileCopyMoveJobTest, start_writeRefineThread)
{
    DFileCopyMoveJobPrivate *jobd = job->d_func();
    ASSERT_TRUE(jobd);
    jobd->m_copyRefineFlag = DFileCopyMoveJob::OpenFromFileProccessOver;
    StubExt stl;
    stl.set_lamda(&DFileCopyMoveJobPrivate::writeToFileByQueue, []() {return false;});
    EXPECT_TRUE(jobd->writeRefineThread());

    stl.reset(&DFileCopyMoveJobPrivate::writeToFileByQueue);
    stl.set_lamda(&DFileCopyMoveJobPrivate::writeToFileByQueue, []() {return true;});
    QFuture<void> future = QtConcurrent::run([ = ]() {
        jobd->setRefineCopyProccessSate(DFileCopyMoveJob::ReadFileProccessOver);
    });
    EXPECT_TRUE(jobd->writeRefineThread());
    job->stop();
}

static int writecount = 0;
class devicetest8 : public DFileDevice
{
    bool open(OpenMode mode) override
    {
        Q_UNUSED(mode);
        return false;
    }
    bool resize(qint64 size) override
    {
        Q_UNUSED(size);
        return false;
    }
    bool seek(qint64 pos) override
    {
        Q_UNUSED(pos);
        return false;
    }
protected:
    qint64 readData(char *data, qint64 maxlen) override
    {
        Q_UNUSED(data);
        Q_UNUSED(maxlen);
        return -1;
    }
    qint64 writeData(const char *data, qint64 len) override
    {
        Q_UNUSED(data);
        Q_UNUSED(len);
        return 0;
    }
};
class devicetest9 : public DFileDevice
{
    bool open(OpenMode mode) override
    {
        Q_UNUSED(mode);
        return true;
    }
    bool resize(qint64 size) override
    {
        Q_UNUSED(size);
        return false;
    }
    bool seek(qint64 pos) override
    {
        Q_UNUSED(pos);
        return false;
    }
protected:
    qint64 readData(char *data, qint64 maxlen) override
    {
        Q_UNUSED(data);
        Q_UNUSED(maxlen);
        return -1;
    }
    qint64 writeData(const char *data, qint64 len) override
    {
        Q_UNUSED(data);
        Q_UNUSED(len);
        return -1;
    }
};

class devicetest10 : public  DFileDevice
{
    bool open(OpenMode mode) override
    {
        Q_UNUSED(mode);
        return true;
    }
    bool resize(qint64 size) override
    {
        Q_UNUSED(size);
        return false;
    }
    bool seek(qint64 pos) override
    {
        Q_UNUSED(pos);
        return false;
    }
protected:
    qint64 readData(char *data, qint64 maxlen) override
    {
        Q_UNUSED(data);
        Q_UNUSED(maxlen);
        return -1;
    }
    qint64 writeData(const char *data, qint64 len) override
    {
        Q_UNUSED(data);
        Q_UNUSED(len);
        return -1;
    }
};

class devicetest11 : public DFileDevice
{
public:
    explicit devicetest11(QObject *parent = nullptr)
        : DFileDevice(parent)
    {

    }
    bool open(OpenMode mode) override
    {
        Q_UNUSED(mode);
        return true;
    }
    bool resize(qint64 size) override
    {
        Q_UNUSED(size);
        return false;
    }
    bool seek(qint64 pos) override
    {
        Q_UNUSED(pos);
        return false;
    }
protected:
    qint64 readData(char *data, qint64 maxlen) override
    {
        Q_UNUSED(data);
        Q_UNUSED(maxlen);
        return -1;
    }
    qint64 writeData(const char *data, qint64 len) override
    {
        Q_UNUSED(data);
        Q_UNUSED(len);
        if (writecount % 2 > 0) {
            writecount++;
            return -1;
        }
        writecount++;
        return 1;
    }
};

ssize_t writetest(int, const void *, size_t)
{
    if (writecount % 2 == 0) {
        writecount++;
        return 1;
    }
    writecount++;
    return -1;
}

qint64 iowritetest(const char *, qint64)
{
    if (writecount % 2 == 0) {
        writecount++;
        return 1;
    }
    writecount++;
    return -1;
}

TEST_F(DFileCopyMoveJobTest, start_writeToFileByQueue)
{
    DFileCopyMoveJobPrivate *jobd = job->d_func();
    ASSERT_TRUE(jobd);
    EXPECT_TRUE(jobd->writeToFileByQueue());
    StubExt stl;
    DUrl from, to, dirurl;
    from.setScheme(FILE_SCHEME);
    to.setScheme(FILE_SCHEME);
    from.setPath(QDir::currentPath() + "/ut_test");
    QProcess::execute("mkdir " + from.path());
    to.setPath(QDir::currentPath() + "/zut_writeRefine_tst");
    dirurl = to;
    QThread::msleep(100);
    dirurl.setPath(QDir::currentPath() + "/ut_test1");
    QProcess::execute("mkdir " + dirurl.path());
    QSharedPointer<DFileHandler>  handler(DFileService::instance()->createFileHandler(nullptr, from));
    DAbstractFileInfoPointer frominfo = DFileService::instance()->createFileInfo(nullptr, from);
    DAbstractFileInfoPointer toinfo = DFileService::instance()->createFileInfo(nullptr, to);
    DAbstractFileInfoPointer dirinfo = DFileService::instance()->createFileInfo(nullptr, dirurl);
    DFileCopyMoveJobPrivate::FileCopyInfoPointer copyinfo(new DFileCopyMoveJobPrivate::FileCopyInfo());
    copyinfo->isdir = true;
    copyinfo->toinfo = dirinfo;
    copyinfo->handler = handler;
    jobd->writeQueueEnqueue(copyinfo);
    EXPECT_FALSE(jobd->writeToFileByQueue());

    stl.set_lamda(&DFileCopyMoveJobPrivate::stateCheck, []() {return true;});
    copyinfo->buffer = new char[1024];
    jobd->writeQueueEnqueue(copyinfo);
    EXPECT_FALSE(jobd->writeToFileByQueue());
    if (copyinfo->buffer) {
        delete [] copyinfo->buffer;
        copyinfo->buffer = nullptr;
    }

    jobd->writeQueueEnqueue(copyinfo);
    copyinfo->frominfo = frominfo;
    copyinfo->isdir = true;

    EXPECT_TRUE(jobd->writeToFileByQueue());

    jobd->m_skipFileQueue.push_back(frominfo->fileUrl());
    copyinfo->isdir = false;
    jobd->m_writeOpenFd.insert(frominfo->fileUrl(), 10);
    jobd->writeQueueEnqueue(copyinfo);
    EXPECT_TRUE(jobd->writeToFileByQueue());

    jobd->m_skipFileQueue.clear();
    copyinfo->frominfo = frominfo;
    copyinfo->toinfo = toinfo;
    copyinfo->buffer = new char[16];
    copyinfo->buffer[0] = '1';
    copyinfo->buffer[1] = '2';
    copyinfo->buffer[2] = '3';
    copyinfo->buffer[4] = '\n';
    copyinfo->size = 3;
    copyinfo->closeflag = false;
    jobd->writeQueueEnqueue(copyinfo);
    DFileCopyMoveJobPrivate::FileCopyInfoPointer copyinfoover(
        new DFileCopyMoveJobPrivate::FileCopyInfo(*copyinfo.data()));
    copyinfoover->closeflag = true;
    copyinfoover->buffer = new char[16];
    copyinfoover->buffer[0] = '1';
    copyinfoover->buffer[1] = '2';
    copyinfoover->buffer[2] = '3';
    copyinfoover->buffer[4] = '\n';
    jobd->writeQueueEnqueue(copyinfoover);
    EXPECT_TRUE(jobd->writeToFileByQueue());
    if (copyinfo->buffer) {
        delete [] copyinfo->buffer;
        copyinfo->buffer = nullptr;
    }

    if (copyinfoover->buffer) {
        delete [] copyinfoover->buffer;
        copyinfoover->buffer = nullptr;
    }

    jobd->m_skipFileQueue.clear();
    stl.set_lamda(&DFileCopyMoveJobPrivate::setAndhandleError, []() {
        return DFileCopyMoveJob::RetryAction;
    });
    stl.set_lamda(write, []() {return -1;});
    stl.set_lamda(lseek, []() {return false;});
    jobd->writeQueueEnqueue(copyinfo);
    copyinfo->buffer = new char[16];
    copyinfo->buffer[0] = '1';
    copyinfo->buffer[1] = '2';
    copyinfo->buffer[2] = '3';
    copyinfo->buffer[4] = '\n';
    EXPECT_FALSE(jobd->writeToFileByQueue());
    if (copyinfo->buffer) {
        delete [] copyinfo->buffer;
        copyinfo->buffer = nullptr;
    }
    stl.reset(&DFileCopyMoveJobPrivate::setAndhandleError);

    jobd->m_skipFileQueue.clear();
    stl.set_lamda(&DFileCopyMoveJobPrivate::setAndhandleError, []() {
        return DFileCopyMoveJob::SkipAction;
    });
    copyinfo->buffer = new char[16];
    copyinfo->buffer[0] = '1';
    copyinfo->buffer[1] = '2';
    copyinfo->buffer[2] = '3';
    copyinfo->buffer[4] = '\n';
    jobd->writeQueueEnqueue(copyinfo);
    EXPECT_TRUE(jobd->writeToFileByQueue());
    if (copyinfo->buffer) {
        delete [] copyinfo->buffer;
        copyinfo->buffer = nullptr;
    }
    stl.reset(&DFileCopyMoveJobPrivate::setAndhandleError);

    jobd->m_skipFileQueue.clear();
    stl.set_lamda(&DFileCopyMoveJobPrivate::setAndhandleError, []() {
        return DFileCopyMoveJob::CancelAction;
    });
    copyinfo->buffer = new char[16];
    copyinfo->buffer[0] = '1';
    copyinfo->buffer[1] = '2';
    copyinfo->buffer[2] = '3';
    copyinfo->buffer[4] = '\n';
    jobd->writeQueueEnqueue(copyinfo);
    EXPECT_FALSE(jobd->writeToFileByQueue());
    if (copyinfo->buffer) {
        delete [] copyinfo->buffer;
        copyinfo->buffer = nullptr;
    }
    stl.reset(&DFileCopyMoveJobPrivate::setAndhandleError);

    stl.set(write, writetest);
    stl.set_lamda(&DFileCopyMoveJobPrivate::checkFreeSpace, []() {return true;});
    jobd->m_skipFileQueue.clear();
    stl.set_lamda(&DFileCopyMoveJobPrivate::setAndhandleError, []() {
        return DFileCopyMoveJob::RetryAction;
    });
    copyinfo->buffer = new char[16];
    copyinfo->buffer[0] = '1';
    copyinfo->buffer[1] = '2';
    copyinfo->buffer[2] = '3';
    copyinfo->buffer[4] = '\n';
    jobd->writeQueueEnqueue(copyinfo);
    EXPECT_FALSE(jobd->writeToFileByQueue());
    if (copyinfo->buffer) {
        delete [] copyinfo->buffer;
        copyinfo->buffer = nullptr;
    }
    stl.reset(&DFileCopyMoveJobPrivate::setAndhandleError);

    jobd->m_skipFileQueue.clear();
    stl.set_lamda(&DFileCopyMoveJobPrivate::setAndhandleError, []() {
        return DFileCopyMoveJob::SkipAction;
    });
    copyinfo->buffer = new char[16];
    copyinfo->buffer[0] = '1';
    copyinfo->buffer[1] = '2';
    copyinfo->buffer[2] = '3';
    copyinfo->buffer[4] = '\n';
    jobd->writeQueueEnqueue(copyinfo);
    EXPECT_TRUE(jobd->writeToFileByQueue());
    if (copyinfo->buffer) {
        delete [] copyinfo->buffer;
        copyinfo->buffer = nullptr;
    }
    stl.reset(&DFileCopyMoveJobPrivate::setAndhandleError);

    jobd->m_skipFileQueue.clear();
    stl.set_lamda(&DFileCopyMoveJobPrivate::setAndhandleError, []() {
        return DFileCopyMoveJob::CancelAction;
    });
    copyinfo->buffer = new char[16];
    copyinfo->buffer[0] = '1';
    copyinfo->buffer[1] = '2';
    copyinfo->buffer[2] = '3';
    copyinfo->buffer[4] = '\n';
    jobd->writeQueueEnqueue(copyinfo);
    EXPECT_FALSE(jobd->writeToFileByQueue());
    if (copyinfo->buffer) {
        delete [] copyinfo->buffer;
        copyinfo->buffer = nullptr;
    }

    job->stop();

    TestHelper::deleteTmpFile(from.path());
    TestHelper::deleteTmpFile(to.path());
    TestHelper::deleteTmpFile(dirurl.path());
    job->stop();
}
