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

#include <ddiskmanager.h>
#include <dblockdevice.h>
#include "deviceinfo/udisklistener.h"
#include "dabstractfilewatcher.h"
#include "ddialog.h"
#define private public
#define protected public
#include "tag/tagmanager.h"
#include "tests/testhelper.h"
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
            DFMEventFuture (*processEventAsync)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *) = []
                    (const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *) {
                return DFMEventFuture(QFuture<QVariant>());
            };

            stl.set((DFMEventFuture(DFMEventDispatcher::*)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *))\
                    ADDR(DFMEventDispatcher,processEventAsync),processEventAsync);
            void (*showNewWindow)(const DUrl &, const bool &) = [](const DUrl &, const bool &){return;};
            stl.set(ADDR(WindowManager,showNewWindow),showNewWindow);
            bool (*cd)(const DUrl &) = [](const DUrl &){return false;};
            stl.set(ADDR(DFileManagerWindow,cd),cd);
            void (*show)(void *) = [](void *){};
            stl.set(ADDR(QWidget,show),show);

            void (*showErrorDialog)(const QString &, const QString &) = []
                    (const QString &, const QString &){};
            stl.set(ADDR(DialogManager,showErrorDialog),showErrorDialog);

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
            fptr pDDialogExec = (fptr)(&DDialog::exec);
            int (*stub_DDialog_exec)(void) = [](void)->int{return QDialog::Rejected;};
            stl.set(pQDialogExec, stub_DDialog_exec);
            stl.set(pDDialogExec, stub_DDialog_exec);

            void (*mount)(const QString &) = [](const QString &){};
            stl.set(ADDR(UDiskListener,mount),mount);
            void (*unmount)(const QString &) = [](const QString &){};
            stl.set(ADDR(UDiskListener,unmount),unmount);

            controller = AppController::instance();
            QString fileName = QString::number(QDateTime::currentDateTime().toMSecsSinceEpoch());
            tempFilePath =  QStandardPaths::standardLocations(QStandardPaths::TempLocation).first() + "/" + fileName;
        }
        void TearDown()
        {
            QProcess::execute("rm -f " + tempFilePath);
        }
    };
}

TEST_F(AppControllerTest,start_registerUrlHandle){
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

    bool (*openFiles)(const QObject *, const DUrlList &, const bool ) = []
            (const QObject *, const DUrlList &, const bool ){return true;};
    stl.set(ADDR(DFileService,openFiles),openFiles);
    bool (*openFile)(const QObject *, const DUrl &) = []
            (const QObject *, const DUrl &){return true;};
    stl.set(ADDR(DFileService,openFiles),openFile);
    url.setScheme(FILE_SCHEME);
    url.setPath("~/Music");
    EXPECT_NO_FATAL_FAILURE(controller->actionOpen(dMakeEventPointer<DFMUrlListBaseEvent>(nullptr,DUrlList())));
    EXPECT_NO_FATAL_FAILURE(controller->actionOpen(dMakeEventPointer<DFMUrlListBaseEvent>(nullptr,DUrlList() << url)));

    DUrl recenturl,searchurl,burnurl;
    recenturl.setScheme(RECENT_SCHEME);
    searchurl.setScheme(SEARCH_SCHEME);
    burnurl.setScheme(BURN_SCHEME);
    DAbstractFileInfoPointer (*createFileInfo)(const QObject *, const DUrl &) = [](const QObject *, const DUrl &){
        return DAbstractFileInfoPointer();
    };
    stl.set(ADDR(DFileService,createFileInfo),createFileInfo);
    bool (*isVaultFile)(void *) = [](void *){return true;};
    stl.set(ADDR(DUrl,isVaultFile),isVaultFile);
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

    DAbstractFileInfoPointer (*createFileInfo2)(const QObject *, const DUrl &) = [](const QObject *, const DUrl &){
        return DAbstractFileInfoPointer(new testFileInfo1(DUrl()));
    };
    stl.set(ADDR(DFileService,createFileInfo),createFileInfo2);
    EXPECT_NO_FATAL_FAILURE(controller->actionOpen(dMakeEventPointer<DFMUrlListBaseEvent>(nullptr,DUrlList() << url << burnurl)));
    stl.reset(ADDR(DFileService,createFileInfo));
    stl.reset(ADDR(DUrl,isVaultFile));

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

    DAbstractFileInfoPointer (*createFileInfo1)(void *,const QObject *, const DUrl &) = [](void *,const QObject *, const DUrl &url){
        return DAbstractFileInfoPointer(new testFileInfo(url));
    };
    stl.set(ADDR(DFileService,createFileInfo),createFileInfo1);
    EXPECT_NO_FATAL_FAILURE(controller->actionOpen(dMakeEventPointer<DFMUrlListBaseEvent>(nullptr,DUrlList() << url)));
}

