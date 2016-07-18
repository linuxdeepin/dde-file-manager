#include "abstractfilecontroller.h"

AbstractFileController::AbstractFileController(QObject *parent)
    : QObject(parent)
{

}

bool AbstractFileController::openFile(const DUrl &fileUrl, bool &accepted) const
{
    Q_UNUSED(fileUrl)

    accepted = false;

    return false;
}

bool AbstractFileController::compressFiles(const DUrlList &urlList, bool &accepted) const
{
    Q_UNUSED(urlList)

    accepted = false;

    return false;
}

bool AbstractFileController::decompressFile(const DUrl &fileUrl, bool &accepted) const
{
    Q_UNUSED(fileUrl)

    accepted = false;

    return false;
}

bool AbstractFileController::decompressFileHere(const DUrl &fileUrl, bool &accepted) const
{
    Q_UNUSED(fileUrl)

    accepted = false;

    return false;
}

bool AbstractFileController::copyFiles(const DUrlList &urlList, bool &accepted) const
{
    Q_UNUSED(urlList)

    accepted = false;

    return false;
}

bool AbstractFileController::renameFile(const DUrl &oldUrl, const DUrl &newUrl, bool &accepted) const
{
    Q_UNUSED(oldUrl)
    Q_UNUSED(newUrl)

    accepted = false;

    return false;
}

bool AbstractFileController::deleteFiles(const DUrlList &urlList, const FMEvent &event, bool &accepted) const
{
    Q_UNUSED(urlList)
    Q_UNUSED(event)
    accepted = false;

    return false;
}

bool AbstractFileController::moveToTrash(const DUrlList &urlList, bool &accepted) const
{
    Q_UNUSED(urlList)

    accepted = false;

    return false;
}

bool AbstractFileController::cutFiles(const DUrlList &urlList, bool &accepted) const
{
    Q_UNUSED(urlList)

    accepted = false;

    return false;
}

bool AbstractFileController::pasteFile(PasteType type, const DUrlList &urlList,
                                       const FMEvent &event, bool &accepted) const
{
    Q_UNUSED(type)
    Q_UNUSED(event)
    Q_UNUSED(urlList)

    accepted = false;

    return false;
}

bool AbstractFileController::restoreFile(const DUrl &srcUrl, const DUrl &tarUrl, const FMEvent &event, bool &accepted) const
{
    Q_UNUSED(srcUrl)
    Q_UNUSED(tarUrl)
    Q_UNUSED(event)
    accepted = false;

    return false;
}

bool AbstractFileController::newFolder(const FMEvent &event, bool &accepted) const
{
    Q_UNUSED(event)

    accepted = false;

    return false;
}

bool AbstractFileController::newFile(const DUrl &toUrl, bool &accepted) const
{
    Q_UNUSED(toUrl)

    accepted = false;

    return false;
}

bool AbstractFileController::newDocument(const DUrl &toUrl, bool &accepted) const
{
    Q_UNUSED(toUrl)

    accepted = false;

    return false;
}

bool AbstractFileController::addUrlMonitor(const DUrl &url, bool &accepted) const
{
    Q_UNUSED(url)

    accepted = false;

    return false;
}

bool AbstractFileController::removeUrlMonitor(const DUrl &url, bool &accepted) const
{
    Q_UNUSED(url)

    accepted = false;

    return false;
}

bool AbstractFileController::openFileLocation(const DUrl &fileUrl, bool &accepted) const
{
    Q_UNUSED(fileUrl)

    accepted = false;

    return false;
}

const QList<AbstractFileInfoPointer> AbstractFileController::getChildren(const DUrl &fileUrl, QDir::Filters filters, bool &accepted) const
{
    Q_UNUSED(fileUrl)
    Q_UNUSED(filters)

    accepted = false;

    return QList<AbstractFileInfoPointer>();
}

const AbstractFileInfoPointer AbstractFileController::createFileInfo(const DUrl &fileUrl, bool &accepted) const
{
    Q_UNUSED(fileUrl)

    accepted = false;

    return AbstractFileInfoPointer();
}

const DDirIteratorPointer AbstractFileController::createDirIterator(const DUrl &fileUrl, QDir::Filters filters,
                                                                    QDirIterator::IteratorFlags flags, bool &accepted) const
{
    Q_UNUSED(fileUrl)
    Q_UNUSED(filters)
    Q_UNUSED(flags)

    accepted = false;

    return DDirIteratorPointer();
}

bool AbstractFileController::createSymlink(const DUrl &fileUrl, const DUrl &linkToUrl, bool &accepted) const
{
    Q_UNUSED(fileUrl)
    Q_UNUSED(linkToUrl)

    accepted = false;

    return false;
}

bool AbstractFileController::openInTerminal(const DUrl &fileUrl, bool &accepted) const
{
    Q_UNUSED(fileUrl)

    accepted = false;

    return false;
}
