#include "filemenumanager.h"
#include "dfilemenu.h"

#include "../app/global.h"
#include "../app/fmevent.h"
#include "../app/filesignalmanager.h"

#include "../controllers/appcontroller.h"
#include "../controllers/filecontroller.h"
#include "../controllers/filejob.h"

QMap<FileMenuManager::MenuAction, QString> FileMenuManager::m_actionKeys;
QMap<FileMenuManager::MenuAction, DAction*> FileMenuManager::m_actions;
QList<QString> FileMenuManager::m_cutItems;

DFileMenu *FileMenuManager::createFileMenu(const QVector<MenuAction> &disableList)
{
    QVector<MenuAction> actionKeys;

    actionKeys.reserve(11);

    actionKeys << Open << OpenWith
               << Separator
               << Compress << Separator
               << Copy << Cut
               << Rename << Delete << CompleteDeletion
               << Separator
               << Property;

    return genereteMenuByKeys(actionKeys, disableList);
}

DFileMenu *FileMenuManager::createFolderMenu(const QVector<MenuAction> &disableList)
{
    QVector<MenuAction> actionKeys;

    actionKeys.reserve(11);

    actionKeys << Open
               << OpenInNewWindow
               << Separator
               << Compress << Separator
               << Copy << Cut
               << Rename << Delete
               << Separator
               << Property;

    return genereteMenuByKeys(actionKeys, disableList);
}

DFileMenu *FileMenuManager::createViewSpaceAreaMenu(const QVector<MenuAction> &disableList)
{
    QVector<MenuAction> actionKeys;

    actionKeys.reserve(9);

    actionKeys << OpenInNewWindow
               << Separator
               << NewFolder << NewDocument
               << Separator
               << Paste
               << SelectAll
               << Separator
               << Property;

    return genereteMenuByKeys(actionKeys, disableList);
}

DFileMenu *FileMenuManager::createRecentLeftBarMenu(const QVector<MenuAction> &disableList)
{
    QVector<MenuAction> actionKeys;

    actionKeys.reserve(2);

    actionKeys << OpenInNewWindow << ClearRecent;

    return genereteMenuByKeys(actionKeys, disableList);
}

DFileMenu *FileMenuManager::createRecentFileMenu(const QVector<MenuAction> &disableList)
{
    QVector<MenuAction> actionKeys;

    actionKeys.reserve(12);

    actionKeys << Open << OpenWith
               << OpenFileLocation
               << Separator
               << Compress << Separator
               << Copy << Cut
               << Rename << Delete
               << Separator
               << Property;

    return genereteMenuByKeys(actionKeys, disableList);
}

DFileMenu *FileMenuManager::createRecentViewSpaceAreaMenu(const QVector<MenuAction> &disableList)
{
    QVector<MenuAction> actionKeys;

    actionKeys.reserve(6);

    actionKeys << ClearRecent
               << Separator
               << DisplayAs
               << SortBy
               << Separator
               << Property;

    QMap<MenuAction, QVector<MenuAction> > subMenu;
    QVector<MenuAction> subActionKeys;

    subActionKeys << IconView << ListView;

    subMenu[DisplayAs] = subActionKeys;
    subActionKeys.clear();

    subActionKeys << Name << Size << Type << CreatedDate << LastModifiedDate;

    subMenu[SortBy] = subActionKeys;

    return genereteMenuByKeys(actionKeys, disableList, false, subMenu);
}

DFileMenu *FileMenuManager::createDefaultBookMarkMenu(const QVector<MenuAction> &disableList)
{
    QVector<MenuAction> actionKeys;

    actionKeys.reserve(7);

    actionKeys << Open
               << OpenInNewWindow
               << Separator
               << Remove
               << Rename
               << Separator
               << Property;

    return genereteMenuByKeys(actionKeys, disableList);
}

DFileMenu *FileMenuManager::createCustomBookMarkMenu(const QVector<MenuAction> &disableList)
{
    QVector<MenuAction> actionKeys;

    actionKeys.reserve(10);

    actionKeys << Open
               << OpenInNewWindow
               << Separator
               << Copy
               << Cut
               << Rename
               << Separator
               << Remove
               << Separator
               << Property;

    return genereteMenuByKeys(actionKeys, disableList);
}

DFileMenu *FileMenuManager::createTrashLeftBarMenu(const QVector<MenuAction> &disableList)
{
    QVector<MenuAction> actionKeys;

    actionKeys.reserve(4);

    actionKeys << Open << OpenInNewWindow
               << Separator << ClearTrash;

    return genereteMenuByKeys(actionKeys, disableList);
}