TEST_F(AppControllerTest,start_actionOpenDisk){
    url.setScheme(FILE_SCHEME);
    url.setPath("~/Music");
    Stub stl;
    void (*actionOpen)(const QSharedPointer<DFMUrlListBaseEvent> &, const bool) = []
            (const QSharedPointer<DFMUrlListBaseEvent> &, const bool){};
    stl.set(ADDR(AppController,actionOpen),actionOpen);
    DUrl rooturl("dfmroot:///sdb1.localdisk");
    QString (*scheme)(void *) = [](void *){
        return QString(DFMROOT_SCHEME);
    };
    stl.set(ADDR(DAbstractFileInfo,scheme),scheme);
    EXPECT_NO_FATAL_FAILURE(controller->actionOpenDisk(
                                dMakeEventPointer<DFMUrlBaseEvent>(nullptr, url)));
    stl.reset(ADDR(DAbstractFileInfo,scheme));

    QString (*scheme1)(void *) = [](void *){
        return QString(BURN_SCHEME);
    };
    stl.set(ADDR(QUrl,scheme),scheme1);
    QString (*burnDestDevice)(void *) = [](void *){ return QString("oo");};
    stl.set(ADDR(DUrl,burnDestDevice),burnDestDevice);

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

    DAbstractFileInfoPointer (*createFileInfo1)(void *,const QObject *, const DUrl &) = [](void *,const QObject *, const DUrl &url){
        return DAbstractFileInfoPointer(new testFileInfo(url));
    };
    stl.set(ADDR(DFileService,createFileInfo),createFileInfo1);

    QStringList (*resolveDeviceNode)(QString, QVariantMap) = [](QString, QVariantMap){
        return QStringList() << QString("ooo");
    };
    stl.set(ADDR(DDiskManager,resolveDeviceNode),resolveDeviceNode);

    QByteArrayList (*mountPoints)(void *)= [](void *){
        return QByteArrayList() << QByteArray("oooo");
    };
    stl.set(ADDR(DBlockDevice,mountPoints),mountPoints) ;

    EXPECT_NO_FATAL_FAILURE(controller->actionOpenDisk(
                                dMakeEventPointer<DFMUrlBaseEvent>(nullptr, url)));
    stl.reset(ADDR(QUrl,scheme));
    stl.reset(ADDR(DUrl,burnDestDevice));
    stl.reset(ADDR(DDiskManager,resolveDeviceNode));
    stl.reset(ADDR(DBlockDevice,mountPoints));

    EXPECT_NO_FATAL_FAILURE(controller->actionOpenDisk(
                                dMakeEventPointer<DFMUrlBaseEvent>(nullptr, url)));
    stl.reset(ADDR(DFileService,createFileInfo));
    EXPECT_NO_FATAL_FAILURE(controller->actionOpenDisk(
                                dMakeEventPointer<DFMUrlBaseEvent>(nullptr, DUrl())));
    void (*actionMount)(const QSharedPointer<DFMUrlBaseEvent> &) = [](const QSharedPointer<DFMUrlBaseEvent> &){};
    stl.set(ADDR(AppController,actionMount),actionMount);

    EXPECT_NO_FATAL_FAILURE(controller->actionOpenDisk(
                                dMakeEventPointer<DFMUrlBaseEvent>(nullptr, rooturl)));

}

TEST_F(AppControllerTest,start_asyncOpenDisk){
    url.setScheme(FILE_SCHEME);
    url.setPath("~/Music");
    if (!controller->m_fmEvent ) {
        controller->m_fmEvent = dMakeEventPointer<DFMUrlListBaseEvent>(nullptr,DUrlList() << url);
    }
    void (*actionOpen)(const QSharedPointer<DFMUrlListBaseEvent> &, const bool) = []
            (const QSharedPointer<DFMUrlListBaseEvent> &, const bool){};
    stl.set(ADDR(AppController,actionOpen),actionOpen);
    EXPECT_NO_FATAL_FAILURE(controller->asyncOpenDisk(url.toString()));
}

TEST_F(AppControllerTest,start_actionOpenInNewWindow){
    url.setScheme(FILE_SCHEME);
    url.setPath("~/Music");
    void (*actionMount)(const QSharedPointer<DFMUrlBaseEvent> &) = [](const QSharedPointer<DFMUrlBaseEvent> &){};
    stl.set(ADDR(AppController,actionMount),actionMount);
    QVariant (*processEvent)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *) = []
            (const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *) {
        return QVariant();
    };
    stl.set((QVariant(DFMEventDispatcher::*)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *))\
            ADDR(DFMEventDispatcher,processEvent),processEvent);

    EXPECT_NO_FATAL_FAILURE(controller->actionOpenInNewWindow(dMakeEventPointer<DFMUrlListBaseEvent>(nullptr, DUrlList() << url)));
    DUrl temp;
    temp.setScheme(FILE_SCHEME);
    temp.setPath("~/Videos");
    EXPECT_NO_FATAL_FAILURE(controller->actionOpenInNewTab(dMakeEventPointer<DFMUrlBaseEvent>(nullptr,  url)));
}

