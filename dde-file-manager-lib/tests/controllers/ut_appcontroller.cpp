#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include <controllers/appcontroller.h>
#include <QProcess>
#include <QStandardPaths>
#include <QDateTime>
#include <QUrl>
#include <QFile>
#include <QtConcurrent>

#include "tests/testhelper.h"

using namespace testing;
namespace  {
    class AppControllerTest : public Test {

    protected:
        QString tempFilePath;
        AppController *controller;
        DUrl url;
    protected:
        void SetUp()
        {
            controller = AppController::instance();
            QString fileName = QString::number(QDateTime::currentDateTime().toMSecsSinceEpoch());
            tempFilePath =  QStandardPaths::standardLocations(QStandardPaths::TempLocation).first() + "/" + fileName;
            QProcess::execute("touch " + tempFilePath);
        }
        void TearDown()
        {
            QProcess::execute("rm -f " + tempFilePath);
            QProcess::execute("killall dde-file-manager");
        }
    };
}

TEST_F(AppControllerTest,start_registerUrlHandle){
    controller->registerUrlHandle();
}

TEST_F(AppControllerTest,start_manager_interface){
    StartManagerInterface *interface = controller->startManagerInterface();
    EXPECT_NE(interface, nullptr);
}


TEST_F(AppControllerTest,create_file_no_target_file){
    QString fileName = QString::number(QDateTime::currentDateTime().toMSecsSinceEpoch());
    QString filePath = AppController::instance()->createFile(tempFilePath + 123, "/tmp", fileName,0);
    EXPECT_EQ(filePath, "");
}


TEST_F(AppControllerTest,can_create_file){
    QString fileName = QString::number(QDateTime::currentDateTime().toMSecsSinceEpoch() + 1);
    QString filePath = AppController::instance()->createFile(tempFilePath, "/tmp",fileName,0);
    QUrl url = QUrl::fromLocalFile(filePath);
    QFile file(url.toLocalFile());
    EXPECT_EQ(true, file.exists());
    if (file.exists()) {
        file.remove();
    }
}

TEST_F(AppControllerTest,start_openAction){
    url.setScheme(FILE_SCHEME);
    url.setPath("~/Music");
    TestHelper::runInLoop([=]{
        AppController::instance()->actionOpen(dMakeEventPointer<DFMUrlListBaseEvent>(nullptr,DUrlList()));
    },200);
    TestHelper::runInLoop([=]{
        AppController::instance()->actionOpen(dMakeEventPointer<DFMUrlListBaseEvent>(nullptr, DUrlList() << url));
    },200);
    TestHelper::runInLoop([=]{
        DUrl temp;
        temp.setScheme(FILE_SCHEME);
        temp.setPath("~/Videos");
        AppController::instance()->actionOpen(dMakeEventPointer<DFMUrlListBaseEvent>(nullptr, DUrlList() << url << temp));
    },200);
}

TEST_F(AppControllerTest,start_actionOpenDisk){
    url.setScheme(FILE_SCHEME);
    url.setPath("~/Music");
    TestHelper::runInLoop([=]{
        AppController::instance()->actionOpenDisk(dMakeEventPointer<DFMUrlBaseEvent>(nullptr, url));
    },200);

    TestHelper::runInLoop([=]{
        AppController::instance()->actionOpenDisk(dMakeEventPointer<DFMUrlBaseEvent>(nullptr, DUrl()));
    },200);
}

TEST_F(AppControllerTest,start_actionOpenInNewWindow){
    url.setScheme(FILE_SCHEME);
    url.setPath("~/Music");
    TestHelper::runInLoop([=]{
        AppController::instance()->actionOpenInNewWindow(dMakeEventPointer<DFMUrlListBaseEvent>(nullptr,DUrlList()));
    },200);
    TestHelper::runInLoop([=]{
        AppController::instance()->actionOpenInNewWindow(dMakeEventPointer<DFMUrlListBaseEvent>(nullptr, DUrlList() << url));
    },200);
    TestHelper::runInLoop([=]{
        DUrl temp;
        temp.setScheme(FILE_SCHEME);
        temp.setPath("~/Videos");
        AppController::instance()->actionOpenInNewWindow(dMakeEventPointer<DFMUrlListBaseEvent>(nullptr, DUrlList() << url << temp));
    },200);

    TestHelper::runInLoop([=]{
        AppController::instance()->actionOpenInNewTab(dMakeEventPointer<DFMUrlBaseEvent>(nullptr,  url));
    },200);
}

