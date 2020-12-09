#include "dialogs/dialogmanager.h"
#include "dfmevent.h"
#include "fileoperations/filejob.h"
#include "gvfs/qdiskinfo.h"

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
    DUrl url("file:///home");

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
    DUrl url("file:///home");

    FileJob job(FileJob::Delete);
    job.setJobId(jobId);
    job.setProperty("pathlist", QVariant(url.toLocalFile()));
    m_pTester->addJob(&job);

    m_pTester->abortJobByDestinationUrl(url);
}

TEST_F(TestDialogManager, testShowCopyMoveToSelfDialog)
{
    QMap<QString, QString> jobDetail;
    jobDetail.insert("job1", "10000");
    m_pTester->showCopyMoveToSelfDialog(jobDetail);
}

TEST_F(TestDialogManager, testShowUrlWrongDialog)
{
    DUrl url("file:///home");
    m_pTester->showUrlWrongDialog(url);
}

TEST_F(TestDialogManager, testShowRunExcutableScriptDialog)
{
    DUrl url("file:///home");
    quint64 winId = 12345678;
    m_pTester->showRunExcutableScriptDialog(url, winId);
}

TEST_F(TestDialogManager, testShowRunExcutableFileDialog)
{
    DUrl url("file:///home");
    quint64 winId = 12345678;
    m_pTester->showRunExcutableFileDialog(url, winId);
}

TEST_F(TestDialogManager, testShowAskIfAddExcutableFlagAndRunDialog)
{
    DUrl url("file:///home");
    quint64 winId = 12345678;
    m_pTester->showAskIfAddExcutableFlagAndRunDialog(url, winId);
}

TEST_F(TestDialogManager, testShowRenameNameSameErrorDialog)
{
    QString name = "unit_test_dialog";
    DFMEvent event;
    m_pTester->showRenameNameSameErrorDialog(name, event);
}

// 测试文件命名为“..”时，弹出提示框
TEST_F(TestDialogManager, testShowRenameNameDotDotErrorDialog)
{
    DFMEvent event;
    int result = m_pTester->showRenameNameDotDotErrorDialog(event);
    EXPECT_TRUE(result == 0 || result == 1);
}

TEST_F(TestDialogManager, testShowOpticalBlankConfirmationDialog)
{
    DUrl url("file:///home");
    DFMUrlBaseEvent event(nullptr, url);
    m_pTester->showOpticalBlankConfirmationDialog(event);
}

TEST_F(TestDialogManager, testShowOpticalImageOpSelectionDialog)
{
    DUrl url("file:///home");
    DFMUrlBaseEvent event(nullptr, url);
    m_pTester->showOpticalImageOpSelectionDialog(event);
}

TEST_F(TestDialogManager, testShowOpticalJobFailureDialog)
{
    int type = FileJob::OpticalBlank;
    QString err("unit_test_info");
    QStringList details("");
    m_pTester->showOpticalJobFailureDialog(type, err, details);
}

TEST_F(TestDialogManager, testShowOpticalJobCompletionDialog)
{
    QString msg("unit_test_name");
    QString icon("dfm_vault");
    m_pTester->showOpticalJobCompletionDialog(msg, icon);
}

TEST_F(TestDialogManager, testShowDeleteFilesClearTrashDialog)
{
    DUrlList list;
    list.push_back(DUrl("file:///home"));
    DFMUrlListBaseEvent event(nullptr, list);
    m_pTester->showDeleteFilesClearTrashDialog(event);
}

TEST_F(TestDialogManager, testShowRemoveBookMarkDialog)
{
    DFMEvent event;
    m_pTester->showRemoveBookMarkDialog(event);
}

TEST_F(TestDialogManager, testShowOpenWithDialog)
{
    DFMEvent event;
    m_pTester->showOpenWithDialog(event);
}

TEST_F(TestDialogManager, testShowOpenFilesWithDialog)
{
    DFMEvent event;
    m_pTester->showOpenFilesWithDialog(event);
}