DFileMenu *FileMenuManager::createTrashFileMenu(const QVector<MenuAction> &disableList)
{
    QVector<MenuAction> actionKeys;

    actionKeys.reserve(12);

    actionKeys << Open << OpenWith
               << Separator
               << Compress << Separator
               << Copy << Cut << Separator
               << Restore << CompleteDeletion
               << Separator
               << Property;

    return genereteMenuByKeys(actionKeys, disableList);
}

DFileMenu *FileMenuManager::createTrashFolderMenu(const QVector<MenuAction> &disableList)
{
    QVector<MenuAction> actionKeys;

    actionKeys.reserve(12);

    actionKeys << Open << OpenInNewWindow
               << Separator
               << Compress << Separator
               << Copy << Cut << Separator
               << Restore << CompleteDeletion
               << Separator
               << Property;

    return genereteMenuByKeys(actionKeys, disableList);
}

DFileMenu *FileMenuManager::createTrashViewSpaceAreaMenu(const QVector<MenuAction> &disableList)
{
    QVector<MenuAction> actionKeys;

    actionKeys.reserve(7);

    actionKeys << OpenInNewWindow
               << Separator
               << Paste
               << SelectAll
               << ClearTrash
               << Separator
               << Property;

    return genereteMenuByKeys(actionKeys, disableList);
}

DFileMenu *FileMenuManager::createDiskLeftBarMenu(const QVector<FileMenuManager::MenuAction> &disableList)
{
    QVector<MenuAction> actionKeys;

    actionKeys.reserve(4);

    actionKeys << Open
               << OpenInNewWindow
               << Separator
               << Property;

    return genereteMenuByKeys(actionKeys, disableList);
}

DFileMenu *FileMenuManager::createDiskViewMenu(const QVector<FileMenuManager::MenuAction> &disableList)
{
    QVector<MenuAction> actionKeys;

    actionKeys.reserve(6);

    actionKeys << Open << OpenInNewWindow
               << Separator
               << Mount << Unmount
               << Separator
               << Property;

    return genereteMenuByKeys(actionKeys, disableList);
}

DFileMenu *FileMenuManager::createToolBarSettingsMenu(const QVector<FileMenuManager::MenuAction> &disableList)
{
    QVector<MenuAction> actionKeys;

    actionKeys.reserve(5);

    actionKeys << NewWindow << Separator
               << Help
               << About
               << Exit;

    return genereteMenuByKeys(actionKeys, disableList);
}

DFileMenu *FileMenuManager::createToolBarSortMenu(const QVector<FileMenuManager::MenuAction> &disableList)
{
    QVector<MenuAction> actionKeys;

    actionKeys.reserve(5);

    actionKeys << Name << Size
               << Type
               << CreatedDate
               << LastModifiedDate;

    return genereteMenuByKeys(actionKeys, disableList, true);
}

DFileMenu *FileMenuManager::createListViewHeaderMenu(const QVector<FileMenuManager::MenuAction> &disableList)
{
    QVector<MenuAction> actionKeys;

    actionKeys.reserve(4);

    actionKeys << Size
               << Type
               << CreatedDate
               << LastModifiedDate;

    return genereteMenuByKeys(actionKeys, disableList, true);
}

FileMenuManager::FileMenuManager()
{
    qRegisterMetaType<QMap<QString, QString>>("QMap<QString, QString>");
    qRegisterMetaType<QList<QUrl>>("QList<QUrl>");
}