TEST_F(AppControllerTest,start_actionOpenDiskInNewWindow){
    url.setScheme(FILE_SCHEME);
    url.setPath("~/Videos");
    void (*actionMount)(const QSharedPointer<DFMUrlBaseEvent> &) = [](const QSharedPointer<DFMUrlBaseEvent> &){};
     stl.set(ADDR(AppController,actionMount),actionMount);
    EXPECT_NO_FATAL_FAILURE(controller->actionOpenDiskInNewWindow(dMakeEventPointer<DFMUrlBaseEvent>(nullptr, url)));
    EXPECT_NO_FATAL_FAILURE(controller->actionOpenDiskInNewWindow(dMakeEventPointer<DFMUrlBaseEvent>(nullptr, DUrl())));
    EXPECT_NO_FATAL_FAILURE(controller->actionOpenDiskInNewTab(dMakeEventPointer<DFMUrlBaseEvent>(nullptr, url)));
    EXPECT_NO_FATAL_FAILURE(controller->actionOpenDiskInNewTab(dMakeEventPointer<DFMUrlBaseEvent>(nullptr, DUrl())));
    QString (*scheme)(void *) = [](void *){
        return QString(DFMROOT_SCHEME);
    };
    stl.set(ADDR(DAbstractFileInfo,scheme),scheme);
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
    bool (*startDetached)(const QString &, const QStringList &) = [](const QString &, const QStringList &){
        return true;
    };
    stl.set((bool(*)(const QString &, const QStringList &))ADDR(QProcess,startDetached),startDetached);
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
    void (*showOpenFilesWithDialog)(const DFMEvent &) = [](const DFMEvent &){};
    stl.set(ADDR(DialogManager,showOpenFilesWithDialog),showOpenFilesWithDialog);
    EXPECT_NO_FATAL_FAILURE(controller->actionOpenFilesWithCustom(dMakeEventPointer<DFMUrlListBaseEvent>
                                                                  (nullptr, DUrlList() << url << temp)));
    bool (*openFileLocation)(const QObject *, const DUrl &) = [](const QObject *, const DUrl &){return true;};
    stl.set(ADDR(DFileService,openFileLocation),openFileLocation);
    EXPECT_NO_FATAL_FAILURE(controller->actionOpenFileLocation(dMakeEventPointer<DFMUrlListBaseEvent>
                                                               (nullptr, DUrlList() << url << temp)));
    TestHelper::deleteTmpFile(temp.toLocalFile());
}

TEST_F(AppControllerTest,start_actionOpenWithCustom){
    url.setScheme(FILE_SCHEME);
    url.setPath("~/Videos");
    void (*showOpenWithDialog)(const DFMEvent &) = [](const DFMEvent &){};
    stl.set(ADDR(DialogManager,showOpenWithDialog),showOpenWithDialog);
    EXPECT_NO_FATAL_FAILURE(controller->actionOpenWithCustom(dMakeEventPointer<DFMUrlBaseEvent>
                                                               (nullptr, url)));
}

TEST_F(AppControllerTest,start_actionCompressOp){
    url.setScheme(FILE_SCHEME);
    url.setPath("~/Videos");
    QString fileName = QString::number(QDateTime::currentDateTime().toMSecsSinceEpoch());
    QVariant (*processEvent)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *) = []
            (const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *) {
        return QVariant();
    };
    stl.set((QVariant(DFMEventDispatcher::*)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *))\
            ADDR(DFMEventDispatcher,processEvent),processEvent);

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
    QVariant (*processEvent)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *) = []
            (const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *) {
        return QVariant();
    };
    stl.set((QVariant(DFMEventDispatcher::*)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *))\
            ADDR(DFMEventDispatcher,processEvent),processEvent);
    EXPECT_NO_FATAL_FAILURE(controller->actionCut(dMakeEventPointer<DFMUrlListBaseEvent>
                                                  (nullptr, DUrlList() << url)));
}

TEST_F(AppControllerTest,start_actionCopy){
    url.setScheme(FILE_SCHEME);
    url.setPath("~/Videos");
    QVariant (*processEvent)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *) = []
            (const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *) {
        return QVariant();
    };
    stl.set((QVariant(DFMEventDispatcher::*)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *))\
            ADDR(DFMEventDispatcher,processEvent),processEvent);

    EXPECT_NO_FATAL_FAILURE(controller->actionCopy(dMakeEventPointer<DFMUrlListBaseEvent>
                                                   (nullptr, DUrlList() << url)));
}

