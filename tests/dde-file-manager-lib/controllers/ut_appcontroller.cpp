#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include <QProcess>
#include <QStandardPaths>
#include <QDateTime>
#include <QUrl>
#include <QFile>
#include <QtConcurrent>
#include <QDebug>
#include <QWidget>
#include <QSharedPointer>
#include <QAction>
#include <QDialog>
#include <QVariant>
#include <ddiskdevice.h>
#include <QApplication>

#include <ddiskmanager.h>
#include <dblockdevice.h>
#include "deviceinfo/udisklistener.h"
#include "dabstractfilewatcher.h"
#include "ddialog.h"
#define private public
#define protected public
#include "views/dtagedit.h"
#include "tag/tagmanager.h"
#include "testhelper.h"
#include "stub.h"
#include "stubext.h"
#include "interfaces/dfmeventdispatcher.h"
#include "interfaces/dfmabstracteventhandler.h"
#include "views/windowmanager.h"
#include "views/dfilemanagerwindow.h"
#include "interfaces/dfileinfo.h"
#include "dialogs/dialogmanager.h"
#include "shutil/fileutils.h"
#include "views/dtoolbar.h"
#include "gvfs/secretmanager.h"
#include "controllers/appcontroller.h"
#include "interfaces/dumountmanager.h"
#include "utils.h"

DFM_USE_NAMESPACE

using namespace stub_ext;

QString createTestIsoFile()
{
    QFile f("/tmp/test.sh");
    if (f.open(QIODevice::WriteOnly|QIODevice::Truncate))
    {
        QTextStream in(&f);
        in << "#!/bin/bash\n";
        in << "if [ -d \"/tmp/tmpDirForIso\"] \nthen \n";
        in << "    rm -rf /tmp/tmpDirForIso\nfi \n";
        in << "if [ -f \"/tmp/utForFM.iso\"] \nthen \n";
        in << "    rm /tmp/utForFM.iso\nfi \n";
        in << "mkdir -p /tmp/tmpDirForIso && cd /tmp/tmpDirForIso\n";
        in << "touch helloworld.txt\n";
        in << "mkisofs -V TestISO -o /tmp/utForFM.iso /tmp/tmpDirForIso\n";
        f.close();
        f.setPermissions(f.permissions() | QFileDevice::ExeUser);
        QProcess::execute("/bin/sh", QStringList() << QFileInfo(f).absoluteFilePath());
        QProcess::execute("rm", QStringList() << QFileInfo(f).absoluteFilePath());
        QProcess::execute("rm", QStringList() << "-rf" << "/tmp/tmpDirForIso");
        QFile iso("/tmp/utForFM.iso");
        if (iso.exists())
            return "/tmp/utForFM.iso";
    }
    return QString();
}

using namespace testing;
namespace  {
    class AppControllerTest : public Test {

    protected:
        QString tempFilePath;
        AppController *controller;
        DUrl url;
        StubExt stl;
    protected:
        void SetUp()
        {
            DFMEventFuture (*processEventAsynclamda)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *) = []
                    (const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *) {
                return DFMEventFuture(QFuture<QVariant>());
            };

            stl.set((DFMEventFuture(DFMEventDispatcher::*)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *))\
                    ADDR(DFMEventDispatcher,processEventAsync),processEventAsynclamda);
            void (*showNewWindowlamda)(const DUrl &, const bool &) = [](const DUrl &, const bool &){return;};
            stl.set(ADDR(WindowManager,showNewWindow),showNewWindowlamda);
            bool (*cdlamda)(const DUrl &) = [](const DUrl &){return false;};
            stl.set(ADDR(DFileManagerWindow,cd),cdlamda);
            void (*showlamda)(void *) = [](void *){};
            stl.set(ADDR(QWidget,show),showlamda);

            void (*showErrorDialoglamda)(const QString &, const QString &) = []
                    (const QString &, const QString &){};
            stl.set(ADDR(DialogManager,showErrorDialog),showErrorDialoglamda);

            bool (*ghostSignallamda)(const DUrl &, DAbstractFileWatcher::SignalType3, const DUrl &, const int) = []
                    (const DUrl &, DAbstractFileWatcher::SignalType3, const DUrl &, const int){return true;};
            stl.set((bool (*)(const DUrl &, DAbstractFileWatcher::SignalType3, const DUrl &, const int))\
                    ADDR(DAbstractFileWatcher,ghostSignal),ghostSignallamda);
            bool (*ghostSignal1lamda)(const DUrl &, DAbstractFileWatcher::SignalType1, const DUrl &) = []
                    (const DUrl &, DAbstractFileWatcher::SignalType1, const DUrl &){return true;};
            bool (*ghostSignal2lamda)(const DUrl &, DAbstractFileWatcher::SignalType2 , const DUrl &, const DUrl &) = []
                    (const DUrl &, DAbstractFileWatcher::SignalType2 , const DUrl &, const DUrl &){return true;};
            stl.set((bool (*)(const DUrl &, DAbstractFileWatcher::SignalType1, const DUrl &))\
                    ADDR(DAbstractFileWatcher,ghostSignal),ghostSignal1lamda);
            stl.set((bool (*)(const DUrl &, DAbstractFileWatcher::SignalType2 , const DUrl &, const DUrl &))\
                    ADDR(DAbstractFileWatcher,ghostSignal),ghostSignal2lamda);
            typedef int(*fptr)(QDialog*);
            fptr pQDialogExec = (fptr)(&QDialog::exec);
            fptr pDDialogExec = (fptr)(&DDialog::exec);
            int (*stub_DDialog_exec)(void) = [](void)->int{return QDialog::Rejected;};
            stl.set(pQDialogExec, stub_DDialog_exec);
            stl.set(pDDialogExec, stub_DDialog_exec);

            void (*mountlamda)(const QString &) = [](const QString &){};
            stl.set(ADDR(UDiskListener,mount),mountlamda);
            void (*unmountlamda)(const QString &) = [](const QString &){};
            stl.set(ADDR(UDiskListener,unmount),unmountlamda);
            void (*unmountlamda2)(const QString &) = [](const QString &){};
            stl.set(ADDR(DUMountManager, umountBlock), unmountlamda2);
            void (*unmountlamda3)(const QVariantMap &) = [](const QVariantMap &){};
            stl.set(ADDR(DBlockDevice, unmount), unmountlamda3);

            controller = AppController::instance();
            QString fileName = QString::number(QDateTime::currentDateTime().toMSecsSinceEpoch());
            tempFilePath =  QStandardPaths::standardLocations(QStandardPaths::TempLocation).first() + "/" + fileName;
        }

        void TearDown()
        {
            QProcess::execute("rm -f " + tempFilePath);
            TestHelper::runInLoop([=](){
            }, 200);
        }
    };
}

TEST_F(AppControllerTest,start_registerUrlHandle){
    TestHelper::runInLoop([=](){
    });
    EXPECT_NO_FATAL_FAILURE(controller->registerUrlHandle());
}

TEST_F(AppControllerTest,start_manager_interface){
    StartManagerInterface *interface = controller->startManagerInterface();
    EXPECT_NE(interface, nullptr);
}


TEST_F(AppControllerTest,create_file_no_target_file){
    QString fileName = QString::number(QDateTime::currentDateTime().toMSecsSinceEpoch());
    QString filePath = controller->createFile(tempFilePath + 123, "/tmp", fileName,0);
    EXPECT_EQ(filePath, "");
}


