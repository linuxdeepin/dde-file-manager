#include "dabstractfilecontroller.h"

DAbstractFileController::DAbstractFileController(QObject *parent)
    : QObject(parent)
{

}

bool DAbstractFileController::openFile(const DUrl &fileUrl, bool &accepted) const
{
    Q_UNUSED(fileUrl)

    accepted = false;

    return false;
}

bool DAbstractFileController::openFileByApp(const DUrl &fileUrl, const QString &app, bool &accepted) const
{
    Q_UNUSED(fileUrl)
    Q_UNUSED(app)

    accepted = false;

    return false;
}

bool DAbstractFileController::compressFiles(const DUrlList &urlList, bool &accepted) const
{
    Q_UNUSED(urlList)

    accepted = false;

    return false;
}

bool DAbstractFileController::decompressFile(const DUrlList &fileUrlList, bool &accepted) const
{
    Q_UNUSED(fileUrlList)

    accepted = false;

    return false;
}

bool DAbstractFileController::decompressFileHere(const DUrlList &fileUrlList, bool &accepted) const
{
    Q_UNUSED(fileUrlList)

    accepted = false;

    return false;
}

bool DAbstractFileController::copyFilesToClipboard(const DUrlList &urlList, bool &accepted) const
{
    Q_UNUSED(urlList)

    accepted = false;

    return false;
}

bool DAbstractFileController::renameFile(const DUrl &oldUrl, const DUrl &newUrl, bool &accepted) const
{
    Q_UNUSED(oldUrl)
    Q_UNUSED(newUrl)

    accepted = false;

    return false;
}

bool DAbstractFileController::deleteFiles(const DFMEvent &event, bool &accepted) const
{
    Q_UNUSED(event)
    accepted = false;

    return false;
}

DUrlList DAbstractFileController::moveToTrash(const DFMEvent &event, bool &accepted) const
{
    Q_UNUSED(event)

    accepted = false;

    return DUrlList();
}

bool DAbstractFileController::cutFilesToClipboard(const DUrlList &urlList, bool &accepted) const
{
    Q_UNUSED(urlList)

    accepted = false;

    return false;
}

DUrlList DAbstractFileController::pasteFile(PasteType type, const DUrl &targetUrl, const DFMEvent &event, bool &accepted) const
{
    Q_UNUSED(type)
    Q_UNUSED(targetUrl)
    Q_UNUSED(event)

    accepted = false;

    return DUrlList();
}

bool DAbstractFileController::restoreFile(const DUrl &srcUrl, const DUrl &tarUrl, const DFMEvent &event, bool &accepted) const
{
    Q_UNUSED(srcUrl)
    Q_UNUSED(tarUrl)
    Q_UNUSED(event)
    accepted = false;

    return false;
}

bool DAbstractFileController::newFolder(const DFMEvent &event, bool &accepted) const
{
    Q_UNUSED(event)

    accepted = false;

    return false;
}

bool DAbstractFileController::newFile(const DUrl &toUrl, bool &accepted) const
{
    Q_UNUSED(toUrl)

    accepted = false;

    return false;
}

bool DAbstractFileController::newDocument(const DUrl &toUrl, bool &accepted) const
{
    Q_UNUSED(toUrl)

    accepted = false;

    return false;
}

bool DAbstractFileController::addUrlMonitor(const DUrl &url, bool &accepted) const
{
    Q_UNUSED(url)

    accepted = false;

    return false;
}

bool DAbstractFileController::removeUrlMonitor(const DUrl &url, bool &accepted) const
{
    Q_UNUSED(url)

    accepted = false;

    return false;
}

bool DAbstractFileController::openFileLocation(const DUrl &fileUrl, bool &accepted) const
{
    Q_UNUSED(fileUrl)

    accepted = false;

    return false;
}

const QList<DAbstractFileInfoPointer> DAbstractFileController::getChildren(const DUrl &fileUrl, const QStringList &nameFilters,
                                                                         QDir::Filters filters, QDirIterator::IteratorFlags flags,
                                                                         bool &accepted) const
{
    Q_UNUSED(fileUrl)
    Q_UNUSED(nameFilters)
    Q_UNUSED(filters)
    Q_UNUSED(flags)

    accepted = false;

    return QList<DAbstractFileInfoPointer>();
}

const DAbstractFileInfoPointer DAbstractFileController::createFileInfo(const DUrl &fileUrl, bool &accepted) const
{
    Q_UNUSED(fileUrl)

    accepted = false;

    return DAbstractFileInfoPointer();
}

const DDirIteratorPointer DAbstractFileController::createDirIterator(const DUrl &fileUrl, const QStringList &nameFilters,
                                                                    QDir::Filters filters, QDirIterator::IteratorFlags flags,
                                                                    bool &accepted) const
{
    Q_UNUSED(fileUrl)
    Q_UNUSED(nameFilters)
    Q_UNUSED(filters)
    Q_UNUSED(flags)

    accepted = false;

    return DDirIteratorPointer();
}

bool DAbstractFileController::createSymlink(const DUrl &fileUrl, const DUrl &linkToUrl, bool &accepted) const
{
    Q_UNUSED(fileUrl)
    Q_UNUSED(linkToUrl)

    accepted = false;

    return false;
}

bool DAbstractFileController::openInTerminal(const DUrl &fileUrl, bool &accepted) const
{
    Q_UNUSED(fileUrl)

    accepted = false;

    return false;
}
