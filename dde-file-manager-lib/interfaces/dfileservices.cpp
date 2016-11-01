#include "dfileservices.h"
#include "dabstractfilecontroller.h"
#include "dabstractfileinfo.h"

#include "app/filesignalmanager.h"
#include "dfmevent.h"
#include "app/define.h"
#include "controllers/jobcontroller.h"
#include "views/windowmanager.h"
#include "dfileinfo.h"
#include "models/trashfileinfo.h"

#include "shutil/fileutils.h"

#include "dialogs/dialogmanager.h"

#include "deviceinfo/udisklistener.h"
#include "interfaces/dfmglobal.h"
#include "widgets/singleton.h"

#include <ddialog.h>

#include <QUrl>
#include <QDebug>
#include <QtConcurrent/QtConcurrentRun>
#include <QFileDialog>
#include <QClipboard>
#include <QApplication>
#include <QMimeData>
#include <QTimer>

DWIDGET_USE_NAMESPACE

#define FILTER(type)\
    ({bool ok = true;\
    if (type >= OpenFile) {\
        if (d_func()->whitelist >= OpenFile && !d_func()->whitelist.testFlag(type))\
            ok = false;\
        else if (d_func()->blacklist.testFlag(type)) \
            ok = false;\
    } ok;})
#define FILTER_RETURN(type, value...)\
    if (!FILTER(type)) return value;

#define TRAVERSE(url, Code)\
    do {\
        {const char *function_name = ({QByteArray name(__FUNCTION__); name[0] = name.at(0) & char(0xdf); name.data();});\
        FileOperatorType type = (FileOperatorType)d_func()->fileOperatorTypeEnum.keyToValue(function_name);\
        if (!FILTER(type)) break;}\
        QList<DAbstractFileController*> &&list = getHandlerTypeByUrl(url);\
        bool accepted = false;\
        for(DAbstractFileController *controller : list) {\
            Code\
        }\
        list = getHandlerTypeByUrl(url, true);\
        for(DAbstractFileController *controller : list) {\
            Code\
        }\
    } while(false);

class DFileServicePrivate
{
public:
    static QMultiHash<const HandlerType, DAbstractFileController*> controllerHash;
    static QHash<const DAbstractFileController*, HandlerType> handlerHash;

    DFileService::FileOperatorTypes whitelist;
    DFileService::FileOperatorTypes blacklist;
    QMetaEnum fileOperatorTypeEnum;
};

QMultiHash<const HandlerType, DAbstractFileController*> DFileServicePrivate::controllerHash;
QHash<const DAbstractFileController*, HandlerType> DFileServicePrivate::handlerHash;
QMultiHash<const HandlerType, std::function<DAbstractFileController*()>> DFileService::m_controllerCreatorHash;

DFileService::DFileService(QObject *parent)
    : QObject(parent)
    , d_ptr(new DFileServicePrivate())
{
    qRegisterMetaType<DFMEvent>(QT_STRINGIFY(DFMEvent));
    qRegisterMetaType<QDir::Filters>(QT_STRINGIFY(QDir::Filters));
    qRegisterMetaType<QList<DAbstractFileInfoPointer>>(QT_STRINGIFY(QList<DAbstractFileInfoPointer>));
    qRegisterMetaType<DUrl>(QT_STRINGIFY(DUrl));

    d_ptr->fileOperatorTypeEnum = metaObject()->enumerator(metaObject()->indexOfEnumerator(QT_STRINGIFY(FileOperatorType)));
}

DFileService::~DFileService()
{

}

DFileService *DFileService::instance()
{
    static DFileService services;

    return &services;
}

void DFileService::setFileUrlHandler(const QString &scheme, const QString &host,
                                     DAbstractFileController *controller)
{
    if(DFileServicePrivate::handlerHash.contains(controller))
        return;

    const HandlerType &type = HandlerType(scheme, host);

    DFileServicePrivate::handlerHash[controller] = type;
    DFileServicePrivate::controllerHash.insertMulti(type, controller);

    connect(controller, &DAbstractFileController::childrenAdded,
            instance(), &DFileService::childrenAdded);
    connect(controller, &DAbstractFileController::childrenRemoved,
            instance(), &DFileService::childrenRemoved);
    connect(controller, &DAbstractFileController::childrenUpdated,
            instance(), &DFileService::childrenUpdated);
}

