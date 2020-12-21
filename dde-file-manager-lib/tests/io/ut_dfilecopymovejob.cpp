#include <gtest/gtest.h>
#include <QDateTime>

#define private public
#define protected public

#include "dfilecopymovejob.h"
#include "private/dfilecopymovejob_p.h"
#include "interfaces/dfileservices.h"
#include "interfaces/dfileinfo.h"
#include "testhelper.h"
#include "dfmglobal.h"
#include "stub.h"
#include "controllers/vaultcontroller.h"
#include "dgiofiledevice.h"
#include "dabstractfilewatcher.h"
#include "dlocalfiledevice.h"

#include <QThread>
#include <QProcess>
#include <QtConcurrent>
#include <QByteArray>
#include <QtDebug>
#include <QVariant>
#include <QDialog>

using namespace testing;
DFM_USE_NAMESPACE

class DFileCopyMoveJobTest:public testing::Test{

public:

    QSharedPointer<DFileCopyMoveJob> job;
    virtual void SetUp() override{
        job.reset(new DFileCopyMoveJob());
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

        typedef int(*fptr)(QDialog*);
        fptr pQDialogExec = (fptr)(&QDialog::exec);
        int (*stub_DDialog_exec)(void) = [](void)->int{return QDialog::Rejected;};
        stl.set(pQDialogExec, stub_DDialog_exec);
        std::cout << "start DFileCopyMoveJobTest" << std::endl;
    }

    virtual void TearDown() override{
        std::cout << "end DFileCopyMoveJobTest" << std::endl;
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
        if (error == DFileCopyMoveJob::FileExistsError)
        {
            return DFileCopyMoveJob::CancelAction;
        }
        if (error == DFileCopyMoveJob::UnknowError)
        {
            return DFileCopyMoveJob::CancelAction;
        }
        return DFileCopyMoveJob::SkipAction;
    }
    QSharedPointer<DFileCopyMoveJob> fileJob;
};

class ErrorHandle : public QObject, public DFileCopyMoveJob::Handle
{
public:
    ErrorHandle(QSharedPointer<DFileCopyMoveJob> job)
        : QObject(nullptr)
        , fileJob(job)
    {
    }

    ~ErrorHandle() override
    {
        fileJob->disconnect();
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
        if (error == DFileCopyMoveJob::FileExistsError)
        {
            return DFileCopyMoveJob::CancelAction;
        }
        return DFileCopyMoveJob::NoAction;
    }
    QSharedPointer<DFileCopyMoveJob> fileJob;
};


TEST_F(DFileCopyMoveJobTest,can_job_running) {
    job->setMode(DFileCopyMoveJob::CopyMode);
    DUrl urlsour,target;
    urlsour.setScheme(FILE_SCHEME);
    target.setScheme(FILE_SCHEME);
    urlsour.setPath("~/Pictures/Wallpapers");
    target.setPath("~/test_copy_all");

    if (QThread::currentThread()->loopLevel() <= 0) {
        // 确保对象所在线程有事件循环
        job->moveToThread(qApp->thread());
    }
    job->start(DUrlList() << urlsour, target);
    while(!job->isFinished()) {
        QThread::msleep(100);
    }
    QProcess::execute("mkdir " + target.toLocalFile());
    job->start(DUrlList() << urlsour, target);
    while(!job->isFinished()) {
        QThread::msleep(100);
    }
    urlsour.setPath("/etc/apt");
    job->setFileHints(DFileCopyMoveJob::FollowSymlink);
    job->setFileHints( job->fileHints() | DFileCopyMoveJob::Attributes);
    job->setFileHints( job->fileHints() | DFileCopyMoveJob::ResizeDestinationFile);
    job->setFileHints( job->fileHints() | DFileCopyMoveJob::DontFormatFileName);
    job->setFileHints( job->fileHints() | DFileCopyMoveJob::DontSortInode);
    job->start(DUrlList() << urlsour, target);
    job->togglePause();
    job->togglePause();
    while(!job->isFinished()) {
        QThread::msleep(100);
    }
    job->togglePause();
}

TEST_F(DFileCopyMoveJobTest,can_job_running_cut) {
    job->setMode(DFileCopyMoveJob::CutMode);
    DUrl urlsour,target;
    urlsour.setScheme(FILE_SCHEME);
    target.setScheme(FILE_SCHEME);
    urlsour.setPath("/etc/apt");
    target.setPath("~/test_copy_all/etc");
    QProcess::execute("mkdir " + target.toLocalFile());
    if (QThread::currentThread()->loopLevel() <= 0) {
        // 确保对象所在线程有事件循环
        job->moveToThread(qApp->thread());
    }
    job->start(DUrlList() << urlsour, target);

    EXPECT_EQ(target.toString(),job->targetUrl().toString());
    EXPECT_EQ(-1,job->totalDataSize());
    EXPECT_EQ(0,job->totalFilesCount());

    QThread::msleep(300);
    EXPECT_EQ(true,job->totalDataSize() != -1);
    EXPECT_EQ(true,job->totalFilesCount() != -1);
    EXPECT_EQ(DFileCopyMoveJob::NoError,job->error());
    EXPECT_EQ(true,job->isCanShowProgress());

    job->stop();

    while(!job->isFinished()) {
        QThread::msleep(100);
    }
}

TEST_F(DFileCopyMoveJobTest,can_job_running_refine) {
    job->setMode(DFileCopyMoveJob::CutMode);
    DUrl urlsour,target;
    urlsour.setScheme(FILE_SCHEME);
    target.setScheme(FILE_SCHEME);
    urlsour.setPath("~/test_copy_all/etc");
    target.setPath("~/test_copy_all/etc1");
    if (QThread::currentThread()->loopLevel() <= 0) {
        // 确保对象所在线程有事件循环
        job->moveToThread(qApp->thread());
    }
    job->start(DUrlList() << urlsour, target);


    while(!job->isFinished()) {
        QThread::msleep(100);
    }

    target.setPath("~/test_copy_all");
    QProcess::execute("rm -rf " + target.toLocalFile());
}

TEST_F(DFileCopyMoveJobTest,can_job_running_remove) {
    job->setMode(DFileCopyMoveJob::CutMode);
    job->setFileHints(DFileCopyMoveJob::ForceDeleteFile);
    DUrl urlsour,target;
    urlsour.setScheme(FILE_SCHEME);
    target.setScheme(FILE_SCHEME);
    EXPECT_EQ(DFileCopyMoveJob::ForceDeleteFile,job->fileHints());
    urlsour.setPath("~/test_copy_all");
    if (QThread::currentThread()->loopLevel() <= 0) {
        // 确保对象所在线程有事件循环
        job->moveToThread(qApp->thread());
    }
    job->start(DUrlList() << urlsour, DUrl());
    while(!job->isFinished()) {
        QThread::msleep(100);
    }
    job->start(DUrlList() << urlsour, DUrl());
    while(!job->isFinished()) {
        QThread::msleep(100);
    }
}

