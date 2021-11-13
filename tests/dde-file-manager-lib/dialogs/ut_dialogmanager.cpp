#include "dfmevent.h"
#include "gvfs/qdiskinfo.h"
#include "stub.h"
#include "stubext.h"
#include "dabstractfileinfo.h"
#include "views/windowmanager.h"
#include "dialogs/propertydialog.h"
#include "dialogs/trashpropertydialog.h"
#include "dialogs/computerpropertydialog.h"
#include "deviceinfo/udiskdeviceinfo.h"
#include "deviceinfo/udisklistener.h"
#include "dialogs/filepreviewdialog.h"
#include "dialogs/closealldialogindicator.h"
#include "bluetooth/bluetoothtransdialog.h"
#include "dialogs/ddesktoprenamedialog.h"
#include "interfaces/dfmapplication.h"
#include "io/dfilestatisticsjob.h"
#include "bluetooth/bluetoothtransdialog.h"

#include <gtest/gtest.h>
#include <QWidget>
#include <QGuiApplication>
#include <QListWidget>
#include <DDialog>
#include <QFontMetrics>
#include <ddiskmanager.h>
#include <dblockdevice.h>
#include <qfileinfo.h>
#include <QGuiApplication>
#include <QApplication>
#include <ddiskdevice.h>
#include <QTimer>
#include <QLabel>


using namespace stub_ext;

#define private public
#define protected public
#include "dialogs/dialogmanager.h"
#include "fileoperations/filejob.h"
#include "dialogs/closealldialogindicator.h"
#include "testhelper.h"

namespace  {
    class TestDialogManager : public testing::Test
    {
    public:
        void SetUp() override
        {
            m_pTester = new DialogManager();
            std::cout << "start TestDialogManager" << std::endl;
        }
        void TearDown() override
        {
            delete m_pTester;
            m_pTester = nullptr;
            std::cout << "end TestDialogManager" << std::endl;
        }
    public:
        DialogManager *m_pTester;
    };
}

TEST_F(TestDialogManager, testInit)
{
    EXPECT_NE(nullptr, m_pTester);
}

TEST_F(TestDialogManager, testGetPropertyPos)
{
    int dialogWidth = 10;
    int dialogHeight = 20;
    QPoint result = m_pTester->getPropertyPos(dialogWidth, dialogHeight);
    EXPECT_GE(result.x(), 0);
    EXPECT_GE(result.y(), 0);
}

TEST_F(TestDialogManager, testGetPropertyPos2)
{
    QList<QScreen*>(*stub_screens)() = []()->QList<QScreen*>{
        QList<QScreen*> lst;
        lst.clear();
        return lst;
    };
    Stub stu;
    stu.set(ADDR(QGuiApplication, screens), stub_screens);

    int dialogWidth = 10;
    int dialogHeight = 20;
    QPoint result = m_pTester->getPropertyPos(dialogWidth, dialogHeight);
    EXPECT_GE(result.x(), 0);
    EXPECT_GE(result.y(), 0);
}

TEST_F(TestDialogManager, testGetPerportyPos)
{
    int dialogWidth = 10;
    int dialogHeight = 20;
    int count = 10;
    int index = 5;
    auto screens = qApp->screens();
    QPoint result = m_pTester->getPerportyPos(dialogWidth, dialogHeight, count, index);
    EXPECT_GE(result.x(), 0);
    EXPECT_GE(result.y(), 0);
}

TEST_F(TestDialogManager, testGetPerportyPos2)
{
    QList<QScreen*>(*stub_screens)() = []()->QList<QScreen*>{
        QList<QScreen*> lst;
        lst.clear();
        return lst;
    };
    Stub stu;
    stu.set(ADDR(QGuiApplication, screens), stub_screens);

    int dialogWidth = 10;
    int dialogHeight = 20;
    int count = 10;
    int index = 5;
    QPoint result = m_pTester->getPerportyPos(dialogWidth, dialogHeight, count, index);
    EXPECT_GE(result.x(), 0);
    EXPECT_GE(result.y(), 0);
}

TEST_F(TestDialogManager, testIsTaskDialogEmpty)
{
    EXPECT_TRUE(m_pTester->isTaskDialogEmpty());
}

//int count() const;
TEST_F(TestDialogManager, testIsTaskDialogEmpty2)
{
    int(*stub_count)() = []()->int{
        return 1;
    };
    Stub stu;
    stu.set(ADDR(QListWidget, count), stub_count);
    EXPECT_FALSE(m_pTester->isTaskDialogEmpty());
}

TEST_F(TestDialogManager, testTaskDialog)
{
    DTaskDialog *pResult = m_pTester->taskDialog();
    EXPECT_FALSE(pResult == nullptr);
}

TEST_F(TestDialogManager, testHandleConflictRepsonseConfirmed0)
{
    QSharedPointer<FileJob> job(new FileJob(FileJob::OpticalCheck));
    job->setJobId("10000");
    m_pTester->addJob(job);

    QMap<QString, QString> jobDetail;
    jobDetail.insert("jobId", "10000");
    QMap<QString, QVariant> response;
    response.insert("applyToAll", QVariant(false));
    response.insert("code", QVariant(0));
    m_pTester->handleConflictRepsonseConfirmed(jobDetail, response);
    EXPECT_EQ(true, job->m_isCoExisted);
}

TEST_F(TestDialogManager, testHandleConflictRepsonseConfirmed1)
{
    QSharedPointer<FileJob> job(new FileJob(FileJob::OpticalImageBurn));
    job->setJobId("10000");
    m_pTester->addJob(job);

    QMap<QString, QString> jobDetail;
    jobDetail.insert("jobId", "10000");
    QMap<QString, QVariant> response;
    response.insert("applyToAll", QVariant(false));
    response.insert("code", QVariant(1));
    m_pTester->handleConflictRepsonseConfirmed(jobDetail, response);
    EXPECT_EQ(true, job->m_isReplaced);
}

TEST_F(TestDialogManager, testHandleConflictRepsonseConfirmed2)
{
    QSharedPointer<FileJob> job(new FileJob(FileJob::OpticalBlank));
    job->setJobId("10000");
    m_pTester->addJob(job);

    QMap<QString, QString> jobDetail;
    jobDetail.insert("jobId", "10000");
    QMap<QString, QVariant> response;
    response.insert("applyToAll", QVariant(false));
    response.insert("code", QVariant(2));
    m_pTester->handleConflictRepsonseConfirmed(jobDetail, response);
    EXPECT_EQ(true, job->m_isSkip);
}

TEST_F(TestDialogManager, testHandleConflictRepsonseConfirmed3)
{
    QSharedPointer<FileJob> job(new FileJob(FileJob::OpticalBurn));
    job->setJobId("10000");
    m_pTester->addJob(job);

    QMap<QString, QString> jobDetail;
    jobDetail.insert("jobId", "10000");
    QMap<QString, QVariant> response;
    response.insert("applyToAll", QVariant(false));
    response.insert("code", QVariant(3));
    m_pTester->handleConflictRepsonseConfirmed(jobDetail, response);
    EXPECT_EQ(true, job->m_isCoExisted);
}

TEST_F(TestDialogManager, testRemoveJob)
{
    QSharedPointer<FileJob> job(new FileJob(FileJob::Restore));
    job->setJobId("10000");
    m_pTester->addJob(job);

    m_pTester->removeJob("10000", true);
    bool b = m_pTester->m_jobs.contains("10000");
    EXPECT_EQ(false, b);
}

TEST_F(TestDialogManager, testRemoveJob2)
{
    QSharedPointer<FileJob> job(new FileJob(FileJob::Restore));
    job->setJobId("10000");
    m_pTester->addJob(job);
    m_pTester->removeJob("10000", true);
    bool b = m_pTester->m_jobs.contains("10000");
    EXPECT_EQ(b, false);
}

TEST_F(TestDialogManager, testRemoveJob3)
{
    QSharedPointer<FileJob> job(new FileJob(FileJob::Restore));
    job->setJobId("10000");
    job->m_isOpticalJob = true;
    job->m_isFinished = false;
    m_pTester->addJob(job);
    m_pTester->removeJob("10000", false);
    bool b = m_pTester->m_jobs.contains("10000");
    EXPECT_EQ(b, true);
}

TEST_F(TestDialogManager, testGetJobIdByUrl)
{
    QString jobId = "10000";
    DUrl url("file:///home");

    QSharedPointer<FileJob> job(new FileJob(FileJob::Delete));
    job->setJobId(jobId);
    job->setProperty("pathlist", QVariant(url.toLocalFile()));
    m_pTester->addJob(job);
    QString result = m_pTester->getJobIdByUrl(url);

    EXPECT_STREQ(jobId.toStdString().c_str(), result.toStdString().c_str());
}

