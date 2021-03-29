#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include <DWaterProgress>
#include <QTest>
#include <QPlainTextEdit>
#include <QPushButton>
#include <DWaterProgress>
#include <qprocess.h>
#include <interfaces/dfmstandardpaths.h>
#include "stub.h"

#define private public
#include "views/dfmvaultremoveprogressview.h"

DWIDGET_USE_NAMESPACE
namespace  {
    class TestDFMVaultRemoveProgressView : public testing::Test
    {
    public:
        QSharedPointer<DFMVaultRemoveProgressView> m_view;

        virtual void SetUp() override
        {
            m_view = QSharedPointer<DFMVaultRemoveProgressView>(new DFMVaultRemoveProgressView());
            m_view->show();
            std::cout << "start TestDFMVaultRemoveProgressView" << std::endl;
        }

        virtual void TearDown() override
        {
            m_view->close();
            std::cout << "end TestDFMVaultRemoveProgressView" << std::endl;
        }
    };
}


TEST_F(TestDFMVaultRemoveProgressView, tst_clear)
{
    m_view->clear();
    EXPECT_EQ(static_cast<int>(0), m_view->m_vaultRmProgressBar->value());
    EXPECT_EQ(static_cast<int>(0), m_view->m_iFiles);
    EXPECT_EQ(static_cast<int>(0), m_view->m_iRmFiles);
    EXPECT_EQ(static_cast<int>(0), m_view->m_iRmDir);
}

TEST_F(TestDFMVaultRemoveProgressView, tst_statisticsFiles)
{
    QString homePath = QDir::currentPath();//DFMStandardPaths::location(DFMStandardPaths::HomePath);
    QString path = homePath + "/TestDFMVaultRemoveProgressView/";
    QProcess::execute("mkdir " + path);

    QProcess::execute("touch " + path + "test.txt");
    QProcess::execute("mkdir " + path + "child");
    QProcess::execute("touch " + path + "child/" "test.txt");

    m_view->statisticsFiles(path);

    EXPECT_EQ(3, m_view->m_iFiles);

    QProcess::execute("rm -r " + path);
}


TEST_F(TestDFMVaultRemoveProgressView, tst_removeFile)
{
    QString homePath =QDir::currentPath();
    QString path = homePath + "/TestDFMVaultRemoveProgressView/";
    QProcess::execute("mkdir " + path);

    QProcess::execute("touch " + path + "test.txt");
    QProcess::execute("mkdir " + path + "child");
    QProcess::execute("touch " + path + "child/" "test.txt");

    m_view->statisticsFiles(path);
    m_view->removeFileInDir(path + "child");

    EXPECT_EQ(1, m_view->m_iRmFiles);

    QProcess::execute("rm -r " + path);
}

TEST_F(TestDFMVaultRemoveProgressView, tst_onFileRemove)
{
    m_view->onFileRemove(10);
    EXPECT_EQ(10, m_view->m_vaultRmProgressBar->value());
}

TEST_F(TestDFMVaultRemoveProgressView, tst_removeVault)
{
    bool (*st_statisticsFiles)(const QString &) = [](const QString &)->bool {
        // do nothing.
        return true;
    };
    Stub stub;
    stub.set(ADDR(DFMVaultRemoveProgressView, statisticsFiles), st_statisticsFiles);

    void (*st_removeFileInDir)(const QString &) = [](const QString &){
        // do nothing.
    };
    stub.set(ADDR(DFMVaultRemoveProgressView, removeFileInDir), st_removeFileInDir);

    bool (*st_rmdir)(const QString &) = [](const QString &)->bool {
        // do nothing.
        return true;
    };
    stub.set(ADDR(QDir, rmdir), st_rmdir);

    void (*st_removeFinished)(bool) = [](bool) {
        // do nothing.
    };
    stub.set(ADDR(DFMVaultRemoveProgressView, removeFinished), st_removeFinished);

    void (*st_start)() = [](){
        // do nothing.
    };
    stub.set(ADDR(DWaterProgress, start), st_start);

    // recored: these code will cause broken occasionally
    // but has been solved.
    EXPECT_NO_FATAL_FAILURE(m_view->removeVault("", ""));
}