void DFileService::unsetFileUrlHandler(DAbstractFileController *controller)
{
    if(!DFileServicePrivate::handlerHash.contains(controller))
        return;

    DFileServicePrivate::controllerHash.remove(DFileServicePrivate::handlerHash.value(controller), controller);

    disconnect(controller, &DAbstractFileController::childrenAdded,
            instance(), &DFileService::childrenAdded);
    disconnect(controller, &DAbstractFileController::childrenRemoved,
            instance(), &DFileService::childrenRemoved);
    disconnect(controller, &DAbstractFileController::childrenUpdated,
            instance(), &DFileService::childrenUpdated);
}

void DFileService::clearFileUrlHandler(const QString &scheme, const QString &host)
{
    const HandlerType handler(scheme, host);

    for(const DAbstractFileController *controller : DFileServicePrivate::controllerHash.values(handler)) {
        disconnect(controller, &DAbstractFileController::childrenAdded,
                instance(), &DFileService::childrenAdded);
        disconnect(controller, &DAbstractFileController::childrenRemoved,
                instance(), &DFileService::childrenRemoved);
        disconnect(controller, &DAbstractFileController::childrenUpdated,
                instance(), &DFileService::childrenUpdated);
    }

    DFileServicePrivate::controllerHash.remove(handler);
    m_controllerCreatorHash.remove(handler);
}

void DFileService::setFileOperatorWhitelist(FileOperatorTypes list)
{
    Q_D(DFileService);

    d->whitelist = list;
}

DFileService::FileOperatorTypes DFileService::fileOperatorWhitelist() const
{
    Q_D(const DFileService);

    return d->whitelist;
}

void DFileService::setFileOperatorBlacklist(FileOperatorTypes list)
{
    Q_D(DFileService);

    d->blacklist = list;
}

DFileService::FileOperatorTypes DFileService::fileOperatorBlacklist() const
{
    Q_D(const DFileService);

    return d->blacklist;
}

bool DFileService::openFile(const DUrl &fileUrl) const
{
    TRAVERSE(fileUrl, {
                 bool ok = controller->openFile(fileUrl, accepted);

                 if(accepted) {
                     emit fileOpened(fileUrl);

                     return ok;
                 }
             })

    return false;
}

bool DFileService::compressFiles(const DUrlList &urlList) const
{
    if (urlList.isEmpty())
        return false;
    TRAVERSE(urlList.first(), {
                 bool ok = controller->compressFiles(urlList, accepted);

                 if(accepted) {
                     return ok;
                 }
             })

    return false;
}


bool DFileService::decompressFile(const DUrlList urllist) const{
    TRAVERSE(urllist.at(0), {
                 bool ok = controller->decompressFile(urllist, accepted);

                 if(accepted) {
                     return ok;
                 }
             })

    return false;
}

bool DFileService::decompressFileHere(const DUrlList urllist) const{
    TRAVERSE(urllist.at(0), {
                 bool ok = controller->decompressFileHere(urllist, accepted);

                 if(accepted) {
                     return ok;
                 }
             })

    return false;
}

bool DFileService::copyFilesToClipboard(const DUrlList &urlList) const
{
    if(urlList.isEmpty())
        return false;

    TRAVERSE(urlList.first(), {
                 bool ok = controller->copyFilesToClipboard(urlList, accepted);

                 if(accepted)
                     return ok;
             })

     return false;
}

bool DFileService::renameFile(const DUrl &oldUrl, const DUrl &newUrl, const DFMEvent &event) const
{
    FILTER_RETURN(RenameFile, false)

    const DAbstractFileInfoPointer &f = createFileInfo(newUrl);

    if (f->exists()){
        dialogManager->showRenameNameSameErrorDialog(f->fileDisplayName(), event);
        return false;
    }

    if (renameFile(oldUrl, newUrl)) {
        DFMEvent e = event;

        e << (DUrlList() << newUrl);

        TIMER_SINGLESHOT(200, {
            emit fileSignalManager->requestSelectFile(e);
        }, e);

        return true;
    }

    return false;
}