TEST_F(TestDialogManager, testGetJobIdByUrl2)
{
    QString jobId = "10000";
    DUrl url("file:///home");

    QSharedPointer<FileJob> job(new FileJob(FileJob::Delete));
    job->setJobId(jobId);
    job->setProperty("pathlist1", QVariant(url.toLocalFile()));
    m_pTester->addJob(job);
    QString result = m_pTester->getJobIdByUrl(url);

    EXPECT_STRNE(jobId.toStdString().c_str(), result.toStdString().c_str());
}

TEST_F(TestDialogManager, testRemoveAllJobs)
{
    QSharedPointer<FileJob> job(new FileJob(FileJob::Trash));
    job->setJobId("10000");
    m_pTester->addJob(job);

    m_pTester->removeAllJobs();
    int count = m_pTester->m_jobs.count();
    EXPECT_EQ(count, 0);
}

TEST_F(TestDialogManager, testUpdateJob)
{
    QSharedPointer<FileJob> job(new FileJob(FileJob::Move));
    job->setJobId("10000");
    job->m_isCanShowProgress = false;
    m_pTester->addJob(job);

    m_pTester->updateJob();
    bool b = m_pTester->m_jobs.contains("10000");
    EXPECT_EQ(b, true);
}

TEST_F(TestDialogManager, testUpdateJob2)
{
    QSharedPointer<FileJob> job(new FileJob(FileJob::Move));
    job->setJobId("10000");
    FileJob::Msec_For_Display = -1;
    m_pTester->addJob(job);

    qint64(*stub_currentMesec)() = []()->qint64{
        return 0;
    };
    Stub stu;
    stu.set(ADDR(FileJob, currentMsec), stub_currentMesec);

    m_pTester->updateJob();
    bool b = m_pTester->m_jobs.contains("10000");
    EXPECT_EQ(b, true);
}

TEST_F(TestDialogManager, testUpdateJob3)
{
    QSharedPointer<FileJob> job(new FileJob(FileJob::Move));
    job->setJobId("10000");
    FileJob::Msec_For_Display = -1;
    job->m_isJobAdded = true;
    m_pTester->addJob(job);

    qint64(*stub_currentMesec)() = []()->qint64{
        return 0;
    };
    Stub stu;
    stu.set(ADDR(FileJob, currentMsec), stub_currentMesec);

    m_pTester->updateJob();
    bool b = m_pTester->m_jobs.contains("10000");
    EXPECT_EQ(b, true);
}

TEST_F(TestDialogManager, testStartAndStopUpdateJobTimer)
{
    m_pTester->startUpdateJobTimer();
    EXPECT_NO_FATAL_FAILURE(m_pTester->stopUpdateJobTimer());
}

TEST_F(TestDialogManager, testAbortJob)
{
    QSharedPointer<FileJob> job(new FileJob(FileJob::Copy));
    job->setJobId("10000");
    m_pTester->addJob(job);

    QMap<QString, QString> jobDetail;
    jobDetail.insert("job1", "10000");

    m_pTester->abortJob(jobDetail);
    bool b = m_pTester->m_jobs.contains("10000");
    EXPECT_EQ(b, true);
}

TEST_F(TestDialogManager, testAbortJobByDestinationUrl)
{
    QString jobId = "10000";
    DUrl url("file:///home");

    QSharedPointer<FileJob> job(new FileJob(FileJob::Delete));
    job->setJobId(jobId);
    job->setProperty("pathlist", QVariant(url.toLocalFile()));
    m_pTester->addJob(job);

    m_pTester->abortJobByDestinationUrl(url);
    bool b = m_pTester->m_jobs.contains("10000");
    EXPECT_EQ(b, true);
}

TEST_F(TestDialogManager, testShowCopyMoveToSelfDialog)
{
    QMap<QString, QString> jobDetail;
    jobDetail.insert("job1", "10000");

    int(*stub_exec)() = []()->int{
        return 0;
    };
    stub_ext::StubExt stu;
    stu.set(VADDR(Dtk::Widget::DDialog, exec), stub_exec);

    EXPECT_NO_FATAL_FAILURE(m_pTester->showCopyMoveToSelfDialog(jobDetail));
}

TEST_F(TestDialogManager, testShowUrlWrongDialog)
{
    DUrl url("file:///home");
    EXPECT_NO_FATAL_FAILURE(m_pTester->showUrlWrongDialog(url));
}

TEST_F(TestDialogManager, testShowRunExcutableScriptDialog)
{
    DUrl url("file:///home");
    quint64 winId = 12345678;

    int(*stub_exec)() = []()->int{
        return 0;
    };
    stub_ext::StubExt stu;
    stu.set(VADDR(Dtk::Widget::DDialog, exec), stub_exec);

    int code = m_pTester->showRunExcutableScriptDialog(url, winId);
    EXPECT_EQ(code ,0);
}

TEST_F(TestDialogManager, testShowRunExcutableFileDialog)
{
    DUrl url("file:///home");
    quint64 winId = 12345678;

    int(*stub_exec)() = []()->int{
        return 0;
    };
    stub_ext::StubExt stu;
    stu.set(VADDR(Dtk::Widget::DDialog, exec), stub_exec);

    int code = m_pTester->showRunExcutableFileDialog(url, winId);
    EXPECT_EQ(code, 0);
}

TEST_F(TestDialogManager, testShowAskIfAddExcutableFlagAndRunDialog)
{
    DUrl url("file:///home");
    quint64 winId = 12345678;

    int(*stub_exec)() = []()->int{
        return 0;
    };
    stub_ext::StubExt stu;
    stu.set(VADDR(Dtk::Widget::DDialog, exec), stub_exec);

    int code = m_pTester->showAskIfAddExcutableFlagAndRunDialog(url, winId);
    EXPECT_EQ(code, 0);
}

TEST_F(TestDialogManager, testShowRenameNameSameErrorDialog)
{
    QString name = "unit_test_dialog";
    DFMEvent event;

    int(*stub_exec)() = []()->int{
        return 0;
    };
    stub_ext::StubExt stu;
    stu.set(VADDR(Dtk::Widget::DDialog, exec), stub_exec);

    int code = m_pTester->showRenameNameSameErrorDialog(name, event);
    EXPECT_EQ(code, 0);
    m_pTester->showRenameBusyErrDialog(event);
}

// 测试文件命名为“..”时，弹出提示框
TEST_F(TestDialogManager, testShowRenameNameDotDotErrorDialog)
{
    DFMEvent event;

    int(*stub_exec)() = []()->int{
        return 0;
    };
    stub_ext::StubExt stu;
    stu.set(VADDR(Dtk::Widget::DDialog, exec), stub_exec);

    int result = m_pTester->showRenameNameDotDotErrorDialog(event);
    EXPECT_TRUE(result == 0 || result == 1);
}

TEST_F(TestDialogManager, testShowOpticalBlankConfirmationDialog)
{
    DUrl url("file:///home");
    DFMUrlBaseEvent event(nullptr, url);

    int(*stub_exec)() = []()->int{
        return 0;
    };
    stub_ext::StubExt stu;
    stu.set(VADDR(Dtk::Widget::DDialog, exec), stub_exec);

    int code = m_pTester->showOpticalBlankConfirmationDialog(event);
    EXPECT_EQ(code ,0);
}

TEST_F(TestDialogManager, testShowOpticalImageOpSelectionDialog)
{
    DUrl url("file:///home");
    DFMUrlBaseEvent event(nullptr, url);

    int(*stub_exec)() = []()->int{
        return 0;
    };
    stub_ext::StubExt stu;
    stu.set(VADDR(Dtk::Widget::DDialog, exec), stub_exec);

    int code = m_pTester->showOpticalImageOpSelectionDialog(event);
    EXPECT_EQ(code ,0);
}

TEST_F(TestDialogManager, testShowOpticalJobFailureDialog)
{
    int type = FileJob::OpticalBlank;
    QString err("unit_test_info");
    QStringList details("");

    int(*stub_exec)() = []()->int{
        return 0;
    };
    stub_ext::StubExt stu;
    stu.set(VADDR(Dtk::Widget::DDialog, exec), stub_exec);

    void(*stub2_show)() = []()->void{};
    stub_ext::StubExt stu2;
    stu2.set(ADDR(QWidget, show), stub2_show);

    EXPECT_NO_FATAL_FAILURE(m_pTester->showOpticalJobFailureDialog(type, err, details));
}

TEST_F(TestDialogManager, testShowOpticalJobFailureDialog2)
{
    int type = FileJob::OpticalBurn;
    QString err("unit_test_info");
    QStringList details("");

    int(*stub_exec)(Dtk::Widget::DDialog*) = [](Dtk::Widget::DDialog* pdlg)->int{
        emit pdlg->buttonClicked(1, "");
        return 0;
    };
    stub_ext::StubExt stu;
    stu.set(VADDR(Dtk::Widget::DDialog, exec), stub_exec);

    void(*stub2_show)() = []()->void{};
    stub_ext::StubExt stu2;
    stu2.set(ADDR(QWidget, show), stub2_show);

    EXPECT_NO_FATAL_FAILURE(m_pTester->showOpticalJobFailureDialog(type, err, details));
}

