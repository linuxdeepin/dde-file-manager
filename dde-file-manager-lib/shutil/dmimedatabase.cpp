#include "dmimedatabase.h"
#include "interfaces/durl.h"

#include <stdlib.h>

char* DMimeDatabase::XDG_RUNTIME_DIR = getenv("XDG_RUNTIME_DIR");

DMimeDatabase::DMimeDatabase():
    QMimeDatabase()
{

}

bool DMimeDatabase::isGvfsFile(const QString &fileName)
{
    QString gvfsDir = QString("%1/gvfs").arg(QString(XDG_RUNTIME_DIR));
    if (fileName.startsWith(gvfsDir) && DUrl(fileName) != DUrl(gvfsDir)){
        return true;
    }
    return false;
}

QMimeType DMimeDatabase::mimeTypeForFile(const QString &fileName, QMimeDatabase::MatchMode mode) const
{
    QFileInfo info(fileName);
    if (info.isFile() && isGvfsFile(fileName)){
        mode = MatchExtension;
    }
    return QMimeDatabase::mimeTypeForFile(fileName, mode);
}

QMimeType DMimeDatabase::mimeTypeForFile(const QFileInfo &fileInfo, QMimeDatabase::MatchMode mode) const
{
    if (fileInfo.isFile() && isGvfsFile(fileInfo.absoluteFilePath())){
        mode = MatchExtension;
    }
    return QMimeDatabase::mimeTypeForFile(fileInfo, mode);
}