TEST_F(TestDialogManager, testShowPropertyDialog)
{
    DUrlList list;
    list.push_back(DUrl("file:///home"));
    DFMUrlListBaseEvent event(nullptr, list);
    m_pTester->showPropertyDialog(event);
}

TEST_F(TestDialogManager, testShowShareOptionsInPropertyDialog)
{
    DUrlList list;
    list.push_back(DUrl("file:///home"));
    DFMUrlListBaseEvent event(nullptr, list);
    m_pTester->showShareOptionsInPropertyDialog(event);
}

TEST_F(TestDialogManager, testShowTrashPropertyDialog)
{
    DFMEvent event;
    m_pTester->showTrashPropertyDialog(event);
}

TEST_F(TestDialogManager, testShowComputerPropertyDialog)
{
    m_pTester->showComputerPropertyDialog();
}

TEST_F(TestDialogManager, testShowDevicePropertyDialog)
{
    DFMEvent event;
    m_pTester->showDevicePropertyDialog(event);
}

TEST_F(TestDialogManager, testShowDiskErrorDialog)
{
    QString id = "123";
    QString errorText = "test error";
    m_pTester->showDiskErrorDialog(id, errorText);
}

TEST_F(TestDialogManager, testShowBreakSymlinkDialog)
{
    QString targetName = "test";
    DUrl linkfile("file:///file");
    m_pTester->showBreakSymlinkDialog(targetName, linkfile);
}

TEST_F(TestDialogManager, testShowConnectToServerDialog)
{
    quint64 winId(12345678);
    m_pTester->showConnectToServerDialog(winId);
}

TEST_F(TestDialogManager, testShowUserSharePasswordSettingDialog)
{
    quint64 winId(12345678);
    m_pTester->showUserSharePasswordSettingDialog(winId);
}

TEST_F(TestDialogManager, testShowGlobalSettingsDialog)
{
    quint64 winId(12345678);
    m_pTester->showGlobalSettingsDialog(winId);
}

TEST_F(TestDialogManager, testShowDiskSpaceOutOfUsedDialogLater)
{
    m_pTester->showDiskSpaceOutOfUsedDialogLater();
}

TEST_F(TestDialogManager, testShowDiskSpaceOutOfUsedDialog)
{
    m_pTester->showDiskSpaceOutOfUsedDialog();
}

TEST_F(TestDialogManager, testShow4gFat32Dialog)
{
    m_pTester->show4gFat32Dialog();
}

TEST_F(TestDialogManager, testShowFailToCreateSymlinkDialog)
{
    QString error("test error");
    m_pTester->showFailToCreateSymlinkDialog(error);
}

TEST_F(TestDialogManager, testShowMoveToTrashConflictDialog)
{
    DUrlList lst;
    m_pTester->showMoveToTrashConflictDialog(lst);
}

TEST_F(TestDialogManager, testShowDeleteSystemPathWarnDialog)
{
    quint64 winId(12345678);
    m_pTester->showDeleteSystemPathWarnDialog(winId);
}

TEST_F(TestDialogManager, testShowFilePreviewDialog)
{
    DUrlList selectUrls;
    selectUrls << DUrl("file:///testfile2");
    DUrlList entryUrls;
    entryUrls << DUrl("file:///testfile2");
    m_pTester->showFilePreviewDialog(selectUrls, entryUrls);
}

TEST_F(TestDialogManager, testShowRestoreFailedDialog)
{
    DUrlList lst;
    lst << DUrl("file:///testfile2");
    m_pTester->showRestoreFailedDialog(lst);
}

TEST_F(TestDialogManager, testShowRestoreFailedDialog2)
{
    DUrlList lst;
    lst << DUrl("file:///testfile2") << DUrl("file:///testfile1");
    m_pTester->showRestoreFailedDialog(lst);
}

TEST_F(TestDialogManager, testShowRestoreFailedPerssionDialog)
{
    QString srcPath("test srcPath");
    QString targetPath("test targetPath");
    m_pTester->showRestoreFailedPerssionDialog(srcPath, targetPath);
}

