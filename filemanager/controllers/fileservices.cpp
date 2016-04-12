#include "fileservices.h"
#include "abstractfilecontroller.h"

#include "../app/filesignalmanager.h"
#include "../app/fmevent.h"
#include "../app/global.h"
#include "../models/abstractfileinfo.h"
#include "../controllers/filejob.h"
#include <QUrl>
#include <QDebug>

#define TRAVERSE(Code) \
    QList<AbstractFileController*> &&list = getHandlerTypeByUrl(fileUrl);\
    bool accepted = false;\
    for(AbstractFileController *controller : list) {\
        Code\
    }\
    list = getHandlerTypeByUrl(fileUrl, true);\
    for(AbstractFileController *controller : list) {\
        Code\
    }

QMultiHash<HandlerType, AbstractFileController*> FileServices::m_controllerHash;
QHash<AbstractFileController*, HandlerType> FileServices::m_handlerHash;

FileServices::FileServices()
    : QObject()
{
    m_thread = new QThread(this);

    moveToThread(m_thread);

    qRegisterMetaType<FMEvent>("FMEvent");
    qRegisterMetaType<QDir::Filters>("QDir::Filters");
    qRegisterMetaType<QList<AbstractFileInfo*>>("QList<AbstractFileInfo*>");

    m_thread->start();
}

FileServices *FileServices::instance()
{
    static FileServices services;

    return &services;
}

void FileServices::setFileUrlHandler(const QString &scheme, const QString &host,
                                     AbstractFileController *controller)
{
    if(m_handlerHash.contains(controller))
        return;

    const HandlerType type = HandlerType(scheme, host);

    m_handlerHash[controller] = type;
    m_controllerHash.insertMulti(type, controller);

    connect(controller, &AbstractFileController::childrenAdded,
            instance(), &FileServices::childrenAdded);
    connect(controller, &AbstractFileController::childrenRemoved,
            instance(), &FileServices::childrenRemoved);
}

void FileServices::unsetFileUrlHandler(AbstractFileController *controller)
{
    if(!m_handlerHash.contains(controller))
        return;

    m_controllerHash.remove(m_handlerHash.value(controller), controller);

    disconnect(controller, &AbstractFileController::childrenAdded,
            instance(), &FileServices::childrenAdded);
    disconnect(controller, &AbstractFileController::childrenRemoved,
            instance(), &FileServices::childrenRemoved);
}

void FileServices::clearFileUrlHandler(const QString &scheme, const QString &host)
{
    const HandlerType handler(scheme, host);

    for(const AbstractFileController *controller : m_controllerHash.values(handler)) {
        connect(controller, &AbstractFileController::childrenAdded,
                instance(), &FileServices::childrenAdded);
        connect(controller, &AbstractFileController::childrenRemoved,
                instance(), &FileServices::childrenRemoved);
    }

    m_controllerHash.remove(handler);
}

bool FileServices::openFile(const QString &fileUrl) const
{
    TRAVERSE({
                 bool ok = controller->openFile(fileUrl, accepted);

                 if(accepted) {
                     emit fileOpened(fileUrl);

                     return ok;
                 }
             })

    return false;
}

bool FileServices::renameFile(const QString &fileUrl, const QString &newUrl) const
{
    TRAVERSE({
                 bool ok = controller->renameFile(fileUrl, newUrl, accepted);

                 if(accepted)
                    return ok;
             })

    return false;
}

bool FileServices::addUrlMonitor(const QString &fileUrl) const
{
    TRAVERSE({
                 bool ok = controller->addUrlMonitor(fileUrl, accepted);

                 if(accepted)
                    return ok;
             })

    return false;
}

bool FileServices::removeUrlMonitor(const QString &fileUrl) const
{
    TRAVERSE({
                 bool ok = controller->removeUrlMonitor(fileUrl, accepted);

                 if(accepted)
                    return ok;
             })

     return false;
}

AbstractFileInfo *FileServices::createFileInfo(const QString &fileUrl) const
{
    TRAVERSE({
                 AbstractFileInfo *info = controller->createFileInfo(fileUrl, accepted);

                 if(accepted)
                     return info;
             })

    return Q_NULLPTR;
}