TEST_F(TestDialogManager, testShowOpticalJobFailureDialog3)
{
    int type = FileJob::OpticalCheck;
    QString err("unit_test_info");
    QStringList details("");

    int(*stub_exec)(Dtk::Widget::DDialog*) = [](Dtk::Widget::DDialog* pdlg)->int{
        emit pdlg->buttonClicked(0, "");
        return 0;
    };
    stub_ext::StubExt stu;
    stu.set(VADDR(Dtk::Widget::DDialog, exec), stub_exec);

    void(*stub2_show)() = []()->void{};
    stub_ext::StubExt stu2;
    stu2.set(ADDR(QWidget, show), stub2_show);

    EXPECT_NO_FATAL_FAILURE(m_pTester->showOpticalJobFailureDialog(type, err, details));
}

TEST_F(TestDialogManager, testShowOpticalJobCompletionDialog)
{
    QString msg("unit_test_name");
    QString icon("dfm_vault");

    int(*stub_exec)() = []()->int{
        return 0;
    };
    stub_ext::StubExt stu;
    stu.set(VADDR(Dtk::Widget::DDialog, exec), stub_exec);

    EXPECT_NO_FATAL_FAILURE(m_pTester->showOpticalJobCompletionDialog(msg, icon));
}

TEST_F(TestDialogManager, testShowDeleteFilesClearTrashDialog)
{
    DUrlList list;
    list.push_back(DUrl("file:///home"));
    DFMUrlListBaseEvent event(nullptr, list);

    int(*stub_exec)() = []()->int{
        return 0;
    };
    stub_ext::StubExt stu;
    stu.set(VADDR(Dtk::Widget::DDialog, exec), stub_exec);

    int code = m_pTester->showDeleteFilesClearTrashDialog(event);
    EXPECT_EQ(code, 0);
}

TEST_F(TestDialogManager, testShowDeleteFilesClearTrashDialog2)
{
    DUrlList list;
    list << DUrl("file:///home") << DUrl("file:///home");
    DFMUrlListBaseEvent event(nullptr, list);

    int(*stub_exec)() = []()->int{
        return 0;
    };
    stub_ext::StubExt stu;
    stu.set(VADDR(Dtk::Widget::DDialog, exec), stub_exec);

    int code = m_pTester->showDeleteFilesClearTrashDialog(event);
    EXPECT_EQ(code, 0);
}

TEST_F(TestDialogManager, testShowDeleteFilesClearTrashDialog3)
{
    DUrlList list;
    list << DUrl::fromTrashFile("/");
    DFMUrlListBaseEvent event(nullptr, list);

    int(*stub_exec)() = []()->int{
        return 0;
    };
    stub_ext::StubExt stu;
    stu.set(VADDR(Dtk::Widget::DDialog, exec), stub_exec);

    int(*stub_filesCount)() = []()->int{
        return 1;
    };
    stub_ext::StubExt stub;
    stub.set(VADDR(DAbstractFileInfo, filesCount), stub_filesCount);

    int code = m_pTester->showDeleteFilesClearTrashDialog(event);
    EXPECT_EQ(code, 0);
}

TEST_F(TestDialogManager, testShowDeleteFilesClearTrashDialog4)
{
    DUrlList list;
    list << DUrl::fromTrashFile("/");
    DFMUrlListBaseEvent event(nullptr, list);

    int(*stub_exec)() = []()->int{
        return 0;
    };
    stub_ext::StubExt stu;
    stu.set(VADDR(Dtk::Widget::DDialog, exec), stub_exec);

    int(*stub_filesCount)() = []()->int{
        return 2;
    };
    stub_ext::StubExt stub;
    stub.set(VADDR(DAbstractFileInfo, filesCount), stub_filesCount);

    int code = m_pTester->showDeleteFilesClearTrashDialog(event);
    EXPECT_EQ(code, 0);
}

TEST_F(TestDialogManager, testShowDeleteFilesClearTrashDialog5)
{
    DUrlList list;
    list << DUrl("trash:///utest");
    DFMUrlListBaseEvent event(nullptr, list);

    int(*stub_exec)() = []()->int{
        return 0;
    };
    stub_ext::StubExt stu;
    stu.set(VADDR(Dtk::Widget::DDialog, exec), stub_exec);

    int code = m_pTester->showDeleteFilesClearTrashDialog(event);
    EXPECT_EQ(code, 0);
}

TEST_F(TestDialogManager, testShowDeleteFilesClearTrashDialog6)
{
    DUrlList list;
    list << DUrl("trash:///utest") << DUrl("trash:///utest1");
    DFMUrlListBaseEvent event(nullptr, list);

    int(*stub_exec)() = []()->int{
        return 0;
    };
    stub_ext::StubExt stu;
    stu.set(VADDR(Dtk::Widget::DDialog, exec), stub_exec);

    int code = m_pTester->showDeleteFilesClearTrashDialog(event);
    EXPECT_EQ(code, 0);
}

TEST_F(TestDialogManager, testShowDeleteFilesClearTrashDialog7)
{
    DUrlList list;
    list << DUrl("recent:///utest") << DUrl("recent:///utest1");
    DFMUrlListBaseEvent event(nullptr, list);

    int(*stub_exec)() = []()->int{
        return 0;
    };
    stub_ext::StubExt stu;
    stu.set(VADDR(Dtk::Widget::DDialog, exec), stub_exec);

    int code = m_pTester->showDeleteFilesClearTrashDialog(event);
    EXPECT_EQ(code, 0);
}

TEST_F(TestDialogManager, testShowRemoveBookMarkDialog)
{
    DFMEvent event;

    int(*stub_exec)() = []()->int{
        return 0;
    };
    stub_ext::StubExt stu;
    stu.set(VADDR(Dtk::Widget::DDialog, exec), stub_exec);

    int code = m_pTester->showRemoveBookMarkDialog(event);
    EXPECT_EQ(code, 0);
}

TEST_F(TestDialogManager, testShowOpenWithDialog)
{
    DFMEvent event;

    int(*stub_exec)() = []()->int{
        return 0;
    };
    stub_ext::StubExt stu;
    stu.set(VADDR(QDialog, exec), stub_exec);

    static QWidget* w = nullptr;
    QWidget * (*stub_getWindowById)(quint64) = [](quint64){
        w = new QWidget();
        return w;
    };

    Stub stub;
    stub.set(ADDR(WindowManager, getWindowById), stub_getWindowById);

    EXPECT_NO_FATAL_FAILURE(m_pTester->showOpenWithDialog(event));

    if (w) {
        delete w;
        w = nullptr;
    }
}
#ifndef __arm__
TEST_F(TestDialogManager, testShowOpenFilesWithDialog)
{
    DFMEvent event;
    EXPECT_NO_FATAL_FAILURE(m_pTester->showOpenFilesWithDialog(event));
}
#endif
TEST_F(TestDialogManager, testShowPropertyDialog)
{
    DUrlList list;
    list << DUrl("file:///home");
    DFMUrlListBaseEvent event(nullptr, list);

    void(*stu_start)(const DUrlList &) = [](const DUrlList &){};
    Stub stu2;
    stu2.set((void(DFileStatisticsJob::*)(const DUrlList &))ADDR(DFileStatisticsJob, start), stu_start);

    void(*stub3_show)() = []()->void{};
    stub_ext::StubExt stu3;
    stu3.set(ADDR(QWidget, show), stub3_show);

#ifdef __arm__
    void(*stub_showComputerPropertyDialog)()=[](){};
    stu2.set(ADDR(DialogManager, showComputerPropertyDialog), stub_showComputerPropertyDialog);
#endif

    EXPECT_NO_FATAL_FAILURE(m_pTester->showPropertyDialog(event));

    QEventLoop loop;
    QTimer::singleShot(200, nullptr, [&loop](){loop.exit();});
    loop.exec();
}
#ifndef __arm__
TEST_F(TestDialogManager, testShowPropertyDialog2)
{
    DUrlList list;
    list << DUrl::fromComputerFile("/");
    DFMUrlListBaseEvent event(nullptr, list);

    void(*stu_start)(const DUrlList &) = [](const DUrlList &){};
    Stub stu2;
    stu2.set((void(DFileStatisticsJob::*)(const DUrlList &))ADDR(DFileStatisticsJob, start), stu_start);

    void(*stub3_show)() = []()->void{};
    stub_ext::StubExt stu3;
    stu3.set(ADDR(QWidget, show), stub3_show);

    EXPECT_NO_FATAL_FAILURE(m_pTester->showPropertyDialog(event));

    QEventLoop loop;
    QTimer::singleShot(200, nullptr, [&loop](){loop.exit();});
    loop.exec();
}
#endif
TEST_F(TestDialogManager, testShowPropertyDialog3)
{
    DUrlList list;
    list << DUrl::fromTrashFile("/");
    DFMUrlListBaseEvent event(nullptr, list);

    void(*stu_start)(const DUrlList &) = [](const DUrlList &){};
    Stub stu2;
    stu2.set((void(DFileStatisticsJob::*)(const DUrlList &))ADDR(DFileStatisticsJob, start), stu_start);

    void(*stub3_show)() = []()->void{};
    stub_ext::StubExt stu3;
    stu3.set(ADDR(QWidget, show), stub3_show);

#ifdef __arm__
    void(*stub_showComputerPropertyDialog)()=[](){};
    stu2.set(ADDR(DialogManager, showComputerPropertyDialog), stub_showComputerPropertyDialog);
#endif

    EXPECT_NO_FATAL_FAILURE(m_pTester->showPropertyDialog(event));

    QEventLoop loop;
    QTimer::singleShot(200, nullptr, [&loop](){loop.exit();});
    loop.exec();
}

