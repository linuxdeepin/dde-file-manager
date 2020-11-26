#include "dialogs/dialogmanager.h"
#include "dfmevent.h"
#include "fileoperations/filejob.h"

#include <gtest/gtest.h>
#include <QWidget>

namespace  {
    class TestDialogManager : public testing::Test
    {
    public:
        void SetUp() override
        {
            m_pTester = new DialogManager();
            std::cout << "start TestDialogManager";
        }
        void TearDown() override
        {
            delete m_pTester;
            m_pTester = nullptr;
            std::cout << "end TestDialogManager";
        }
    public:
        DialogManager *m_pTester;
    };
}

TEST_F(TestDialogManager, testInit)
{
//    int a = 0;
}

TEST_F(TestDialogManager, testGetPropertyPos)
{
    int dialogWidth = 800;
    int dialogHeight = 500;
    QPoint result = m_pTester->getPropertyPos(dialogWidth, dialogHeight);
    EXPECT_GT(result.x(), 0);
    EXPECT_GT(result.y(), 0);
}

TEST_F(TestDialogManager, testGetPerportyPos)
{
    int dialogWidth = 800;
    int dialogHeight = 500;
    int count = 10;
    int index = 5;
    QPoint result = m_pTester->getPerportyPos(dialogWidth, dialogHeight, count, index);
    EXPECT_GT(result.x(), 0);
    EXPECT_GT(result.y(), 0);
}

TEST_F(TestDialogManager, testIsTaskDialogEmpty)
{
    EXPECT_TRUE(m_pTester->isTaskDialogEmpty());
}

TEST_F(TestDialogManager, testTaskDialog)
{
    DTaskDialog *pResult = m_pTester->taskDialog();
    EXPECT_FALSE(pResult == nullptr);
}

TEST_F(TestDialogManager, testHandleConflictRepsonseConfirmed0)
{
    FileJob job(FileJob::OpticalCheck);
    job.setJobId("10000");
    m_pTester->addJob(&job);

    QMap<QString, QString> jobDetail;
    jobDetail.insert("jobId", "10000");
    QMap<QString, QVariant> response;
    response.insert("applyToAll", QVariant(false));
    response.insert("code", QVariant(0));
    m_pTester->handleConflictRepsonseConfirmed(jobDetail, response);
}

TEST_F(TestDialogManager, testHandleConflictRepsonseConfirmed1)
{
    FileJob job(FileJob::OpticalImageBurn);
    job.setJobId("10000");
    m_pTester->addJob(&job);

    QMap<QString, QString> jobDetail;
    jobDetail.insert("jobId", "10000");
    QMap<QString, QVariant> response;
    response.insert("applyToAll", QVariant(false));
    response.insert("code", QVariant(1));
    m_pTester->handleConflictRepsonseConfirmed(jobDetail, response);
}

TEST_F(TestDialogManager, testHandleConflictRepsonseConfirmed2)
{
    FileJob job(FileJob::OpticalBlank);
    job.setJobId("10000");
    m_pTester->addJob(&job);

    QMap<QString, QString> jobDetail;
    jobDetail.insert("jobId", "10000");
    QMap<QString, QVariant> response;
    response.insert("applyToAll", QVariant(false));
    response.insert("code", QVariant(2));
    m_pTester->handleConflictRepsonseConfirmed(jobDetail, response);
}

TEST_F(TestDialogManager, testHandleConflictRepsonseConfirmed3)
{
    FileJob job(FileJob::OpticalBurn);
    job.setJobId("10000");
    m_pTester->addJob(&job);

    QMap<QString, QString> jobDetail;
    jobDetail.insert("jobId", "10000");
    QMap<QString, QVariant> response;
    response.insert("applyToAll", QVariant(false));
    response.insert("code", QVariant(3));
    m_pTester->handleConflictRepsonseConfirmed(jobDetail, response);
}

TEST_F(TestDialogManager, testRemoveJob)
{
    FileJob job(FileJob::Restore);
    job.setJobId("10000");
    m_pTester->addJob(&job);

    m_pTester->removeJob("10000", true);
}

TEST_F(TestDialogManager, testGetJobIdByUrl)
{
    QString jobId = "10000";
    DUrl url("file://home");

    FileJob job(FileJob::Delete);
    job.setJobId(jobId);
    job.setProperty("pathlist", QVariant(url.toLocalFile()));
    m_pTester->addJob(&job);
    QString result = m_pTester->getJobIdByUrl(url);

    EXPECT_STREQ(jobId.toStdString().c_str(), result.toStdString().c_str());
}