void FileServices::getChildren(const FMEvent &event, QDir::Filters filters) const
{
    if(QThread::currentThread() != m_thread) {
        ASYN_CALL_SLOT(this, getChildren, event, filters);

        return;
    }

    const QString &fileUrl = event.fileUrl();

    TRAVERSE({
                 const QList<AbstractFileInfo*> &&list = controller->getChildren(fileUrl, filters, accepted);

                 if(accepted) {
                     emit childrenUpdated(event, std::move(list));

                     return;
                 }
             })
}

QList<AbstractFileController*> FileServices::getHandlerTypeByUrl(const QString &fileUrl,
                                                                 bool ignoreHost, bool ignoreScheme)
{
    QUrl url(fileUrl);

    return m_controllerHash.values(HandlerType(ignoreScheme ? "" : url.scheme(),
                                               ignoreHost ? "" : url.path()));
}


void FileServices::doOpen(const QString &url)
{
    QDir dir(url);
    if(dir.exists())
    {
        FMEvent event;
        event = url;
        event = FMEvent::FileView;
        emit fileSignalManager->requestChangeCurrentUrl(event);
        return;
    }
    if(QFile::exists(url))
    {
        openFile(url);
        return;
    }
    const QString &scheme = QUrl(url).scheme();
    if(scheme == RECENT_SCHEME ||
       scheme == BOOKMARK_SCHEME ||
       scheme == TRASH_SCHEME)
    {
        FMEvent event;
        event = url;
        event = FMEvent::FileView;
        emit fileSignalManager->requestChangeCurrentUrl(event);
    }
}

void FileServices::doOpenNewWindow(const QString &url)
{
    emit fileSignalManager->requestOpenNewWindowByUrl(url);
}

void FileServices::doOpenFileLocation(const QString &url)
{
    QDir dir(url);
    if(dir.exists())
    {
        if(!dir.cdUp())
            return;
        FMEvent event;
        event = dir.path();
        event = FMEvent::FileView;
        emit fileSignalManager->requestChangeCurrentUrl(event);
        return;
    }
    QFileInfo file(url);
    if(file.exists(url))
    {
        openFile(QUrl::fromLocalFile(file.absolutePath()).toString());
        return;
    }
}

void FileServices::doRename(const QString &url, int windowId)
{
    //notify view to be in editing mode

    FMEvent event;

    event = url;
    event = FMEvent::Menu;
    event = windowId;

    emit fileSignalManager->requestRename(event);
}

/**
 * @brief FileServices::doDelete
 * @param url
 *
 * Trash file or directory with the given url address.
 */