TEST_F(AppControllerTest,start_actionOpenDiskInNewWindow){
    url.setScheme(FILE_SCHEME);
    url.setPath("~/Videos");
    TestHelper::runInLoop([=]{
        AppController::instance()->actionOpenDiskInNewWindow(dMakeEventPointer<DFMUrlBaseEvent>(nullptr, url));
    },200);

    TestHelper::runInLoop([=]{
        AppController::instance()->actionOpenDiskInNewWindow(dMakeEventPointer<DFMUrlBaseEvent>(nullptr, DUrl()));
    },200);
    TestHelper::runInLoop([=]{
        AppController::instance()->actionOpenDiskInNewTab(dMakeEventPointer<DFMUrlBaseEvent>(nullptr, url));
    },200);

    TestHelper::runInLoop([=]{
        AppController::instance()->actionOpenDiskInNewTab(dMakeEventPointer<DFMUrlBaseEvent>(nullptr, DUrl()));
    },200);
}

TEST_F(AppControllerTest,start_actionOpenAsAdmin){
    url.setScheme(FILE_SCHEME);
    url.setPath("~/Videos");
    TestHelper::runInLoop([=]{
        AppController::instance()->actionOpenAsAdmin(dMakeEventPointer<DFMUrlBaseEvent>(nullptr, url));
    },200);
}

TEST_F(AppControllerTest,start_actionOpenFilesWithCustom){
    url.setScheme(FILE_SCHEME);
    url.setPath("~/Videos");
    QString fileName = QString::number(QDateTime::currentDateTime().toMSecsSinceEpoch());
    DUrl temp;
    temp.setScheme(FILE_SCHEME);
    temp.setPath("./"+fileName+"_4.txt");
    QProcess::execute("touch " + temp.toLocalFile());
    TestHelper::runInLoop([=]{
        AppController::instance()->actionOpenFilesWithCustom(dMakeEventPointer<DFMUrlListBaseEvent>(nullptr, DUrlList() << url << temp));
    },200);
    TestHelper::runInLoop([=]{
        AppController::instance()->actionOpenFileLocation(dMakeEventPointer<DFMUrlListBaseEvent>(nullptr, DUrlList() << url << temp));
    },200);
    QProcess::execute("rm " + temp.toLocalFile());
}

TEST_F(AppControllerTest,start_actionCompress){
    url.setScheme(FILE_SCHEME);
    url.setPath("~/Videos");
    QString fileName = QString::number(QDateTime::currentDateTime().toMSecsSinceEpoch());
//    DUrl temp;
//    temp.setScheme(FILE_SCHEME);
//    temp.setPath("./"+fileName+".txt");
//    QProcess::execute("touch " + temp.toLocalFile());
//    TestHelper::runInLoop([=]{
//        AppController::instance()->actionCompress(dMakeEventPointer<DFMUrlListBaseEvent>(nullptr, DUrlList() << url));
//    },200);
//    QProcess::execute("rm " + temp.toLocalFile());
}
TEST_F(AppControllerTest,start_actionDecompress){
    url.setScheme(FILE_SCHEME);
    url.setPath("~/Videos");
//    QProcess::execute("tar -zcvf zkl_zcfcc.tar.gz "+url.toLocalFile());
//    url.setPath("./zkl_zcfcc.tar.gz");
//    controller->actionDecompress(dMakeEventPointer<DFMUrlListBaseEvent>(nullptr, DUrlList() << url));
//    controller->actionDecompressHere(dMakeEventPointer<DFMUrlListBaseEvent>(nullptr, DUrlList() << url));
//    url.setPath("./zkl_zcfcc.tar.gz");
//    QProcess::execute("rm "+url.toLocalFile());
//    url.setPath("./Videos");
//    QProcess::execute("rm -rf "+url.toLocalFile());
}

