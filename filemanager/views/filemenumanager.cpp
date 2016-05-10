#include "filemenumanager.h"
#include "dfilemenu.h"

#include "../app/global.h"
#include "../app/filesignalmanager.h"
#include "../app/fmevent.h"
#include "../shutil/fileutils.h"

#include "../controllers/fileservices.h"

#include "../dialogs/propertydialog.h"
#include "../views/windowmanager.h"
#include "../shutil/standardpath.h"
#include "../controllers/filejob.h"
#include "../controllers/bookmarkmanager.h"
#include <QFileDialog>
#include <QDesktopServices>
#include <QApplication>
#include <QMimeData>
#include <QClipboard>
#include <QScreen>

QMap<MenuAction, QString> FileMenuManager::m_actionKeys;
QMap<MenuAction, DAction*> FileMenuManager::m_actions;
QVector<MenuAction> FileMenuManager::m_sortActionTypes;


DFileMenu *FileMenuManager::createRecentLeftBarMenu(const QVector<MenuAction> &disableList)
{
    QVector<MenuAction> actionKeys;

    actionKeys.reserve(2);

    actionKeys << MenuAction::OpenInNewWindow
               << MenuAction::ClearRecent
               << MenuAction::Property;

    return genereteMenuByKeys(actionKeys, disableList);
}

DFileMenu *FileMenuManager::createDefaultBookMarkMenu()
{
    QVector<MenuAction> actionKeys;
    QVector<MenuAction> disableList;

    actionKeys.reserve(7);

    actionKeys << MenuAction::OpenInNewWindow
               << MenuAction::Property;

    return genereteMenuByKeys(actionKeys, disableList);
}

DFileMenu *FileMenuManager::createCustomBookMarkMenu(const QVector<MenuAction> &disableList)
{
    QVector<MenuAction> actionKeys;

    actionKeys.reserve(10);

    actionKeys << MenuAction::OpenInNewWindow
               << MenuAction::Rename
               << MenuAction::Remove
               << MenuAction::Property;

    return genereteMenuByKeys(actionKeys, disableList);
}

DFileMenu *FileMenuManager::createTrashLeftBarMenu(const QVector<MenuAction> &disableList)
{
    QVector<MenuAction> actionKeys;

    actionKeys.reserve(4);

    actionKeys << MenuAction::OpenInNewWindow
               << MenuAction::ClearTrash
               << MenuAction::Property;

    return genereteMenuByKeys(actionKeys, disableList);
}

DFileMenu *FileMenuManager::createDiskLeftBarMenu(const QVector<MenuAction> &disableList)
{
    QVector<MenuAction> actionKeys;

    actionKeys.reserve(4);

    actionKeys << MenuAction::Open
               << MenuAction::OpenInNewWindow
               << MenuAction::Separator
               << MenuAction::Property;

    return genereteMenuByKeys(actionKeys, disableList);
}

DFileMenu *FileMenuManager::createDiskViewMenu(const QVector<MenuAction> &disableList)
{
    QVector<MenuAction> actionKeys;

    actionKeys.reserve(6);

    actionKeys << MenuAction::Open << MenuAction::OpenInNewWindow
               << MenuAction::Separator
               << MenuAction::Mount << MenuAction::Unmount
               << MenuAction::Separator
               << MenuAction::Property;

    return genereteMenuByKeys(actionKeys, disableList);
}

DFileMenu *FileMenuManager::createToolBarSettingsMenu(const QVector<MenuAction> &disableList)
{
    QVector<MenuAction> actionKeys;

    actionKeys.reserve(5);

    actionKeys << MenuAction::NewWindow
               << MenuAction::Separator
               << MenuAction::Settings
               << MenuAction::Help
               << MenuAction::About
               << MenuAction::Exit;

    return genereteMenuByKeys(actionKeys, disableList);
}

