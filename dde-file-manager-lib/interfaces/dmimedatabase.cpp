#include "dmimedatabase.h"
#include "shutil/fileutils.h"

#include <QFileInfo>

DFM_BEGIN_NAMESPACE

DMimeDatabase::DMimeDatabase()
{

}

QMimeType DMimeDatabase::mimeTypeForFile(const QString &fileName, QMimeDatabase::MatchMode mode) const
{
    QFileInfo fileInfo(fileName);

    // Ignore the mode argument if file is retome file
    if (!fileInfo.isDir() && FileUtils::isGvfsMountFile(fileInfo.absoluteFilePath()))
        return QMimeDatabase::mimeTypeForFile(fileName, QMimeDatabase::MatchExtension);

    return QMimeDatabase::mimeTypeForFile(fileName, mode);
}

QMimeType DMimeDatabase::mimeTypeForFile(const QFileInfo &fileInfo, QMimeDatabase::MatchMode mode) const
{
    // Ignore the mode argument if file is retome file
    if (!fileInfo.isDir() && FileUtils::isGvfsMountFile(fileInfo.absoluteFilePath()))
        return QMimeDatabase::mimeTypeForFile(fileInfo, QMimeDatabase::MatchExtension);

    return QMimeDatabase::mimeTypeForFile(fileInfo, mode);
}

QMimeType DMimeDatabase::mimeTypeForUrl(const QUrl &url) const
{
    if (url.isLocalFile())
        return mimeTypeForFile(url.toLocalFile());

    return QMimeDatabase::mimeTypeForUrl(url);
}

DFM_END_NAMESPACE
