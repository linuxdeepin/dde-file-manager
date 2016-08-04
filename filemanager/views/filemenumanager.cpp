#include "filemenumanager.h"
#include "dfilemenu.h"

#include "../app/global.h"
#include "../app/filesignalmanager.h"
#include "../app/fmevent.h"
#include "../app/filemanagerapp.h"

#include "../shutil/fileutils.h"
#include "../shutil/standardpath.h"

#include "../dialogs/propertydialog.h"

#include "../views/windowmanager.h"

#include "../controllers/fileservices.h"
#include "../controllers/filejob.h"
#include "../controllers/bookmarkmanager.h"
#include "../controllers/appcontroller.h"
#include "../controllers/trashmanager.h"

#include "widgets/singleton.h"

#include <QFileDialog>
#include <QDesktopServices>
#include <QApplication>
#include <QMimeData>
#include <QClipboard>
#include <QScreen>
#include <QMetaObject>
#include <QMetaEnum>

QMap<MenuAction, QString> FileMenuManager::m_actionKeys;
QMap<MenuAction, DAction*> FileMenuManager::m_actions;
QVector<MenuAction> FileMenuManager::m_sortActionTypes;


DFileMenu *FileMenuManager::createRecentLeftBarMenu(const QSet<MenuAction> &disableList)
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
    QSet<MenuAction> disableList;

    actionKeys.reserve(2);

    actionKeys << MenuAction::OpenInNewWindow
               << MenuAction::Property;

    return genereteMenuByKeys(actionKeys, disableList);
}

DFileMenu *FileMenuManager::createNetworkMarkMenu()
{
    QVector<MenuAction> actionKeys;
    QSet<MenuAction> disableList;

    actionKeys.reserve(1);

    actionKeys << MenuAction::OpenInNewWindow;

    return genereteMenuByKeys(actionKeys, disableList);
}

DFileMenu *FileMenuManager::createCustomBookMarkMenu(const DUrl &url, const QSet<MenuAction> &disableList)
{
    Q_UNUSED(disableList)

    QVector<MenuAction> actionKeys;
    QSet<MenuAction> disableActionKeys;

    actionKeys.reserve(10);

    actionKeys << MenuAction::OpenInNewWindow
               << MenuAction::Rename
               << MenuAction::Remove
               << MenuAction::Property;

    if (!QDir(url.path()).exists()){
        disableActionKeys << MenuAction::OpenInNewWindow
                          << MenuAction::Rename
                          << MenuAction::Property;
    }

    return genereteMenuByKeys(actionKeys, disableActionKeys);
}

DFileMenu *FileMenuManager::createTrashLeftBarMenu(const QSet<MenuAction> &disableList)
{
    QVector<MenuAction> actionKeys;

    actionKeys.reserve(4);

    actionKeys << MenuAction::OpenInNewWindow;
    actionKeys << MenuAction::ClearTrash;
    actionKeys<< MenuAction::Property;

    if (TrashManager::isEmpty()){
        QSet<MenuAction> tmp_disableList = disableList;

        tmp_disableList << MenuAction::ClearTrash;

        return genereteMenuByKeys(actionKeys, tmp_disableList);
    }

    return genereteMenuByKeys(actionKeys, disableList);
}

DFileMenu *FileMenuManager::createDiskLeftBarMenu(const QSet<MenuAction> &disableList)
{
    QVector<MenuAction> actionKeys;

    actionKeys.reserve(4);

    actionKeys << MenuAction::Open
               << MenuAction::OpenInNewWindow
               << MenuAction::Separator
               << MenuAction::Property;

    return genereteMenuByKeys(actionKeys, disableList);
}

DFileMenu *FileMenuManager::createDiskViewMenu(const QSet<MenuAction> &disableList)
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

DFileMenu *FileMenuManager::createToolBarSettingsMenu(const QSet<MenuAction> &disableList)
{
    QVector<MenuAction> actionKeys;

    actionKeys.reserve(5);

    actionKeys << MenuAction::NewWindow
               << MenuAction::Separator;
//               << MenuAction::Settings
    #ifndef ARCH_MIPSEL
        actionKeys  << MenuAction::Help;
    #endif
    actionKeys  << MenuAction::About
                << MenuAction::Exit;

    return genereteMenuByKeys(actionKeys, disableList);
}

DFileMenu *FileMenuManager::createToolBarSortMenu(const QSet<MenuAction> &disableList)
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

DFileMenu *FileMenuManager::createListViewHeaderMenu(const QSet<MenuAction> &disableList)
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

QSet<MenuAction> FileMenuManager::getDisableActionList(const DUrl &fileUrl)
{
    DUrlList list;

    list << fileUrl;

    return getDisableActionList(list);
}

