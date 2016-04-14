#include "abstractfilecontroller.h"

AbstractFileController::AbstractFileController(QObject *parent)
    : QObject(parent)
{

}

bool AbstractFileController::openFile(const QString &fileUrl, bool &accepted) const
{
    Q_UNUSED(fileUrl)

    accepted = false;

    return false;
}

bool AbstractFileController::copyFiles(const QList<QString> &urlList, bool &accepted) const
{
    Q_UNUSED(urlList)

    accepted = false;

    return false;
}

bool AbstractFileController::renameFile(const QString &oldUrl, const QString &newUrl, bool &accepted) const
{
    Q_UNUSED(oldUrl)
    Q_UNUSED(newUrl)

    accepted = false;

    return false;
}

bool AbstractFileController::deleteFiles(const QList<QString> &urlList, bool &accepted) const
{
    Q_UNUSED(urlList)

    accepted = false;

    return false;
}

bool AbstractFileController::moveToTrash(const QList<QString> &urlList, bool &accepted) const
{
    Q_UNUSED(urlList)

    accepted = false;

    return false;
}

bool AbstractFileController::cutFiles(const QList<QString> &urlList, bool &accepted) const
{
    Q_UNUSED(urlList)

    accepted = false;

    return false;
}

bool AbstractFileController::pasteFile(PasteType type, const QList<QString> &urlList,
                                       const FMEvent &event, bool &accepted) const
{
    Q_UNUSED(type)
    Q_UNUSED(event)
    Q_UNUSED(urlList)

    accepted = false;

    return false;
}

bool AbstractFileController::newFolder(const QString &toUrl, bool &accepted) const
{
    Q_UNUSED(toUrl)

    accepted = false;

    return false;
}

bool AbstractFileController::newFile(const QString &toUrl, bool &accepted) const
{
    Q_UNUSED(toUrl)

    accepted = false;

    return false;
}

bool AbstractFileController::newDocument(const QString &toUrl, bool &accepted) const
{
    Q_UNUSED(toUrl)

    accepted = false;

    return false;
}

bool AbstractFileController::addUrlMonitor(const QString &url, bool &accepted) const
{
    Q_UNUSED(url)

    accepted = false;

    return false;
}

bool AbstractFileController::removeUrlMonitor(const QString &url, bool &accepted) const
{
    Q_UNUSED(url)

    accepted = false;

    return false;
}

bool AbstractFileController::openFileLocation(const QString &fileUrl, bool &accepted) const
{
    Q_UNUSED(fileUrl)

    accepted = false;

    return false;
}

const QList<AbstractFileInfo*> AbstractFileController::getChildren(const QString &fileUrl, QDir::Filters filter, bool &accepted) const
{
    Q_UNUSED(fileUrl)
    Q_UNUSED(filter)

    accepted = false;

    return QList<AbstractFileInfo*>();
}

AbstractFileInfo *AbstractFileController::createFileInfo(const QString &fileUrl, bool &accepted) const
{
    Q_UNUSED(fileUrl)

    accepted = false;

    return Q_NULLPTR;
}