TEST_F(TestDialogManager, testShowPropertyDialog4)
{
    DUrlList list;
    list << DUrl("file:///home") << DUrl("file:///jerry2");
    DFMUrlListBaseEvent event(nullptr, list);
    DFMEvent e;

    void(*stu_start)(const DUrlList &) = [](const DUrlList &){};
    Stub stu2;
    stu2.set((void(DFileStatisticsJob::*)(const DUrlList &))ADDR(DFileStatisticsJob, start), stu_start);

    PropertyDialog dlg(e, DUrl("file:///home"));
    m_pTester->m_propertyDialogs.insert(DUrl("file:///home"), &dlg);

    void(*stub3_show)() = []()->void{};
    stub_ext::StubExt stu3;
    stu3.set(ADDR(QWidget, show), stub3_show);

#ifdef __arm__
    void(*stub_showComputerPropertyDialog)()=[](){};
    stu2.set(ADDR(DialogManager, showComputerPropertyDialog), stub_showComputerPropertyDialog);
#endif

    EXPECT_NO_FATAL_FAILURE(m_pTester->showPropertyDialog(event));

    QEventLoop loop;
    QTimer::singleShot(200, nullptr, [&loop](){loop.exit();});
    loop.exec();
}

TEST_F(TestDialogManager, testShowPropertyDialog5)
{
    DUrlList list;
    list << DUrl("file:///home") << DUrl("file:///jerry2")
        << DUrl("file:///home") << DUrl("file:///jerry2")
        << DUrl("file:///home") << DUrl("file:///jerry2")
        << DUrl("file:///home") << DUrl("file:///jerry2")
        << DUrl("file:///home") << DUrl("file:///jerry2")
        << DUrl("file:///home") << DUrl("file:///jerry2")
        << DUrl("file:///home") << DUrl("file:///jerry2")
        << DUrl("file:///home") << DUrl("file:///jerry2")
        << DUrl("file:///home") << DUrl("file:///jerry2")
        << DUrl("file:///home") << DUrl("file:///jerry2");
    DFMUrlListBaseEvent event(nullptr, list);

    void(*stu_start)(const DUrlList &) = [](const DUrlList &){};
    Stub stu2;
    stu2.set((void(DFileStatisticsJob::*)(const DUrlList &))ADDR(DFileStatisticsJob, start), stu_start);

    void(*stub3_show)() = []()->void{};
    stub_ext::StubExt stu3;
    stu3.set(ADDR(QWidget, show), stub3_show);

#ifdef __arm__
    void(*stub_showComputerPropertyDialog)()=[](){};
    stu2.set(ADDR(DialogManager, showComputerPropertyDialog), stub_showComputerPropertyDialog);
#endif

    EXPECT_NO_FATAL_FAILURE(m_pTester->showPropertyDialog(event));

    QEventLoop loop;
    QTimer::singleShot(200, nullptr, [&loop](){loop.exit();});
    loop.exec();
}

TEST_F(TestDialogManager, testShowShareOptionsInPropertyDialog)
{
    DUrlList list;
    list.push_back(DUrl("file:///home"));
    DFMUrlListBaseEvent event(nullptr, list);
    void(*stu_start)(const DUrlList &) = [](const DUrlList &){};
    Stub stu;
    stu.set((void(DFileStatisticsJob::*)(const DUrlList &))ADDR(DFileStatisticsJob, start), stu_start);

    EXPECT_NO_FATAL_FAILURE(m_pTester->showShareOptionsInPropertyDialog(event));
}

TEST_F(TestDialogManager, testShowTrashPropertyDialog)
{
    DFMEvent event;
    void(*stu_start)(const DUrlList &) = [](const DUrlList &){};
    Stub stu2;
    stu2.set((void(DFileStatisticsJob::*)(const DUrlList &))ADDR(DFileStatisticsJob, start), stu_start);

    void(*stub3_show)() = []()->void{};
    stub_ext::StubExt stu3;
    stu3.set(ADDR(QWidget, show), stub3_show);

    EXPECT_NO_FATAL_FAILURE(m_pTester->showTrashPropertyDialog(event));
    QEventLoop loop;
    QTimer::singleShot(200, nullptr, [&loop](){loop.exit();});
    loop.exec();
}

TEST_F(TestDialogManager, testShowTrashPropertyDialog2)
{
    void(*stu_start)(const DUrlList &) = [](const DUrlList &){};
    Stub stu3;
    stu3.set((void(DFileStatisticsJob::*)(const DUrlList &))ADDR(DFileStatisticsJob, start), stu_start);

    TrashPropertyDialog* pdlg = new TrashPropertyDialog(DUrl::fromTrashFile("/"));
    m_pTester->m_trashDialog = pdlg;
    DFMEvent event;

    void(*stub4_show)() = []()->void{};
    stub_ext::StubExt stu4;
    stu4.set(ADDR(QWidget, show), stub4_show);

    EXPECT_NO_FATAL_FAILURE(m_pTester->showTrashPropertyDialog(event));
    QEventLoop loop;
    QTimer::singleShot(200, nullptr, [&loop](){loop.exit();});
    loop.exec();
}

TEST_F(TestDialogManager, testShowTrashPropertyDialog3)
{
    DFMEvent event;

    void(*stub_show)(TrashPropertyDialog*) = [](TrashPropertyDialog* pdlg){
        emit pdlg->finished(0);
    };
    Stub stu;
    stu.set(ADDR(QWidget, show), stub_show);

    void(*stu_start)(const DUrlList &) = [](const DUrlList &){};
    Stub stu2;
    stu2.set((void(DFileStatisticsJob::*)(const DUrlList &))ADDR(DFileStatisticsJob, start), stu_start);

    void(*stub3_show)() = []()->void{};
    stub_ext::StubExt stu3;
    stu3.set(ADDR(QWidget, show), stub3_show);

    EXPECT_NO_FATAL_FAILURE(m_pTester->showTrashPropertyDialog(event));
    QEventLoop loop;
    QTimer::singleShot(200, nullptr, [&loop](){loop.exit();});
    loop.exec();
}

TEST_F(TestDialogManager, testShowComputerPropertyDialog)
{
    void(*stub4_show)() = []()->void{};
    stub_ext::StubExt stu4;
    stu4.set(ADDR(QWidget, show), stub4_show);

    void(*stub3_show)() = []()->void{};
    stub_ext::StubExt stu3;
    stu3.set(ADDR(QWidget, show), stub3_show);

#ifdef __arm__
    Stub stu2;
    void(*stub_showComputerPropertyDialog)()=[](){};
    stu2.set(ADDR(DialogManager, showComputerPropertyDialog), stub_showComputerPropertyDialog);
#endif

    EXPECT_NO_FATAL_FAILURE(m_pTester->showComputerPropertyDialog());
    QEventLoop loop;
    QTimer::singleShot(200, nullptr, [&loop](){loop.exit();});
    loop.exec();
}

TEST_F(TestDialogManager, testShowComputerPropertyDialog2)
{
    //ComputerPropertyDialog dlg;
    //m_pTester->m_computerDialog = &dlg;

    void(*stub4_show)() = []()->void{};
    stub_ext::StubExt stu4;
    stu4.set(ADDR(QWidget, show), stub4_show);

    void(*stub3_show)() = []()->void{};
    stub_ext::StubExt stu3;
    stu3.set(ADDR(QWidget, show), stub3_show);

#ifdef __arm__
    Stub stu2;
    void(*stub_showComputerPropertyDialog)()=[](){};
    stu2.set(ADDR(DialogManager, showComputerPropertyDialog), stub_showComputerPropertyDialog);
#endif

    EXPECT_NO_FATAL_FAILURE(m_pTester->showComputerPropertyDialog());
    QEventLoop loop;
    QTimer::singleShot(200, nullptr, [&loop](){loop.exit();});
    loop.exec();
}