TEST_F(AppControllerTest,start_actionRename1){
    url.setScheme(FILE_SCHEME);
    url.setPath("~/Videos");
    QVariant (*processEvent)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *) = []
            (const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *) {
        return QVariant();
    };
    stl.set((QVariant(DFMEventDispatcher::*)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *))\
            ADDR(DFMEventDispatcher,processEvent),processEvent);
    QString fileName = QString::number(QDateTime::currentDateTime().toMSecsSinceEpoch());
    DUrl temp;
    temp.setScheme(FILE_SCHEME);
    temp.setPath("./"+fileName+"_2.txt");
    QProcess::execute("touch " + temp.toLocalFile());
    EXPECT_NO_FATAL_FAILURE(controller->actionRename(dMakeEventPointer<DFMUrlListBaseEvent>(nullptr, DUrlList() << temp)));
    EXPECT_NO_FATAL_FAILURE(controller->actionRename(dMakeEventPointer<DFMUrlListBaseEvent>(nullptr, DUrlList() << temp << DUrl())));
    QProcess::execute("rm " + temp.toLocalFile());
}

TEST_F(AppControllerTest,start_actionBookmarkandother){
    DUrl temp;
    temp.setScheme(FILE_SCHEME);
    QString fileName = QString::number(QDateTime::currentDateTime().toMSecsSinceEpoch());
    temp.setPath("./"+fileName+".txt");
    QProcess::execute("touch " + temp.toLocalFile());
    DUrl temp1 = temp;
    temp1.setPath("./"+fileName+"_sys.txt");

    QVariant (*processEvent)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *) = []
            (const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *) {
        return QVariant();
    };
    stl.set((QVariant(DFMEventDispatcher::*)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *))\
            ADDR(DFMEventDispatcher,processEvent),processEvent);

    EXPECT_NO_FATAL_FAILURE(controller->actionAddToBookMark(dMakeEventPointer<DFMUrlBaseEvent>
                                                            (nullptr, temp)));
    EXPECT_NO_FATAL_FAILURE(controller->actionBookmarkRename(dMakeEventPointer<DFMUrlBaseEvent>
                                                             (nullptr, temp)));
    EXPECT_NO_FATAL_FAILURE(controller->actionBookmarkRemove(dMakeEventPointer<DFMUrlBaseEvent>
                                                             (nullptr, temp)));
//    DUrlList (*moveToTrash)(const QObject *, const DUrlList &) = [](const QObject *, const DUrlList &){
//        return DUrlList();
//    };
//    stl.set(ADDR(DFileService,moveToTrash),moveToTrash);
//    TestHelper::([=](){
//    EXPECT_NO_FATAL_FAILURE(controller->actionDelete(dMakeEventPointer<DFMUrlListBaseEvent>
//                                                     (nullptr, DUrlList() << temp)));
//    });
    bool (*createSymlink)(const QObject *, const DUrl &) = []
            (const QObject *, const DUrl &){
        return true;
    };
    stl.set((bool(DFileService::*)(const QObject *, const DUrl &) const)ADDR(DFileService,createSymlink),createSymlink);
    EXPECT_NO_FATAL_FAILURE(controller->actionCreateSymlink(dMakeEventPointer<DFMUrlBaseEvent>
                                                            (nullptr, temp1)));
    stl.reset((bool(DFileService::*)(const QObject *, const DUrl &) const)ADDR(DFileService,createSymlink));

    bool (*sendToDesktop)(const QObject *, const DUrlList &) = [](const QObject *, const DUrlList &){return true;};
    stl.set(ADDR(DFileService,sendToDesktop),sendToDesktop);
    EXPECT_NO_FATAL_FAILURE(controller->actionSendToDesktop(dMakeEventPointer<DFMUrlListBaseEvent>
                                                            (nullptr, DUrlList() << temp)));
    EXPECT_NO_FATAL_FAILURE(controller->actionSendToBluetooth());
    QAction action;
    action.connect(&action,&QAction::triggered,controller,&AppController::actionSendToBluetooth);
    emit action.triggered();
    void (*sendToBluetooth)(const DUrlList &) = [](const DUrlList &){};
    stl.set(ADDR(DFileService,sendToBluetooth),sendToBluetooth);
    EXPECT_NO_FATAL_FAILURE(controller->actionSendToBluetooth());
    bool (*deleteFiles)(const QObject *, const DUrlList &, bool, bool, bool) = []
            (const QObject *, const DUrlList &, bool, bool, bool){
        return true;
    };
    stl.set(ADDR(DFileService,deleteFiles),deleteFiles);
    EXPECT_NO_FATAL_FAILURE(controller->actionCompleteDeletion(dMakeEventPointer<DFMUrlListBaseEvent>
                                                               (nullptr, DUrlList())));
    TestHelper::deleteTmpFiles(QStringList() << temp.toLocalFile() << temp1.toLocalFile());
}