QSet<MenuAction> FileMenuManager::getDisableActionList(const DUrlList &urlList)
{
    QSet<MenuAction> disableList;

    for (const DUrl &fileUrl : urlList) {
        const AbstractFileInfoPointer &fileInfo = fileService->createFileInfo(fileUrl);

        if (fileInfo) {
            disableList += fileInfo->disableMenuActionList();
        }
    }

    const QClipboard *clipboard = qApp->clipboard();
    const QByteArray &data = clipboard->mimeData()->data("x-special/gnome-copied-files");

    if (!data.startsWith("cut\n") && !data.startsWith("copy\n")){
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
    m_actionKeys[MenuAction::OpenDisk] = QObject::tr("Open");
    m_actionKeys[MenuAction::OpenDiskInNewWindow] = QObject::tr("Open in new window");
    m_actionKeys[MenuAction::OpenWith] = QObject::tr("Open with");
    m_actionKeys[MenuAction::OpenWithCustom] = QObject::tr("Others");
    m_actionKeys[MenuAction::OpenFileLocation] = QObject::tr("Open file loaction");
    m_actionKeys[MenuAction::Compress] = QObject::tr("Compress");
    m_actionKeys[MenuAction::Decompress] = QObject::tr("Extract");
    m_actionKeys[MenuAction::DecompressHere] = QObject::tr("Extract here");
    m_actionKeys[MenuAction::Cut] = QObject::tr("Cut");
    m_actionKeys[MenuAction::Copy] = QObject::tr("Copy");
    m_actionKeys[MenuAction::Paste] = QObject::tr("Paste");
    m_actionKeys[MenuAction::Rename] = QObject::tr("Rename");
    m_actionKeys[MenuAction::Remove] = QObject::tr("Remove");
    m_actionKeys[MenuAction::CreateSymlink] = QObject::tr("Create link");
    m_actionKeys[MenuAction::SendToDesktop] = QObject::tr("Send to desktop");
    m_actionKeys[MenuAction::AddToBookMark] = QObject::tr("Add to bookmark");
    m_actionKeys[MenuAction::Delete] = QObject::tr("Throw to Trash");
    m_actionKeys[MenuAction::CompleteDeletion] = QObject::tr("Permanently delete");
    m_actionKeys[MenuAction::Property] = QObject::tr("Property");

    m_actionKeys[MenuAction::NewFolder] = QObject::tr("New folder");
    m_actionKeys[MenuAction::NewWindow] = QObject::tr("New window");
    m_actionKeys[MenuAction::SelectAll] = QObject::tr("Select all");
    m_actionKeys[MenuAction::ClearRecent] = QObject::tr("Clear recent history");
    m_actionKeys[MenuAction::ClearTrash] = QObject::tr("Empty Trash");
    m_actionKeys[MenuAction::DisplayAs] = QObject::tr("Display as");
    m_actionKeys[MenuAction::SortBy] = QObject::tr("Sort by");
    m_actionKeys[MenuAction::NewDocument] = QObject::tr("New document");
    m_actionKeys[MenuAction::NewWord] = QObject::tr("Word");
    m_actionKeys[MenuAction::NewExcel] = QObject::tr("Excel");
    m_actionKeys[MenuAction::NewPowerpoint] = QObject::tr("PowerPoint");
    m_actionKeys[MenuAction::NewText] = QObject::tr("Text");
    m_actionKeys[MenuAction::OpenInTerminal] = QObject::tr("Open in terminal");
    m_actionKeys[MenuAction::Restore] = QObject::tr("Restore");
    m_actionKeys[MenuAction::RestoreAll] = QObject::tr("Restore all");
    m_actionKeys[MenuAction::Mount] = QObject::tr("Mount");
    m_actionKeys[MenuAction::Unmount]= QObject::tr("Unmount");
    m_actionKeys[MenuAction::Eject]= QObject::tr("Eject");
    m_actionKeys[MenuAction::Name] = QObject::tr("Name");
    m_actionKeys[MenuAction::Size] = QObject::tr("Size");
    m_actionKeys[MenuAction::Type] = QObject::tr("Type");
    m_actionKeys[MenuAction::CreatedDate] = QObject::tr("Time created");
    m_actionKeys[MenuAction::LastModifiedDate] = QObject::tr("Time modified");
    m_actionKeys[MenuAction::Settings] = QObject::tr("Settings");
    m_actionKeys[MenuAction::Help] = QObject::tr("Help");
    m_actionKeys[MenuAction::About] = QObject::tr("About");
    m_actionKeys[MenuAction::Exit] = QObject::tr("Exit");
    m_actionKeys[MenuAction::IconView] = QObject::tr("Icon");
    m_actionKeys[MenuAction::ListView] = QObject::tr("List");
    m_actionKeys[MenuAction::ExtendView] = QObject::tr("Extend");
    m_actionKeys[MenuAction::SetAsWallpaper] = QObject::tr("Set as wallpaper");
    m_actionKeys[MenuAction::ForgetPassword] = QObject::tr("Log out and unmount");
    m_actionKeys[MenuAction::DeletionDate] = QObject::tr("Time deleted");
    m_actionKeys[MenuAction::SourcePath] = QObject::tr("Source path");
    m_actionKeys[MenuAction::AbsolutePath] = QObject::tr("Path");
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
                                               const QSet<MenuAction> &disableList,
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
    FMEvent event = menu->event();
    event = FMEvent::Menu;
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

        QMetaEnum metaEnum = QMetaEnum::fromType<MenuAction>();
        QString key = metaEnum.valueToKey(type);
        QString methodKey = QString("action%1").arg(key);
        QString methodSignature = QString("action%1(FMEvent)").arg(key);
        std::string methodString = methodKey.toStdString();
        std::string methodSignatureString = methodSignature.toStdString();
        const char* methodName = methodString.c_str();
        const char* methodSignatureName = methodSignatureString.c_str();

        const QMetaObject* metaObject = appController->metaObject();
//        QStringList methods;
//        for(int i = metaObject->methodOffset(); i < metaObject->methodCount(); ++i){
//            methods << QString::fromLatin1(metaObject->method(i).methodSignature());
//        }
//        qDebug() << methods;
//        qDebug() << methodKey << methodName;
        if (metaObject->indexOfSlot(methodSignatureName) != -1){
            QMetaObject::invokeMethod(appController,
                                      methodName,
                                      Qt::DirectConnection,
                                      Q_ARG(FMEvent, event));
        }else{
            qWarning() << "Appcontroller has no method:" << methodSignature;
        }
    }
}
