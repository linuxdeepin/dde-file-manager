#include "fileinfogatherer.h"

#include <QDirIterator>

FileInfoGatherer::FileInfoGatherer(QObject *parent) : QObject(parent)
{

}

void FileInfoGatherer::fetchFileInformation(const QString &path, int filter)
{
    FileInfoList infolist;

    if(path.isEmpty()) {
        const QFileInfoList list = QDir::drives();

        for(const QFileInfo &info : list) {
            FileInfo fileInfo = FileInfo(info);

            infolist.append(fileInfo);

            emit addFileInfo(path, fileInfo);
        }
    } else {
        QDirIterator dirIt(path, QDir::Filters(filter));
        FileInfo fileInfo;

        while (dirIt.hasNext()) {
            dirIt.next();

            if(dirIt.fileInfo().absoluteFilePath() == path)
                continue;

            fileInfo = FileInfo(dirIt.fileInfo());
            infolist.append(fileInfo);

            emit addFileInfo(path, fileInfo);
        }
    }

    emit updates(path, infolist);
}