TEST_F(AppControllerTest,start_actionNewFolder){
    url.setScheme(FILE_SCHEME);
    url.setPath("./ooooooooooo");
    QVariant (*processEvent)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *) = []
            (const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *) {
        return QVariant();
    };
    stl.set((QVariant(DFMEventDispatcher::*)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *))\
            ADDR(DFMEventDispatcher,processEvent),processEvent);
    DUrl (*newDocumentUrl)(const DAbstractFileInfoPointer, const QString &, const QString &) = []
            (const DAbstractFileInfoPointer, const QString &, const QString &){return DUrl();};
    stl.set(ADDR(FileUtils,newDocumentUrl),newDocumentUrl);
    EXPECT_NO_FATAL_FAILURE(controller->actionNewFolder(dMakeEventPointer<DFMUrlBaseEvent>(nullptr, url)));
    EXPECT_NO_FATAL_FAILURE(controller->actionSelectAll(29489851231423));
    EXPECT_NO_FATAL_FAILURE(controller->actionClearRecent(dMakeEventPointer<DFMMenuActionEvent>
                                                          (nullptr,nullptr,DUrl(),DUrlList(),DFMGlobal::ClearRecent,QModelIndex())));
    QProcess::execute("cp "+ QDir::homePath() + "/.local/share/recently-used.xbel " + QDir::homePath() + "/.local/share/recently-used.xbel.bak");
    EXPECT_NO_FATAL_FAILURE(controller->actionClearRecent());
    QProcess::execute("mv "+ QDir::homePath() + "/.local/share/recently-used.xbel.bak " + QDir::homePath() + "/.local/share/recently-used.xbel");
    QProcess::execute("rm -rf " + url.toLocalFile());
    DFileService::instance()->setDoClearTrashState(true);
    bool (*deleteFiles)(const QObject *, const DUrlList &, bool, bool, bool) = []
            (const QObject *, const DUrlList &, bool, bool, bool){
        return true;
    };
    stl.set(ADDR(DFileService,deleteFiles),deleteFiles);
    EXPECT_NO_FATAL_FAILURE(controller->actionClearTrash(nullptr));
    DFileService::instance()->setDoClearTrashState(false);
    EXPECT_NO_FATAL_FAILURE(controller->actionClearTrash(nullptr));
    DFileService::instance()->setDoClearTrashState(false);

}