bool DFileService::renameFile(const DUrl &oldUrl, const DUrl &newUrl) const
{
    TRAVERSE(oldUrl, {
                 bool ok = controller->renameFile(oldUrl, newUrl, accepted);

                 if (accepted)
                    return ok;
             })

    return false;
}

void DFileService::deleteFiles(const DFMEvent &event) const
{
    FILTER_RETURN(DeleteFiles)

    if (event.fileUrlList().isEmpty())
        return;

    if (QThreadPool::globalInstance()->activeThreadCount() >= MAX_THREAD_COUNT) {
        qDebug() << "Beyond the maximum number of threads!";
        return;
    }

    int result = dialogManager->showDeleteFilesClearTrashDialog(event);

    if (result == 1) {
        QtConcurrent::run(QThreadPool::globalInstance(), this, &DFileService::deleteFilesSync, event);
    }

    return;
}

bool DFileService::deleteFilesSync(const DFMEvent &event) const
{
    if (event.fileUrlList().isEmpty())
        return false;

    TRAVERSE(event.fileUrl(), {
                 bool ok =controller->deleteFiles(event, accepted);

                 if(accepted)
                    return ok;
             })

    return false;
}

void DFileService::moveToTrash(const DFMEvent &event) const
{
    FILTER_RETURN(MoveToTrash)

    if (event.fileUrlList().isEmpty())
        return;

    if (QThreadPool::globalInstance()->activeThreadCount() >= MAX_THREAD_COUNT) {
        qDebug() << "Beyond the maximum number of threads!";
        return;
    }

    if (QThread::currentThread() == qApp->thread()) {
        QtConcurrent::run(QThreadPool::globalInstance(), this, &DFileService::moveToTrashSync, event);

        return;
    }

    moveToTrashSync(event);
}

DUrlList DFileService::moveToTrashSync(const DFMEvent &event) const
{
    if (event.fileUrlList().isEmpty())
        return DUrlList();

    TRAVERSE(event.fileUrl(), {
                 DUrlList list = controller->moveToTrash(event, accepted);

                 if (accepted)
                    return list;
             })

    return DUrlList();
}

bool DFileService::cutFilesToClipboard(const DUrlList &urlList) const
{
    if(urlList.isEmpty())
        return false;

    TRAVERSE(urlList.first(), {
                 bool ok = controller->cutFilesToClipboard(urlList, accepted);

                 if(accepted)
                    return ok;
             })

    return false;
}

void DFileService::pasteFileByClipboard(const DUrl &tarUrl, const DFMEvent &event) const
{
    FILTER_RETURN(PasteFileByClipboard)

    DFMGlobal::ClipboardAction action = DFMGlobal::instance()->clipboardAction();

    if (action == DFMGlobal::UnknowAction)
        return;

    DAbstractFileController::PasteType type = (action == DFMGlobal::CutAction) ? DAbstractFileController::CutType
                                                                               : DAbstractFileController::CopyType;

    const_cast<DFMEvent&>(event) << DUrl::fromQUrlList(DFMGlobal::instance()->clipboardFileUrlList());
    pasteFile(type, tarUrl, event);
}

void DFileService::pasteFile(DAbstractFileController::PasteType type, const DUrl &tarUrl, const DFMEvent &event) const
{
    if(QThreadPool::globalInstance()->activeThreadCount() >= MAX_THREAD_COUNT) {
        qDebug() << "Beyond the maximum number of threads!";
        return;
    }

    if(QThread::currentThread() == qApp->thread()) {
        QtConcurrent::run(QThreadPool::globalInstance(), this, &DFileService::pasteFile, type, tarUrl, event);

        return;
    }

    TRAVERSE(tarUrl, {
                 DUrlList list = controller->pasteFile(type, tarUrl, event, accepted);

                 if(accepted) {
                     DFMEvent e = event;

                     e << list;

                     metaObject()->invokeMethod(const_cast<DFileService*>(this), "laterRequestSelectFiles",
                                                Qt::QueuedConnection, Q_ARG(DFMEvent, e));

                     return;
                 }
             })
}