void FileMenuManager::initData()
{
    m_actionKeys[Open] = QObject::tr("Open");
    m_actionKeys[OpenInNewWindow] = QObject::tr("Open in new window");
    m_actionKeys[OpenWith] = QObject::tr("Open with");
    m_actionKeys[OpenFileLocation] = QObject::tr("Open file loaction");
    m_actionKeys[Compress] = QObject::tr("Compress");
    m_actionKeys[Decompress] = QObject::tr("Decompress");
    m_actionKeys[Cut] = QObject::tr("Cut");
    m_actionKeys[Copy] = QObject::tr("Copy");
    m_actionKeys[Paste] = QObject::tr("Paste");
    m_actionKeys[Rename] = QObject::tr("Rename");
    m_actionKeys[Remove] = QObject::tr("Remove");
    m_actionKeys[Delete] = QObject::tr("Delete");
    m_actionKeys[CompleteDeletion] = QObject::tr("Complete deletion");
    m_actionKeys[Property] = QObject::tr("Property");

    m_actionKeys[NewFolder] = QObject::tr("New Folder");
    m_actionKeys[NewFile] = QObject::tr("New File");
    m_actionKeys[NewWindow] = QObject::tr("New Window");
    m_actionKeys[SelectAll] = QObject::tr("Select All");
    m_actionKeys[ClearRecent] = QObject::tr("Clear Recent");
    m_actionKeys[ClearTrash] = QObject::tr("Clear Trash");
    m_actionKeys[DisplayAs] = QObject::tr("Display As");
    m_actionKeys[SortBy] = QObject::tr("Sort By");
    m_actionKeys[NewDocument] = QObject::tr("New Document");
    m_actionKeys[Restore] = QObject::tr("Restore");
    m_actionKeys[CompleteDeletion] = QObject::tr("Complete Deletion");
    m_actionKeys[Mount] = QObject::tr("Mount");
    m_actionKeys[Unmount]= QObject::tr("Unmount");
    m_actionKeys[Name] = QObject::tr("Name");
    m_actionKeys[Size] = QObject::tr("Size");
    m_actionKeys[Type] = QObject::tr("Type");
    m_actionKeys[CreatedDate] = QObject::tr("Created Date");
    m_actionKeys[LastModifiedDate] = QObject::tr("Last Modified Date");
    m_actionKeys[Help] = QObject::tr("Help");
    m_actionKeys[About] = QObject::tr("About");
    m_actionKeys[Exit] = QObject::tr("Exit");
    m_actionKeys[IconView] = QObject::tr("Icon View");
    m_actionKeys[ListView] = QObject::tr("List View");
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
        if (key == Separator){
            menu->addSeparator();
        }else{
            DAction *action = m_actions.value(key);

            if(!action)
                continue;

            action->setCheckable(checkable);
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

void FileMenuManager::doOpen(const QString &url)
{
    QDir dir(url);
    if(dir.exists())
    {
        FMEvent event;
        event.dir = url;
        event.source = FMEvent::FileView;
        emit fileSignalManager->requestChangeCurrentUrl(event);
        return;
    }
    if(QFile::exists(url))
    {
        emit fileSignalManager->requestOpenFile(url);
        return;
    }
    const QString &scheme = QUrl(url).scheme();
    if(scheme == RECENT_SCHEME ||
       scheme == BOOKMARK_SCHEME ||
       scheme == TRASH_SCHEME)
    {
        FMEvent event;
        event.dir = url;
        event.source = FMEvent::FileView;
        emit fileSignalManager->requestChangeCurrentUrl(event);
    }
}

void FileMenuManager::doOpenFileLocation(const QString &url)
{
    QDir dir(url);
    if(dir.exists())
    {
        if(!dir.cdUp())
            return;
        FMEvent event;
        event.dir = dir.path();
        event.source = FMEvent::FileView;
        emit fileSignalManager->requestChangeCurrentUrl(event);
        return;
    }
    QFileInfo file(url);
    if(file.exists(url))
    {
        emit fileSignalManager->requestOpenFile(file.absolutePath());
        return;
    }
}

void FileMenuManager::doRename(const QString &url)
{
    //notify view to be in editing mode

    FMEvent event;

    event.dir = url;
    event.source = FMEvent::Menu;

    /// TODO
    event.windowId = -1;

    emit fileSignalManager->requestRename(event);
}

/**
 * @brief FileMenuManager::doDelete
 * @param url
 *
 * Trash file or directory with the given url address.
 */
void FileMenuManager::doDelete(const QList<QString> &urls)
{
    FileJob * job = new FileJob;
    QThread * thread = new QThread;
    job->moveToThread(thread);
    connect(this, &FileMenuManager::startMoveToTrash, job, &FileJob::doMoveToTrash);
    connect(job, &FileJob::finished, job, &FileJob::deleteLater);
    connect(job, &FileJob::finished, thread, &QThread::quit);
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    thread->start();
    QList<QUrl> qurls;
    for(int i = 0; i < urls.size(); i++)
    {
        qurls << QUrl(urls.at(i));
    }
    emit startMoveToTrash(qurls);
}

/**
 * @brief FileMenuManager::doCompleteDeletion
 * @param url
 *
 * Permanently delete file or directory with the given url.
 */
void FileMenuManager::doCompleteDeletion(const QList<QString> &urls)
{
    FileJob * job = new FileJob;
    QThread * thread = new QThread;
    job->moveToThread(thread);
    connect(this, &FileMenuManager::startCompleteDeletion, job, &FileJob::doDelete);
    connect(job, &FileJob::finished, job, &FileJob::deleteLater);
    connect(job, &FileJob::finished, thread, &QThread::quit);
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    thread->start();
    QList<QUrl> qurls;
    for(int i = 0; i < urls.size(); i++)
    {
        qurls << QUrl(urls.at(i));
    }
    emit startCompleteDeletion(qurls);
}

void FileMenuManager::doSorting(MenuAction action)
{
    qDebug() << action << Name;
    switch(action)
    {
    case Name:
        emit fileSignalManager->requestViewSort(0, Global::FileNameRole);
        break;
    case Size:
        emit fileSignalManager->requestViewSort(0, Global::FileSizeRole);
        break;
    case Type:
        fileSignalManager->requestViewSort(0, Global::FileSizeRole);
        break;
    case CreatedDate:
        emit fileSignalManager->requestViewSort(0, Global::FileLastModified);
        break;
    case LastModifiedDate:
        emit fileSignalManager->requestViewSort(0, Global::FileCreated);
        break;
    default:
        qDebug() << "unkown action type";
    }
}

void FileMenuManager::doCopy(const QList<QString> &urls)
{
    m_cutItems.clear();
    QList<QUrl> urlList;
    QMimeData *mimeData = new QMimeData;
    QByteArray ba;
    ba.append("copy");
    for(int i = 0; i < urls.size(); i++)
    {
        QString path = urls.at(i);
        ba.append("\n");
        ba.append(QUrl::fromLocalFile(path).toString());
        urlList.append(QUrl(path));
    }
    mimeData->setText("copy");
    mimeData->setUrls(urlList);
    mimeData->setData("x-special/gnome-copied-files", ba);
    QApplication::clipboard()->setMimeData(mimeData);
}

void FileMenuManager::doPaste(const QString &url)
{
    QDir dir(url);
    //Make sure the target directory exists.
    if(!dir.exists())
        return;
    const QClipboard *clipboard = QApplication::clipboard();
    const QMimeData *mimeData = clipboard->mimeData();

    if(mimeData->hasUrls())
    {
        if(mimeData->text() == "copy")
        {
            QList<QUrl> urls = mimeData->urls();
            FileJob * job = new FileJob;
            dialogManager->addJob(job);
            QThread * thread = new QThread;
            job->moveToThread(thread);
            connect(this, &FileMenuManager::startCopy, job, &FileJob::doCopy);
            connect(job, &FileJob::finished, job, &FileJob::deleteLater);
            connect(job, &FileJob::finished, thread, &QThread::quit);
            connect(thread, &QThread::finished, thread, &QThread::deleteLater);
            thread->start();
            emit startCopy(urls, url);
        }
        else if(mimeData->text() == "cut")
        {
            //Todo: paste on different physical device.
            QList<QUrl> urls = mimeData->urls();
            for(int i = 0; i < urls.size(); i++)
            {
                QUrl qurl = urls.at(i);
                QFileInfo fileInfo(qurl.path());
                QFile file(qurl.path());
                file.rename(dir.absolutePath() + "/" + fileInfo.fileName());
            }
        }
    }
    else if(!mimeData->data("x-special/gnome-copied-files").isEmpty())
    {

    }
}

void FileMenuManager::doCut(const QList<QString> &urls)
{
    m_cutItems = urls;
    QList<QUrl> urlList;
    QMimeData *mimeData = new QMimeData;
    QByteArray ba;
    ba.append("cut");
    for(int i = 0; i < urls.size(); i++)
    {
        QString path = urls.at(i);
        ba.append("\n");
        ba.append(QUrl::fromLocalFile(path).toString());
        urlList.append(QUrl(path));
    }
    mimeData->setText("cut");
    mimeData->setUrls(urlList);
    mimeData->setData("x-special/gnome-copied-files", ba);
    QApplication::clipboard()->setMimeData(mimeData);
}


void FileMenuManager::actionTriggered(DAction *action)
{
    DFileMenu *menu = qobject_cast<DFileMenu *>(sender());
    QList<QString> urls = menu->getUrls();
    QString dir = menu->getDir();
    MenuAction type = (MenuAction)action->data().toInt();
    switch(type)
    {
    case Open:
        if(urls.size() > 0)
        {
            doOpen(urls.at(0));
        }
        break;
    case OpenInNewWindow:break;
    case OpenWith:break;
    case OpenFileLocation:doOpenFileLocation(dir);break;
    case Compress:break;
    case Decompress:break;
    case Cut:doCut(urls);break;
    case Copy:doCopy(urls);break;
    case Paste:doPaste(dir);break;
    case Rename:break;
    case Remove:break;
    case Delete:doDelete(urls);break;
    case CompleteDeletion:doCompleteDeletion(urls);break;
    case Property:break;
    case NewFolder:break;
    case NewFile:break;
    case NewWindow:break;
    case SelectAll:break;
    case ClearRecent:break;
    case ClearTrash:break;
    case DisplayAs:break;
    case SortBy:break;
    case NewDocument:break;
    case Restore:break;
    case Mount:break;
    case Unmount:break;
    case Name:doSorting(type);break;
    case Size:doSorting(type);break;
    case Type:doSorting(type);break;
    case CreatedDate:doSorting(type);break;
    case LastModifiedDate:doSorting(type);break;
    case Help:break;
    case About:break;
    case Exit:break;
    case IconView:break;
    case ListView:break;
    default:
        qDebug() << "unknown action type";
    }
}