TEST_F(AppControllerTest,start_actionCut){
    url.setScheme(FILE_SCHEME);
    url.setPath("~/Videos");
    QString fileName = QString::number(QDateTime::currentDateTime().toMSecsSinceEpoch());
    DUrl temp;
    temp.setScheme(FILE_SCHEME);
    temp.setPath("./"+fileName+"_3.txt");
    QProcess::execute("touch " + temp.toLocalFile());
    TestHelper::runInLoop([=]{
        AppController::instance()->actionCut(dMakeEventPointer<DFMUrlListBaseEvent>(nullptr, DUrlList() << temp));
    },200);
    QProcess::execute("rm " + temp.toLocalFile());
}

TEST_F(AppControllerTest,start_actionCopy){
    url.setScheme(FILE_SCHEME);
    url.setPath("~/Videos");
    QString fileName = QString::number(QDateTime::currentDateTime().toMSecsSinceEpoch());
    TestHelper::runInLoop([=]{
        AppController::instance()->actionCopy(dMakeEventPointer<DFMUrlListBaseEvent>(nullptr, DUrlList() << url));
    },200);
}

TEST_F(AppControllerTest,start_actionPaste){
    url.setScheme(FILE_SCHEME);
    url.setPath("./vatest_app");
    QProcess::execute("touch " + url.toLocalFile());
    TestHelper::runInLoop([=]{
        AppController::instance()->actionPaste(dMakeEventPointer<DFMUrlBaseEvent>(nullptr, url));
    },200);
    QProcess::execute("rm -rf " + url.toLocalFile());
}

TEST_F(AppControllerTest,start_actionRename){
    url.setScheme(FILE_SCHEME);
    url.setPath("~/Videos");
    QString fileName = QString::number(QDateTime::currentDateTime().toMSecsSinceEpoch());
    DUrl temp;
    temp.setScheme(FILE_SCHEME);
    temp.setPath("./"+fileName+"_2.txt");
    QProcess::execute("touch " + temp.toLocalFile());
    TestHelper::runInLoop([=]{
        AppController::instance()->actionRename(dMakeEventPointer<DFMUrlListBaseEvent>(nullptr, DUrlList() << temp));
    },200);
    TestHelper::runInLoop([=]{
        AppController::instance()->actionRename(dMakeEventPointer<DFMUrlListBaseEvent>(nullptr, DUrlList() << temp << DUrl()));
    },200);
    QProcess::execute("rm " + temp.toLocalFile());
}
TEST_F(AppControllerTest,start_actionBookmarkandother){
    DUrl temp;
    temp.setScheme(FILE_SCHEME);
    QString fileName = QString::number(QDateTime::currentDateTime().toMSecsSinceEpoch());
    temp.setPath("./"+fileName+"_1.txt");
    QProcess::execute("touch " + temp.toLocalFile());
    DUrl temp1 = temp;
    temp1.setPath("./"+fileName+"_sys.txt");

    AppController::instance()->actionAddToBookMark(dMakeEventPointer<DFMUrlBaseEvent>(nullptr, temp));
    AppController::instance()->actionBookmarkRename(dMakeEventPointer<DFMUrlBaseEvent>(nullptr, temp));
    AppController::instance()->actionBookmarkRemove(dMakeEventPointer<DFMUrlBaseEvent>(nullptr, temp));
    AppController::instance()->actionDelete(dMakeEventPointer<DFMUrlListBaseEvent>(nullptr, DUrlList() << temp));

//        controller->actionCreateSymlink(dMakeEventPointer<DFMUrlBaseEvent>(nullptr, temp1));
    controller->actionSendToDesktop(dMakeEventPointer<DFMUrlListBaseEvent>(nullptr, DUrlList() << temp));
    controller->actionSendToBluetooth();
//    controller->actionCompleteDeletion(dMakeEventPointer<DFMUrlListBaseEvent>(nullptr, DUrlList() << temp));
    QProcess::execute("rm " + temp.toLocalFile());
    QProcess::execute("rm " + temp1.toLocalFile());
}