void DFileService::restoreFile(const DUrl &srcUrl, const DUrl &tarUrl, const DFMEvent &event) const
{
    if(QThreadPool::globalInstance()->activeThreadCount() >= MAX_THREAD_COUNT) {
        qDebug() << "Beyond the maximum number of threads!";
        return;
    }

    if(QThread::currentThread() == qApp->thread()) {
        QtConcurrent::run(QThreadPool::globalInstance(), this, &DFileService::restoreFile, srcUrl, tarUrl, event);

        return;
    }
    TRAVERSE(srcUrl, {
                 controller->restoreFile(srcUrl, tarUrl, event, accepted);

                 if(accepted)
                 return;
             })
}

bool DFileService::newFolder(const DFMEvent &event) const
{
    TRAVERSE(event.fileUrl(), {
                 bool ok = controller->newFolder(event, accepted);

                 if(accepted)
                    return ok;
             })

    return false;
}

bool DFileService::newFile(const DUrl &toUrl) const
{
    TRAVERSE(toUrl, {
                 bool ok = controller->newFile(toUrl, accepted);

                 if(accepted)
                    return ok;
             })

    return false;
}

bool DFileService::newDocument(const DUrl &toUrl) const
{
    TRAVERSE(toUrl, {
                 bool ok = controller->newDocument(toUrl, accepted);

                 if(accepted)
                    return ok;
             })

    return false;
}

bool DFileService::addUrlMonitor(const DUrl &fileUrl) const
{
    TRAVERSE(fileUrl, {
                 bool ok = controller->addUrlMonitor(fileUrl, accepted);

                 if(accepted)
                    return ok;
             })

    return false;
}

bool DFileService::removeUrlMonitor(const DUrl &fileUrl) const
{
    TRAVERSE(fileUrl, {
                 bool ok = controller->removeUrlMonitor(fileUrl, accepted);

                 if(accepted)
                    return ok;
             })

    return false;
}

bool DFileService::openFileLocation(const DUrl &fileUrl) const
{
    TRAVERSE(fileUrl, {
                 bool ok = controller->openFileLocation(fileUrl, accepted);

                 if(accepted)
                    return ok;
             })

    return false;
}

bool DFileService::createSymlink(const DUrl &fileUrl, const DFMEvent &event) const
{
    FILTER_RETURN(CreateSymlink, false)

    QString linkName = getSymlinkFileName(fileUrl);
    QString linkPath = QFileDialog::getSaveFileName(WindowManager::getWindowById(event.windowId()),
                                                    QObject::tr("Create symlink"), linkName);

    return createSymlink(fileUrl, DUrl::fromLocalFile(linkPath));
}

bool DFileService::createSymlink(const DUrl &fileUrl, const DUrl &linkToUrl) const
{
    TRAVERSE(fileUrl, {
                 bool ok = controller->createSymlink(fileUrl, linkToUrl, accepted);

                 if(accepted)
                     return ok;
             })

    return false;
}

bool DFileService::sendToDesktop(const DFMEvent &event) const
{
    const QString &desktopPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);

    if (desktopPath.isEmpty())
        return false;

    const QDir &desktopDir(desktopPath);
    bool ok = true;

    for (const DUrl &url : event.fileUrlList()) {
        const QString &linkName = getSymlinkFileName(url, desktopDir);

        ok = ok && createSymlink(url, DUrl::fromLocalFile(desktopDir.filePath(linkName)));
    }

    return ok;
}

bool DFileService::openInTerminal(const DUrl &fileUrl) const
{
    TRAVERSE(fileUrl, {
                 bool ok = controller->openInTerminal(fileUrl, accepted);

                 if(accepted)
                     return ok;
             })

    return false;
}

void DFileService::openNewWindow(const DUrl &fileUrl) const
{
    emit fileSignalManager->requestOpenNewWindowByUrl(fileUrl, true);
}

const DAbstractFileInfoPointer DFileService::createFileInfo(const DUrl &fileUrl) const
{
    const DAbstractFileInfoPointer &info = DAbstractFileInfo::getFileInfo(fileUrl);

    if (info)
        return info;

    TRAVERSE(fileUrl, {
                 const DAbstractFileInfoPointer &info = controller->createFileInfo(fileUrl, accepted);

                 if(accepted)
                     return info;
             })

    return DAbstractFileInfoPointer();
}