DFileMenu *FileMenuManager::createToolBarSortMenu(const QVector<MenuAction> &disableList)
{
    QVector<MenuAction> actionKeys;

    actionKeys.reserve(5);

    actionKeys << MenuAction::Name << MenuAction::Size
               << MenuAction::Type
               << MenuAction::CreatedDate
               << MenuAction::LastModifiedDate;
    m_sortActionTypes = actionKeys;
    DFileMenu *menu = genereteMenuByKeys(actionKeys, disableList, true);
    return menu;
}

DFileMenu *FileMenuManager::createListViewHeaderMenu(const QVector<MenuAction> &disableList)
{
    QVector<MenuAction> actionKeys;

    actionKeys.reserve(4);

    actionKeys << MenuAction::Size
               << MenuAction::Type
               << MenuAction::CreatedDate
               << MenuAction::LastModifiedDate;

    DFileMenu *menu = genereteMenuByKeys(actionKeys, disableList, true);
    return menu;
}

QVector<MenuAction> FileMenuManager::getDisableActionList(const DUrl &fileUrl)
{
    const AbstractFileInfoPointer &fileInfo = fileService->createFileInfo(fileUrl);
    QVector<MenuAction> disableList;
    const AbstractFileInfoPointer &parentInfo = fileService->createFileInfo(fileInfo->parentUrl());


    const QClipboard *clipboard = qApp->clipboard();
    const QMimeData *mimeData = clipboard->mimeData();

    qDebug() << mimeData->hasUrls() << mimeData->urls();
    if (!mimeData->hasUrls()){
        disableList << MenuAction::Paste;
    }

    return disableList;
}

FileMenuManager::FileMenuManager()
{
    qRegisterMetaType<QMap<QString, QString>>("QMap<QString, QString>");
    qRegisterMetaType<QList<QUrl>>("QList<QUrl>");
}

void FileMenuManager::initData()
{
    m_actionKeys[MenuAction::Open] = QObject::tr("Open");
    m_actionKeys[MenuAction::OpenInNewWindow] = QObject::tr("Open in new window");
    m_actionKeys[MenuAction::OpenWith] = QObject::tr("Open with");
    m_actionKeys[MenuAction::OpenWithCustom] = QObject::tr("Others");
    m_actionKeys[MenuAction::OpenFileLocation] = QObject::tr("Open file loaction");
    m_actionKeys[MenuAction::Compress] = QObject::tr("Compress");
    m_actionKeys[MenuAction::Decompress] = QObject::tr("Decompress");
    m_actionKeys[MenuAction::DecompressHere] = QObject::tr("Decompress to here");
    m_actionKeys[MenuAction::Cut] = QObject::tr("Cut");
    m_actionKeys[MenuAction::Copy] = QObject::tr("Copy");
    m_actionKeys[MenuAction::Paste] = QObject::tr("Paste");
    m_actionKeys[MenuAction::Rename] = QObject::tr("Rename");
    m_actionKeys[MenuAction::Remove] = QObject::tr("Remove");
    m_actionKeys[MenuAction::CreateSoftLink] = QObject::tr("Create link");
    m_actionKeys[MenuAction::SendToDesktop] = QObject::tr("Send to desktop");
    m_actionKeys[MenuAction::AddToBookMark] = QObject::tr("Add to bookmark");
    m_actionKeys[MenuAction::Delete] = QObject::tr("Throw to trash");
    m_actionKeys[MenuAction::CompleteDeletion] = QObject::tr("Complete deletion");
    m_actionKeys[MenuAction::Property] = QObject::tr("Property");

    m_actionKeys[MenuAction::NewFolder] = QObject::tr("New Folder");
    m_actionKeys[MenuAction::NewFile] = QObject::tr("New File");
    m_actionKeys[MenuAction::NewWindow] = QObject::tr("New Window");
    m_actionKeys[MenuAction::SelectAll] = QObject::tr("Select All");
    m_actionKeys[MenuAction::ClearRecent] = QObject::tr("Clear Recent");
    m_actionKeys[MenuAction::ClearTrash] = QObject::tr("Clear Trash");
    m_actionKeys[MenuAction::DisplayAs] = QObject::tr("Display As");
    m_actionKeys[MenuAction::SortBy] = QObject::tr("Sort By");
    m_actionKeys[MenuAction::NewDocument] = QObject::tr("New Document");
    m_actionKeys[MenuAction::NewWord] = QObject::tr("Word");
    m_actionKeys[MenuAction::NewExcel] = QObject::tr("Excel");
    m_actionKeys[MenuAction::NewPowerpoint] = QObject::tr("PowerPoint");
    m_actionKeys[MenuAction::NewText] = QObject::tr("Text");
    m_actionKeys[MenuAction::OpenInTerminal] = QObject::tr("Open in terminal");
    m_actionKeys[MenuAction::Restore] = QObject::tr("Restore");
    m_actionKeys[MenuAction::RestoreAll] = QObject::tr("Restore all");
    m_actionKeys[MenuAction::CompleteDeletion] = QObject::tr("Complete Deletion");
    m_actionKeys[MenuAction::Mount] = QObject::tr("Mount");
    m_actionKeys[MenuAction::Unmount]= QObject::tr("Unmount");
    m_actionKeys[MenuAction::Name] = QObject::tr("Name");
    m_actionKeys[MenuAction::Size] = QObject::tr("Size");
    m_actionKeys[MenuAction::Type] = QObject::tr("Type");
    m_actionKeys[MenuAction::CreatedDate] = QObject::tr("Created Date");
    m_actionKeys[MenuAction::LastModifiedDate] = QObject::tr("Last Modified Date");
    m_actionKeys[MenuAction::Settings] = QObject::tr("Settings");
    m_actionKeys[MenuAction::Help] = QObject::tr("Help");
    m_actionKeys[MenuAction::About] = QObject::tr("About");
    m_actionKeys[MenuAction::Exit] = QObject::tr("Exit");
    m_actionKeys[MenuAction::IconView] = QObject::tr("Icon");
    m_actionKeys[MenuAction::ListView] = QObject::tr("List");
    m_actionKeys[MenuAction::ExtendView] = QObject::tr("Extend");
    m_actionKeys[MenuAction::SetAsWallpaper] = QObject::tr("Set as wallpaper");
}