TEST_F(AppControllerTest,start_actionNewFolder){
    url.setScheme(FILE_SCHEME);
    url.setPath("./ooooooooooo");
    controller->actionNewFolder(dMakeEventPointer<DFMUrlBaseEvent>(nullptr, url));
    controller->actionSelectAll(29489851231423);
    controller->actionClearRecent(dMakeEventPointer<DFMMenuActionEvent>(nullptr,nullptr,DUrl(),DUrlList(),DFMGlobal::ClearRecent,QModelIndex()));
    QProcess::execute("cp "+ QDir::homePath() + "/.local/share/recently-used.xbel " + QDir::homePath() + "/.local/share/recently-used.xbel.bak");
    controller->actionClearRecent();
    QProcess::execute("mv "+ QDir::homePath() + "/.local/share/recently-used.xbel.bak " + QDir::homePath() + "/.local/share/recently-used.xbel");
    QProcess::execute("rm -rf " + url.toLocalFile());
//    controller->actionClearTrash(nullptr);
}

TEST_F(AppControllerTest,start_actionNewFile){
    url.setScheme(FILE_SCHEME);
    url.setPath("./ztt_test_app.txt");
    controller->actionNewText(dMakeEventPointer<DFMUrlBaseEvent>(nullptr,url));
    AppController::instance()->actionDelete(dMakeEventPointer<DFMUrlListBaseEvent>(nullptr, DUrlList() << url));
    controller->actionNewWord(dMakeEventPointer<DFMUrlBaseEvent>(nullptr,url));
    AppController::instance()->actionDelete(dMakeEventPointer<DFMUrlListBaseEvent>(nullptr, DUrlList() << url));
    controller->actionNewExcel(dMakeEventPointer<DFMUrlBaseEvent>(nullptr,url));
    AppController::instance()->actionDelete(dMakeEventPointer<DFMUrlListBaseEvent>(nullptr, DUrlList() << url));
    controller->actionNewPowerpoint(dMakeEventPointer<DFMUrlBaseEvent>(nullptr,url));
    AppController::instance()->actionDelete(dMakeEventPointer<DFMUrlListBaseEvent>(nullptr, DUrlList() << url));
//    controller->actionMount(dMakeEventPointer<DFMUrlBaseEvent>(nullptr,url));
    controller->actionRestore(dMakeEventPointer<DFMUrlListBaseEvent>(nullptr,DUrlList() << url));
    controller->actionRestoreAll(dMakeEventPointer<DFMUrlBaseEvent>(nullptr,url));
}

TEST_F(AppControllerTest, start_actionOpenInTerminal){
    url.setScheme(FILE_SCHEME);
    url.setPath("./");
    controller->actionOpenInTerminal(dMakeEventPointer<DFMUrlListBaseEvent>(nullptr,DUrlList() << url));
}

TEST_F(AppControllerTest, start_actionProperty){
    url.setScheme(FILE_SCHEME);
    url.setPath("./");
    controller->actionProperty(dMakeEventPointer<DFMUrlListBaseEvent>(nullptr,DUrlList() << url));
}

TEST_F(AppControllerTest, start_actionNewWindow){
    url.setScheme(FILE_SCHEME);
    url.setPath("~/Videos");
    controller->actionNewWindow(dMakeEventPointer<DFMUrlListBaseEvent>(nullptr,DUrlList() << url));
}

TEST_F(AppControllerTest, start_actionExit){
    controller->actionExit(53561231655);
}

TEST_F(AppControllerTest, start_actionSetAsWallpaper){
    url.setScheme(FILE_SCHEME);
    url.setPath("~/Pictures/Pictures/Wallpapers/abc-123.jpg");
    controller->actionSetAsWallpaper(dMakeEventPointer<DFMUrlBaseEvent>(nullptr,url));
}