TEST_F(AppControllerTest,can_create_file){
    QString fileName = QString::number(QDateTime::currentDateTime().toMSecsSinceEpoch() + 1);
    QProcess::execute("touch " + tempFilePath);
    QString filePath = AppController::instance()->createFile(tempFilePath, "/tmp",fileName,0);
    TestHelper::deleteTmpFile(tempFilePath);
    QUrl url = QUrl::fromLocalFile(filePath);
    QFile file(url.toLocalFile());
    EXPECT_EQ(true, file.exists());
    if (file.exists()) {
        file.remove();
    }
}

TEST_F(AppControllerTest,start_openAction){
    bool (*openFileslamda)(const QObject *, const DUrlList &, const bool ) = []
            (const QObject *, const DUrlList &, const bool ){return true;};
    stl.set(ADDR(DFileService,openFiles),openFileslamda);
    bool (*openFilelamda)(const QObject *, const DUrl &) = []
            (const QObject *, const DUrl &){return true;};
    stl.set(ADDR(DFileService,openFiles),openFilelamda);
    url.setScheme(FILE_SCHEME);
    url.setPath("~/Music");
    EXPECT_NO_FATAL_FAILURE(controller->actionOpen(dMakeEventPointer<DFMUrlListBaseEvent>(nullptr,DUrlList())));
    EXPECT_NO_FATAL_FAILURE(controller->actionOpen(dMakeEventPointer<DFMUrlListBaseEvent>(nullptr,DUrlList() << url)));

    DUrl recenturl,searchurl,burnurl;
    recenturl.setScheme(RECENT_SCHEME);
    searchurl.setScheme(SEARCH_SCHEME);
    burnurl.setScheme(BURN_SCHEME);
    DAbstractFileInfoPointer (*createFileInfolamda)(const QObject *, const DUrl &) = [](const QObject *, const DUrl &){
        return DAbstractFileInfoPointer();
    };
    stl.set(ADDR(DFileService,createFileInfo),createFileInfolamda);
    bool (*isVaultFilelamda)(void *) = [](void *){return true;};
    stl.set(ADDR(DUrl,isVaultFile),isVaultFilelamda);
    EXPECT_NO_FATAL_FAILURE(controller->actionOpen(dMakeEventPointer<DFMUrlListBaseEvent>(nullptr,DUrlList() << url << burnurl)));
    class testFileInfo1 : public DFileInfo {
    public:
        explicit testFileInfo1(const DUrl &fileUrl) :
            DFileInfo(fileUrl)
        {

        }
        bool canRedirectionFileUrl() const override {
            return true;
        }
        DUrl redirectedFileUrl() const override {
            return fileUrl();
        }
    };

    DAbstractFileInfoPointer (*createFileInfo2lamda)(const QObject *, const DUrl &) = [](const QObject *, const DUrl &){
        return DAbstractFileInfoPointer(new testFileInfo1(DUrl()));
    };
    stl.set(ADDR(DFileService,createFileInfo),createFileInfo2lamda);
    EXPECT_NO_FATAL_FAILURE(controller->actionOpen(dMakeEventPointer<DFMUrlListBaseEvent>(nullptr,DUrlList() << url << burnurl)));
}

TEST_F(AppControllerTest,start_openAction_one) {
    url.setScheme(FILE_SCHEME);
    url.setPath("~/Music");

    DUrl recenturl,searchurl,burnurl;
    recenturl.setScheme(RECENT_SCHEME);
    searchurl.setScheme(SEARCH_SCHEME);
    burnurl.setScheme(BURN_SCHEME);

    EXPECT_NO_FATAL_FAILURE(controller->actionOpen(dMakeEventPointer<DFMUrlListBaseEvent>(nullptr,DUrlList() << url
                                                                                          << burnurl << recenturl << searchurl)));
    class testFileInfo : public DFileInfo {
    public:
        explicit testFileInfo(const DUrl &fileUrl) :
            DFileInfo(fileUrl)
        {

        }
        bool isGvfsMountFile() const override {
            return true;
        }
    };

    DAbstractFileInfoPointer (*createFileInfo1lamda)(void *,const QObject *, const DUrl &) = [](void *,const QObject *, const DUrl &url){
        return DAbstractFileInfoPointer(new testFileInfo(url));
    };
    stl.set(ADDR(DFileService,createFileInfo),createFileInfo1lamda);
    EXPECT_NO_FATAL_FAILURE(controller->actionOpen(dMakeEventPointer<DFMUrlListBaseEvent>(nullptr,DUrlList() << url)));
}

TEST_F(AppControllerTest,start_actionOpenDisk){
    url.setScheme(FILE_SCHEME);
    url.setPath("~/Music");
    Stub stl;
    void (*actionOpenlamda)(const QSharedPointer<DFMUrlListBaseEvent> &, const bool) = []
            (const QSharedPointer<DFMUrlListBaseEvent> &, const bool){};
    stl.set(ADDR(AppController,actionOpen),actionOpenlamda);
    DUrl rooturl("dfmroot:///sdb1.localdisk");
    QString (*schemelamda)(void *) = [](void *){
        return QString(DFMROOT_SCHEME);
    };

    {
        Stub stl;
        stl.set(ADDR(DAbstractFileInfo,scheme),schemelamda);
        EXPECT_NO_FATAL_FAILURE(controller->actionOpenDisk(
                                    dMakeEventPointer<DFMUrlBaseEvent>(nullptr, url)));
    }

    QString (*scheme1lamda)(void *) = [](void *){
        return QString(BURN_SCHEME);
    };


    QString (*burnDestDevicelamda)(void *) = [](void *){ return QString("oo");};

    class testFileInfo : public DFileInfo {
    public:
        explicit testFileInfo(const DUrl &fileUrl) :
            DFileInfo(fileUrl)
        {

        }
        bool isGvfsMountFile() const override {
            return true;
        }
    };

    DAbstractFileInfoPointer (*createFileInfo1lamda)(void *,const QObject *, const DUrl &) = [](void *,const QObject *, const DUrl &url){
        return DAbstractFileInfoPointer(new testFileInfo(url));
    };

    QStringList (*resolveDeviceNodelamda)(QString, QVariantMap) = [](QString, QVariantMap){
        return QStringList() << QString("ooo");
    };


    QByteArrayList (*mountPointslamda)(void *)= [](void *){
        return QByteArrayList() << QByteArray("oooo");
    };
    {
        Stub stl;
        stl.set(ADDR(QUrl,scheme),scheme1lamda);
        stl.set(ADDR(DUrl,burnDestDevice),burnDestDevicelamda);
        stl.set(ADDR(DDiskManager,resolveDeviceNode),resolveDeviceNodelamda);
        stl.set(ADDR(DBlockDevice,mountPoints),mountPointslamda);
        stl.set(ADDR(DFileService,createFileInfo),createFileInfo1lamda);
        EXPECT_NO_FATAL_FAILURE(controller->actionOpenDisk(
                                    dMakeEventPointer<DFMUrlBaseEvent>(nullptr, url)));
    }
    {
        Stub stl;
        stl.set(ADDR(DFileService,createFileInfo),createFileInfo1lamda);
        EXPECT_NO_FATAL_FAILURE(controller->actionOpenDisk(
                                    dMakeEventPointer<DFMUrlBaseEvent>(nullptr, url)));
    }

    EXPECT_NO_FATAL_FAILURE(controller->actionOpenDisk(
                                dMakeEventPointer<DFMUrlBaseEvent>(nullptr, DUrl())));
    void (*actionMountlamda)(const QSharedPointer<DFMUrlBaseEvent> &) = [](const QSharedPointer<DFMUrlBaseEvent> &){};
    stl.set(ADDR(AppController,actionMount),actionMountlamda);

    EXPECT_NO_FATAL_FAILURE(controller->actionOpenDisk(
                                dMakeEventPointer<DFMUrlBaseEvent>(nullptr, rooturl)));

}