TEST_F(TestDialogManager, testShowDevicePropertyDialog)
{
    DFMEvent event;

    static QWidget * w = nullptr;
    QWidget*(*stub_getWindowById)(quint64) = [](quint64)->QWidget*{
        w = new QWidget();
        return w;
    };
    Stub stub;
    stub.set(ADDR(WindowManager, getWindowById), stub_getWindowById);

    void(*stub3_show)() = []()->void{};
    stub_ext::StubExt stu3;
    stu3.set(ADDR(QWidget, show), stub3_show);

    EXPECT_NO_FATAL_FAILURE(m_pTester->showDevicePropertyDialog(event));

    if (w) {
        delete w;
        w = nullptr;
    }
}

TEST_F(TestDialogManager, testShowDiskErrorDialog)
{
    QString id = "123";
    QString errorText = "test error";

    UDiskDeviceInfoPointer(*stub_getDevice)(const QString&) = [](const QString&)->UDiskDeviceInfoPointer{
        QExplicitlySharedDataPointer<UDiskDeviceInfo> pin(new UDiskDeviceInfo);
        return pin;
    };
    Stub stu;
    stu.set(ADDR(UDiskListener, getDevice), stub_getDevice);

    int(*stub_exec)() = []()->int{
        return 1;
    };
    stub_ext::StubExt stub;
    stub.set(VADDR(QDialog, exec), stub_exec);

    EXPECT_NO_FATAL_FAILURE(m_pTester->showDiskErrorDialog(id, errorText));
}

TEST_F(TestDialogManager, testShowBreakSymlinkDialog)
{
    QString targetName = "test";
    DUrl linkfile("trash:///file");

    int(*stub_exec)() = []()->int{
        return 0;
    };
    stub_ext::StubExt stub;
    stub.set(VADDR(QDialog, exec), stub_exec);

    EXPECT_NO_FATAL_FAILURE(m_pTester->showBreakSymlinkDialog(targetName, linkfile));
}

TEST_F(TestDialogManager, testShowBreakSymlinkDialog2)
{
    QString targetName = "test";
    DUrl linkfile("file:///file");

    int(*stub_exec)() = []()->int{
        return 0;
    };
    stub_ext::StubExt stub;
    stub.set(VADDR(QDialog, exec), stub_exec);

    EXPECT_NO_FATAL_FAILURE(m_pTester->showBreakSymlinkDialog(targetName, linkfile));
}

TEST_F(TestDialogManager, testShowConnectToServerDialog)
{
    void(*stub3_show)() = []()->void{};
    stub_ext::StubExt stu3;
    stu3.set(ADDR(QWidget, show), stub3_show);

    quint64 winId(12345678);
    EXPECT_NO_FATAL_FAILURE(m_pTester->showConnectToServerDialog(winId));
}

TEST_F(TestDialogManager, testShowConnectToServerDialog2)
{
    quint64 winId(12345678);

    static QWidget* w = nullptr;
    QWidget*(*stub_getWindowById)(quint64) = [](quint64)->QWidget*{
        w = new QWidget();
        return w;
    };
    Stub stub;
    stub.set(ADDR(WindowManager, getWindowById), stub_getWindowById);

    void(*stub3_show)() = []()->void{};
    stub_ext::StubExt stu3;
    stu3.set(ADDR(QWidget, show), stub3_show);

    EXPECT_NO_FATAL_FAILURE(m_pTester->showConnectToServerDialog(winId));

    if (w) {
        delete w;
        w = nullptr;
    }
}

TEST_F(TestDialogManager, testShowUserSharePasswordSettingDialog)
{
    quint64 winId(12345678);

    static QWidget* w = nullptr;
    QWidget*(*stub_getWindowById)(quint64) = [](quint64)->QWidget*{
        w = new QWidget();
        return w;
    };
    Stub stub;
    stub.set(ADDR(WindowManager, getWindowById), stub_getWindowById);

    void(*stub3_show)() = []()->void{};
    stub_ext::StubExt stu3;
    stu3.set(ADDR(QWidget, show), stub3_show);

    EXPECT_NO_FATAL_FAILURE(m_pTester->showUserSharePasswordSettingDialog(winId));

    if (w) {
        delete w;
        w = nullptr;
    }
}

TEST_F(TestDialogManager, testShowGlobalSettingsDialog)
{
    quint64 winId(12345678);

    static QWidget* w = nullptr;
    QWidget*(*stub_getWindowById)(quint64) = [](quint64)->QWidget*{
        w = new QWidget();
        return w;
    };
    Stub stub;
    stub.set(ADDR(WindowManager, getWindowById), stub_getWindowById);

    // 此段代码用于调试
//    QObject*(*stu_instance)() = []()->QObject*{
//        QObject * obj = new QObject();
//        obj->deleteLater();
//        return obj;
//    };
//    Stub stu2;
//    stu2.set(ADDR(DFMApplication, instance), stu_instance);

    void(*stub3_show)() = []()->void{};
    stub_ext::StubExt stu3;
    stu3.set(ADDR(QWidget, show), stub3_show);

    EXPECT_NO_FATAL_FAILURE(m_pTester->showGlobalSettingsDialog(winId));

    if (w) {
        delete w;
        w = nullptr;
    }
}

TEST_F(TestDialogManager, testShowGlobalSettingsDialog2)
{
    quint64 winId(12345678);

    static QWidget* w = nullptr;
    QWidget*(*stub_getWindowById)(quint64) = [](quint64)->QWidget*{
        w = new QWidget();
        w->setProperty("isSettingDialogShown", QVariant(true));
        return w;
    };
    Stub stub;
    stub.set(ADDR(WindowManager, getWindowById), stub_getWindowById);

    void(*stub3_show)() = []()->void{};
    stub_ext::StubExt stu3;
    stu3.set(ADDR(QWidget, show), stub3_show);

    EXPECT_NO_FATAL_FAILURE(m_pTester->showGlobalSettingsDialog(winId));

    if (w) {
        delete w;
        w = nullptr;
    }
}

TEST_F(TestDialogManager, testShowDiskSpaceOutOfUsedDialogLater)
{
    int(*stub_exec)() = []()->int{
        return 1;
    };
    stub_ext::StubExt stub;
    stub.set(VADDR(QDialog, exec), stub_exec);
    EXPECT_NO_FATAL_FAILURE(m_pTester->showDiskSpaceOutOfUsedDialog());
//    QEventLoop loop;
//    QTimer::singleShot(300, nullptr, [&loop](){loop.exit();});
//    loop.exec();
}

TEST_F(TestDialogManager, testShowDiskSpaceOutOfUsedDialog)
{
    int(*stub_exec)() = []()->int{
        return 1;
    };
    stub_ext::StubExt stub;
    stub.set(VADDR(QDialog, exec), stub_exec);

    EXPECT_NO_FATAL_FAILURE(m_pTester->showDiskSpaceOutOfUsedDialog());
}

TEST_F(TestDialogManager, testShow4gFat32Dialog)
{
    int(*stub_exec)() = []()->int{
        return 1;
    };
    stub_ext::StubExt stub;
    stub.set(VADDR(QDialog, exec), stub_exec);

    EXPECT_NO_FATAL_FAILURE(m_pTester->show4gFat32Dialog());
}

TEST_F(TestDialogManager, testShowFailToCreateSymlinkDialog)
{
    QString error("test error");

    int(*stub_exec)() = []()->int{
        return 1;
    };
    stub_ext::StubExt stub;
    stub.set(VADDR(QDialog, exec), stub_exec);

    EXPECT_NO_FATAL_FAILURE(m_pTester->showFailToCreateSymlinkDialog(error));
}

TEST_F(TestDialogManager, testShowMoveToTrashConflictDialog)
{
    DUrlList lst;

    int(*stub_exec)() = []()->int{
        return 1;
    };
    stub_ext::StubExt stub;
    stub.set(VADDR(QDialog, exec), stub_exec);

    EXPECT_NO_FATAL_FAILURE(m_pTester->showMoveToTrashConflictDialog(lst));
}

TEST_F(TestDialogManager, testShowDeleteSystemPathWarnDialog)
{
    quint64 winId(12345678);

    int(*stub_exec)() = []()->int{
        return 1;
    };
    stub_ext::StubExt stub;
    stub.set(VADDR(QDialog, exec), stub_exec);

    EXPECT_NO_FATAL_FAILURE(m_pTester->showDeleteSystemPathWarnDialog(winId));
}