TEST_F(DFileCopyMoveJobTest,can_job_running_error) {
    job->setMode(DFileCopyMoveJob::CopyMode);
    DUrl urlsour,target;
    urlsour.setScheme(FILE_SCHEME);
    target.setScheme(FILE_SCHEME);
    urlsour.setPath("~/Wallpapers");
    target.setPath("/usr/bin");
    if (QThread::currentThread()->loopLevel() <= 0) {
        // 确保对象所在线程有事件循环
        job->moveToThread(qApp->thread());
    }
    job->start(DUrlList() << urlsour, target);
    while(!job->isFinished()) {
        QThread::msleep(100);
    }
}

TEST_F(DFileCopyMoveJobTest,get_errorToString) {
    DFileCopyMoveJobPrivate * jobd = reinterpret_cast<DFileCopyMoveJobPrivate *>(qGetPtrHelper(job->d_ptr));
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
}

TEST_F(DFileCopyMoveJobTest,start_setState) {
    DFileCopyMoveJobPrivate * jobd = job->d_func();
    ASSERT_TRUE(jobd);
    jobd->setState(DFileCopyMoveJob::SleepState);
    jobd->setState(DFileCopyMoveJob::RunningState);
    EXPECT_TRUE(jobd->stateCheck());
    QThread::msleep(100);
    jobd->setState(DFileCopyMoveJob::SleepState);
    jobd->setState(DFileCopyMoveJob::RunningState);
    jobd->setState(DFileCopyMoveJob::PausedState);
    QtConcurrent::run([=]() {
        EXPECT_FALSE(jobd->stateCheck());
    });
    QThread::msleep(300);

    job->stop();
    jobd->setState(DFileCopyMoveJob::StoppedState);
    EXPECT_FALSE(jobd->stateCheck());
}

TEST_F(DFileCopyMoveJobTest,start_setError) {
    DFileCopyMoveJobPrivate * jobd = job->d_func();
    ASSERT_TRUE(jobd);
    jobd->setError(DFileCopyMoveJob::NoError);
    jobd->setError(DFileCopyMoveJob::NoError);
    jobd->setError(DFileCopyMoveJob::PermissionError);
    jobd->setError(DFileCopyMoveJob::PermissionError);
}

TEST_F(DFileCopyMoveJobTest,start_handleError) {
    DFileCopyMoveJobPrivate * jobd = job->d_func();
    ASSERT_TRUE(jobd);
    DUrl url;
    url.setScheme(FILE_SCHEME);
    url.setPath("~/Videos");
    DAbstractFileInfoPointer source = DFileService::instance()->createFileInfo(nullptr,url);
    jobd->setError(DFileCopyMoveJob::NoError);
    EXPECT_EQ(DFileCopyMoveJob::NoAction,jobd->handleError(source, source));
//    job->taskDailogClose();
    jobd->setError(DFileCopyMoveJob::SpecialFileError);
//    EXPECT_EQ(DFileCopyMoveJob::CancelAction,jobd->handleError(source, source));
    job->setActionOfErrorType(DFileCopyMoveJob::SpecialFileError,DFileCopyMoveJob::SkipAction);
    EXPECT_EQ(DFileCopyMoveJob::SkipAction,jobd->handleError(source, source));
    jobd->setError(DFileCopyMoveJob::FileExistsError);
    EXPECT_EQ(DFileCopyMoveJob::CoexistAction,jobd->handleError(source, source));
    jobd->setError(DFileCopyMoveJob::DirectoryExistsError);
    EXPECT_EQ(DFileCopyMoveJob::CoexistAction,jobd->handleError(source, source));
    ErrorHandleMe hanle(job);
    jobd->setState(DFileCopyMoveJob::StoppedState);
    job->setErrorHandle(&hanle, hanle.thread());
    jobd->setState(DFileCopyMoveJob::RunningState);
    TestHelper::runInLoop([=](){
        jobd->setError(DFileCopyMoveJob::DirectoryExistsError);
        EXPECT_EQ(DFileCopyMoveJob::CoexistAction,jobd->handleError(source, source));
    },200);

    job->setErrorHandle(&hanle, hanle.thread());
    hanle.moveToThread(qApp->thread());
    TestHelper::runInLoop([=](){
        jobd->setError(DFileCopyMoveJob::SpecialFileError);
        EXPECT_EQ(DFileCopyMoveJob::SkipAction,jobd->handleError(source, source));
    },200);

    job->setErrorHandle(&hanle, hanle.thread());
    hanle.moveToThread(qApp->thread());
    TestHelper::runInLoop([=](){
        jobd->setError(DFileCopyMoveJob::FileExistsError);
        EXPECT_EQ(DFileCopyMoveJob::CancelAction,jobd->handleError(source, source));
    },200);

    ErrorHandle hanlee(job);
    job->setErrorHandle(&hanlee, hanlee.thread());
    TestHelper::runInLoop([=](){
        jobd->setError(DFileCopyMoveJob::DirectoryExistsError);
        jobd->handleError(source, source);
    },200);

    job->taskDailogClose();
    jobd->setError(DFileCopyMoveJob::SpecialFileError);
    EXPECT_EQ(DFileCopyMoveJob::CancelAction,jobd->handleError(source, source));
}

TEST_F(DFileCopyMoveJobTest,start_setAndhandleError) {
    DFileCopyMoveJobPrivate * jobd = job->d_func();
    ASSERT_TRUE(jobd);
    DUrl url;
    url.setScheme(FILE_SCHEME);
    url.setPath("~/Videos");
    DAbstractFileInfoPointer source = DFileService::instance()->createFileInfo(nullptr,url);
    EXPECT_EQ(DFileCopyMoveJob::CoexistAction,jobd->setAndhandleError(
                  DFileCopyMoveJob::FileExistsError,source, source));
    EXPECT_EQ(DFileCopyMoveJob::NoAction,jobd->setAndhandleError(
                  DFileCopyMoveJob::NoError,DAbstractFileInfoPointer(nullptr), DAbstractFileInfoPointer(nullptr)));
    EXPECT_EQ(DFileCopyMoveJob::CancelAction,jobd->setAndhandleError(
                  DFileCopyMoveJob::CancelError,DAbstractFileInfoPointer(nullptr), DAbstractFileInfoPointer(nullptr)));
}

TEST_F(DFileCopyMoveJobTest,start_isRunning) {
    DFileCopyMoveJobPrivate * jobd = job->d_func();
    ASSERT_TRUE(jobd);
    EXPECT_FALSE(jobd->isRunning());
}