void FileServices::doDelete(const QList<QString> &urls)
{
    FileJob * job = new FileJob;
    QThread * thread = new QThread;
    job->moveToThread(thread);
    dialogManager->addJob(job);
    connect(this, &FileServices::startMoveToTrash, job, &FileJob::doMoveToTrash);
    connect(job, &FileJob::finished, dialogManager, &DialogManager::removeJob);
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
 * @brief FileServices::doCompleteDeletion
 * @param url
 *
 * Permanently delete file or directory with the given url.
 */
void FileServices::doCompleteDeletion(const QList<QString> &urls)
{
    FileJob * job = new FileJob;
    QThread * thread = new QThread;
    job->moveToThread(thread);
    dialogManager->addJob(job);
    connect(this, &FileServices::startCompleteDeletion, job, &FileJob::doDelete);
    connect(job, &FileJob::finished, dialogManager, &DialogManager::removeJob);
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

void FileServices::doSorting(FileMenuManager::MenuAction type)
{
    qDebug() << type;
    switch(type)
    {
    case FileMenuManager::Name:
        emit fileSignalManager->requestViewSort(0, Global::FileNameRole);
        break;
    case FileMenuManager::Size:
        emit fileSignalManager->requestViewSort(0, Global::FileSizeRole);
        break;
    case FileMenuManager::Type:
        fileSignalManager->requestViewSort(0, Global::FileSizeRole);
        break;
    case FileMenuManager::CreatedDate:
        emit fileSignalManager->requestViewSort(0, Global::FileLastModified);
        break;
    case FileMenuManager::LastModifiedDate:
        emit fileSignalManager->requestViewSort(0, Global::FileCreated);
        break;
    default:
        qDebug() << "unkown action type";
    }
}

void FileServices::doCopy(const QList<QString> &urls)
{
    QList<QUrl> urlList;
    QMimeData *mimeData = new QMimeData;
    QByteArray ba;
    ba.append("copy");
    for(int i = 0; i < urls.size(); i++)
    {
        QString path = urls.at(i);
        ba.append("\n");
        ba.append(urls.at(i));
        urlList.append(QUrl(path));
    }
    mimeData->setText("copy");
    mimeData->setUrls(urlList);
    mimeData->setData("x-special/gnome-copied-files", ba);
    QApplication::clipboard()->setMimeData(mimeData);
}

void FileServices::doPaste(const QString &url)
{
    QUrl localUrl(url);
    QDir dir(localUrl.toLocalFile());
    //Make sure the target directory exists.
    if(!dir.exists())
        return;
    const QClipboard *clipboard = QApplication::clipboard();
    const QMimeData *mimeData = clipboard->mimeData();

    if(!mimeData->data("x-special/gnome-copied-files").isEmpty())
    {
        QByteArray ba = mimeData->data("x-special/gnome-copied-files");
        QTextStream text(&ba);
        QString type = text.readLine();
        if(type == "cut")
        {
            while(!text.atEnd())
            {
                QUrl qurl(text.readLine());
                QFileInfo fileInfo(qurl.path());
                QFile file(qurl.path());
                file.rename(dir.absolutePath() + "/" + fileInfo.fileName());
            }
        }
        else if(type == "copy")
        {
            QList<QUrl> urls;
            while(!text.atEnd())
            {
                QUrl qurl(text.readLine());
                urls.append(qurl);
            }

            FileJob * job = new FileJob;
            dialogManager->addJob(job);
            QThread * thread = new QThread;
            job->moveToThread(thread);
            connect(this, &FileServices::startCopy, job, &FileJob::doCopy);
            connect(job, &FileJob::finished, dialogManager, &DialogManager::removeJob);
            connect(job, &FileJob::finished, thread, &QThread::quit);
            connect(thread, &QThread::finished, thread, &QThread::deleteLater);
            thread->start();
            emit startCopy(urls, localUrl.toLocalFile());
        }
    }
}

void FileServices::doCut(const QList<QString> &urls)
{
    QList<QUrl> urlList;
    QMimeData *mimeData = new QMimeData;
    QByteArray ba;
    ba.append("cut");
    for(int i = 0; i < urls.size(); i++)
    {
        QString path = urls.at(i);
        ba.append("\n");
        ba.append(urls.at(i));
        urlList.append(QUrl(path));
    }
    mimeData->setText("cut");
    mimeData->setUrls(urlList);
    mimeData->setData("x-special/gnome-copied-files", ba);
    QApplication::clipboard()->setMimeData(mimeData);
}

void FileServices::doNewFolder(const QString &url)
{
    QUrl localUrl(url);
    //Todo:: check if mkdir is ok
    QDir dir(localUrl.toLocalFile());
    dir.mkdir(FileServices::checkDuplicateName(dir.absolutePath() + "/New Folder"));
}

void FileServices::doNewDocument(const QString &url)
{
    Q_UNUSED(url)
}

void FileServices::doNewFile(const QString &url)
{
    QUrl localUrl(url);
    //Todo:: check if mkdir is ok
    QDir dir(localUrl.toLocalFile());
    QString name = FileServices::checkDuplicateName(dir.absolutePath() + "/New File");
    QFile file(name);
    if(file.open(QIODevice::WriteOnly))
    {
        file.close();
    }
}

void FileServices::doSelectAll(int windowId)
{
    fileSignalManager->requestViewSelectAll(windowId);
}

void FileServices::doRemove(const QString &url)
{
    QUrl localUrl(url);
    fileSignalManager->requestBookmarkRemove(localUrl.toLocalFile());
}

QString FileServices::checkDuplicateName(const QString &name)
{
    QString destUrl = name;
    QFile file(destUrl);
    QFileInfo startInfo(destUrl);
    int num = 1;
    while (file.exists())
    {
        num++;
        destUrl = QString("%1/%2 %3").arg(startInfo.absolutePath()).
                arg(startInfo.fileName()).arg(num);
        file.setFileName(destUrl);
    }
    return destUrl;
}