TEST_F(TestDialogManager, testShowFilePreviewDialog)
{
    void(*stub3_show)() = []()->void{};
    stub_ext::StubExt stu3;
    stu3.set(ADDR(QWidget, show), stub3_show);

    DUrlList selectUrls;
    selectUrls << DUrl("file:///testfile2");
    DUrlList entryUrls;
    entryUrls << DUrl("file:///testfile2");
    EXPECT_NO_FATAL_FAILURE(m_pTester->showFilePreviewDialog(selectUrls, entryUrls));
}

TEST_F(TestDialogManager, testShowFilePreviewDialog2)
{
    void(*stub3_show)() = []()->void{};
    stub_ext::StubExt stu3;
    stu3.set(ADDR(QWidget, show), stub3_show);

    DUrlList selectUrls;
    selectUrls.clear();
    DUrlList entryUrls;
    entryUrls.clear();
    EXPECT_NO_FATAL_FAILURE(m_pTester->showFilePreviewDialog(selectUrls, entryUrls));
}

TEST_F(TestDialogManager, testShowFilePreviewDialog3)
{
    DUrlList selectUrls;
    selectUrls << DUrl("file:///testfile2");
    DUrlList entryUrls;
    entryUrls << DUrl("file:///testfile2");

    bool(*stub_isWayLand)() = []()->bool{
        return true;
    };
    Stub stu;
    stu.set(ADDR(DFMGlobal, isWayLand), stub_isWayLand);

    void(*stub3_show)() = []()->void{};
    stub_ext::StubExt stu3;
    stu3.set(ADDR(QWidget, show), stub3_show);

    DUrlList previewUrllist;
    previewUrllist << DUrl("file:///testfile2") << DUrl("file:///home2");
    dde_file_manager::FilePreviewDialog filePreviewDialog(previewUrllist);
    m_pTester->m_filePreviewDialog = &filePreviewDialog;

    EXPECT_NO_FATAL_FAILURE(m_pTester->showFilePreviewDialog(selectUrls, entryUrls));

    emit m_pTester->m_filePreviewDialog->signalCloseEvent();
}

TEST_F(TestDialogManager, testShowFilePreviewDialog4)
{
    DUrlList selectUrls;
    selectUrls << DUrl("file:///testfile2");
    DUrlList entryUrls;
    entryUrls << DUrl("file:///testfile2");

    bool(*stub_isWayLand)() = []()->bool{
        return true;
    };
    Stub stu;
    stu.set(ADDR(DFMGlobal, isWayLand), stub_isWayLand);

    bool(*stub_DUrlListCompare)(DUrlList) = [](DUrlList)->bool{
        return true;
    };
    Stub stu2;
    stu2.set(ADDR(DialogManager, DUrlListCompare), stub_DUrlListCompare);

    void(*stub3_show)() = []()->void{};
    stub_ext::StubExt stu3;
    stu3.set(ADDR(QWidget, show), stub3_show);

    DUrlList previewUrllist;
    previewUrllist << DUrl("file:///testfile2") << DUrl("file:///home2");
    dde_file_manager::FilePreviewDialog filePreviewDialog(previewUrllist);
    m_pTester->m_filePreviewDialog = &filePreviewDialog;

    EXPECT_NO_FATAL_FAILURE(m_pTester->showFilePreviewDialog(selectUrls, entryUrls));
}

TEST_F(TestDialogManager, testShowRestoreFailedDialog)
{
    DUrlList lst;
    lst << DUrl("file:///testfile2");

    int(*stub_exec)() = []()->int{
        return 1;
    };
    stub_ext::StubExt stub;
    stub.set(VADDR(QDialog, exec), stub_exec);

    EXPECT_NO_FATAL_FAILURE(m_pTester->showRestoreFailedDialog(lst));
}

TEST_F(TestDialogManager, testShowRestoreFailedDialog2)
{
    DUrlList lst;
    lst << DUrl("file:///testfile2") << DUrl("file:///testfile1");

    int(*stub_exec)() = []()->int{
        return 1;
    };
    stub_ext::StubExt stub;
    stub.set(VADDR(QDialog, exec), stub_exec);

    EXPECT_NO_FATAL_FAILURE(m_pTester->showRestoreFailedDialog(lst));
}

TEST_F(TestDialogManager, testShowRestoreFailedPerssionDialog)
{
    QString srcPath("test srcPath");
    QString targetPath("test targetPath");

    int(*stub_exec)() = []()->int{
        return 1;
    };
    stub_ext::StubExt stub;
    stub.set(VADDR(QDialog, exec), stub_exec);

    EXPECT_NO_FATAL_FAILURE(m_pTester->showRestoreFailedPerssionDialog(srcPath, targetPath));
}

TEST_F(TestDialogManager, testShowRestoreFailedSourceNotExists)
{
    DUrlList lst;
    lst << DUrl("file:///testfile2");

    int(*stub_exec)() = []()->int{
        return 1;
    };
    stub_ext::StubExt stub;
    stub.set(VADDR(QDialog, exec), stub_exec);

    EXPECT_NO_FATAL_FAILURE(m_pTester->showRestoreFailedSourceNotExists(lst));
}

TEST_F(TestDialogManager, testShowRestoreFailedSourceNotExists2)
{
    DUrlList lst;
    lst << DUrl("file:///testfile2") << DUrl("file:///testfile1");

    int(*stub_exec)() = []()->int{
        return 1;
    };
    stub_ext::StubExt stub;
    stub.set(VADDR(QDialog, exec), stub_exec);

    EXPECT_NO_FATAL_FAILURE(m_pTester->showRestoreFailedSourceNotExists(lst));
}

TEST_F(TestDialogManager, testShowMultiFilesRenameDialog)
{
    QList<DUrl> selectedUrls;
    selectedUrls << DUrl("file:///home");

    size_t(*stub_getCurrentModeIndex)() = []()->size_t{
        return 1;
    };
    Stub stu;
    stu.set(ADDR(DDesktopRenameDialog, getCurrentModeIndex), stub_getCurrentModeIndex);

    int(*stub_exec)() = []()->int{
        return 1;
    };
    stub_ext::StubExt stub;
    stub.set(VADDR(QDialog, exec), stub_exec);

    EXPECT_NO_FATAL_FAILURE(m_pTester->showMultiFilesRenameDialog(selectedUrls));
}

TEST_F(TestDialogManager, testShowMultiFilesRenameDialog2)
{
    QList<DUrl> selectedUrls;
    selectedUrls << DUrl("file:///home");

    size_t(*stub_getCurrentModeIndex)() = []()->size_t{
        return 2;
    };
    Stub stu;
    stu.set(ADDR(DDesktopRenameDialog, getCurrentModeIndex), stub_getCurrentModeIndex);

    int(*stub_exec)() = []()->int{
        return 1;
    };
    stub_ext::StubExt stub;
    stub.set(VADDR(QDialog, exec), stub_exec);

    EXPECT_NO_FATAL_FAILURE(m_pTester->showMultiFilesRenameDialog(selectedUrls));
}

TEST_F(TestDialogManager, testShowMultiFilesRenameDialog3)
{
    QList<DUrl> selectedUrls;
    selectedUrls << DUrl("file:///home");

    size_t(*stub_getCurrentModeIndex)() = []()->size_t{
        return 0;
    };
    Stub stu;
    stu.set(ADDR(DDesktopRenameDialog, getCurrentModeIndex), stub_getCurrentModeIndex);

    int(*stub_exec)() = []()->int{
        return 1;
    };
    stub_ext::StubExt stub;
    stub.set(VADDR(QDialog, exec), stub_exec);

    EXPECT_NO_FATAL_FAILURE(m_pTester->showMultiFilesRenameDialog(selectedUrls));
}


TEST_F(TestDialogManager, testShowAddUserShareFailedDialog)
{
    QString sharePath("/home");

    int(*stub_exec)() = []()->int{
        return 1;
    };
    stub_ext::StubExt stub;
    stub.set(VADDR(QDialog, exec), stub_exec);

    EXPECT_NO_FATAL_FAILURE(m_pTester->showAddUserShareFailedDialog(sharePath));
}

TEST_F(TestDialogManager, testShowNoPermissionDialog)
{
    DUrlList list;
    list.push_back(DUrl("file:///home"));
    DFMUrlListBaseEvent event(nullptr, list);

    int(*stub_exec)() = []()->int{
        return 1;
    };
    stub_ext::StubExt stub;
    stub.set(VADDR(QDialog, exec), stub_exec);

    void(*stub2_setPixmap)(const QPixmap &) = [](const QPixmap &)->void{};
    stub_ext::StubExt stub2;
    stub2.set(VADDR(QLabel, setPixmap), stub2_setPixmap);

    EXPECT_NO_FATAL_FAILURE(m_pTester->showNoPermissionDialog(event));
}

