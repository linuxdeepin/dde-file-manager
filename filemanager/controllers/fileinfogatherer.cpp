#include "fileinfogatherer.h"
#include "desktopfileinfo.h"

#include <QDirIterator>

FileInfoGatherer::FileInfoGatherer(QObject *parent) : QObject(parent)
{

}

void FileInfoGatherer::fetchFileInformation(const QString &path, int filter)
{
    QList<FileInfo*> infolist;

    if(path.isEmpty()) {
        const QFileInfoList list = QDir::drives();

        for(const QFileInfo &info : list) {
            FileInfo *fileInfo = new FileInfo(info);

            infolist.append(fileInfo);
        }
    } else {
        QDirIterator dirIt(path, QDir::Filters(filter));
        FileInfo *fileInfo;

        while (dirIt.hasNext()) {
            dirIt.next();

            if(dirIt.fileInfo().absoluteFilePath() == path)
                continue;

            if(dirIt.fileInfo().suffix() == DESKTOP_SURRIX)
                fileInfo = new DesktopFileInfo(dirIt.fileInfo());
            else
                fileInfo = new FileInfo(dirIt.fileInfo());

            infolist.append(fileInfo);
        }
    }

    emit updates(path, infolist);
}