TEST_F(DFileCopyMoveJobTest,start_formatFileName) {
    DFileCopyMoveJobPrivate * jobd = job->d_func();
    ASSERT_TRUE(jobd);
    job->setFileHints(DFileCopyMoveJob::DontFormatFileName);
    EXPECT_EQ(jobd->formatFileName("hello"),QString("hello"));
    job->setFileHints(DFileCopyMoveJob::NoHint);
    EXPECT_EQ(jobd->formatFileName("hello"),QString("hello"));
    DUrl url;
    url.setScheme(FILE_SCHEME);
    url.setPath("~/Videos");
    jobd->enterDirectory(DUrl(),DUrl());
    EXPECT_EQ(jobd->formatFileName("hello"),QString("hello"));
    jobd->leaveDirectory();
    jobd->enterDirectory(url,url);
    EXPECT_EQ(jobd->formatFileName("hello"),QString("hello"));
    Stub st;
    QByteArray (*ss)(void *) = [](void *){return QByteArray("vfat");};
    st.set(ADDR(DStorageInfo,fileSystemType),ss);
    EXPECT_EQ(jobd->formatFileName("hello"),QString("hello"));
}

TEST_F(DFileCopyMoveJobTest,start_checkFileSize) {
    DFileCopyMoveJobPrivate * jobd = job->d_func();
    ASSERT_TRUE(jobd);
    EXPECT_TRUE(jobd->checkFreeSpace(16));
    jobd->enterDirectory(DUrl(),DUrl());
    EXPECT_TRUE(jobd->checkFileSize(16));
    EXPECT_TRUE(jobd->checkFreeSpace(16));
    DUrl url;
    url.setScheme(FILE_SCHEME);
    url.setPath("~/Videos");
    jobd->leaveDirectory();
    jobd->enterDirectory(url,url);
    Stub st;
    QByteArray (*ss)(void *) = [](void *){return QByteArray("vfat");};
    st.set(ADDR(DStorageInfo,fileSystemType),ss);
    qint64 size = 5l*1024*1024*1024;
    EXPECT_FALSE(jobd->checkFileSize(size));
}

TEST_F(DFileCopyMoveJobTest,start_getNewFileName) {
    DFileCopyMoveJobPrivate * jobd = job->d_func();
    ASSERT_TRUE(jobd);
    DUrl from,to;
    from.setScheme(FILE_SCHEME);
    from.setPath("./zut_7ztest.7z.2");
    DAbstractFileInfoPointer source = DFileService::instance()->createFileInfo(nullptr,from);
    to = from;
    to.setPath("./");
    DAbstractFileInfoPointer toinfo = DFileService::instance()->createFileInfo(nullptr,to);
    std::cout << from.toLocalFile().toStdString() << std::endl;
    std::cout << jobd->getNewFileName(source,toinfo).toStdString() << std::endl;
    EXPECT_FALSE(jobd->getNewFileName(source,toinfo).isEmpty());
}