TEST_F(TestDialogManager, testShowNoPermissionDialog2)
{
    DUrlList list;
    list.push_back(DUrl("file:///home"));
    list.push_back(DUrl("file:///testfile1"));
    DFMUrlListBaseEvent event(nullptr, list);

    int(*stub_exec)() = []()->int{
        return 1;
    };
    stub_ext::StubExt stub;
    stub.set(VADDR(QDialog, exec), stub_exec);

    void(*stub2_setPixmap)(const QPixmap &) = [](const QPixmap &)->void{};
    stub_ext::StubExt stub2;
    stub2.set(VADDR(QLabel, setPixmap), stub2_setPixmap);

    EXPECT_NO_FATAL_FAILURE(m_pTester->showNoPermissionDialog(event));
}

TEST_F(TestDialogManager, testShowNoPermissionDialog3)
{
    DUrlList list;
    list.clear();
    DFMUrlListBaseEvent event(nullptr, list);

    int(*stub_exec)() = []()->int{
        return 1;
    };
    stub_ext::StubExt stub;
    stub.set(VADDR(QDialog, exec), stub_exec);

    void(*stub2_setPixmap)(const QPixmap &) = [](const QPixmap &)->void{};
    stub_ext::StubExt stub2;
    stub2.set(VADDR(QLabel, setPixmap), stub2_setPixmap);

    EXPECT_NO_FATAL_FAILURE(m_pTester->showNoPermissionDialog(event));
}

TEST_F(TestDialogManager, testShowErrorDialog)
{
    QString title("test title");
    QString message("test message");

    int(*stub_exec)() = []()->int{
        return 1;
    };
    stub_ext::StubExt stub;
    stub.set(VADDR(QDialog, exec), stub_exec);

    EXPECT_NO_FATAL_FAILURE(m_pTester->showErrorDialog(title, message));
}

TEST_F(TestDialogManager, testRemovePropertyDialog)
{
    DUrl url("file:///home");
    m_pTester->m_propertyDialogs.insert(url, nullptr);
    m_pTester->removePropertyDialog(url);
    bool b = m_pTester->m_propertyDialogs.contains(url);
    EXPECT_EQ(b, false);
}
#ifndef __arm__
TEST_F(TestDialogManager, testCloseAllPropertyDialog)
{

    DUrlList list;
    list << DUrl("file:///home") << DUrl("file:///jerry2");
    DFMUrlListBaseEvent event(nullptr, list);
    DFMEvent e;

    void(*stu_start)(const DUrlList &) = [](const DUrlList &){};
    Stub stu;
    stu.set((void(DFileStatisticsJob::*)(const DUrlList &))ADDR(DFileStatisticsJob, start), stu_start);

    PropertyDialog dlg(e, DUrl("file:///home"));
    m_pTester->m_propertyDialogs.insert(DUrl("file:///home"), &dlg);

    TrashPropertyDialog* pdlg = new TrashPropertyDialog(DUrl::fromTrashFile("/"));
    m_pTester->m_trashDialog = pdlg;

#ifndef __arm__
    ComputerPropertyDialog dlg2;
    m_pTester->m_computerDialog = &dlg2;
#endif

    EXPECT_NO_FATAL_FAILURE(m_pTester->closeAllPropertyDialog());

    if (pdlg) {
        pdlg->deleteLater();
        pdlg = nullptr;
    }
}
#endif
TEST_F(TestDialogManager, testUpdateCloseIndicator)
{
    DUrlList list;
    list << DUrl("file:///home") << DUrl("file:///jerry2");
    DFMUrlListBaseEvent event(nullptr, list);
    DFMEvent e;

    void(*stu_start)(const DUrlList &) = [](const DUrlList &){};
    Stub stu;
    stu.set((void(DFileStatisticsJob::*)(const DUrlList &))ADDR(DFileStatisticsJob, start), stu_start);

    PropertyDialog dlg(e, DUrl("file:///home"));
    m_pTester->m_propertyDialogs.insert(DUrl("file:///home"), &dlg);

    m_pTester->updateCloseIndicator();
    EXPECT_NE(nullptr, m_pTester->m_closeIndicatorDialog);
}

TEST_F(TestDialogManager, testRaiseAllPropertyDialog)
{
    void(*stu_start)(const DUrlList &) = [](const DUrlList &){};
    Stub stu;
    stu.set((void(DFileStatisticsJob::*)(const DUrlList &))ADDR(DFileStatisticsJob, start), stu_start);

//    DFMEvent e;
//    PropertyDialog *pDlg = new PropertyDialog(e, DUrl("file://~/Videos"));
//    m_pTester->m_propertyDialogs.insert(DUrl("file://~/Videos"), pDlg);

    void(*stub3_show)() = []()->void{};
    stub_ext::StubExt stu3;
    stu3.set(ADDR(QWidget, show), stub3_show);

    void(*stub4_raise)() = []()->void{
            int a = 0;
    };
    stub_ext::StubExt stu4;
    stu4.set(ADDR(QWidget, raise), stub4_raise);

    EXPECT_NO_FATAL_FAILURE(m_pTester->raiseAllPropertyDialog());

//    if (pDlg)
//        pDlg->deleteLater();
}

TEST_F(TestDialogManager, testHandleFocusChanged)
{
    QWidget old;
    QWidget now;

    m_pTester->m_propertyDialogs.insert(DUrl("file:///home"), qobject_cast<PropertyDialog *>(qApp->activeWindow()));

    void(*stub_raiseAllPropertyDialog)() = [](){

    };
    Stub stu;
    stu.set(ADDR(DialogManager, raiseAllPropertyDialog), stub_raiseAllPropertyDialog);

    void(*stub3_show)() = []()->void{};
    stub_ext::StubExt stu3;
    stu3.set(ADDR(QWidget, show), stub3_show);

    EXPECT_NO_FATAL_FAILURE(m_pTester->handleFocusChanged(&old, &now));
}

TEST_F(TestDialogManager, testHandleFocusChanged2)
{
    QWidget old;
    QWidget now;

    m_pTester->m_closeIndicatorDialog = qobject_cast<CloseAllDialogIndicator *>(qApp->activeWindow());

    void(*stub_raiseAllPropertyDialog)() = [](){};
    Stub stu;
    stu.set(ADDR(DialogManager, raiseAllPropertyDialog), stub_raiseAllPropertyDialog);

    EXPECT_NO_FATAL_FAILURE(m_pTester->handleFocusChanged(&old, &now));
}

TEST_F(TestDialogManager, testShowTaskProgressDlgOnActive)
{
    EXPECT_NO_FATAL_FAILURE(m_pTester->showTaskProgressDlgOnActive());
}

TEST_F(TestDialogManager, testShowTaskProgressDlgOnActive2)
{
    m_pTester->m_taskDialog = nullptr;
    EXPECT_NO_FATAL_FAILURE(m_pTester->showTaskProgressDlgOnActive());
}

TEST_F(TestDialogManager, testShowTaskProgressDlgOnActive3)
{
    QString jobId = "10000";
    DUrl url("file:///home");
    QSharedPointer<FileJob> job(new FileJob(FileJob::Delete));
    job->setJobId(jobId);
    job->setProperty("pathlist", QVariant(url.toLocalFile()));
    QString jobId2 = "10001";
    DUrl url2("file:///home");
    QSharedPointer<FileJob> job2(new FileJob(FileJob::Delete));
    job2->setJobId(jobId2);
    job2->setProperty("pathlist", QVariant(url.toLocalFile()));
    job2->m_isFinished = true;
    m_pTester->m_jobs.insert(jobId, job);
    m_pTester->m_jobs.insert(jobId2, job2);

    EXPECT_NO_FATAL_FAILURE(m_pTester->showTaskProgressDlgOnActive());
}

#ifndef __arm__
TEST_F(TestDialogManager, testShowUnableToLocateDir)
{
    QString dir("/testfile");

    int(*stub_exec)() = []()->int{
        return 1;
    };
    stub_ext::StubExt stub;
    stub.set(VADDR(QDialog, exec), stub_exec);

    int code = m_pTester->showUnableToLocateDir(dir);
    EXPECT_EQ(code, 1);
}
#endif

TEST_F(TestDialogManager, testRefreshPropertyDialogs)
{
    DUrl oldUrl("file:///home");
    DUrl newUrl("file:///home1");

    DUrlList list;
    list << DUrl("file:///home") << DUrl("file:///jerry2");
    DFMUrlListBaseEvent event(nullptr, list);
    DFMEvent e;

    void(*stu_start)(const DUrlList &) = [](const DUrlList &){};
    Stub stu;
    stu.set((void(DFileStatisticsJob::*)(const DUrlList &))ADDR(DFileStatisticsJob, start), stu_start);

    PropertyDialog dlg(e, DUrl("file:///home"));
    m_pTester->m_propertyDialogs.insert(DUrl("file:///home"), &dlg);

    EXPECT_NO_FATAL_FAILURE(m_pTester->refreshPropertyDialogs(oldUrl, newUrl));
}