TEST_F(TestDialogManager, testRemoveAllJobs)
{
    FileJob job(FileJob::Trash);
    job.setJobId("10000");
    m_pTester->addJob(&job);

    m_pTester->removeAllJobs();
}

TEST_F(TestDialogManager, testUpdateJob)
{
//    FileJob job(FileJob::Move);
//    job.setJobId("10000");
//    m_pTester->addJob(&job);

    m_pTester->updateJob();
}

TEST_F(TestDialogManager, testStartAndStopUpdateJobTimer)
{
    m_pTester->startUpdateJobTimer();

    m_pTester->stopUpdateJobTimer();
}

TEST_F(TestDialogManager, testAbortJob)
{
    FileJob job(FileJob::Copy);
    job.setJobId("10000");
    m_pTester->addJob(&job);

    QMap<QString, QString> jobDetail;
    jobDetail.insert("job1", "10000");

    m_pTester->abortJob(jobDetail);
}

TEST_F(TestDialogManager, testAbortJobByDestinationUrl)
{
    QString jobId = "10000";
    DUrl url("file://home");

    FileJob job(FileJob::Delete);
    job.setJobId(jobId);
    job.setProperty("pathlist", QVariant(url.toLocalFile()));
    m_pTester->addJob(&job);

    m_pTester->abortJobByDestinationUrl(url);
}

//TEST_F(TestDialogManager, testShowCopyMoveToSelfDialog)
//{
//    // TODO 有模态弹框 以后处理
////    QMap<QString, QString> jobDetail;
////    jobDetail.insert("job1", "10000");
////    m_pTester->showCopyMoveToSelfDialog(jobDetail);
//}

TEST_F(TestDialogManager, testShowUrlWrongDialog)
{
    DUrl url("file://home");
    m_pTester->showUrlWrongDialog(url);
}

//TEST_F(TestDialogManager, testShowRunExcutableScriptDialog)
//{
//    // TODO 有模态弹框 以后处理
////    DUrl url("file://home");
////    quint64 winId = 12345678;
////    m_pTester->showRunExcutableScriptDialog(url, winId);
//}

//TEST_F(TestDialogManager, testShowAskIfAddExcutableFlagAndRunDialog)
//{
//    // TODO 有模态弹框 以后处理
////    DUrl url("file://home");
////    quint64 winId = 12345678;
////    m_pTester->showAskIfAddExcutableFlagAndRunDialog(url, winId);
//}

//TEST_F(TestDialogManager, testShowRenameNameSameErrorDialog)
//{
//    // TODO 有模态弹框 以后处理
////    QString name = "unit_test_dialog";
////    DFMEvent event;
////    m_pTester->showRenameNameSameErrorDialog(name, event);
//}

////! 测试文件命名为“..”时，弹出提示框
//TEST_F(TestDialogManager, testShowRenameNameDotDotErrorDialog)
//{
//    // TODO 有模态弹框 以后处理
////    DFMEvent event;
////    int result = m_pTester->showRenameNameDotDotErrorDialog(event);
////    EXPECT_TRUE(result == 0 || result == 1);
//}

//TEST_F(TestDialogManager, testShowOpticalBlankConfirmationDialog)
//{
//    // TODO 有模态弹框 以后处理
////    DUrl url("file://home");
////    DFMUrlBaseEvent event(0, url);
////    m_pTester->showOpticalBlankConfirmationDialog(event);
//}

//TEST_F(TestDialogManager, testShowOpticalImageOpSelectionDialog)
//{
//    // TODO 有模态弹框 以后处理
////    DUrl url("file://home");
////    DFMUrlBaseEvent event(0, url);
////    m_pTester->showOpticalImageOpSelectionDialog(event);
//}

//TEST_F(TestDialogManager, testShowOpticalJobFailureDialog)
//{
//    // TODO 有模态弹框 以后处理
////    int type = FileJob::OpticalBlank;
////    QString err("unit_test_info");
////    QStringList details("");
////    m_pTester->showOpticalJobFailureDialog(type, err, details);
//}

//TEST_F(TestDialogManager, testShowOpticalJobCompletionDialog)
//{
//    // TODO 有模态弹框 以后处理
////    QString msg("unit_test_name");
////    QString icon("dfm_vault");
////    m_pTester->showOpticalJobCompletionDialog(msg, icon);
//}