const DDirIteratorPointer DFileService::createDirIterator(const DUrl &fileUrl, const QStringList &nameFilters,
                                                          QDir::Filters filters, QDirIterator::IteratorFlags flags) const
{
    TRAVERSE(fileUrl, {
                 const DDirIteratorPointer iterator = controller->createDirIterator(fileUrl, nameFilters, filters, flags, accepted);

                 if(accepted)
                     return iterator;
             })

    return DDirIteratorPointer();
}

const QList<DAbstractFileInfoPointer> DFileService::getChildren(const DUrl &fileUrl, const QStringList &nameFilters,
                                                               QDir::Filters filters, QDirIterator::IteratorFlags flags, bool *ok)
{
    TRAVERSE(fileUrl, {
                 const QList<DAbstractFileInfoPointer> list = controller->getChildren(fileUrl, nameFilters, filters, flags, accepted);

                 if (ok)
                    *ok = accepted;

                 if(accepted)
                     return list;
             })

    if (ok)
        *ok = false;

    return QList<DAbstractFileInfoPointer>();
}

JobController *DFileService::getChildrenJob(const DUrl &fileUrl, const QStringList &nameFilters,
                                            QDir::Filters filters, QDirIterator::IteratorFlags flags) const
{
    const DDirIteratorPointer &iterator = createDirIterator(fileUrl, nameFilters, filters, flags);

    if (iterator)
        return new JobController(iterator, const_cast<DFileService*>(this));

    return new JobController(fileUrl, nameFilters, filters, const_cast<DFileService*>(this));
}

QList<DAbstractFileController*> DFileService::getHandlerTypeByUrl(const DUrl &fileUrl,
                                                                 bool ignoreHost, bool ignoreScheme)
{
    HandlerType handlerType(ignoreScheme ? "" : fileUrl.scheme(), ignoreHost ? "" : fileUrl.path());
    if(m_controllerCreatorHash.contains(handlerType)) {
        QList<DAbstractFileController*> list = DFileServicePrivate::controllerHash.values(handlerType);

        for(const std::function<DAbstractFileController*()> &creator : m_controllerCreatorHash.values(handlerType)) {
            DAbstractFileController *controller = creator();

            setFileUrlHandler(handlerType.first, handlerType.second, controller);

            list << controller;
        }

        m_controllerCreatorHash.remove(handlerType);

        return list;
    } else {
        return DFileServicePrivate::controllerHash.values(handlerType);
    }
}

QString DFileService::getSymlinkFileName(const DUrl &fileUrl, const QDir &targetDir)
{
    const DAbstractFileInfoPointer &pInfo =  instance()->createFileInfo(fileUrl);

    if (pInfo->exists()) {
        QString baseName = pInfo->baseName();
        QString shortcut = QObject::tr("Shortcut");
        QString linkBaseName;

        int number = 1;

        forever {
            if (pInfo->isFile()) {
                if (number == 1) {
                    linkBaseName = QString("%1 %2.%3").arg(baseName, shortcut, pInfo->suffix());
                } else {
                    linkBaseName = QString("%1 %2%3.%4").arg(baseName, shortcut, QString::number(number), pInfo->suffix());
                }
            } else if (pInfo->isDir()) {
                if (number == 1) {
                    linkBaseName = QString("%1 %2").arg(baseName, shortcut);
                }else{
                    linkBaseName = QString("%1 %2%3").arg(baseName, shortcut, QString::number(number));
                }
            } else if (pInfo->isSymLink()) {
                return QString();
            }

            if (targetDir.path().isEmpty())
                return linkBaseName;

            if (targetDir.exists(linkBaseName)) {
                ++number;
            } else {
                return linkBaseName;
            }
        }
    }

    return QString();
}

void DFileService::openUrl(const DFMEvent &event) const
{
    FILTER_RETURN(OpenUrl)

    const DAbstractFileInfoPointer &fileInfo = createFileInfo(event.fileUrl());

    if (fileInfo && fileInfo->isDir()) {
        emit fileSignalManager->requestChangeCurrentUrl(event);
    } else {
        openFile(event.fileUrl());
    }
}

void DFileService::laterRequestSelectFiles(const DFMEvent &event) const
{
    FileSignalManager *manager = fileSignalManager;

    TIMER_SINGLESHOT_OBJECT(manager, 200, {
                                manager->requestSelectFile(event);
                            }, event, manager)
}