TEST_F(AppControllerTest,start_actionNewFile){
    url.setScheme(FILE_SCHEME);
    url.setPath("./ztt_test_app.txt");
    DUrl tmp(url);
    void (*mount)(const QString &) = [](const QString &){};
    stl.set(ADDR(UDiskListener,mount),mount);
    void (*unmount)(const QString &) = [](const QString &){};
    stl.set(ADDR(UDiskListener,unmount),unmount);
    typedef int(*fptr)(QDialog*);
    fptr pQDialogExec = (fptr)(&QDialog::exec);
    fptr pDDialogExec = (fptr)(&DDialog::exec);
    int (*stub_DDialog_exec)(void) = [](void)->int{return QDialog::Rejected;};
    stl.set(pQDialogExec, stub_DDialog_exec);
    stl.set(pDDialogExec, stub_DDialog_exec);
    tmp.setPath(TestHelper::createTmpDir());
    url.setPath(tmp.toLocalFile() + "/ztt_test_appkkk");
    bool (*cpTemplateFileToTargetDir)(const QString &, const QString &, const QString &, WId ) =
            [](const QString &, const QString &, const QString &, WId ){return true;};
    stl.set(ADDR(FileUtils,cpTemplateFileToTargetDir),cpTemplateFileToTargetDir);
    EXPECT_NO_FATAL_FAILURE(controller->actionNewText(dMakeEventPointer<DFMUrlBaseEvent>(nullptr,url)));
    EXPECT_NO_FATAL_FAILURE(controller->actionNewWord(dMakeEventPointer<DFMUrlBaseEvent>(nullptr,url)));
    EXPECT_NO_FATAL_FAILURE(controller->actionNewExcel(dMakeEventPointer<DFMUrlBaseEvent>(nullptr,url)));
    EXPECT_NO_FATAL_FAILURE(controller->actionNewPowerpoint(dMakeEventPointer<DFMUrlBaseEvent>(nullptr,url)));
    QVariant (*processEventWithEventLoop)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *) = []
            (const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *){
            return QVariant();
    };
    stl.set((QVariant (DFMEventDispatcher::*)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *))
            ADDR(DFMEventDispatcher,processEventWithEventLoop),processEventWithEventLoop);
    EXPECT_NO_FATAL_FAILURE(controller->actionRestore(dMakeEventPointer<DFMUrlListBaseEvent>(nullptr,DUrlList() << url)));


    EXPECT_NO_FATAL_FAILURE(controller->actionRestoreAll(dMakeEventPointer<DFMUrlBaseEvent>(nullptr,url)));
    stl.reset((QVariant (DFMEventDispatcher::*)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *))
            ADDR(DFMEventDispatcher,processEventWithEventLoop));
    bool (*optical_stub)(void *obj) = [](void *obj){return true;};
    stl.set(ADDR(DDiskDevice, optical), optical_stub);
    url = DUrl("dfmroot:///sda1.localdisk");
    EXPECT_NO_FATAL_FAILURE(controller->actionMount(dMakeEventPointer<DFMUrlBaseEvent>(nullptr, url)));

    url = DUrl("dfmroot:///");
    EXPECT_NO_FATAL_FAILURE(controller->actionMount(dMakeEventPointer<DFMUrlBaseEvent>(nullptr, url)));

    QString isoPath = createTestIsoFile();
    url = DUrl("file://" + isoPath);
    void (*actionOpen)(const QSharedPointer<DFMUrlListBaseEvent> &, const bool) = []
            (const QSharedPointer<DFMUrlListBaseEvent> &, const bool){};
    stl.set(ADDR(AppController,actionOpen),actionOpen);

    TestHelper::runInLoop([=](){
        EXPECT_NO_FATAL_FAILURE(controller->actionMountImage(dMakeEventPointer<DFMUrlBaseEvent>(nullptr, url)));
        EXPECT_NO_FATAL_FAILURE(controller->actionMountImage(dMakeEventPointer<DFMUrlBaseEvent>
                                                             (nullptr, DUrl("file:///Empty.iso"))));
    },1000);
    QProcess::execute("rm", QStringList() << isoPath);

    url = DUrl("dfmroot:///fakeDisk.gvfsmp");
    EXPECT_NO_FATAL_FAILURE(controller->actionUnmount(dMakeEventPointer<DFMUrlBaseEvent>(nullptr, url)));
    url = DUrl("dfmroot:///sda1.localdisk");
    EXPECT_NO_FATAL_FAILURE(controller->actionUnmount(dMakeEventPointer<DFMUrlBaseEvent>(nullptr, url)));
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
    QVariant (*processEvent)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *) = []
            (const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *) {
        return QVariant();
    };
    stl.set((QVariant(DFMEventDispatcher::*)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *))\
            ADDR(DFMEventDispatcher,processEvent),processEvent);
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
    void (*showTrashPropertyDialog)(const DFMEvent &) = [](const DFMEvent &){};
    stl.set(ADDR(DialogManager,showTrashPropertyDialog),showTrashPropertyDialog);
    void (*showComputerPropertyDialog)(void *) = [](void*){};
    stl.set(ADDR(DialogManager,showComputerPropertyDialog),showComputerPropertyDialog);
    void (*showPropertyDialog)(const DFMUrlListBaseEvent &) = [](const DFMUrlListBaseEvent &){};
    stl.set(ADDR(DialogManager,showPropertyDialog),showPropertyDialog);
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
    QVariant (*processEvent)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *) = []
            (const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *) {
        return QVariant();
    };
    stl.set((QVariant(DFMEventDispatcher::*)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *))\
            ADDR(DFMEventDispatcher,processEvent),processEvent);
    EXPECT_NO_FATAL_FAILURE(controller->actionNewWindow(dMakeEventPointer<DFMUrlListBaseEvent>
                                                        (nullptr,DUrlList() << url)));

}

TEST_F(AppControllerTest, start_actionExit){
    void (*onLastActivedWindowClosed)(quint64) = [](quint64){};
    stl.set(ADDR(WindowManager,onLastActivedWindowClosed),onLastActivedWindowClosed);
    EXPECT_NO_FATAL_FAILURE(controller->actionExit(53561231655));
}

TEST_F(AppControllerTest, start_actionSetAsWallpaper){
    url.setScheme(FILE_SCHEME);
    url.setPath("~/Pictures/Pictures/Wallpapers/abc-123.jpg");
    bool (*setBackground)(const QString &) = [](const QString &){return true;};
    stl.set(ADDR(FileUtils,setBackground),setBackground);
    EXPECT_NO_FATAL_FAILURE(controller->actionSetAsWallpaper(dMakeEventPointer<DFMUrlBaseEvent>(nullptr,url)));
    bool (*isLocalFile)(void *) = [](void *){return true;};
    stl.set(ADDR(DUrl,isLocalFile),isLocalFile);
    EXPECT_NO_FATAL_FAILURE(controller->actionSetAsWallpaper(dMakeEventPointer<DFMUrlBaseEvent>(nullptr,url)));
}