TEST_F(AppControllerTest,start_asyncOpenDisk){
    url.setScheme(FILE_SCHEME);
    url.setPath("~/Music");
    if (!controller->m_fmEvent ) {
        controller->m_fmEvent = dMakeEventPointer<DFMUrlListBaseEvent>(nullptr,DUrlList() << url);
    }
    void (*actionOpenlamda)(const QSharedPointer<DFMUrlListBaseEvent> &, const bool) = []
            (const QSharedPointer<DFMUrlListBaseEvent> &, const bool){};
    stl.set(ADDR(AppController,actionOpen),actionOpenlamda);
    EXPECT_NO_FATAL_FAILURE(controller->asyncOpenDisk(url.toString()));
}

TEST_F(AppControllerTest,start_actionOpenInNewWindow){
    url.setScheme(FILE_SCHEME);
    url.setPath("~/Music");
    void (*actionMountlamda)(const QSharedPointer<DFMUrlBaseEvent> &) = [](const QSharedPointer<DFMUrlBaseEvent> &){};
    stl.set(ADDR(AppController,actionMount),actionMountlamda);
    QVariant (*processEventlamda)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *) = []
            (const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *) {
        return QVariant();
    };
    stl.set((QVariant(DFMEventDispatcher::*)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *))\
            ADDR(DFMEventDispatcher,processEvent),processEventlamda);

    EXPECT_NO_FATAL_FAILURE(controller->actionOpenInNewWindow(dMakeEventPointer<DFMUrlListBaseEvent>(nullptr, DUrlList() << url)));
    DUrl temp;
    temp.setScheme(FILE_SCHEME);
    temp.setPath("~/Videos");
    EXPECT_NO_FATAL_FAILURE(controller->actionOpenInNewTab(dMakeEventPointer<DFMUrlBaseEvent>(nullptr,  url)));
}

TEST_F(AppControllerTest,start_actionOpenDiskInNewWindow){
    url.setScheme(FILE_SCHEME);
    url.setPath("~/Videos");
    void (*actionMountlamda)(const QSharedPointer<DFMUrlBaseEvent> &) = [](const QSharedPointer<DFMUrlBaseEvent> &){};
     stl.set(ADDR(AppController,actionMount),actionMountlamda);
    EXPECT_NO_FATAL_FAILURE(controller->actionOpenDiskInNewWindow(dMakeEventPointer<DFMUrlBaseEvent>(nullptr, url)));
    EXPECT_NO_FATAL_FAILURE(controller->actionOpenDiskInNewWindow(dMakeEventPointer<DFMUrlBaseEvent>(nullptr, DUrl())));
    EXPECT_NO_FATAL_FAILURE(controller->actionOpenDiskInNewTab(dMakeEventPointer<DFMUrlBaseEvent>(nullptr, url)));
    EXPECT_NO_FATAL_FAILURE(controller->actionOpenDiskInNewTab(dMakeEventPointer<DFMUrlBaseEvent>(nullptr, DUrl())));
    QString (*schemelamda)(void *) = [](void *){
        return QString(DFMROOT_SCHEME);
    };
    stl.set(ADDR(DAbstractFileInfo,scheme),schemelamda);
    EXPECT_NO_FATAL_FAILURE(controller->actionOpenDiskInNewWindow(dMakeEventPointer<DFMUrlBaseEvent>(nullptr, url)));
    EXPECT_NO_FATAL_FAILURE(controller->actionOpenDiskInNewTab(dMakeEventPointer<DFMUrlBaseEvent>(nullptr, url)));
    stl.reset(ADDR(DAbstractFileInfo,scheme));
    DUrl rooturl("dfmroot:///sdb1.localdisk");
    EXPECT_NO_FATAL_FAILURE(controller->actionOpenDiskInNewTab(dMakeEventPointer<DFMUrlBaseEvent>(nullptr, rooturl)));
    EXPECT_NO_FATAL_FAILURE(controller->actionOpenDiskInNewWindow(dMakeEventPointer<DFMUrlBaseEvent>(nullptr, rooturl)));
}

TEST_F(AppControllerTest,start_actionOpenAsAdmin){
    url.setScheme(FILE_SCHEME);
    url.setPath("~/Videos");
    bool (*startDetachedlamda)(const QString &, const QStringList &) = [](const QString &, const QStringList &){
        return true;
    };
    stl.set((bool(*)(const QString &, const QStringList &))ADDR(QProcess,startDetached),startDetachedlamda);
    EXPECT_NO_FATAL_FAILURE(controller->actionOpenAsAdmin(dMakeEventPointer<DFMUrlBaseEvent>(nullptr, url)));
}

TEST_F(AppControllerTest,start_asyncOpenDiskInNewWindow){
    url.setScheme(FILE_SCHEME);
    url.setPath("~/Music");
    if (!controller->m_fmEvent ) {
        controller->m_fmEvent = dMakeEventPointer<DFMUrlListBaseEvent>(nullptr,DUrlList() << url);
    }
    EXPECT_NO_FATAL_FAILURE(controller->asyncOpenDiskInNewWindow(url.toString()));
}

TEST_F(AppControllerTest,start_asyncOpenDiskInNewTab){
    url.setScheme(FILE_SCHEME);
    url.setPath("~/Music");
    if (!controller->m_fmEvent ) {
        controller->m_fmEvent = dMakeEventPointer<DFMUrlListBaseEvent>(nullptr,DUrlList() << url);
    }
    EXPECT_NO_FATAL_FAILURE(controller->asyncOpenDiskInNewTab(url.toString()));
}

TEST_F(AppControllerTest,start_actionOpenFilesWithCustom){
    url.setScheme(FILE_SCHEME);
    url.setPath("~/Videos");
    DUrl temp;
    temp.setScheme(FILE_SCHEME);
    temp.setPath(TestHelper::createTmpFile());
    void (*showOpenFilesWithDialoglamda)(const DFMEvent &) = [](const DFMEvent &){};
    stl.set(ADDR(DialogManager,showOpenFilesWithDialog),showOpenFilesWithDialoglamda);
    EXPECT_NO_FATAL_FAILURE(controller->actionOpenFilesWithCustom(dMakeEventPointer<DFMUrlListBaseEvent>
                                                                  (nullptr, DUrlList() << url << temp)));
    bool (*openFileLocationlamda)(const QObject *, const DUrl &) = [](const QObject *, const DUrl &){return true;};
    stl.set(ADDR(DFileService,openFileLocation),openFileLocationlamda);
    EXPECT_NO_FATAL_FAILURE(controller->actionOpenFileLocation(dMakeEventPointer<DFMUrlListBaseEvent>
                                                               (nullptr, DUrlList() << url << temp)));
    TestHelper::deleteTmpFile(temp.toLocalFile());
}

TEST_F(AppControllerTest,start_actionOpenWithCustom){
    url.setScheme(FILE_SCHEME);
    url.setPath("~/Videos");
    void (*showOpenWithDialoglamda)(const DFMEvent &) = [](const DFMEvent &){};
    stl.set(ADDR(DialogManager,showOpenWithDialog),showOpenWithDialoglamda);
    EXPECT_NO_FATAL_FAILURE(controller->actionOpenWithCustom(dMakeEventPointer<DFMUrlBaseEvent>
                                                               (nullptr, url)));
}