void FileMenuManager::initActions()
{
    foreach (MenuAction key, m_actionKeys.keys()) {
        DAction* action = new DAction(m_actionKeys.value(key), 0);
        action->setData(key);
        m_actions.insert(key, action);
    }
}

DFileMenu *FileMenuManager::genereteMenuByKeys(const QVector<MenuAction> &keys,
                                               const QVector<MenuAction> &disableList,
                                               bool checkable,
                                               const QMap<MenuAction, QVector<MenuAction> > &subMenuList)
{
    if(m_actions.isEmpty()) {
        initData();
        initActions();
    }

    DFileMenu* menu = new DFileMenu;

    connect(menu, &DFileMenu::triggered, fileMenuManger, &FileMenuManager::actionTriggered);

    foreach (MenuAction key, keys) {
        if (key == MenuAction::Separator){
            menu->addSeparator();
        }else{
            DAction *action = m_actions.value(key);

            if(!action)
                continue;

            action->setCheckable(true);
            action->setDisabled(disableList.contains(key));

            menu->addAction(action);

            if(!subMenuList.contains(key))
                continue;

            DFileMenu *subMenu = genereteMenuByKeys(subMenuList.value(key), disableList, checkable);

            subMenu->setParent(action);
            action->setMenu(subMenu);
        }
    }

    return menu;
}

QString FileMenuManager::getActionString(MenuAction type)
{
    if (m_actionKeys.contains(type)){
        return m_actionKeys.value(type);
    }
    return "";
}

