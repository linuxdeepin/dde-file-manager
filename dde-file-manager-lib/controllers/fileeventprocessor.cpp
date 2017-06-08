/**
 * Copyright (C) 2017 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/
#include "fileeventprocessor.h"
#include "dfmevent.h"
#include "dabstractfileinfo.h"
#include "dfileservices.h"
#include "dfmeventdispatcher.h"

#include "appcontroller.h"
#include "views/windowmanager.h"
#include "views/dfilemanagerwindow.h"
#include "shutil/fileutils.h"
#include "singleton.h"
#include "dfmsetting.h"
#include "app/define.h"

#include <QProcess>

DFM_BEGIN_NAMESPACE

static FileEventProcessor *eventProcessor = new FileEventProcessor();

FileEventProcessor::FileEventProcessor()
{

}

static bool isAvfsMounted()
{
    QProcess p;
    QString cmd = "/bin/bash";
    QStringList args;
    args << "-c" << "ps -ax -o 'cmd'|grep '.avfs$'";
    p.start(cmd, args);
    p.waitForFinished();
    QString avfsBase = qgetenv("AVFSBASE");
    QString avfsdDir;
    if(avfsBase.isEmpty()){
        QString home = qgetenv("HOME");
        avfsdDir = home + "/.avfs";
    } else{
        avfsdDir = avfsBase + "/.avfs";
    }

    while (!p.atEnd()) {
        QString result = p.readLine().trimmed();
        if(!result.isEmpty()){
            QStringList datas = result.split(" ");

            if(datas.count() == 2){
                //compare current user's avfs path
                if(datas.last() != avfsdDir)
                    continue;

                if(datas.first() == "avfsd" && QFile::exists(datas.last()))
                    return true;
            }
        }
    }

    return false;
}

static bool processMenuEvent(const QSharedPointer<DFMMenuActionEvent> &event)
{
    switch ((int)event->action()) {
    case DFMGlobal::Open:
        AppController::instance()->actionOpen(dMakeEventPointer<DFMUrlListBaseEvent>(event->sender(), event->selectedUrls()));
        break;
    case DFMGlobal::OpenDisk:
        AppController::instance()->actionOpenDisk(dMakeEventPointer<DFMUrlBaseEvent>(event->sender(), event->selectedUrls().first()));
        break;
    case DFMGlobal::OpenInNewWindow:
        AppController::instance()->actionOpenInNewWindow(dMakeEventPointer<DFMUrlListBaseEvent>(event->sender(), event->selectedUrls()));
        break;
    case DFMGlobal::OpenInNewTab:
        AppController::instance()->actionOpenInNewTab(dMakeEventPointer<DFMUrlBaseEvent>(event->sender(), event->selectedUrls().first()));
        break;
    case DFMGlobal::OpenDiskInNewWindow:
        AppController::instance()->actionOpenDiskInNewWindow(dMakeEventPointer<DFMUrlBaseEvent>(event->sender(), event->selectedUrls().first()));
        break;
    case DFMGlobal::OpenDiskInNewTab:
        AppController::instance()->actionOpenDiskInNewTab(dMakeEventPointer<DFMUrlBaseEvent>(event->sender(), event->selectedUrls().first()));
        break;
    case DFMGlobal::OpenAsAdmin:
        AppController::instance()->actionOpenAsAdmin(dMakeEventPointer<DFMUrlBaseEvent>(event->sender(), event->selectedUrls().isEmpty() ? event->currentUrl() : event->selectedUrls().first()));
        break;
    case DFMGlobal::OpenWithCustom:
        AppController::instance()->actionOpenWithCustom(dMakeEventPointer<DFMUrlBaseEvent>(event->sender(), event->selectedUrls().first()));
        break;
    case DFMGlobal::OpenFileLocation:
        AppController::instance()->actionOpenFileLocation(dMakeEventPointer<DFMUrlListBaseEvent>(event->sender(), event->selectedUrls()));
        break;
    case DFMGlobal::Compress:
        AppController::instance()->actionCompress(dMakeEventPointer<DFMUrlListBaseEvent>(event->sender(), event->selectedUrls()));
        break;
    case DFMGlobal::Decompress:
        AppController::instance()->actionDecompress(dMakeEventPointer<DFMUrlListBaseEvent>(event->sender(), event->selectedUrls()));
        break;
    case DFMGlobal::DecompressHere:
        AppController::instance()->actionDecompressHere(dMakeEventPointer<DFMUrlListBaseEvent>(event->sender(), event->selectedUrls()));
        break;
    case DFMGlobal::Cut:
        AppController::instance()->actionCut(dMakeEventPointer<DFMUrlListBaseEvent>(event->sender(), event->selectedUrls()));
        break;
    case DFMGlobal::Copy:
        AppController::instance()->actionCopy(dMakeEventPointer<DFMUrlListBaseEvent>(event->sender(), event->selectedUrls()));
        break;
    case DFMGlobal::Paste:
        AppController::instance()->actionPaste(dMakeEventPointer<DFMUrlBaseEvent>(event->sender(), event->currentUrl()));
        break;
    case DFMGlobal::Rename:
        AppController::instance()->actionRename(dMakeEventPointer<DFMUrlBaseEvent>(event->sender(), event->selectedUrls().first()));
        break;
    case DFMGlobal::BookmarkRename:
        AppController::instance()->actionBookmarkRename(dMakeEventPointer<DFMUrlBaseEvent>(event->sender(), event->selectedUrls().first()));
        break;
    case DFMGlobal::BookmarkRemove:
        AppController::instance()->actionBookmarkRemove(dMakeEventPointer<DFMUrlBaseEvent>(event->sender(), event->selectedUrls().first()));
        break;
    case DFMGlobal::CreateSymlink:
        AppController::instance()->actionCreateSymlink(dMakeEventPointer<DFMUrlBaseEvent>(event->sender(), event->selectedUrls().first()));
        break;
    case DFMGlobal::SendToDesktop:
        AppController::instance()->actionSendToDesktop(dMakeEventPointer<DFMUrlListBaseEvent>(event->sender(), event->selectedUrls()));
        break;
    case DFMGlobal::AddToBookMark:
        AppController::instance()->actionAddToBookMark(dMakeEventPointer<DFMUrlBaseEvent>(event->sender(), event->selectedUrls().first()));
        break;
    case DFMGlobal::Delete:
        AppController::instance()->actionDelete(dMakeEventPointer<DFMUrlListBaseEvent>(event->sender(), event->selectedUrls()));
        break;
    case DFMGlobal::Property:
        AppController::instance()->actionProperty(dMakeEventPointer<DFMUrlListBaseEvent>(event->sender(), event->selectedUrls().isEmpty() ? DUrlList() << event->currentUrl() : event->selectedUrls()));
        break;
    case DFMGlobal::NewFolder:
        AppController::instance()->actionNewFolder(dMakeEventPointer<DFMUrlBaseEvent>(event->sender(), event->currentUrl()));
        break;
    case DFMGlobal::NewWindow: {
        DUrlList urlList = event->selectedUrls();
        AppController::instance()->actionNewWindow(dMakeEventPointer<DFMUrlListBaseEvent>(event->sender(), urlList));
        break;
    }
    case DFMGlobal::SelectAll:
        AppController::instance()->actionSelectAll(event->windowId());
        break;
    case DFMGlobal::ClearRecent:
        AppController::instance()->actionClearRecent(event);
        break;
    case DFMGlobal::ClearTrash:
        AppController::instance()->actionClearTrash(event->sender());
        break;
    case DFMGlobal::NewWord: /// sub menu
        AppController::instance()->actionNewWord(dMakeEventPointer<DFMUrlBaseEvent>(event->sender(), event->currentUrl()));
        break;
    case DFMGlobal::NewExcel: /// sub menu
        AppController::instance()->actionNewExcel(dMakeEventPointer<DFMUrlBaseEvent>(event->sender(), event->currentUrl()));
        break;
    case DFMGlobal::NewPowerpoint: /// sub menu
        AppController::instance()->actionNewPowerpoint(dMakeEventPointer<DFMUrlBaseEvent>(event->sender(), event->currentUrl()));
        break;
    case DFMGlobal::NewText: /// sub menu
        AppController::instance()->actionNewText(dMakeEventPointer<DFMUrlBaseEvent>(event->sender(), event->currentUrl()));
        break;
    case DFMGlobal::OpenInTerminal:
        AppController::instance()->actionOpenInTerminal(dMakeEventPointer<DFMUrlListBaseEvent>(event->sender(), event->selectedUrls().isEmpty() ? DUrlList() << event->currentUrl() : event->selectedUrls()));
        break;
    case DFMGlobal::Restore:
        AppController::instance()->actionRestore(dMakeEventPointer<DFMUrlListBaseEvent>(event->sender(), event->selectedUrls()));
        break;
    case DFMGlobal::RestoreAll:
        AppController::instance()->actionRestoreAll(dMakeEventPointer<DFMUrlBaseEvent>(event->sender(), event->currentUrl()));
        break;
    case DFMGlobal::CompleteDeletion:
        AppController::instance()->actionCompleteDeletion(dMakeEventPointer<DFMUrlListBaseEvent>(event->sender(), event->selectedUrls()));
        break;
    case DFMGlobal::Mount:
        AppController::instance()->actionMount(dMakeEventPointer<DFMUrlBaseEvent>(event->sender(), event->selectedUrls().first()));
        break;
    case DFMGlobal::Unmount:
        AppController::instance()->actionUnmount(dMakeEventPointer<DFMUrlBaseEvent>(event->sender(), event->selectedUrls().first()));
        break;
    case DFMGlobal::Eject:
        AppController::instance()->actionEject(dMakeEventPointer<DFMUrlBaseEvent>(event->sender(), event->selectedUrls().first()));
        break;
    case DFMGlobal::Settings:
        AppController::instance()->actionSettings(event->windowId());
        break;
    case DFMGlobal::Help:
        AppController::instance()->actionHelp();
        break;
    case DFMGlobal::About:
        AppController::instance()->actionAbout(event->windowId());
        break;
    case DFMGlobal::Exit:
        AppController::instance()->actionExit(event->windowId());
        break;
    case DFMGlobal::SetAsWallpaper:
        AppController::instance()->actionSetAsWallpaper(dMakeEventPointer<DFMUrlBaseEvent>(event->sender(), event->selectedUrls().first()));
        break;
    case DFMGlobal::ForgetPassword:
        AppController::instance()->actionForgetPassword(dMakeEventPointer<DFMUrlBaseEvent>(event->sender(), event->selectedUrls().first()));
        break;
    case DFMGlobal::Share:
        AppController::instance()->actionShare(dMakeEventPointer<DFMUrlListBaseEvent>(event->sender(), event->selectedUrls()));
        break;
    case DFMGlobal::UnShare:
        AppController::instance()->actionUnShare(dMakeEventPointer<DFMUrlBaseEvent>(event->sender(), event->selectedUrls().first()));
        break;
    case DFMGlobal::SetUserSharePassword:
        AppController::instance()->actionSetUserSharePassword(event->windowId());
        break;
    case DFMGlobal::FormatDevice:
        AppController::instance()->actionFormatDevice(dMakeEventPointer<DFMUrlBaseEvent>(event->sender(), event->selectedUrls().first()));
        break;
    }

    return true;
}

bool FileEventProcessor::fmEvent(const QSharedPointer<DFMEvent> &event, QVariant *resultData)
{
    switch (event->type()) {
    case DFMEvent::OpenNewWindow: {
        const QSharedPointer<DFMOpenNewWindowEvent> &e = event.staticCast<DFMOpenNewWindowEvent>();

        for (const DUrl &url : e->urlList()) {
            WindowManager::instance()->showNewWindow(url, e->force());
        }

        break;
    }
    case DFMEvent::ChangeCurrentUrl: {
        const QSharedPointer<DFMChangeCurrentUrlEvent> &e = event.staticCast<DFMChangeCurrentUrlEvent>();

        if (DFileManagerWindow *window = const_cast<DFileManagerWindow*>(qobject_cast<const DFileManagerWindow*>(e->window()))) {
            window->cd(e->fileUrl());
        }

        break;
    }
    case DFMEvent::OpenUrl: {
        const QSharedPointer<DFMOpenUrlEvent> &e = event.staticCast<DFMOpenUrlEvent>();

        //sort urls by files and dirs
        DUrlList dirList;

        foreach (DUrl url, e->urlList()) {
            const DAbstractFileInfoPointer &fileInfo = DFileService::instance()->createFileInfo(Q_NULLPTR, url);

            if (globalSetting->isCompressFilePreview()
                    && isAvfsMounted()
                    && FileUtils::isArchive(url.toLocalFile())
                    && fileInfo->mimeType().name() != "application/vnd.debian.binary-package") {
                const DAbstractFileInfoPointer &info = DFileService::instance()->createFileInfo(Q_NULLPTR, DUrl::fromAVFSFile(url.path()));

                if (info->exists()) {
                    url.setScheme(AVFS_SCHEME);
                    dirList << url;
                    continue;
                }
            }

            if (fileInfo) {
                if (fileInfo->isDir())
                    dirList << url;
                else
                    DThreadUtil::runInMainThread(DFileService::instance(), &DFileService::openFile, event->sender(), url);
            }

            //computer url is virtual dir
            if (url == DUrl::fromComputerFile("/") || url.scheme() == "mount")
                dirList << url;
        }

        if (dirList.isEmpty())
            break;

        QVariant result;

        if (e->dirOpenMode() == DFMOpenUrlEvent::OpenInCurrentWindow) {
            const QSharedPointer<DFMEvent> &newEvent = dMakeEventPointer<DFMChangeCurrentUrlEvent>(event->sender(), dirList.first(), WindowManager::getWindowById(event->windowId()));
            result = DThreadUtil::runInMainThread(DFMEventDispatcher::instance(), static_cast<QVariant(DFMEventDispatcher::*)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *)>(&DFMEventDispatcher::processEvent), newEvent, Q_NULLPTR);
        } else {
            const QSharedPointer<DFMEvent> &newEvent = dMakeEventPointer<DFMOpenNewWindowEvent>(event->sender(), dirList, e->dirOpenMode() == DFMOpenUrlEvent::ForceOpenNewWindow);
            result = DThreadUtil::runInMainThread(DFMEventDispatcher::instance(), static_cast<QVariant(DFMEventDispatcher::*)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *)>(&DFMEventDispatcher::processEvent), newEvent, Q_NULLPTR);
        }

        if (resultData)
            *resultData = result;

        break;
    }
    case DFMEvent::MenuAction:
        return processMenuEvent(event.staticCast<DFMMenuActionEvent>());
    default:
        return false;
    }

    return true;
}

DFM_END_NAMESPACE