TEST_F(TestDialogManager, testhandleConflictRepsonseConfirmed)
{
    QMap<QString, QString> jobDetail;
    jobDetail.insert("jobId", "10000");
    QMap<QString, QVariant> response;

    QString jobId = "10000";
    DUrl url("file:///home1");
    QSharedPointer<FileJob> job(new FileJob(FileJob::Delete));
    job->setJobId(jobId);
    job->setProperty("pathlist", QVariant(url.toLocalFile()));
    m_pTester->m_jobs.insert(jobId, job);

    m_pTester->handleConflictRepsonseConfirmed(jobDetail, response);
    EXPECT_EQ(job->m_isCoExisted, true);
}

TEST_F(TestDialogManager, testhandleConflictRepsonseConfirmed2)
{
    QMap<QString, QString> jobDetail;
    jobDetail.insert("jobId", "10000");
    QMap<QString, QVariant> response;
    response.insert("code", QVariant(1));

    QString jobId = "10000";
    DUrl url("file:///home1");
    QSharedPointer<FileJob> job(new FileJob(FileJob::Delete));
    job->setJobId(jobId);
    job->setProperty("pathlist", QVariant(url.toLocalFile()));
    m_pTester->m_jobs.insert(jobId, job);

    m_pTester->handleConflictRepsonseConfirmed(jobDetail, response);
    EXPECT_EQ(job->m_isReplaced, true);
}

TEST_F(TestDialogManager, testhandleConflictRepsonseConfirmed3)
{
    QMap<QString, QString> jobDetail;
    jobDetail.insert("jobId", "10000");
    QMap<QString, QVariant> response;
    response.insert("code", QVariant(2));

    QString jobId = "10000";
    DUrl url("file:///home1");
    QSharedPointer<FileJob> job(new FileJob(FileJob::Delete));
    job->setJobId(jobId);
    job->setProperty("pathlist", QVariant(url.toLocalFile()));
    m_pTester->m_jobs.insert(jobId, job);

    m_pTester->handleConflictRepsonseConfirmed(jobDetail, response);
    EXPECT_EQ(job->m_isSkip, true);
}

TEST_F(TestDialogManager, testhandleConflictRepsonseConfirmed4)
{
    QMap<QString, QString> jobDetail;
    jobDetail.insert("jobId", "10000");
    QMap<QString, QVariant> response;
    response.insert("code", QVariant(3));

    QString jobId = "10000";
    DUrl url("file:///home1");
    QSharedPointer<FileJob> job(new FileJob(FileJob::Delete));
    job->setJobId(jobId);
    job->setProperty("pathlist", QVariant(url.toLocalFile()));
    m_pTester->m_jobs.insert(jobId, job);

    m_pTester->handleConflictRepsonseConfirmed(jobDetail, response);
    EXPECT_EQ(job->m_isSkip, false);
}

TEST_F(TestDialogManager, testShowMessageDialog)
{
    DialogManager::messageType messageLevel(DialogManager::msgInfo);
    QString title("test title");

    int(*stub_exec)() = []()->int{
        return 1;
    };
    stub_ext::StubExt stub;
    stub.set(VADDR(QDialog, exec), stub_exec);

    int code = m_pTester->showMessageDialog(messageLevel, title);
    EXPECT_EQ(code, 1);
}

TEST_F(TestDialogManager, testShowMessageDialog2)
{
    DialogManager::messageType messageLevel(DialogManager::msgWarn);
    QString title("test title");

    int(*stub_exec)() = []()->int{
        return 1;
    };
    stub_ext::StubExt stub;
    stub.set(VADDR(QDialog, exec), stub_exec);

    int code = m_pTester->showMessageDialog(messageLevel, title);
    EXPECT_EQ(code, 1);
}

TEST_F(TestDialogManager, testShowMessageDialog3)
{
    DialogManager::messageType messageLevel(DialogManager::msgErr);
    QString title("test title");

    int(*stub_exec)() = []()->int{
        return 1;
    };
    stub_ext::StubExt stub;
    stub.set(VADDR(QDialog, exec), stub_exec);

    int code = m_pTester->showMessageDialog(messageLevel, title);
    EXPECT_EQ(code, 1);
}
#ifndef __arm__
TEST_F(TestDialogManager, testShowBluetoothTransferDlg)
{
    DUrlList files;
    files.push_back(DUrl("file:///home"));

    int(*stub_exec)() = []()->int{
        return 1;
    };
    stub_ext::StubExt stub;
    stub.set(VADDR(QDialog, exec), stub_exec);
    stub.set_lamda(ADDR(QWidget, show), []{});
    EXPECT_NO_FATAL_FAILURE(m_pTester->showBluetoothTransferDlg(files));
}


TEST_F(TestDialogManager, testShowBluetoothTransferDlg2)
{
    DUrlList files;
    files << DUrl("file:///home")
          << DUrl("search:///home")
          << DUrl("burn:///home")
          << DUrl("tag:///home");

    bool(*stub_canSendFiles)() = []()->bool{
        return true;
    };
    Stub stu;
    stu.set(ADDR(BluetoothTransDialog, canSendFiles), stub_canSendFiles);

    int(*stub2_exec)() = []()->int{
        return 1;
    };
    stub_ext::StubExt stub2;
    stub2.set(VADDR(QDialog, exec), stub2_exec);
    stub2.set_lamda(ADDR(QWidget, show), []{});

    EXPECT_NO_FATAL_FAILURE(m_pTester->showBluetoothTransferDlg(files));
}
#endif
TEST_F(TestDialogManager, testShowFormatDialog)
{
    QString devId("/dev/UTest");

    int(*stub_exec)() = []()->int{
        return 1;
    };
    stub_ext::StubExt stub;
    stub.set(VADDR(QDialog, exec), stub_exec);

    EXPECT_NO_FATAL_FAILURE(m_pTester->showFormatDialog(devId));
}

TEST_F(TestDialogManager, testShowFormatDialog2)
{
    QString devId("/test/UTest");

    int(*stub_exec)() = []()->int{
        return 1;
    };
    stub_ext::StubExt stub;
    stub.set(VADDR(QDialog, exec), stub_exec);

    EXPECT_NO_FATAL_FAILURE(m_pTester->showFormatDialog(devId));
}

TEST_F(TestDialogManager, testShowFormatDialog3)
{
    QString devId("/dev/UTest");

    bool(*stub_optical)() = []()->bool{
        return false;
    };
    Stub stu;
    stu.set(ADDR(DDiskDevice, optical), stub_optical);

    bool(*stu_removable)() = []()->bool{
        return true;
    };
    Stub stu2;
    stu2.set(ADDR(DDiskDevice, removable), stu_removable);

    int(*stub_exec)() = []()->int{
        return 1;
    };
    stub_ext::StubExt stub;
    stub.set(VADDR(QDialog, exec), stub_exec);

    EXPECT_NO_FATAL_FAILURE(m_pTester->showFormatDialog(devId));
}

TEST_F(TestDialogManager, testDUrlListCompare)
{
    DUrlList urls;
    bool b = m_pTester->DUrlListCompare(urls);
    EXPECT_EQ(b, true);
}

TEST_F(TestDialogManager, testDUrlListCompare2)
{
    DUrlList urls;
    urls << DUrl("file:///home");
    bool b = m_pTester->DUrlListCompare(urls);
    EXPECT_EQ(b, false);
}

TEST_F(TestDialogManager, testDUrlListCompare3)
{
    DUrlList urls;
    urls << DUrl("file:///home");
    m_pTester->m_urlList.push_back(DUrl("file:///home"));
    bool b = m_pTester->DUrlListCompare(urls);
    EXPECT_EQ(b, true);
}

TEST_F(TestDialogManager, testDUrlListCompare4)
{
    DUrlList urls;
    urls << DUrl("file:///home");
    m_pTester->m_urlList.push_back(DUrl("file:///home1"));
    bool b = m_pTester->DUrlListCompare(urls);
    EXPECT_EQ(b, false);
}

TEST_F(TestDialogManager, testShowNormalDeleteConfirmDialog)
{
    DUrlList urls;
    urls << DUrl("");
    DFMUrlListBaseEvent event(nullptr, urls);

    int(*stub_exec)() = []()->int{
        return 0;
    };
    stub_ext::StubExt stu;
    stu.set(VADDR(Dtk::Widget::DDialog, exec), stub_exec);

    int result = m_pTester->showNormalDeleteConfirmDialog(event);
    EXPECT_TRUE(result == 0);
}