void FileMenuManager::actionTriggered(DAction *action)
{

    DFileMenu *menu = qobject_cast<DFileMenu *>(sender());
    DUrlList urls = menu->getUrls();

    if (action->data().isValid()){

        bool flag = false;
        int _type = action->data().toInt(&flag);
        MenuAction type;
        if (flag){
            type = (MenuAction)_type;
        }else{
            qDebug() << action->data().toString();;
            return;
        }
        DUrl fileUrl;

        if(urls.size() > 0)
            fileUrl = urls.first();

        switch(type)
        {
        case MenuAction::Open: {
            FMEvent event;
            event = fileUrl;
            event = FMEvent::Menu;
            event = menu->getWindowId();
            qDebug() << event;
            fileService->openUrl(event);
            break;
        }
        case MenuAction::OpenInNewWindow:
            fileService->openNewWindow(fileUrl);
            break;
        case MenuAction::OpenWithCustom:{
            FMEvent event;

            event = fileUrl;
            event = FMEvent::Menu;
            event = menu->getWindowId();
            emit fileSignalManager->requestShowOpenWithDialog(event);
            break;
        }
        case MenuAction::OpenFileLocation:
            fileUrl.setScheme(FILE_SCHEME);
            fileService->openFileLocation(fileUrl);
            break;
        case MenuAction::Compress:
            fileService->compressFiles(urls);
            break;
        case MenuAction::Decompress:
            fileService->decompressFile(fileUrl);
            break;
        case MenuAction::DecompressHere:
            fileService->decompressFileHere(fileUrl);
            break;
        case MenuAction::Cut:
            fileService->cutFiles(urls);
            break;
        case MenuAction::Copy:
            fileService->copyFiles(urls);
            break;
        case MenuAction::Paste: {
            FMEvent event;

            event = fileUrl;
            event = FMEvent::Menu;
            event = menu->getWindowId();

            fileService->pasteFile(event);
            break;
        }
        case MenuAction::Rename: {
            FMEvent event;

            event = fileUrl;
            event = FMEvent::Menu;
            event = menu->getWindowId();

            emit fileSignalManager->requestRename(event);
            break;
        }
        case MenuAction::Remove: {
            if(fileUrl.isLocalFile()) {
                FMEvent event;

                event = fileUrl;
                event = FMEvent::Menu;
                event = menu->getWindowId();
                fileSignalManager->requestBookmarkRemove(event);
            } else if(fileUrl.isRecentFile()) {
                fileSignalManager->requestRecentFileRemove(urls);
            }
            break;
        }
        case MenuAction::Delete:
            fileService->moveToTrash(urls);
            break;
        case MenuAction::CompleteDeletion:
            fileService->deleteFiles(urls);
            break;
        case MenuAction::CreateSoftLink:{
            FileMenuManager::createSoftLink(menu->getWindowId(), fileUrl.toLocalFile(), QDir::homePath());
            break;
        }
        case MenuAction::SendToDesktop:{
            FileMenuManager::createSoftLink(menu->getWindowId(), fileUrl.toLocalFile(), StandardPath::getDesktopPath());
            break;
        }
        case MenuAction::AddToBookMark:{
            QString dirName = QDir(fileUrl.toLocalFile()).dirName();
            bookmarkManager->writeIntoBookmark(0, dirName, fileUrl);
            FMEvent event;
            event = menu->getWindowId();
            event = fileUrl;
            event = FMEvent::Menu;
            qDebug() << event << dirName;
            emit fileSignalManager->requestBookmarkAdd(dirName, event);
            break;
        }
        case MenuAction::NewFolder:{
            FMEvent event;

            event = fileUrl;
            event = FMEvent::Menu;
            event = menu->getWindowId();
            fileService->newFolder(event);
            break;
        }
        case MenuAction::NewFile:
            fileService->newFile(fileUrl);
            break;
        case MenuAction::SelectAll:
            fileSignalManager->requestViewSelectAll(menu->getWindowId());
            break;
        case MenuAction::ClearRecent:
            fileSignalManager->requestClearRecent();
            break;
        case MenuAction::ClearTrash:
            fileService->deleteFiles(DUrlList() << DUrl::fromLocalFile(TRASHPATH));
            break;
        case MenuAction::DisplayAs:break;
        case MenuAction::SortBy:break;
        case MenuAction::NewDocument:break;

        case MenuAction::NewWord:{
            QString targetFile = FileUtils::newDocmentName(fileUrl.toLocalFile(), QObject::tr("newDoc"), "doc");
            FileJob::SelectedFiles.insert(DUrl::fromLocalFile(targetFile), menu->getWindowId());
            FileUtils::cpTemplateFileToTargetDir(fileUrl.toLocalFile(), QObject::tr("newDoc"), "doc");
            break;
        }
        case MenuAction::NewExcel:{
            QString targetFile = FileUtils::newDocmentName(fileUrl.toLocalFile(), QObject::tr("newExcel"), "xls");
            FileJob::SelectedFiles.insert(DUrl::fromLocalFile(targetFile), menu->getWindowId());
            FileUtils::cpTemplateFileToTargetDir(fileUrl.toLocalFile(), QObject::tr("newExcel"), "xls");
            break;
        }
        case MenuAction::NewPowerpoint:{
            QString targetFile = FileUtils::newDocmentName(fileUrl.toLocalFile(), QObject::tr("newPowerPoint"), "ppt");
            FileJob::SelectedFiles.insert(DUrl::fromLocalFile(targetFile), menu->getWindowId());
            FileUtils::cpTemplateFileToTargetDir(fileUrl.toLocalFile(), QObject::tr("newPowerPoint"), "ppt");
            break;
        }
        case MenuAction::NewText:{
            QString targetFile = FileUtils::newDocmentName(fileUrl.toLocalFile(), QObject::tr("newText"), "txt");
            FileJob::SelectedFiles.insert(DUrl::fromLocalFile(targetFile), menu->getWindowId());
            FileUtils::cpTemplateFileToTargetDir(fileUrl.toLocalFile(), QObject::tr("newText"), "txt");
            break;
        }
        case MenuAction::Mount:
            deviceListener->mount(fileUrl.query());
            break;
        case MenuAction::Unmount:
            deviceListener->unmount(fileUrl.query());
            break;
        case MenuAction::Restore:
            fileSignalManager->requestRestoreTrashFile(urls);
            break;
        case MenuAction::OpenInTerminal:{
            QStringList args;
            args << QString("--working-directory=%1").arg(fileUrl.toLocalFile());
            QProcess::startDetached("x-terminal-emulator", args);
            break;
        }
        case MenuAction::Property: {
            FMEvent event;
            event = fileUrl;
            event = FMEvent::Menu;
            event = menu->getWindowId();
            emit fileSignalManager->requestShowPropertyDialog(event);
            break;
        }
        case MenuAction::NewWindow: {
            fileService->openNewWindow(fileUrl);
            break;
        }
        case MenuAction::Help:break;
        case MenuAction::About:break;
        case MenuAction::Exit:
            emit fileSignalManager->aboutToCloseLastActivedWindow(menu->getWindowId());
            break;
        case MenuAction::SetAsWallpaper:
            FileUtils::setBackground(fileUrl.toLocalFile());
            break;
        default:
            qDebug() << "unknown action type";
            break;
        }
    }
}

void FileMenuManager::createSoftLink(int windowId, const QString &file, const QString &targetDir)
{
    QFileInfo  info(file);
    QString baseName = info.baseName();
    QString linkBaseName;
    if (info.isFile()){
        linkBaseName = QString("%1 link.%2").arg(baseName, info.suffix());
    }else if (info.isDir()){
        linkBaseName = QString("%1 link").arg(baseName);
    }else if (info.isSymLink()){
        return;
    }
    QString linkFile = QString("%1/%2").arg(targetDir, linkBaseName);
    if (targetDir == StandardPath::getDesktopPath()){
        QFile(file).link(linkFile);
    }else{
        QString fileName = QFileDialog::getSaveFileName(WindowManager::getWindowById(windowId), QObject::tr("Create soft link"),
                                   linkFile);
        QFile(file).link(fileName);
    }
}