TEST_F(TestDialogManager, testShowRestoreFailedSourceNotExists)
{
    DUrlList lst;
    lst << DUrl("file:///testfile2");
    m_pTester->showRestoreFailedSourceNotExists(lst);
}

TEST_F(TestDialogManager, testShowRestoreFailedSourceNotExists2)
{
    DUrlList lst;
    lst << DUrl("file:///testfile2") << DUrl("file:///testfile1");
    m_pTester->showRestoreFailedSourceNotExists(lst);
}

TEST_F(TestDialogManager, testShowMultiFilesRenameDialog)
{
    QList<DUrl> selectedUrls;
    selectedUrls << DUrl("file:///home");
    m_pTester->showMultiFilesRenameDialog(selectedUrls);
}

TEST_F(TestDialogManager, testShowAddUserShareFailedDialog)
{
    QString sharePath("/home");
    m_pTester->showAddUserShareFailedDialog(sharePath);
}

TEST_F(TestDialogManager, testShowNoPermissionDialog)
{
    DUrlList list;
    list.push_back(DUrl("file:///home"));
    DFMUrlListBaseEvent event(nullptr, list);
    m_pTester->showNoPermissionDialog(event);
}

TEST_F(TestDialogManager, testShowNoPermissionDialog2)
{
    DUrlList list;
    list.push_back(DUrl("file:///home"));
    list.push_back(DUrl("file:///testfile1"));
    DFMUrlListBaseEvent event(nullptr, list);
    m_pTester->showNoPermissionDialog(event);
}

TEST_F(TestDialogManager, testShowNtfsWarningDialog)
{
    QDiskInfo diskInfo;
    m_pTester->showNtfsWarningDialog(diskInfo);
}

TEST_F(TestDialogManager, testShowErrorDialog)
{
    QString title("test title");
    QString message("test message");
    m_pTester->showErrorDialog(title, message);
}

TEST_F(TestDialogManager, testRemovePropertyDialog)
{
    DUrl url("file:///home");
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
    m_pTester->showTaskProgressDlgOnActive();
}

TEST_F(TestDialogManager, testShowUnableToLocateDir)
{
    QString dir("/testfile");
    m_pTester->showUnableToLocateDir(dir);
}

TEST_F(TestDialogManager, testRefreshPropertyDialogs)
{
    DUrl oldUrl("file:///home");
    DUrl newUrl("file:///home1");
    m_pTester->refreshPropertyDialogs(oldUrl, newUrl);
}

TEST_F(TestDialogManager, testhandleConflictRepsonseConfirmed)
{
    QMap<QString, QString> jobDetail;
    QMap<QString, QVariant> response;
    m_pTester->handleConflictRepsonseConfirmed(jobDetail, response);
}

TEST_F(TestDialogManager, testShowMessageDialog)
{
    DialogManager::messageType messageLevel(DialogManager::msgInfo);
    QString title("test title");
    m_pTester->showMessageDialog(messageLevel, title);
}

TEST_F(TestDialogManager, testShowMessageDialog2)
{
    DialogManager::messageType messageLevel(DialogManager::msgWarn);
    QString title("test title");
    m_pTester->showMessageDialog(messageLevel, title);
}

TEST_F(TestDialogManager, testShowMessageDialog3)
{
    DialogManager::messageType messageLevel(DialogManager::msgErr);
    QString title("test title");
    m_pTester->showMessageDialog(messageLevel, title);
}

TEST_F(TestDialogManager, testShowBluetoothTransferDlg)
{
//    DUrlList files;
//    files.push_back(DUrl("file:///home"));
//    m_pTester->showBluetoothTransferDlg(files);
}

TEST_F(TestDialogManager, testShowFormatDialog)
{
    QString devId("/dev/UTest");
    m_pTester->showFormatDialog(devId);
}

TEST_F(TestDialogManager, testDUrlListCompare)
{
    DUrlList urls;
    m_pTester->DUrlListCompare(urls);
}

TEST_F(TestDialogManager, testDUrlListCompare2)
{
    DUrlList urls;
    urls << DUrl("file:///home");
    m_pTester->DUrlListCompare(urls);
}