//TEST_F(TestDialogManager, testShowDeleteFilesClearTrashDialog)
//{
//    // TODO 有模态弹框 以后处理
////    DUrlList list;
////    list.push_back(DUrl("file://home"));
////    DFMUrlListBaseEvent event(0, list);
////    m_pTester->showDeleteFilesClearTrashDialog(event);
//}

//TEST_F(TestDialogManager, testShowRemoveBookMarkDialog)
//{

//}

//TEST_F(TestDialogManager, testShowOpenWithDialog)
//{

//}

//TEST_F(TestDialogManager, testShowOpenFilesWithDialog)
//{

//}

//TEST_F(TestDialogManager, testShowPropertyDialog)
//{

//}

//TEST_F(TestDialogManager, testShowShareOptionsInPropertyDialog)
//{

//}

//TEST_F(TestDialogManager, testShowTrashPropertyDialog)
//{

//}

//TEST_F(TestDialogManager, testShowComputerPropertyDialog)
//{

//}

//TEST_F(TestDialogManager, testShowDevicePropertyDialog)
//{

//}

//TEST_F(TestDialogManager, testShowDiskErrorDialog)
//{

//}

//TEST_F(TestDialogManager, testShowBreakSymlinkDialog)
//{

//}

//TEST_F(TestDialogManager, testShowConnectToServerDialog)
//{

//}

//TEST_F(TestDialogManager, testShowUserSharePasswordSettingDialog)
//{

//}

//TEST_F(TestDialogManager, testShowGlobalSettingsDialog)
//{

//}

//TEST_F(TestDialogManager, testShowDiskSpaceOutOfUsedDialogLater)
//{

//}

//TEST_F(TestDialogManager, testShowDiskSpaceOutOfUsedDialog)
//{

//}

//TEST_F(TestDialogManager, testShowFailToCreateSymlinkDialog)
//{

//}

//TEST_F(TestDialogManager, testShowMoveToTrashConflictDialog)
//{

//}

//TEST_F(TestDialogManager, testShowDeleteSystemPathWarnDialog)
//{

//}

//TEST_F(TestDialogManager, testShowFilePreviewDialog)
//{

//}

//TEST_F(TestDialogManager, testShowRestoreFailedDialog)
//{

//}

//TEST_F(TestDialogManager, testShowRestoreFailedPerssionDialog)
//{

//}

//TEST_F(TestDialogManager, testShowMultiFilesRenameDialog)
//{

//}

//TEST_F(TestDialogManager, testShowAddUserShareFailedDialog)
//{

//}

//TEST_F(TestDialogManager, testShowNoPermissionDialog)
//{

//}

//TEST_F(TestDialogManager, testShowNtfsWarningDialog)
//{

//}

//TEST_F(TestDialogManager, testShowErrorDialog)
//{

//}

TEST_F(TestDialogManager, testRemovePropertyDialog)
{
    DUrl url("file://home");
    m_pTester->removePropertyDialog(url);
}

TEST_F(TestDialogManager, testCloseAllPropertyDialog)
{
    m_pTester->closeAllPropertyDialog();
}

TEST_F(TestDialogManager, testUpdateCloseIndicator)
{
    m_pTester->updateCloseIndicator();
}

TEST_F(TestDialogManager, testRaiseAllPropertyDialog)
{
    m_pTester->raiseAllPropertyDialog();
}

TEST_F(TestDialogManager, testHandleFocusChanged)
{
    QWidget old;
    QWidget now;
    m_pTester->handleFocusChanged(&old, &now);
}

TEST_F(TestDialogManager, testShowTaskProgressDlgOnActive)
{
//    m_pTester->showTaskProgressDlgOnActive();
}

//TEST_F(TestDialogManager, testShowUnableToLocateDir)
//{

//}

TEST_F(TestDialogManager, testRefreshPropertyDialogs)
{
    DUrl oldUrl("file://home");
    DUrl newUrl("file://home1");
    m_pTester->refreshPropertyDialogs(oldUrl, newUrl);
}

TEST_F(TestDialogManager, testhandleConflictRepsonseConfirmed)
{
    QMap<QString, QString> jobDetail;
    QMap<QString, QVariant> response;
    m_pTester->handleConflictRepsonseConfirmed(jobDetail, response);
}

//TEST_F(TestDialogManager, testShowMessageDialog)
//{

//}

// 有模态弹框,暂不处理
//TEST_F(TestDialogManager, testShowBluetoothTransferDlg)
//{
//    DUrlList files;
//    files.push_back(DUrl("file://home"));
//    m_pTester->showBluetoothTransferDlg(files);
//}