TEST_F(AppControllerTest,start_actionCompressOp){
    url.setScheme(FILE_SCHEME);
    url.setPath("~/Videos");
    QString fileName = QString::number(QDateTime::currentDateTime().toMSecsSinceEpoch());
    QVariant (*processEventlamda)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *) = []
            (const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *) {
        return QVariant();
    };
    stl.set((QVariant(DFMEventDispatcher::*)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *))\
            ADDR(DFMEventDispatcher,processEvent),processEventlamda);

    EXPECT_NO_FATAL_FAILURE(controller->actionCompress(dMakeEventPointer<DFMUrlListBaseEvent>
                                                       (nullptr, DUrlList() << url)));
    EXPECT_NO_FATAL_FAILURE(controller->actionDecompress(dMakeEventPointer<DFMUrlListBaseEvent>
                                 (nullptr, DUrlList() << url)));
    EXPECT_NO_FATAL_FAILURE(controller->actionDecompressHere(dMakeEventPointer<DFMUrlListBaseEvent>
                                     (nullptr, DUrlList() << url)));
}

TEST_F(AppControllerTest,start_actionCut){
    url.setScheme(FILE_SCHEME);
    url.setPath("~/Videos");
    QVariant (*processEventlamda)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *) = []
            (const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *) {
        return QVariant();
    };
    stl.set((QVariant(DFMEventDispatcher::*)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *))\
            ADDR(DFMEventDispatcher,processEvent),processEventlamda);
    EXPECT_NO_FATAL_FAILURE(controller->actionCut(dMakeEventPointer<DFMUrlListBaseEvent>
                                                  (nullptr, DUrlList() << url)));
}

TEST_F(AppControllerTest,start_actionCopy){
    url.setScheme(FILE_SCHEME);
    url.setPath("~/Videos");
    QVariant (*processEventlamda)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *) = []
            (const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *) {
        return QVariant();
    };
    stl.set((QVariant(DFMEventDispatcher::*)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *))\
            ADDR(DFMEventDispatcher,processEvent),processEventlamda);

    EXPECT_NO_FATAL_FAILURE(controller->actionCopy(dMakeEventPointer<DFMUrlListBaseEvent>
                                                   (nullptr, DUrlList() << url)));
}

TEST_F(AppControllerTest,start_actionRename1){
    url.setScheme(FILE_SCHEME);
    url.setPath("~/Videos");
    QVariant (*processEventlamda)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *) = []
            (const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *) {
        return QVariant();
    };
    stl.set((QVariant(DFMEventDispatcher::*)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *))\
            ADDR(DFMEventDispatcher,processEvent),processEventlamda);
    QString fileName = QString::number(QDateTime::currentDateTime().toMSecsSinceEpoch());
    DUrl temp;
    temp.setScheme(FILE_SCHEME);
    temp.setPath("./"+fileName+"_2.txt");
    QProcess::execute("touch " + temp.toLocalFile());
    EXPECT_NO_FATAL_FAILURE(controller->actionRename(dMakeEventPointer<DFMUrlListBaseEvent>(nullptr, DUrlList() << temp)));
    EXPECT_NO_FATAL_FAILURE(controller->actionRename(dMakeEventPointer<DFMUrlListBaseEvent>(nullptr, DUrlList() << temp << DUrl())));
    QProcess::execute("rm " + temp.toLocalFile());
}

bool createSymlinklamd(const QObject *, const DUrl &){
    return true;
};

TEST_F(AppControllerTest,start_actionBookmarkandother){
    DUrl temp;
    temp.setScheme(FILE_SCHEME);
    QString fileName = QString::number(QDateTime::currentDateTime().toMSecsSinceEpoch());
    temp.setPath("./"+fileName+".txt");
    QProcess::execute("touch " + temp.toLocalFile());
    DUrl temp1 = temp;
    temp1.setPath("./"+fileName+"_sys.txt");

    QVariant (*processEventlamda)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *) = []
            (const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *) {
        return QVariant();
    };

    {
        Stub stl;
        stl.set((QVariant(DFMEventDispatcher::*)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *))\
                ADDR(DFMEventDispatcher,processEvent),processEventlamda);

        EXPECT_NO_FATAL_FAILURE(controller->actionAddToBookMark(dMakeEventPointer<DFMUrlBaseEvent>
                                                                (nullptr, temp)));
        EXPECT_NO_FATAL_FAILURE(controller->actionBookmarkRename(dMakeEventPointer<DFMUrlBaseEvent>
                                                                 (nullptr, temp)));
        EXPECT_NO_FATAL_FAILURE(controller->actionBookmarkRemove(dMakeEventPointer<DFMUrlBaseEvent>
                                                                 (nullptr, temp)));


        stl.set((bool(DFileService::*)(const QObject *, const DUrl &) const)ADDR(DFileService,createSymlink),createSymlinklamd);
        EXPECT_NO_FATAL_FAILURE(controller->actionCreateSymlink(dMakeEventPointer<DFMUrlBaseEvent>
                                                                (nullptr, temp1)));
    }

    bool (*sendToDesktoplamda)(const QObject *, const DUrlList &) = [](const QObject *, const DUrlList &){return true;};

    Stub stl;
    stl.set((QVariant(DFMEventDispatcher::*)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *))\
            ADDR(DFMEventDispatcher,processEvent),processEventlamda);
    stl.set(ADDR(DFileService,sendToDesktop),sendToDesktoplamda);
    EXPECT_NO_FATAL_FAILURE(controller->actionSendToDesktop(dMakeEventPointer<DFMUrlListBaseEvent>
                                                            (nullptr, DUrlList() << temp)));
    EXPECT_NO_FATAL_FAILURE(controller->actionSendToBluetooth());
    QAction action;
    action.connect(&action,&QAction::triggered,controller,&AppController::actionSendToBluetooth);
    void (*sendToBluetoothlamda)(const DUrlList &) = [](const DUrlList &){};
    stl.set(ADDR(DFileService,sendToBluetooth),sendToBluetoothlamda);
    emit action.triggered();
    EXPECT_NO_FATAL_FAILURE(controller->actionSendToBluetooth());
    bool (*deleteFileslamda)(const QObject *, const DUrlList &, bool, bool, bool) = []
            (const QObject *, const DUrlList &, bool, bool, bool){
        return true;
    };
    stl.set(ADDR(DFileService,deleteFiles),deleteFileslamda);
    EXPECT_NO_FATAL_FAILURE(controller->actionCompleteDeletion(dMakeEventPointer<DFMUrlListBaseEvent>
                                                               (nullptr, DUrlList())));
    TestHelper::deleteTmpFiles(QStringList() << temp.toLocalFile() << temp1.toLocalFile());
}

TEST_F(AppControllerTest,start_actionNewFolder){
    url.setScheme(FILE_SCHEME);
    url.setPath("./ooooooooooo");
    QVariant (*processEventlamda)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *) = []
            (const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *) {
        return QVariant();
    };
    stl.set((QVariant(DFMEventDispatcher::*)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *))\
            ADDR(DFMEventDispatcher,processEvent),processEventlamda);
    DUrl (*newDocumentUrllamda)(const DAbstractFileInfoPointer, const QString &, const QString &) = []
            (const DAbstractFileInfoPointer, const QString &, const QString &){return DUrl();};
    stl.set(ADDR(FileUtils,newDocumentUrl),newDocumentUrllamda);
    EXPECT_NO_FATAL_FAILURE(controller->actionNewFolder(dMakeEventPointer<DFMUrlBaseEvent>(nullptr, url)));
    EXPECT_NO_FATAL_FAILURE(controller->actionSelectAll(29489851231423));
    EXPECT_NO_FATAL_FAILURE(controller->actionClearRecent(dMakeEventPointer<DFMMenuActionEvent>
                                                          (nullptr,nullptr,DUrl(),DUrlList(),DFMGlobal::ClearRecent,QModelIndex())));
    QProcess::execute("cp "+ QDir::homePath() + "/.local/share/recently-used.xbel " + QDir::homePath() + "/.local/share/recently-used.xbel.bak");
    EXPECT_NO_FATAL_FAILURE(controller->actionClearRecent());
    QProcess::execute("mv "+ QDir::homePath() + "/.local/share/recently-used.xbel.bak " + QDir::homePath() + "/.local/share/recently-used.xbel");
    QProcess::execute("rm -rf " + url.toLocalFile());
    DFileService::instance()->setDoClearTrashState(true);
    bool (*deleteFileslamda)(const QObject *, const DUrlList &, bool, bool, bool) = []
            (const QObject *, const DUrlList &, bool, bool, bool){
        return true;
    };
    stl.set(ADDR(DFileService,deleteFiles),deleteFileslamda);
    EXPECT_NO_FATAL_FAILURE(controller->actionClearTrash(nullptr));
    DFileService::instance()->setDoClearTrashState(false);
    EXPECT_NO_FATAL_FAILURE(controller->actionClearTrash(nullptr));
    DFileService::instance()->setDoClearTrashState(false);

}