TEST_F(AppControllerTest, start_actionShare){
    url.setScheme(FILE_SCHEME);
    url.setPath("~/ut_share_test");
    QProcess::execute("mkdir " + url.toLocalFile());
    void (*showShareOptionsInPropertyDialog)(const DFMUrlListBaseEvent &) = []
            (const DFMUrlListBaseEvent &){};
    stl.set(ADDR(DialogManager,showShareOptionsInPropertyDialog),showShareOptionsInPropertyDialog);
    EXPECT_NO_FATAL_FAILURE(controller->actionShare(dMakeEventPointer<DFMUrlListBaseEvent>(nullptr,DUrlList() << url)));
}

TEST_F(AppControllerTest, start_actionUnShare){
    url.setScheme(FILE_SCHEME);
    url.setPath("~/ut_share_test");
    QVariant (*processEvent)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *) = []
            (const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *) {
        return QVariant();
    };
    stl.set((QVariant(DFMEventDispatcher::*)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *))\
            ADDR(DFMEventDispatcher,processEvent),processEvent);
    EXPECT_NO_FATAL_FAILURE(controller->actionUnShare(dMakeEventPointer<DFMUrlBaseEvent>(nullptr,url)));
    TestHelper::deleteTmpFile(url.toLocalFile());
}

TEST_F(AppControllerTest, start_actionConnectToServer){
    void (*showConnectToServerDialog)(quint64) = [](quint64){};
    stl.set(ADDR(DialogManager,showConnectToServerDialog),showConnectToServerDialog);
    EXPECT_NO_FATAL_FAILURE(controller->actionConnectToServer(
                                static_cast<quint64>(QDateTime::currentDateTime().toMSecsSinceEpoch())));
}

TEST_F(AppControllerTest, start_actionSetUserSharePassword){
    void (*showUserSharePasswordSettingDialog)(quint64) = [](quint64){};
    stl.set(ADDR(DialogManager,showUserSharePasswordSettingDialog),showUserSharePasswordSettingDialog);
    EXPECT_NO_FATAL_FAILURE(controller->actionSetUserSharePassword(static_cast<quint64>
                                                                   (QDateTime::currentDateTime().toMSecsSinceEpoch())));
}

TEST_F(AppControllerTest, start_actionSettings){
    void (*showGlobalSettingsDialog)(quint64) = [](quint64){};
    stl.set(ADDR(DialogManager,showGlobalSettingsDialog),showGlobalSettingsDialog);
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
    void (*handleHotkeyCtrlL)(quint64 ) = [](quint64 ){};
    stl.set(ADDR(DToolBar,handleHotkeyCtrlL),handleHotkeyCtrlL);
    EXPECT_NO_FATAL_FAILURE(controller->actionctrlL(static_cast<quint64>
                                                    (QDateTime::currentDateTime().toMSecsSinceEpoch())));
}

TEST_F(AppControllerTest, start_actionctrlF){
    void (*handleHotkeyCtrlF)(quint64 ) = [](quint64 ){};
    stl.set(ADDR(DToolBar,handleHotkeyCtrlF),handleHotkeyCtrlF);
    EXPECT_NO_FATAL_FAILURE(controller->actionctrlF(static_cast<quint64>
                                                    (QDateTime::currentDateTime().toMSecsSinceEpoch())));
}
QWidget *getWindowById(quint64 winId) {
    return new QWidget();
}
TEST_F(AppControllerTest, start_actionExitCurrentWindow){
    stl.set(ADDR(WindowManager,getWindowById),&getWindowById);
    EXPECT_NO_FATAL_FAILURE(controller->actionExitCurrentWindow(static_cast<quint64>
                                                                (QDateTime::currentDateTime().toMSecsSinceEpoch())));
}

TEST_F(AppControllerTest, start_actionShowHotkeyHelp){

    EXPECT_NO_FATAL_FAILURE(controller->actionShowHotkeyHelp(static_cast<quint64>
                                                             (QDateTime::currentDateTime().toMSecsSinceEpoch())));
    stl.set(ADDR(WindowManager,getWindowById),&getWindowById);
    bool (*startDetached)(const QString &, const QStringList &) = []
            (const QString &, const QStringList &){return true;};
    stl.set((bool (*)(const QString &, const QStringList &))ADDR(QProcess,startDetached),startDetached);
    EXPECT_NO_FATAL_FAILURE(controller->actionShowHotkeyHelp(static_cast<quint64>
                                                             (QDateTime::currentDateTime().toMSecsSinceEpoch())));
}

TEST_F(AppControllerTest, start_actionBack){
    QVariant (*processEvent)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *) = []
            (const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *) {
        return QVariant();
    };
    stl.set((QVariant(DFMEventDispatcher::*)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *))\
            ADDR(DFMEventDispatcher,processEvent),processEvent);
    EXPECT_NO_FATAL_FAILURE(controller->actionBack(static_cast<quint64>
                                                   (QDateTime::currentDateTime().toMSecsSinceEpoch())));
}