TEST_F(DFileCopyMoveJobTest,start_doProcess) {
    DFileCopyMoveJobPrivate * jobd = job->d_func();
    ASSERT_TRUE(jobd);
    ASSERT_TRUE(jobd->doProcess(DUrl(),DAbstractFileInfoPointer(nullptr),DAbstractFileInfoPointer(nullptr)));
    DUrl from,from1,to;
    from.setScheme(FILE_SCHEME);
    from.setPath("./zut_7ztest.7z.2");
    DAbstractFileInfoPointer source = DFileService::instance()->createFileInfo(nullptr,from);
    ErrorHandleMe hanle(job);
    job->setErrorHandle(&hanle, hanle.thread());
    EXPECT_TRUE(jobd->doProcess(DUrl(),source,DAbstractFileInfoPointer(nullptr)));

    from.setPath(TestHelper::createTmpFile());
    source = DFileService::instance()->createFileInfo(nullptr,from);
    EXPECT_TRUE(jobd->doProcess(DUrl(),source,DAbstractFileInfoPointer(nullptr)));

    EXPECT_TRUE(jobd->doProcess(from,source,DAbstractFileInfoPointer(nullptr)));
    from1 = from;
    from1.setPath(TestHelper::createTmpDir());
    source = DFileService::instance()->createFileInfo(nullptr,from1);
    job->setMode(DFileCopyMoveJob::CutMode);
    jobd->setState(DFileCopyMoveJob::IOWaitState);
    job->setFileHints(DFileCopyMoveJob::ForceDeleteFile);
    jobd->setState(DFileCopyMoveJob::RunningState);
    EXPECT_TRUE(jobd->doProcess(from1,source,DAbstractFileInfoPointer(nullptr)));

    from.setPath("./zut_7ztest.7z.2");
    QProcess::execute("touch "+from.toLocalFile());
    to = from;
    to.setPath("./");
    DAbstractFileInfoPointer toinfo = DFileService::instance()->createFileInfo(nullptr,to);
    source = DFileService::instance()->createFileInfo(nullptr,from);
    jobd->enterDirectory(from1,to);
    job->setMode(DFileCopyMoveJob::CopyMode);
    EXPECT_TRUE(jobd->doProcess(from1,source,toinfo));
    jobd->leaveDirectory();
    QProcess::execute("rm " + to.toLocalFile() + "zut_7ztest.7z.2");
    QProcess::execute("rm " + to.toLocalFile() + "zut_7ztest.7z(copy).2");

    DUrl trashurl,desktopurl;
    trashurl.setScheme(FILE_SCHEME);
    trashurl.setPath("~/.local/share/Trash/files/zut_7ztest.7z.2");
    QProcess::execute("touch "+from.toLocalFile());
    DFileService::instance()->moveToTrash(nullptr,DUrlList() << from);
    job->setMode(DFileCopyMoveJob::CutMode);
    desktopurl.setScheme(FILE_SCHEME);
    desktopurl.setPath("~/Desktop");
    source = DFileService::instance()->createFileInfo(nullptr,trashurl);
    DAbstractFileInfoPointer taginfo = DFileService::instance()->createFileInfo(nullptr,desktopurl);
    jobd->enterDirectory(trashurl,desktopurl);
    EXPECT_TRUE(jobd->doProcess(trashurl,source,taginfo));
    jobd->leaveDirectory();
    QProcess::execute("rm " + desktopurl.toLocalFile() + "/zut_7ztest.7z.2");

    from.setPath(TestHelper::createTmpFile());
    job->setMode(DFileCopyMoveJob::CutMode);
    jobd->enterDirectory(from,from);
    source = DFileService::instance()->createFileInfo(nullptr,from);
    EXPECT_FALSE(jobd->doProcess(from,source,source));
    jobd->leaveDirectory();
    TestHelper::deleteTmpFile(from.toLocalFile());

    from.setPath(TestHelper::createTmpFile());
    to = from;
    to.setPath("./" + from.fileName());
    QProcess::execute("mkdir " + to.toLocalFile());
    source = DFileService::instance()->createFileInfo(nullptr,from);
    taginfo = DFileService::instance()->createFileInfo(nullptr,to.parentUrl());
    job->setMode(DFileCopyMoveJob::CopyMode);
    jobd->enterDirectory(from,to);
    EXPECT_TRUE(jobd->doProcess(from,source,taginfo));
    jobd->leaveDirectory();
    TestHelper::deleteTmpFiles(QStringList() << from.toLocalFile() << to.toLocalFile()
                               << to.toLocalFile() + "(copy)");

    from.setPath(TestHelper::createTmpFile());
    to = from;
    DUrl linkurl;
    linkurl = from;
    linkurl.setPath("./zut_linksys");
    QProcess::execute("ln -s "  + from.toLocalFile() + " "+ linkurl.toLocalFile());
    to.setPath("./");
    source = DFileService::instance()->createFileInfo(nullptr,linkurl);
    taginfo = DFileService::instance()->createFileInfo(nullptr,to);
    job->setMode(DFileCopyMoveJob::CopyMode);
    jobd->setState(DFileCopyMoveJob::IOWaitState);
    job->setFileHints(DFileCopyMoveJob::FollowSymlink);
    jobd->setState(DFileCopyMoveJob::RunningState);
    jobd->enterDirectory(from,to);
    EXPECT_TRUE(jobd->doProcess(from,source,taginfo));
    jobd->setState(DFileCopyMoveJob::IOWaitState);
    job->setFileHints(DFileCopyMoveJob::NoHint);
    jobd->setState(DFileCopyMoveJob::RunningState);
    EXPECT_TRUE(jobd->doProcess(from,source,taginfo));
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
    jobd->enterDirectory(from,tagurl);
    source = DFileService::instance()->createFileInfo(nullptr,from);
    taginfo = DFileService::instance()->createFileInfo(nullptr,tagurl);
    DAbstractFileInfoPointer (*sss)(const QObject *, const DUrl &) = [](const QObject *, const DUrl &){
        DUrl tagurl("file:///tmp/zut_test_dir/zut_test_dir");
        return DAbstractFileInfoPointer(new DFileInfo(tagurl));
    };
    DAbstractFileInfoPointer (*sss1)(const QObject *, const DUrl &) = [](const QObject *, const DUrl &){
        return DAbstractFileInfoPointer(nullptr);
    };
    DFileCopyMoveJob::Action (*ssse2)(DFileCopyMoveJob::Error, const DAbstractFileInfoPointer,
                                     const DAbstractFileInfoPointer, const QString &) = [](DFileCopyMoveJob::Error , const DAbstractFileInfoPointer,
            const DAbstractFileInfoPointer , const QString &){
        return DFileCopyMoveJob::SkipAction;
    };
    DFileCopyMoveJob::Action (*ssse3)(DFileCopyMoveJob::Error, const DAbstractFileInfoPointer,
                                     const DAbstractFileInfoPointer, const QString &) = [](DFileCopyMoveJob::Error , const DAbstractFileInfoPointer,
            const DAbstractFileInfoPointer , const QString &){
        return DFileCopyMoveJob::EnforceAction;
    };
    st.set(ADDR(DFileService,createFileInfo),sss);
    EXPECT_TRUE(jobd->doProcess(from,source,taginfo));
    st.set(ADDR(DFileCopyMoveJobPrivate,setAndhandleError),ssse3);
    EXPECT_FALSE(jobd->doProcess(from,source,taginfo));
    TestHelper::deleteTmpFile(tagurl.toLocalFile());

    st.set(ADDR(DFileCopyMoveJobPrivate,setAndhandleError),ssse2);
    EXPECT_TRUE(jobd->doProcess(from,source,taginfo));
    TestHelper::deleteTmpFile(tagurl.toLocalFile());

    st.set(ADDR(DFileCopyMoveJobPrivate,setAndhandleError),ssse3);
    EXPECT_FALSE(jobd->doProcess(from,source,taginfo));
    st.reset(ADDR(DFileCopyMoveJobPrivate,setAndhandleError));
    st.set(ADDR(DFileService,createFileInfo),sss1);
    EXPECT_FALSE(jobd->doProcess(from,source,taginfo));
    jobd->leaveDirectory();
    TestHelper::deleteTmpFiles(QStringList() << from.toLocalFile() << tagurl.toLocalFile());

    st.reset(ADDR(DFileService,createFileInfo));

    from.setPath(TestHelper::createTmpFile());
    to = from;
    to.setPath("./" + from.fileName());
    QProcess::execute("touch " + to.toLocalFile());
    source = DFileService::instance()->createFileInfo(nullptr,from);
    taginfo = DFileService::instance()->createFileInfo(nullptr,to.parentUrl());
    job->setMode(DFileCopyMoveJob::CopyMode);
    jobd->enterDirectory(from,to);
    DFileCopyMoveJob::Action (*sss2)(DFileCopyMoveJob::Error, const DAbstractFileInfoPointer,
                                     const DAbstractFileInfoPointer, const QString &) = [](DFileCopyMoveJob::Error , const DAbstractFileInfoPointer,
            const DAbstractFileInfoPointer , const QString &){
        return DFileCopyMoveJob::ReplaceAction;
    };
    st.set(ADDR(DFileCopyMoveJobPrivate,setAndhandleError),sss2);
    EXPECT_TRUE(jobd->doProcess(from,source,taginfo));
    EXPECT_TRUE(jobd->doProcess(to,source,taginfo));
    DFileCopyMoveJob::Action (*sss3)(DFileCopyMoveJob::Error, const DAbstractFileInfoPointer,
                                     const DAbstractFileInfoPointer, const QString &) = [](DFileCopyMoveJob::Error , const DAbstractFileInfoPointer,
            const DAbstractFileInfoPointer , const QString &){
        return DFileCopyMoveJob::MergeAction;
    };
    st.set(ADDR(DFileCopyMoveJobPrivate,setAndhandleError),sss3);
    EXPECT_FALSE(jobd->doProcess(from,source,taginfo));
    TestHelper::deleteTmpFiles(QStringList() << from.toLocalFile() << to.toLocalFile());
    from.setPath(TestHelper::createTmpDir());
    to = from;
    to.setPath("./" + from.fileName());
    QProcess::execute("mkdir " + to.toLocalFile());
    source = DFileService::instance()->createFileInfo(nullptr,from);
    taginfo = DFileService::instance()->createFileInfo(nullptr,to.parentUrl());
    st.set(ADDR(DFileCopyMoveJobPrivate,setAndhandleError),sss2);
    EXPECT_FALSE(jobd->doProcess(from,source,taginfo));
    st.set(ADDR(DFileCopyMoveJobPrivate,setAndhandleError),sss3);
    EXPECT_TRUE(jobd->doProcess(from,source,taginfo));

    DFileCopyMoveJob::Action (*sss4)(DFileCopyMoveJob::Error, const DAbstractFileInfoPointer,
                                     const DAbstractFileInfoPointer, const QString &) = [](DFileCopyMoveJob::Error , const DAbstractFileInfoPointer,
            const DAbstractFileInfoPointer , const QString &){
        return DFileCopyMoveJob::CoexistAction;
    };
    st.set(ADDR(DFileCopyMoveJobPrivate,setAndhandleError),sss4);
    EXPECT_TRUE(jobd->doProcess(from,source,taginfo));

    DFileCopyMoveJob::Action (*sss5)(DFileCopyMoveJob::Error, const DAbstractFileInfoPointer,
                                     const DAbstractFileInfoPointer, const QString &) = [](DFileCopyMoveJob::Error , const DAbstractFileInfoPointer,
            const DAbstractFileInfoPointer , const QString &){
        return DFileCopyMoveJob::SkipAction;
    };
    st.set(ADDR(DFileCopyMoveJobPrivate,setAndhandleError),sss5);
    EXPECT_TRUE(jobd->doProcess(from,source,taginfo));
    jobd->leaveDirectory();
    TestHelper::deleteTmpFiles(QStringList() << from.toLocalFile() << to.toLocalFile());
    QProcess::execute("rm -rf " + to.toLocalFile() + "(copy)");

    bool (*sss6)(qint64) = [](qint64) {return false;};
    st.set(ADDR(DFileCopyMoveJobPrivate,checkFreeSpace),sss6);
    job->setMode(DFileCopyMoveJob::CopyMode);
    jobd->setState(DFileCopyMoveJob::IOWaitState);
    job->setFileHints(DFileCopyMoveJob::NoHint);
    jobd->setState(DFileCopyMoveJob::RunningState);
    from.setPath(TestHelper::createTmpFile());
    to.setPath("./");
    source = DFileService::instance()->createFileInfo(nullptr,from);
    taginfo = DFileService::instance()->createFileInfo(nullptr,to);
    jobd->enterDirectory(from,to);
    st.set(ADDR(DFileCopyMoveJobPrivate,setAndhandleError),sss5);
    EXPECT_TRUE(jobd->doProcess(from,source,taginfo));
    DFileCopyMoveJob::Action (*sss7)(DFileCopyMoveJob::Error, const DAbstractFileInfoPointer,
                                     const DAbstractFileInfoPointer, const QString &) = [](DFileCopyMoveJob::Error , const DAbstractFileInfoPointer,
            const DAbstractFileInfoPointer , const QString &){
        return DFileCopyMoveJob::EnforceAction;
    };
    st.set(ADDR(DFileCopyMoveJobPrivate,setAndhandleError),sss7);
    EXPECT_TRUE(jobd->doProcess(from,source,taginfo));
    DFileCopyMoveJob::Action (*sss8)(DFileCopyMoveJob::Error, const DAbstractFileInfoPointer,
                                     const DAbstractFileInfoPointer, const QString &) = [](DFileCopyMoveJob::Error , const DAbstractFileInfoPointer,
            const DAbstractFileInfoPointer , const QString &){
        return DFileCopyMoveJob::NoAction;
    };
    st.set(ADDR(DFileCopyMoveJobPrivate,setAndhandleError),sss8);
    EXPECT_FALSE(jobd->doProcess(from,source,taginfo));

    st.reset(ADDR(DFileCopyMoveJobPrivate,checkFreeSpace));
    bool (*sss9)(qint64) = [](qint64) {return false;};
    st.set(ADDR(DFileCopyMoveJobPrivate,checkFileSize),sss9);
    st.set(ADDR(DFileCopyMoveJobPrivate,setAndhandleError),sss5);
    TestHelper::deleteTmpFiles(QStringList() << to.toLocalFile()+from.fileName());
    EXPECT_TRUE(jobd->doProcess(from,source,taginfo));
    st.set(ADDR(DFileCopyMoveJobPrivate,setAndhandleError),sss7);
    EXPECT_TRUE(jobd->doProcess(from,source,taginfo));
    TestHelper::deleteTmpFiles(QStringList() << to.toLocalFile()+from.fileName());
    st.set(ADDR(DFileCopyMoveJobPrivate,setAndhandleError),sss8);
    EXPECT_FALSE(jobd->doProcess(from,source,taginfo));


    jobd->leaveDirectory();
    TestHelper::deleteTmpFiles(QStringList() << from.toLocalFile() <<
                               to.toLocalFile()+from.fileName());
//    Stub st;
//    DAbstractFileInfo::FileType (*sss)(void *) = [](void *){return DAbstractFileInfo::CharDevice;};
//    typedef DAbstractFileInfo::FileType (DFileInfo::*fptr)(void*);
//    fptr A_foo = (fptr)(&DFileInfo::fileType);   //obtaining an address
//    st.set(A_foo,sss);
//    EXPECT_FALSE(jobd->doProcess(DUrl(),source,DAbstractFileInfoPointer(nullptr)));

//    DAbstractFileInfo::FileType (*ss1)(void *) = [](void *){return DAbstractFileInfo::BlockDevice;};
//    st.set(ADDR(DFileInfo,fileType),ss1);
//    EXPECT_FALSE(jobd->doProcess(DUrl(),source,DAbstractFileInfoPointer(nullptr)));

//    DAbstractFileInfo::FileType (*ss2)(void *) = [](void *){return DAbstractFileInfo::FIFOFile;};
//    st.set(ADDR(DFileInfo,fileType),ss2);
//    EXPECT_FALSE(jobd->doProcess(DUrl(),source,DAbstractFileInfoPointer(nullptr)));

//    DAbstractFileInfo::FileType (*ss3)(void *) = [](void *){return DAbstractFileInfo::SocketFile;};
//    st.set(ADDR(DFileInfo,fileType),ss3);
//    EXPECT_FALSE(jobd->doProcess(DUrl(),source,DAbstractFileInfoPointer(nullptr)));
}