TEST_F(AppControllerTest, start_actionShare){
    url.setScheme(FILE_SCHEME);
    url.setPath("~/ut_share_test");
    QProcess::execute("mkdir " + url.toLocalFile());
    controller->actionShare(dMakeEventPointer<DFMUrlListBaseEvent>(nullptr,DUrlList() << url));
}

TEST_F(AppControllerTest, start_actionUnShare){
    url.setScheme(FILE_SCHEME);
    url.setPath("~/ut_share_test");
    controller->actionUnShare(dMakeEventPointer<DFMUrlBaseEvent>(nullptr,url));
}

TEST_F(AppControllerTest, start_actionConnectToServer){
    controller->actionConnectToServer(static_cast<quint64>(QDateTime::currentDateTime().toMSecsSinceEpoch()));
}

TEST_F(AppControllerTest, start_actionSetUserSharePassword){
    controller->actionSetUserSharePassword(static_cast<quint64>(QDateTime::currentDateTime().toMSecsSinceEpoch()));
}

TEST_F(AppControllerTest, start_actionSettings){
    controller->actionSettings(static_cast<quint64>(QDateTime::currentDateTime().toMSecsSinceEpoch()));
}

TEST_F(AppControllerTest, start_actionFormatDevice){
    controller->actionFormatDevice(dMakeEventPointer<DFMUrlBaseEvent>(nullptr,url));
}

TEST_F(AppControllerTest, start_actionOpticalBlank){
    controller->actionFormatDevice(dMakeEventPointer<DFMUrlBaseEvent>(nullptr,url));
}

TEST_F(AppControllerTest, start_actionctrlL){
    controller->actionctrlL(static_cast<quint64>(QDateTime::currentDateTime().toMSecsSinceEpoch()));
}

TEST_F(AppControllerTest, start_actionctrlF){
    controller->actionctrlF(static_cast<quint64>(QDateTime::currentDateTime().toMSecsSinceEpoch()));
}

TEST_F(AppControllerTest, start_actionExitCurrentWindow){
    controller->actionExitCurrentWindow(static_cast<quint64>(QDateTime::currentDateTime().toMSecsSinceEpoch()));
}

TEST_F(AppControllerTest, start_actionShowHotkeyHelp){
    controller->actionShowHotkeyHelp(static_cast<quint64>(QDateTime::currentDateTime().toMSecsSinceEpoch()));
}

TEST_F(AppControllerTest, start_actionBack){
    controller->actionBack(static_cast<quint64>(QDateTime::currentDateTime().toMSecsSinceEpoch()));
}

TEST_F(AppControllerTest, start_actionForward){
    controller->actionForward(static_cast<quint64>(QDateTime::currentDateTime().toMSecsSinceEpoch()));
}

TEST_F(AppControllerTest, start_actionForgetPassword){
    controller->actionForgetPassword(dMakeEventPointer<DFMUrlBaseEvent>(nullptr,url));
}

TEST_F(AppControllerTest, start_actionOpenFileByApp){
    controller->actionOpenFileByApp();
}

TEST_F(AppControllerTest, start_actionSendToRemovableDisk){
    controller->actionSendToRemovableDisk();
}

TEST_F(AppControllerTest, start_actionStageFileForBurning){
    controller->actionStageFileForBurning();
}

TEST_F(AppControllerTest, start_actionGetTagsThroughFiles){
    QStringList list = controller->actionGetTagsThroughFiles(dMakeEventPointer<DFMGetTagsThroughFilesEvent>(nullptr,DUrlList() << url));
    EXPECT_TRUE(list.isEmpty());
    controller->actionRemoveTagsOfFile(dMakeEventPointer<DFMRemoveTagsOfFileEvent>(nullptr,url, list));
    controller->actionChangeTagColor(dMakeEventPointer<DFMChangeTagColorEvent>(nullptr,Qt::red, url));
    controller->showTagEdit(QRect(0,0,0,0),QPoint(0,0),DUrlList() << url);
}