TEST_F(AppControllerTest,start_actionNewFile){
    url.setScheme(FILE_SCHEME);
    url.setPath("./ztt_test_app.txt");
    DUrl tmp(url);
    void (*mountlamda)(const QString &) = [](const QString &){};
    stl.set(ADDR(UDiskListener,mount),mountlamda);
    void (*unmountlamda)(const QString &) = [](const QString &){};
    stl.set(ADDR(UDiskListener,unmount),unmountlamda);
    typedef int(*fptr)(QDialog*);
    fptr pQDialogExec = (fptr)(&QDialog::exec);
    fptr pDDialogExec = (fptr)(&DDialog::exec);
    int (*stub_DDialog_exec)(void) = [](void)->int{return QDialog::Rejected;};
    stl.set(pQDialogExec, stub_DDialog_exec);
    stl.set(pDDialogExec, stub_DDialog_exec);
    tmp.setPath(TestHelper::createTmpDir());
    url.setPath(tmp.toLocalFile() + "/ztt_test_appkkk");
    bool (*cpTemplateFileToTargetDirlamda)(const QString &, const QString &, const QString &, WId ) =
            [](const QString &, const QString &, const QString &, WId ){return true;};
    stl.set(ADDR(FileUtils,cpTemplateFileToTargetDir),cpTemplateFileToTargetDirlamda);
    EXPECT_NO_FATAL_FAILURE(controller->actionNewText(dMakeEventPointer<DFMUrlBaseEvent>(nullptr,url)));
    EXPECT_NO_FATAL_FAILURE(controller->actionNewWord(dMakeEventPointer<DFMUrlBaseEvent>(nullptr,url)));
    EXPECT_NO_FATAL_FAILURE(controller->actionNewExcel(dMakeEventPointer<DFMUrlBaseEvent>(nullptr,url)));
    EXPECT_NO_FATAL_FAILURE(controller->actionNewPowerpoint(dMakeEventPointer<DFMUrlBaseEvent>(nullptr,url)));
    QVariant (*processEventWithEventLooplamda)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *) = []
            (const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *){
            return QVariant();
    };
    stl.set((QVariant (DFMEventDispatcher::*)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *))
            ADDR(DFMEventDispatcher,processEventWithEventLoop),processEventWithEventLooplamda);
    EXPECT_NO_FATAL_FAILURE(controller->actionRestore(dMakeEventPointer<DFMUrlListBaseEvent>(nullptr,DUrlList() << url)));


    EXPECT_NO_FATAL_FAILURE(controller->actionRestoreAll(dMakeEventPointer<DFMUrlBaseEvent>(nullptr,url)));
    stl.reset((QVariant (DFMEventDispatcher::*)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *))
            ADDR(DFMEventDispatcher,processEventWithEventLoop));
    bool (*optical_stublamda)(void *obj) = [](void *obj){return true;};
    stl.set(ADDR(DDiskDevice, optical), optical_stublamda);
    url = DUrl("dfmroot:///sda1.localdisk");
    EXPECT_NO_FATAL_FAILURE(controller->actionMount(dMakeEventPointer<DFMUrlBaseEvent>(nullptr, url)));

    url = DUrl("dfmroot:///");
    EXPECT_NO_FATAL_FAILURE(controller->actionMount(dMakeEventPointer<DFMUrlBaseEvent>(nullptr, url)));

    QString isoPath = createTestIsoFile();
    url = DUrl("file://" + isoPath);
    void (*actionOpenlamda)(const QSharedPointer<DFMUrlListBaseEvent> &, const bool) = []
            (const QSharedPointer<DFMUrlListBaseEvent> &, const bool){};
    stl.set(ADDR(AppController,actionOpen),actionOpenlamda);

    TestHelper::runInLoop([=](){
        EXPECT_NO_FATAL_FAILURE(controller->actionMountImage(dMakeEventPointer<DFMUrlBaseEvent>(nullptr, url)));
        EXPECT_NO_FATAL_FAILURE(controller->actionMountImage(dMakeEventPointer<DFMUrlBaseEvent>
                                                             (nullptr, DUrl("file:///Empty.iso"))));
    },1000);
    QProcess::execute("rm", QStringList() << isoPath);

    url = DUrl("dfmroot:///fakeDisk.gvfsmp");
    EXPECT_NO_FATAL_FAILURE(controller->actionUnmount(dMakeEventPointer<DFMUrlBaseEvent>(nullptr, url)));
    url = DUrl("dfmroot:///sda1.localdisk");
//    EXPECT_NO_FATAL_FAILURE(controller->actionUnmount(dMakeEventPointer<DFMUrlBaseEvent>(nullptr, url)));
    url = DUrl("dfmroots:///justQuery");
    url.setQuery("testQuery");
    EXPECT_NO_FATAL_FAILURE(controller->actionUnmount(dMakeEventPointer<DFMUrlBaseEvent>(nullptr, url)));


    url = DUrl("dfmroot:///fakeDisk.gvfsmp");
    TestHelper::runInLoop([=](){
        EXPECT_NO_FATAL_FAILURE(controller->actionEject(dMakeEventPointer<DFMUrlBaseEvent>(nullptr, url)));
    },1000);

    url = DUrl("dfmroots:///fakeDisk.gvfsmp");
    EXPECT_NO_FATAL_FAILURE(controller->actionEject(dMakeEventPointer<DFMUrlBaseEvent>(nullptr, url)));

    url = DUrl("dfmroot:///fakeDisk.gvfsmp");
    EXPECT_NO_FATAL_FAILURE(controller->actionSafelyRemoveDrive(dMakeEventPointer<DFMUrlBaseEvent>(nullptr, url)));
    url = DUrl("dfmroots:///fakeDisk.gvfsmp");
    EXPECT_NO_FATAL_FAILURE(controller->actionSafelyRemoveDrive(dMakeEventPointer<DFMUrlBaseEvent>(nullptr, url)));

    url = DUrl("dfmroot:///fakeDisk.localdisk");
    QWidget w;
    auto e = dMakeEventPointer<DFMUrlBaseEvent>(nullptr, url);
    e->setWindowId(w.winId());
    TestHelper::runInLoop([=](){
        EXPECT_NO_FATAL_FAILURE(controller->actionFormatDevice(e));
    },1000);

    TestHelper::deleteTmpFile(tmp.toLocalFile());
}

TEST_F(AppControllerTest, start_actionOpenInTerminal){
    url.setScheme(FILE_SCHEME);
    url.setPath("./");
    QVariant (*processEventlamda)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *) = []
            (const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *) {
        return QVariant();
    };
    stl.set((QVariant(DFMEventDispatcher::*)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *))\
            ADDR(DFMEventDispatcher,processEvent),processEventlamda);
    EXPECT_NO_FATAL_FAILURE(controller->actionOpenInTerminal(dMakeEventPointer<DFMUrlListBaseEvent>
                                                             (nullptr,DUrlList() << url)));
}

