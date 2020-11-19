#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include <controllers/appcontroller.h>
#include <QProcess>
#include <QStandardPaths>
#include <QDateTime>
#include <QUrl>
#include <QFile>

using namespace testing;
namespace  {
    class AppControllerTest : public Test {

    protected:
        QString tempFilePath;
        AppController *controller;
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