TEST_F(AppControllerTest, start_actionForward){
    QVariant (*processEvent)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *) = []
            (const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *) {
        return QVariant();
    };
    stl.set((QVariant(DFMEventDispatcher::*)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *))\
            ADDR(DFMEventDispatcher,processEvent),processEvent);
    EXPECT_NO_FATAL_FAILURE(controller->actionForward(
                                static_cast<quint64>(QDateTime::currentDateTime().toMSecsSinceEpoch())));
}

TEST_F(AppControllerTest, start_actionForgetPassword){
    void (*actionUnmount)(const QSharedPointer<DFMUrlBaseEvent> &) = [](const QSharedPointer<DFMUrlBaseEvent> &){};
    stl.set(ADDR(AppController,actionUnmount),actionUnmount);
    void (*clearPasswordByLoginObj)(const QJsonObject &) = [](const QJsonObject &){};
    stl.set(ADDR(SecretManager,clearPasswordByLoginObj),clearPasswordByLoginObj);
    url.setUrl("dfmroot:///%252Frun%252Fuser%252F1000%252Fgvfs%252Fsmb-share%253Aserver%253D10.8.12.125%252Cshare%253D%25E9%25");
    EXPECT_NO_FATAL_FAILURE(controller->actionForgetPassword(dMakeEventPointer<DFMUrlBaseEvent>(nullptr,url)));
}

TEST_F(AppControllerTest, start_actionOpenFileByApp){
    EXPECT_NO_FATAL_FAILURE(controller->actionOpenFileByApp());
    QAction *myAction = new QAction("actionNametests");
    myAction->connect(myAction,&QAction::triggered,controller,
                      &AppController::actionOpenFileByApp);

    stl.set_lamda(&DFileService::openFileByApp,[](){return false;});
    bool (*openFilesByApp)(const QString &, const QStringList &) = []
            (const QString &, const QStringList &){
        return true;
    };
    stl.set(&FileUtils::openFilesByApp,openFilesByApp);
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
    QList<QString> (*getTagsThroughFiles)(const QObject *, const QList<DUrl> &, const bool) = []
            (const QObject *, const QList<DUrl> &, const bool){
        QList<QString> tt;
        return tt;
    };
    stl.set(ADDR(DFileService,getTagsThroughFiles),getTagsThroughFiles);
    QStringList list = controller->actionGetTagsThroughFiles(dMakeEventPointer<DFMGetTagsThroughFilesEvent>(nullptr,DUrlList() << url));
    EXPECT_TRUE(list.isEmpty());

    bool (*removeTagsOfFile)(const QObject *, const DUrl &, const QList<QString> &) = []
            (const QObject *, const DUrl &, const QList<QString> &){
        return true;
    };
    stl.set(ADDR(DFileService,removeTagsOfFile),removeTagsOfFile);
    EXPECT_FALSE(controller->actionRemoveTagsOfFile(dMakeEventPointer<DFMRemoveTagsOfFileEvent>(nullptr,url, list)));
    bool (*changeTagColor)(const QString &, const QString &) = []
            (const QString &, const QString &){
        return true;
    };
    stl.set((bool(TagManager::*)(const QString &, const QString &))ADDR(TagManager,changeTagColor),changeTagColor);
    QString (*getColorNameByColor)(const QColor &) = [](const QColor &){
        return QString();
    };
    stl.set(ADDR(TagManager,getColorNameByColor),getColorNameByColor);
    controller->actionChangeTagColor(dMakeEventPointer<DFMChangeTagColorEvent>(nullptr,Qt::red, url));
    controller->showTagEdit(QRect(0,0,0,0),QPoint(0,0),DUrlList() << url);
}

TEST_F(AppControllerTest, start_doSubscriberAction){
    url.setScheme(FILE_SCHEME);
    url.setPath("~/Music");
    if (!controller->m_fmEvent ) {
        controller->m_fmEvent = dMakeEventPointer<DFMUrlListBaseEvent>(nullptr,DUrlList() << url);
    }
    void (*asyncOpenDisk)(const QString &) = [](const QString &){};
    stl.set(ADDR(AppController,asyncOpenDisk),asyncOpenDisk);
    void (*asyncOpenDiskInNewWindow)(const QString &) = [](const QString &){};
    stl.set(ADDR(AppController,asyncOpenDiskInNewWindow),asyncOpenDiskInNewWindow);
    void (*asyncOpenDiskInNewTab)(const QString &) = [](const QString &){};
    stl.set(ADDR(AppController,asyncOpenDiskInNewTab),asyncOpenDiskInNewTab);
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