TEST_F(DFileCopyMoveJobTest,start_mergeDirectory) {
    DFileCopyMoveJobPrivate * jobd = job->d_func();
    ASSERT_TRUE(jobd);
    Stub st;
    bool (*isVaultFile)(QString) = [](QString){return true;};
    DUrl from,to;
    from.setScheme(FILE_SCHEME);
    from.setPath(TestHelper::createTmpDir());
    to = from;
    to.setPath("./fsjkaaaghfgggggsdfgdsfgsdfgsdfgsdfgaajksdhgajskdhgjkashdjkfasfsdsgsdfgsdfgsdfglah\
               gdrgdfgsdfgsdfgsdsgddddddddddddddddddddddddddddddddgsdggsdsdfffffggggggggggsdfgsdgsdfgg\
               grgsdfgdfjklasndjkasvvskncnkz.hnjxvjkrshgkejrhguierhgkjashdfgjlkashdfgjkahsgjhasjghakghja");
    DAbstractFileInfoPointer frominfo = DFileService::instance()->createFileInfo(nullptr,from);
    DAbstractFileInfoPointer toinfo = DFileService::instance()->createFileInfo(nullptr,to);
    st.set(ADDR(VaultController,isVaultFile),isVaultFile);
    EXPECT_FALSE(jobd->mergeDirectory(nullptr,frominfo,toinfo));
    QSharedPointer<DFileHandler>  handler(DFileService::instance()->createFileHandler(nullptr,from));
    to.setPath("/zut_mergeDirectory_kk");
    toinfo = DFileService::instance()->createFileInfo(nullptr,to);
    EXPECT_TRUE(jobd->mergeDirectory(handler,frominfo,toinfo));
    to.setPath("./zut_mergeDirectory_kk");
    toinfo = DFileService::instance()->createFileInfo(nullptr,to);
    EXPECT_TRUE(jobd->mergeDirectory(handler,frominfo,toinfo));
    TestHelper::deleteTmpFiles(QStringList() << to.toLocalFile() << from.toLocalFile());
    DDirIteratorPointer (*createDirIterator)(const QObject , const DUrl , const QStringList , QDir::Filters,
                                                   QDirIterator::IteratorFlags, bool silent , bool isgvfs ) =\
            [] (const QObject , const DUrl , const QStringList , QDir::Filters,
            QDirIterator::IteratorFlags, bool silent, bool isgvfs){
        return DDirIteratorPointer(nullptr);
    };
    st.set(ADDR(DFileService,createDirIterator),createDirIterator);
    from.setPath(TestHelper::createTmpDir());
    frominfo = DFileService::instance()->createFileInfo(nullptr,from);
    QProcess::execute("touch " + from.toLocalFile() + "/z_ut_iuuuuuu");
    EXPECT_FALSE(jobd->mergeDirectory(handler,frominfo,toinfo));
    st.reset(ADDR(DFileService,createDirIterator));
    bool (*stateCheck)(void *) = [](void *){return false;};
    st.set(ADDR(DFileCopyMoveJobPrivate,stateCheck),stateCheck);
    EXPECT_FALSE(jobd->mergeDirectory(handler,frominfo,toinfo));
    TestHelper::deleteTmpFiles(QStringList() << from.toLocalFile());
}