TEST_F(AppControllerTest, start_actionProperty){
    url.setScheme(FILE_SCHEME);
    url.setPath("./");
    typedef int(*fptr)(QDialog*);
    fptr pQDialogExec = (fptr)(&QDialog::exec);
    fptr pDDialogExec = (fptr)(&DDialog::exec);
    int (*stub_DDialog_exec)(void) = [](void)->int{return QDialog::Rejected;};
    stl.set(pQDialogExec, stub_DDialog_exec);
    stl.set(pDDialogExec, stub_DDialog_exec);
    controller->disconnect();
    void (*showTrashPropertyDialoglamda)(const DFMEvent &) = [](const DFMEvent &){};
    stl.set(ADDR(DialogManager,showTrashPropertyDialog),showTrashPropertyDialoglamda);
    void (*showComputerPropertyDialoglamda)(void *) = [](void*){};
    stl.set(ADDR(DialogManager,showComputerPropertyDialog),showComputerPropertyDialoglamda);
    void (*showPropertyDialoglamda)(const DFMUrlListBaseEvent &) = [](const DFMUrlListBaseEvent &){};
    stl.set(ADDR(DialogManager,showPropertyDialog),showPropertyDialoglamda);
    EXPECT_NO_FATAL_FAILURE(controller->actionProperty(dMakeEventPointer<DFMUrlListBaseEvent>
                                                       (nullptr,DUrlList() << url)));
    url.setPath("/bin");
    EXPECT_NO_FATAL_FAILURE(controller->actionProperty(dMakeEventPointer<DFMUrlListBaseEvent>
                                                       (nullptr,DUrlList() << url)));

//    QString (*scheme)(void *) = [](void *){return QString()};
//    stl.set(ADDR(DAbstractFileInfo,scheme),scheme);
    url.setUrl("dfmroot:///Desktop.userdir");
    EXPECT_NO_FATAL_FAILURE(controller->actionProperty(dMakeEventPointer<DFMUrlListBaseEvent>
                                                       (nullptr,DUrlList() << url)));

    url.setScheme(FILE_SCHEME);
    url.setPath("~/Desktop/dde-computer.desktop");
    EXPECT_NO_FATAL_FAILURE(controller->actionProperty(dMakeEventPointer<DFMUrlListBaseEvent>
                                                       (nullptr,DUrlList() << url)));
    url.setPath("~/Desktop/dde-trash.desktop");
    EXPECT_NO_FATAL_FAILURE(controller->actionProperty(dMakeEventPointer<DFMUrlListBaseEvent>
                                                       (nullptr,DUrlList() << url)));
    url = DUrl::fromTrashFile("/");
    EXPECT_NO_FATAL_FAILURE(controller->actionProperty(dMakeEventPointer<DFMUrlListBaseEvent>
                                                       (nullptr,DUrlList() << url)));
    url = DUrl::fromComputerFile("/");
    EXPECT_NO_FATAL_FAILURE(controller->actionProperty(dMakeEventPointer<DFMUrlListBaseEvent>
                                                       (nullptr,DUrlList() << url)));

    QString (*burnFilePath)(void *) = [](void *){ return QString("///");};
    stl.set(ADDR(DUrl,burnFilePath),burnFilePath);
    url.setScheme(BURN_SCHEME);
    url.setPath("/testes/dfds//");
    EXPECT_NO_FATAL_FAILURE(controller->actionProperty(dMakeEventPointer<DFMUrlListBaseEvent>
                                                       (nullptr,DUrlList() << url)));
}

TEST_F(AppControllerTest, start_actionNewWindow){
    url.setScheme(FILE_SCHEME);
    url.setPath("~/Videos");
    QVariant (*processEventlamda)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *) = []
            (const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *) {
        return QVariant();
    };
    stl.set((QVariant(DFMEventDispatcher::*)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *))\
            ADDR(DFMEventDispatcher,processEvent),processEventlamda);
    EXPECT_NO_FATAL_FAILURE(controller->actionNewWindow(dMakeEventPointer<DFMUrlListBaseEvent>
                                                        (nullptr,DUrlList() << url)));

}

TEST_F(AppControllerTest, start_actionExit){
    void (*onLastActivedWindowClosedlamda)(quint64) = [](quint64){};
    stl.set(ADDR(WindowManager,onLastActivedWindowClosed),onLastActivedWindowClosedlamda);
    EXPECT_NO_FATAL_FAILURE(controller->actionExit(53561231655));
}

TEST_F(AppControllerTest, start_actionSetAsWallpaper){
    url.setScheme(FILE_SCHEME);
    url.setPath("~/Pictures/Pictures/Wallpapers/abc-123.jpg");
    bool (*setBackgroundlamda)(const QString &) = [](const QString &){return true;};
    stl.set(ADDR(FileUtils,setBackground),setBackgroundlamda);
    EXPECT_NO_FATAL_FAILURE(controller->actionSetAsWallpaper(dMakeEventPointer<DFMUrlBaseEvent>(nullptr,url)));
    bool (*isLocalFilelamda)(void *) = [](void *){return true;};
    stl.set(ADDR(DUrl,isLocalFile),isLocalFilelamda);
    EXPECT_NO_FATAL_FAILURE(controller->actionSetAsWallpaper(dMakeEventPointer<DFMUrlBaseEvent>(nullptr,url)));
}

TEST_F(AppControllerTest, start_actionShare){
    url.setScheme(FILE_SCHEME);
    url.setPath("~/ut_share_test");
    QProcess::execute("mkdir " + url.toLocalFile());
    void (*showShareOptionsInPropertyDialoglamda)(const DFMUrlListBaseEvent &) = []
            (const DFMUrlListBaseEvent &){};
    stl.set(ADDR(DialogManager,showShareOptionsInPropertyDialog),showShareOptionsInPropertyDialoglamda);
    EXPECT_NO_FATAL_FAILURE(controller->actionShare(dMakeEventPointer<DFMUrlListBaseEvent>(nullptr,DUrlList() << url)));
}

TEST_F(AppControllerTest, start_actionUnShare){
    url.setScheme(FILE_SCHEME);
    url.setPath("~/ut_share_test");
    QVariant (*processEventlamda)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *) = []
            (const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *) {
        return QVariant();
    };
    stl.set((QVariant(DFMEventDispatcher::*)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *))\
            ADDR(DFMEventDispatcher,processEvent),processEventlamda);
    EXPECT_NO_FATAL_FAILURE(controller->actionUnShare(dMakeEventPointer<DFMUrlBaseEvent>(nullptr,url)));
    TestHelper::deleteTmpFile(url.toLocalFile());
}

TEST_F(AppControllerTest, start_actionConnectToServer){
    void (*showConnectToServerDialoglamda)(quint64) = [](quint64){};
    stl.set(ADDR(DialogManager,showConnectToServerDialog),showConnectToServerDialoglamda);
    EXPECT_NO_FATAL_FAILURE(controller->actionConnectToServer(
                                static_cast<quint64>(QDateTime::currentDateTime().toMSecsSinceEpoch())));
}

TEST_F(AppControllerTest, start_actionSetUserSharePassword){
    void (*showUserSharePasswordSettingDialoglamda)(quint64) = [](quint64){};
    stl.set(ADDR(DialogManager,showUserSharePasswordSettingDialog),showUserSharePasswordSettingDialoglamda);
    EXPECT_NO_FATAL_FAILURE(controller->actionSetUserSharePassword(static_cast<quint64>
                                                                   (QDateTime::currentDateTime().toMSecsSinceEpoch())));
}