TEST_F(DFileCopyMoveJobTest,start_run_moveMOde) {
    DUrl from;
    from.setScheme(FILE_SCHEME);
    from.setPath(TestHelper::createTmpDir());
    QProcess::execute("touch " + from.toLocalFile() + "/z_ut_iuuuuuu");
    job->setMode(DFileCopyMoveJob::MoveMode);
    Stub st;
    bool (*isFromLocalFile)(const DUrlList &) = [](const DUrlList &){return false;};
    st.set(ADDR(DFileCopyMoveJob,isFromLocalFile),isFromLocalFile);
    job->start(DUrlList() << from,DUrl());
    while(!job->isFinished()) {
        QThread::msleep(100);
    }

    job->setCurTrashData(QVariant::fromValue(std::forward<DUrlList>(DUrlList() << from)));
    job->setActionOfErrorType(DFileCopyMoveJob::NoError,DFileCopyMoveJob::NoAction);
    EXPECT_EQ(DFileCopyMoveJob::NoError,job->error());
    EXPECT_EQ(DFileCopyMoveJob::NoHint, job->fileHints());
    EXPECT_FALSE(job->targetUrl().isValid());
    EXPECT_TRUE(job->fileStatisticsIsFinished());
    EXPECT_FALSE(job->completedDirectorys().empty());
    EXPECT_TRUE(job->isCanShowProgress());
    job->setSysncState(true);
    job->setSysncQuitState(true);
    EXPECT_TRUE(job->getSysncState());
    job->waitSysncEnd();

}

TEST_F(DFileCopyMoveJobTest,start_supportActions) {
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
    EXPECT_EQ(DFileCopyMoveJob::CancelAction,
              job->supportActions(DFileCopyMoveJob::UnknowError));
}

TEST_F(DFileCopyMoveJobTest,start_process) {
    DFileCopyMoveJobPrivate * jobd = job->d_func();
    ASSERT_TRUE(jobd);
    DUrl from,to;
    from.setScheme(FILE_SCHEME);
    from.setPath(TestHelper::createTmpFile());
    to = from;
    to.setPath("./");
    DAbstractFileInfoPointer toinfo = DFileService::instance()->createFileInfo(nullptr,to);
    EXPECT_FALSE(jobd->process(from,toinfo));
    TestHelper::deleteTmpFiles(QStringList() << from.toLocalFile() << to.toLocalFile() + from.fileName());
}
TEST_F(DFileCopyMoveJobTest,start_doRemoveFile) {
    DFileCopyMoveJobPrivate * jobd = job->d_func();
    ASSERT_TRUE(jobd);
    DUrl from,to;
    from.setScheme(FILE_SCHEME);
    QString path = TestHelper::createTmpFile();
    from.setPath(path+"_ooo");
    QSharedPointer<DFileHandler>  handler(DFileService::instance()->createFileHandler(nullptr,from));
    DAbstractFileInfoPointer frominfo = DFileService::instance()->createFileInfo(nullptr,from);
    EXPECT_TRUE(jobd->doRemoveFile(handler,frominfo));

    Stub st;
    bool (*isVaultFile)(QString) = [](QString){return true;};
    st.set(ADDR(VaultController,isVaultFile),isVaultFile);
    from.setPath(path);
    handler.reset(DFileService::instance()->createFileHandler(nullptr,from));
    frominfo = DFileService::instance()->createFileInfo(nullptr,from);
    VaultController::FileBaseInfo (*getFileInfo)(const DUrl &) = [](const DUrl &){
        VaultController::FileBaseInfo stl;
        stl.isWritable = false;
        return stl;
    };
    st.set(ADDR(VaultController,getFileInfo),getFileInfo);
    EXPECT_TRUE(jobd->doRemoveFile(handler,frominfo));
    TestHelper::deleteTmpFiles(QStringList() << path);
}

TEST_F(DFileCopyMoveJobTest,start_doRenameFile) {
    DFileCopyMoveJobPrivate * jobd = job->d_func();
    ASSERT_TRUE(jobd);
    DUrl from,to;
    from.setScheme(FILE_SCHEME);
    from.setPath(TestHelper::createTmpFile());
    DUrl linkurl,linkurlnew;
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
    QSharedPointer<DFileHandler>  handler(DFileService::instance()->createFileHandler(nullptr,linkurl));
    DAbstractFileInfoPointer oldinfo = DFileService::instance()->createFileInfo(nullptr,linkurl);
    DAbstractFileInfoPointer newinfo = DFileService::instance()->createFileInfo(nullptr,linkurlnew);
    jobd->enterDirectory(linkurl, linkurlnew);
    EXPECT_FALSE(jobd->doRenameFile(handler,oldinfo,newinfo));

    jobd->setState(DFileCopyMoveJob::RunningState);
    QProcess::execute("chmod 0777 " + to.toLocalFile());
    EXPECT_TRUE(jobd->doRenameFile(handler,oldinfo,newinfo));
    jobd->leaveDirectory();
    TestHelper::deleteTmpFiles(QStringList() << linkurlnew.toLocalFile() << linkurl.toLocalFile() << from.toLocalFile() << to.toLocalFile());
}

TEST_F(DFileCopyMoveJobTest,start_doLinkFile) {
    DFileCopyMoveJobPrivate * jobd = job->d_func();
    ASSERT_TRUE(jobd);
    DUrl from,to;
    from.setScheme(FILE_SCHEME);
    from.setPath("/bin");
    QString linkpath = "/tmp/zut_syslink_tset";
    QSharedPointer<DFileHandler>  handler(DFileService::instance()->createFileHandler(nullptr,from));
    DAbstractFileInfoPointer fileinfo = DFileService::instance()->createFileInfo(nullptr,from);
    EXPECT_FALSE(jobd->doLinkFile(handler,fileinfo,linkpath));
    from.setPath("/tmp/zut_syslink_tset");
    EXPECT_FALSE(jobd->doLinkFile(handler,fileinfo,linkpath));
    TestHelper::deleteTmpFiles(QStringList() << from.toLocalFile() << linkpath);
}