TEST_F(AppControllerTest, start_actionSettings){
    void (*showGlobalSettingsDialoglamda)(quint64) = [](quint64){};
    stl.set(ADDR(DialogManager,showGlobalSettingsDialog),showGlobalSettingsDialoglamda);
    EXPECT_NO_FATAL_FAILURE(controller->actionSettings(static_cast<quint64>
                                                       (QDateTime::currentDateTime().toMSecsSinceEpoch())));
}

TEST_F(AppControllerTest, start_actionFormatDevice){
    EXPECT_NO_FATAL_FAILURE(controller->actionFormatDevice(dMakeEventPointer<DFMUrlBaseEvent>(nullptr,url)));
}

TEST_F(AppControllerTest, start_actionOpticalBlank){
    EXPECT_NO_FATAL_FAILURE(controller->actionFormatDevice(dMakeEventPointer<DFMUrlBaseEvent>(nullptr,url)));
}

TEST_F(AppControllerTest, start_actionctrlL){
    void (*handleHotkeyCtrlLlamda)(quint64 ) = [](quint64 ){};
    stl.set(ADDR(DToolBar,handleHotkeyCtrlL),handleHotkeyCtrlLlamda);
    EXPECT_NO_FATAL_FAILURE(controller->actionctrlL(static_cast<quint64>
                                                    (QDateTime::currentDateTime().toMSecsSinceEpoch())));
}

TEST_F(AppControllerTest, start_actionctrlF){
    void (*handleHotkeyCtrlFlamda)(quint64 ) = [](quint64 ){};
    stl.set(ADDR(DToolBar,handleHotkeyCtrlF),handleHotkeyCtrlFlamda);
    EXPECT_NO_FATAL_FAILURE(controller->actionctrlF(static_cast<quint64>
                                                    (QDateTime::currentDateTime().toMSecsSinceEpoch())));
}
QWidget *getWindowByIdlamda(quint64 winId) {
    QWidget *pWidget = new QWidget();
    pWidget->deleteLater();
    return pWidget;
}
TEST_F(AppControllerTest, start_actionExitCurrentWindow){
    stl.set(ADDR(WindowManager,getWindowById),&getWindowByIdlamda);
    EXPECT_NO_FATAL_FAILURE(controller->actionExitCurrentWindow(static_cast<quint64>
                                                                (QDateTime::currentDateTime().toMSecsSinceEpoch())));
}

TEST_F(AppControllerTest, start_actionShowHotkeyHelp){

    EXPECT_NO_FATAL_FAILURE(controller->actionShowHotkeyHelp(static_cast<quint64>
                                                             (QDateTime::currentDateTime().toMSecsSinceEpoch())));
    stl.set(ADDR(WindowManager,getWindowById),&getWindowByIdlamda);
    bool (*startDetachedlamda)(const QString &, const QStringList &) = []
            (const QString &, const QStringList &){return true;};
    stl.set((bool (*)(const QString &, const QStringList &))ADDR(QProcess,startDetached),startDetachedlamda);
    EXPECT_NO_FATAL_FAILURE(controller->actionShowHotkeyHelp(static_cast<quint64>
                                                             (QDateTime::currentDateTime().toMSecsSinceEpoch())));
}

TEST_F(AppControllerTest, start_actionBack){
    QVariant (*processEventlamda)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *) = []
            (const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *) {
        return QVariant();
    };
    stl.set((QVariant(DFMEventDispatcher::*)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *))\
            ADDR(DFMEventDispatcher,processEvent),processEventlamda);
    EXPECT_NO_FATAL_FAILURE(controller->actionBack(static_cast<quint64>
                                                   (QDateTime::currentDateTime().toMSecsSinceEpoch())));
}

TEST_F(AppControllerTest, start_actionForward){
    QVariant (*processEventlamda)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *) = []
            (const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *) {
        return QVariant();
    };
    stl.set((QVariant(DFMEventDispatcher::*)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *))\
            ADDR(DFMEventDispatcher,processEvent),processEventlamda);
    EXPECT_NO_FATAL_FAILURE(controller->actionForward(
                                static_cast<quint64>(QDateTime::currentDateTime().toMSecsSinceEpoch())));
}

TEST_F(AppControllerTest, start_actionForgetPassword){
    void (*actionUnmountlamda)(const QSharedPointer<DFMUrlBaseEvent> &) = [](const QSharedPointer<DFMUrlBaseEvent> &){};
    stl.set(ADDR(AppController,actionUnmount),actionUnmountlamda);
    void (*clearPasswordByLoginObjlamda)(const QJsonObject &) = [](const QJsonObject &){};
    stl.set(ADDR(SecretManager,clearPasswordByLoginObj),clearPasswordByLoginObjlamda);
    url.setUrl("dfmroot:///%252Frun%252Fuser%252F1000%252Fgvfs%252Fsmb-share%253Aserver%253D10.8.12.125%252Cshare%253D%25E9%25");
    EXPECT_NO_FATAL_FAILURE(controller->actionForgetPassword(dMakeEventPointer<DFMUrlBaseEvent>(nullptr,url)));
}

TEST_F(AppControllerTest, start_actionOpenFileByApp){
    EXPECT_NO_FATAL_FAILURE(controller->actionOpenFileByApp());
    QAction *myAction = new QAction("actionNametests");
    myAction->connect(myAction,&QAction::triggered,controller,
                      &AppController::actionOpenFileByApp);

    stl.set_lamda(&DFileService::openFileByApp,[](){return false;});
    bool (*openFilesByApplamda)(const QString &, const QStringList &) = []
            (const QString &, const QStringList &){
        return true;
    };
    stl.set(&FileUtils::openFilesByApp,openFilesByApplamda);
    stl.set_lamda(&DFileService::pasteFile,[](){return DUrlList();});
    DUrl url1,url2;
    url1.setScheme(TAG_SCHEME);
    url1.setPath(TestHelper::createTmpFile());

    myAction->setProperty("url",QVariant::fromValue(url1));
    myAction->setProperty("app",QVariant::fromValue(
                              QString("deepin-editor")));

    emit myAction->triggered();

    myAction->setProperty("urls",QVariant::fromValue(DUrlList() << url1 << url2));
    emit myAction->triggered();
    myAction->disconnect();
    myAction->deleteLater();
    TestHelper::deleteTmpFile(url1.toLocalFile());
}

TEST_F(AppControllerTest, start_actionSendToRemovableDisk){
    EXPECT_NO_FATAL_FAILURE(controller->actionSendToRemovableDisk());
    QAction *myAction = new QAction("actionNametests");
    myAction->connect(myAction,&QAction::triggered,controller,
                      &AppController::actionSendToRemovableDisk);
    QVariant targetUrl("test_appcon");
    myAction->setProperty("mounted_root_uri",targetUrl);
    DUrl url1,url2;
    url1.setScheme(TAG_SCHEME);
    url1.setPath(TestHelper::createTmpFile());
    myAction->setProperty("urlList",QVariant::fromValue(QStringList() <<
                                                        url1.toString()
                                                        << url2.toString()));
    myAction->setProperty("blkDevice",QVariant("sr0"));

    stl.set_lamda(&DFileService::pasteFile,[](){return DUrlList();});
    emit myAction->triggered();
    TestHelper::runInLoop([](){});
    myAction->disconnect();
    myAction->deleteLater();
    TestHelper::deleteTmpFile(url1.toLocalFile());
}

TEST_F(AppControllerTest, start_actionStageFileForBurning){
    EXPECT_NO_FATAL_FAILURE(controller->actionStageFileForBurning());

    QAction *myAction = new QAction("actionNametests");
    myAction->connect(myAction,&QAction::triggered,controller,
                      &AppController::actionStageFileForBurning);
    stl.set_lamda(&DDiskDevice::eject,[](){});
    stl.set_lamda(&DFileService::pasteFile,[](){return DUrlList();});
    myAction->setProperty("dest_drive",QVariant("sr0"));
    DUrl url1,url2;
    url1.setScheme(TAG_SCHEME);
    url1.setPath(TestHelper::createTmpFile());
    myAction->setProperty("urlList",QVariant::fromValue(
                              QStringList() << url1.toString() << url2.toString()));
    emit myAction->triggered();
    TestHelper::runInLoop([](){});

    stl.set_lamda(&DBlockDevice::drive,[](){return "sr0";});
    stl.set_lamda(&DDiskDevice::optical,[](){return true;});
    emit myAction->triggered();
    TestHelper::runInLoop([](){});
    myAction->deleteLater();
    TestHelper::deleteTmpFile(url1.toLocalFile());
}

TEST_F(AppControllerTest, start_actionGetTagsThroughFiles){
    QList<QString> (*getTagsThroughFileslamda)(const QObject *, const QList<DUrl> &, const bool) = []
            (const QObject *, const QList<DUrl> &, const bool){
        QList<QString> tt;
        return tt;
    };
    stl.set(ADDR(DFileService,getTagsThroughFiles),getTagsThroughFileslamda);
    QStringList list = controller->actionGetTagsThroughFiles(dMakeEventPointer<DFMGetTagsThroughFilesEvent>(nullptr,DUrlList() << url));
    EXPECT_TRUE(list.isEmpty());

    bool (*removeTagsOfFilelamda)(const QObject *, const DUrl &, const QList<QString> &) = []
            (const QObject *, const DUrl &, const QList<QString> &){
        return true;
    };
    stl.set(ADDR(DFileService,removeTagsOfFile),removeTagsOfFilelamda);
    EXPECT_FALSE(controller->actionRemoveTagsOfFile(dMakeEventPointer<DFMRemoveTagsOfFileEvent>(nullptr,url, list)));
    bool (*changeTagColorlamda)(const QString &, const QString &) = []
            (const QString &, const QString &){
        return true;
    };
    stl.set((bool(TagManager::*)(const QString &, const QString &))ADDR(TagManager,changeTagColor),changeTagColorlamda);
    QString (*getColorNameByColorlamda)(const QColor &) = [](const QColor &){
        return QString();
    };
    stl.set(ADDR(TagManager,getColorNameByColor),getColorNameByColorlamda);
    controller->actionChangeTagColor(dMakeEventPointer<DFMChangeTagColorEvent>(nullptr,Qt::red, url));

    stub_ext::StubExt stub; //在不影响原有覆盖率下打桩
    stub.set_lamda(&AppController::showTagEdit, [=](){
        QRect parentRect(0,0,0,0);
        QPoint globalPos(0,0);
        DUrlList fileList{url};
        DTagEdit *tagEdit = new DTagEdit();
        auto subValue = parentRect.height() - globalPos.y();
        if (subValue < 98) {
            tagEdit->setArrowDirection(DArrowRectangle::ArrowDirection::ArrowBottom);
        }
        tagEdit->setBaseSize(160, 98);
        tagEdit->setFilesForTagging(fileList);
        tagEdit->setFocusOutSelfClosing(true);
        QList<QString> sameTagsInDiffFiles{ DFileService::instance()->getTagsThroughFiles(nullptr, fileList) };
        tagEdit->setDefaultCrumbs(sameTagsInDiffFiles);
        tagEdit->show(globalPos.x(), globalPos.y());
        tagEdit->close();
        if (tagEdit) {
            delete tagEdit;
            tagEdit = nullptr;
        }
    });

    controller->showTagEdit(QRect(0,0,0,0),QPoint(0,0),DUrlList() << url);
    QApplication::closeAllWindows();
}

TEST_F(AppControllerTest, start_doSubscriberAction){
    url.setScheme(FILE_SCHEME);
    url.setPath("~/Music");
    if (!controller->m_fmEvent ) {
        controller->m_fmEvent = dMakeEventPointer<DFMUrlListBaseEvent>(nullptr,DUrlList() << url);
    }
    void (*asyncOpenDisklamda)(const QString &) = [](const QString &){};
    stl.set(ADDR(AppController,asyncOpenDisk),asyncOpenDisklamda);
    void (*asyncOpenDiskInNewWindowlamda)(const QString &) = [](const QString &){};
    stl.set(ADDR(AppController,asyncOpenDiskInNewWindow),asyncOpenDiskInNewWindowlamda);
    void (*asyncOpenDiskInNewTablamda)(const QString &) = [](const QString &){};
    stl.set(ADDR(AppController,asyncOpenDiskInNewTab),asyncOpenDiskInNewTablamda);
    EXPECT_NO_FATAL_FAILURE(controller->setEventKey(AppController::Open));
    EXPECT_NO_FATAL_FAILURE(controller->doSubscriberAction(url.toString()));
    EXPECT_NO_FATAL_FAILURE(controller->setEventKey(AppController::OpenNewWindow));
    EXPECT_NO_FATAL_FAILURE(controller->doSubscriberAction(url.toString()));
    EXPECT_NO_FATAL_FAILURE(controller->setEventKey(AppController::OpenNewTab));
    EXPECT_NO_FATAL_FAILURE(controller->doSubscriberAction(url.toString()));
    EXPECT_NO_FATAL_FAILURE(controller->setEventKey(static_cast<AppController::ActionType>(4)));
    EXPECT_NO_FATAL_FAILURE(controller->doSubscriberAction(url.toString()));
}

TEST_F(AppControllerTest, start_createGVfSManager){
    EXPECT_NO_FATAL_FAILURE(controller->createGVfSManager());
}

TEST_F(AppControllerTest, start_createUserShareManager){
    EXPECT_NO_FATAL_FAILURE(controller->createUserShareManager());
}

TEST_F(AppControllerTest, start_createDBusInterface){
    EXPECT_NO_FATAL_FAILURE(controller->createDBusInterface());
}

TEST_F(AppControllerTest, start_showErrorDialog){
    EXPECT_NO_FATAL_FAILURE(controller->showErrorDialog("test","tet"));
    TestHelper::runInLoop([=](){
    });
}

TEST_F(AppControllerTest, start_actionRemoveStashedMount) {
    RemoteMountsStashManager::stashRemoteMount("/run/user/1000/gvfs/smb-share:server=1.2.3.4,share=test", "test");

    auto e = dMakeEventPointer<DFMUrlBaseEvent>(nullptr, DUrl("dfmroot:///smb://1.2.3.4/test.remote"));
    EXPECT_NO_FATAL_FAILURE(controller->actionRemoveStashedMount(e));

    // cover RemoteMountsStashManager
    RemoteMountsStashManager::stashRemoteMount("/run/user/1000/gvfs/smb-share:server=1.2.3.4,share=test", "test");
    auto mounts = RemoteMountsStashManager::remoteMounts();
    EXPECT_TRUE(1 == mounts.size());
    EXPECT_TRUE("test" == RemoteMountsStashManager::getDisplayNameByConnUrl("smb://1.2.3.4/test"));
    EXPECT_TRUE("smb://1.2.3.4/test" == RemoteMountsStashManager::normalizeConnUrl("/smb://1.2.3.4/test.remote"));
    EXPECT_NO_FATAL_FAILURE(RemoteMountsStashManager::clearRemoteMounts());
    EXPECT_NO_FATAL_FAILURE(RemoteMountsStashManager::stashCurrentMounts());
}