TEST_F(DFileCopyMoveJobTest,start_doCopyFile) {
    DFileCopyMoveJobPrivate * jobd = job->d_func();
    ASSERT_TRUE(jobd);
    DUrl from,to;
    from.setScheme(FILE_SCHEME);
    from.setPath(TestHelper::createTmpFile());
    QFile filefrom(from.toLocalFile());
    if(filefrom.open(QIODevice::WriteOnly)) {
        filefrom.write("fneninfuefuen",64);
        filefrom.close();
    }
    Stub stl;
    DFileDevice* (*createFileDevice)(const QObject *, const DUrl &) = [](const QObject *, const DUrl &){
        DFileDevice * device = nullptr;
        return device;
    };
    stl.set(ADDR(DFileService, createFileDevice),createFileDevice);
    QSharedPointer<DFileHandler>  handler(DFileService::instance()->createFileHandler(nullptr,from));
    DAbstractFileInfoPointer frominfo = DFileService::instance()->createFileInfo(nullptr,from);
    to = from;
    to.setPath("/tmp/zut_test_file_device");
    DAbstractFileInfoPointer toinfo = DFileService::instance()->createFileInfo(nullptr,to);
    EXPECT_FALSE(jobd->doCopyFile(frominfo,toinfo,handler));

    DFileDevice* (*createFileDevice1)(void *,const QObject *, const DUrl &url) = [](void *, const QObject *, const DUrl &url){
        DFileDevice * device = nullptr;
        if (url.toLocalFile() == "/tmp/zut_test_file_device") {
            return device;
        }
        device = new DLocalFileDevice();
        device->setFileUrl(url);
        return device;
    };

    stl.set(ADDR(DFileService, createFileDevice),createFileDevice1);
    EXPECT_FALSE(jobd->doCopyFile(frominfo,toinfo,handler));

    stl.reset(ADDR(DFileService, createFileDevice));
    bool (*isVaultFile)(QString) = [](QString){return true;};
    stl.set(ADDR(VaultController,isVaultFile),isVaultFile);
    to.setPath("./fsjkaaaghfgggggsdfgdsfgsdfgsdfgsdfgaajksdhgajskdhgjkashdjkfasfsdsgsdfgsdfgsdfglah\
               gdrgdfgsdfgsdfgsdsgddddddddddddddddddddddddddddddddgsdggsdsdfffffggggggggggsdfgsdgsdfgg\
               grgsdfgdfjklasndjkasvvskncnkz.hnjxvjkrshgkejrhguierhgkjashdfgjlkashdfgjkahsgjhasjghakghja");
    toinfo = DFileService::instance()->createFileInfo(nullptr,to);
    EXPECT_FALSE(jobd->doCopyFile(frominfo,toinfo,handler));
    to.setPath("/tmp/zut_test_file_device");
    toinfo = DFileService::instance()->createFileInfo(nullptr,to);
    stl.reset(ADDR(VaultController,isVaultFile));
    class devicetest : public  DFileDevice
    {
        bool open(OpenMode mode) override{
            Q_UNUSED(mode);
            return false;
        }
        bool resize(qint64 size) override {
            Q_UNUSED(size);
            return false;
        }
    protected:
        qint64 readData(char *data, qint64 maxlen) override{
            Q_UNUSED(data);
            Q_UNUSED(maxlen);
            return 0;
        }
        qint64 writeData(const char *data, qint64 len) override {
            Q_UNUSED(data);
            Q_UNUSED(len);
            return 0;
        }
    };
    DFileDevice* (*createFileDevice2)(void *,const QObject *, const DUrl &) = [](void *,const QObject *, const DUrl & url){
        DFileDevice * device = nullptr;
        if (url.toLocalFile() != "/tmp/zut_test_file_device"){
            device = new DLocalFileDevice();
            device->setFileUrl(url);
        }
        else{
            device = new devicetest();
        }
        return device;
    };
    stl.set(ADDR(DFileService, createFileDevice),createFileDevice2);
    EXPECT_FALSE(jobd->doCopyFile(frominfo,toinfo,handler));
    DFileDevice* (*createFileDevice3)(void *,const QObject *, const DUrl &) = [](void *,const QObject *, const DUrl & url){
        DFileDevice * device = nullptr;
        if (url.toLocalFile() == "/tmp/zut_test_file_device"){
            device = new DLocalFileDevice();
            device->setFileUrl(url);
        }
        else{
            device = new devicetest();
        }
        return device;
    };
    stl.set(ADDR(DFileService, createFileDevice),createFileDevice3);
    EXPECT_FALSE(jobd->doCopyFile(frominfo,toinfo,handler));
    DFileDevice* (*createFileDevice4)(void *,const QObject *, const DUrl &) = [](void *,const QObject *, const DUrl & url){
        DFileDevice * device = nullptr;
        if (url.toLocalFile() == "/tmp/zut_test_file_device"){
            device = new DLocalFileDevice();
            device->setFileUrl(url);
        }
        else{
            device = new devicetest();
            device->setErrorString("Permission denied");
        }
        return device;
    };
    stl.set(ADDR(DFileService, createFileDevice),createFileDevice4);
    EXPECT_FALSE(jobd->doCopyFile(frominfo,toinfo,handler));
    class devicetest1 : public  DFileDevice
    {
        bool open(OpenMode mode) override{
            Q_UNUSED(mode);
            return true;
        }
        bool resize(qint64 size) override {
            Q_UNUSED(size);
            return false;
        }
    protected:
        qint64 readData(char *data, qint64 maxlen) override{
            Q_UNUSED(data);
            Q_UNUSED(maxlen);
            return 0;
        }
        qint64 writeData(const char *data, qint64 len) override {
            Q_UNUSED(data);
            Q_UNUSED(len);
            return 0;
        }
    };
    DFileDevice* (*createFileDevice5)(void *,const QObject *, const DUrl &) = [](void *,const QObject *, const DUrl & url){
        DFileDevice * device = nullptr;
        if (url.toLocalFile() != "/tmp/zut_test_file_device"){
            device = new DLocalFileDevice();
            device->setFileUrl(url);
        }
        else{
            device = new devicetest1();
        }
        return device;
    };
    job->setFileHints(DFileCopyMoveJob::ResizeDestinationFile);
    stl.set(ADDR(DFileService, createFileDevice),createFileDevice5);
    EXPECT_FALSE(jobd->doCopyFile(frominfo,toinfo,handler));
    DFileCopyMoveJob::Action (*ssse3)(DFileCopyMoveJob::Error, const DAbstractFileInfoPointer,
                                     const DAbstractFileInfoPointer, const QString &) = [](DFileCopyMoveJob::Error , const DAbstractFileInfoPointer,
            const DAbstractFileInfoPointer , const QString &){
        return DFileCopyMoveJob::SkipAction;
    };
    stl.set(ADDR(DFileCopyMoveJobPrivate,setAndhandleError),ssse3);
    EXPECT_TRUE(jobd->doCopyFile(frominfo,toinfo,handler));
    stl.reset(ADDR(DFileCopyMoveJobPrivate,setAndhandleError));
    job->setFileHints(DFileCopyMoveJob::NoHint);
    class devicetest2 : public  DFileDevice
    {

    protected:
        qint64 readData(char *data, qint64 maxlen) override{
            Q_UNUSED(data);
            Q_UNUSED(maxlen);
            QThread::msleep(200);
            return 0;
        }
        qint64 writeData(const char *data, qint64 len) override {
            Q_UNUSED(data);
            Q_UNUSED(len);
            return 0;
        }
    };

    DFileDevice* (*createFileDevice6)(void *,const QObject *, const DUrl &) = [](void *,const QObject *, const DUrl & url){
        DFileDevice * device = nullptr;
        if (url.toLocalFile() == "/tmp/zut_test_file_device"){
            device = new DLocalFileDevice();
            device->setFileUrl(url);
        }
        else{
            device = new devicetest2();
        }
        return device;
    };
    stl.set(ADDR(DFileService, createFileDevice),createFileDevice6);
    QFuture<void> future = QtConcurrent::run([=](){
        EXPECT_FALSE(jobd->doCopyFile(frominfo,toinfo,handler));
    });
    QThread::msleep(100);
    job->stop();
    while (future.isRunning()){
        QThread::msleep(100);
    }
    jobd->setState(DFileCopyMoveJob::RunningState);
    class devicetest3 : public  DFileDevice
    {
        bool seek(qint64 pos) override{
            return false;
        }
    protected:
        qint64 readData(char *data, qint64 maxlen) override{
            Q_UNUSED(data);
            Q_UNUSED(maxlen);
            return -1;
        }
        qint64 writeData(const char *data, qint64 len) override {
            Q_UNUSED(data);
            Q_UNUSED(len);
            return 0;
        }
    };

    DFileDevice* (*createFileDevice7)(void *,const QObject *, const DUrl &) = [](void *,const QObject *, const DUrl & url){
        DFileDevice * device = nullptr;
        if (url.toLocalFile() == "/tmp/zut_test_file_device"){
            device = new DLocalFileDevice();
            device->setFileUrl(url);
        }
        else{
            device = new devicetest3();
        }
        return device;
    };
    stl.set(ADDR(DFileService, createFileDevice),createFileDevice7);
    EXPECT_FALSE(jobd->doCopyFile(frominfo,toinfo,handler));

    DFileDevice* (*createFileDevice8)(void *,const QObject *, const DUrl &) = [](void *,const QObject *, const DUrl & url){
        DFileDevice * device = nullptr;
        if (url.toLocalFile() == "/tmp/zut_test_file_device"){
            device = new DLocalFileDevice();
            device->setFileUrl(url);
        }
        else{
            device = new devicetest3();
        }
        return device;
    };
    stl.set(ADDR(DFileService, createFileDevice),createFileDevice8);
    stl.set(ADDR(DFileCopyMoveJobPrivate,setAndhandleError),ssse3);
    EXPECT_TRUE(jobd->doCopyFile(frominfo,toinfo,handler));
    DFileCopyMoveJob::Action (*ssse4)(DFileCopyMoveJob::Error, const DAbstractFileInfoPointer,
                                     const DAbstractFileInfoPointer, const QString &) = [](DFileCopyMoveJob::Error , const DAbstractFileInfoPointer,
            const DAbstractFileInfoPointer , const QString &){
        return DFileCopyMoveJob::RetryAction;
    };
    stl.set(ADDR(DFileCopyMoveJobPrivate,setAndhandleError),ssse4);
    EXPECT_FALSE(jobd->doCopyFile(frominfo,toinfo,handler));


    stl.reset(ADDR(DFileCopyMoveJobPrivate,setAndhandleError));
    class devicetest4 : public  DFileDevice
    {
    protected:
        qint64 readData(char *data, qint64 maxlen) override{
            Q_UNUSED(data);
            Q_UNUSED(maxlen);
            return 0;
        }
        qint64 writeData(const char *data, qint64 len) override {
            Q_UNUSED(data);
            Q_UNUSED(len);
            QThread::msleep(200);
            return 0;
        }
    };

    DFileDevice* (*createFileDevice9)(void *,const QObject *, const DUrl &) = [](void *,const QObject *, const DUrl & url){
        DFileDevice * device = nullptr;
        if (url.toLocalFile() != "/tmp/zut_test_file_device"){
            device = new DLocalFileDevice();
            device->setFileUrl(url);
        }
        else{
            device = new devicetest4();
        }
        return device;
    };
    stl.set(ADDR(DFileService, createFileDevice),createFileDevice9);
    QFuture<void> future1 = QtConcurrent::run([=](){
        EXPECT_FALSE(jobd->doCopyFile(frominfo,toinfo,handler));
    });
    QThread::msleep(100);
    job->stop();
    while (future1.isRunning()){
        QThread::msleep(100);
    }
    jobd->setState(DFileCopyMoveJob::RunningState);
    class devicetest5 : public  DFileDevice
    {
    protected:
        qint64 readData(char *data, qint64 maxlen) override{
            Q_UNUSED(data);
            Q_UNUSED(maxlen);
            return -1;
        }
        qint64 writeData(const char *data, qint64 len) override {
            Q_UNUSED(data);
            Q_UNUSED(len);
            return -1;
        }
    };

    DFileDevice* (*createFileDevice10)(void *,const QObject *, const DUrl &) = [](void *,const QObject *, const DUrl & url){
        DFileDevice * device = nullptr;
        if (url.toLocalFile() != "/tmp/zut_test_file_device"){
            device = new DLocalFileDevice();
            device->setFileUrl(url);
        }
        else{
            device = new devicetest5();
        }
        return device;
    };
    stl.set(ADDR(DFileService, createFileDevice),createFileDevice10);
    EXPECT_FALSE(jobd->doCopyFile(frominfo,toinfo,handler));

    DFileDevice* (*createFileDevice11)(void *,const QObject *, const DUrl &) = [](void *,const QObject *, const DUrl & url){
        DFileDevice * device = nullptr;
        if (url.toLocalFile() != "/tmp/zut_test_file_device"){
            device = new DLocalFileDevice();
            device->setFileUrl(url);
        }
        else{
            device = new devicetest2();
        }
        return device;
    };
    stl.set(ADDR(DFileService, createFileDevice),createFileDevice11);
    stl.set(ADDR(DFileCopyMoveJobPrivate,setAndhandleError),ssse3);
    EXPECT_TRUE(jobd->doCopyFile(frominfo,toinfo,handler));

    TestHelper::deleteTmpFiles(QStringList() << from.toLocalFile() << to.toLocalFile());
}
