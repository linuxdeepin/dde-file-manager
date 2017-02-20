#include "avfsfileinfo.h"

#include "dfileinfo.h"
#include "private/dabstractfileinfo_p.h"
#include "dfilesystemmodel.h"
#include "controllers/avfsfilecontroller.h"
#include "shutil/fileutils.h"

#include <QFileInfo>
#include <QStandardPaths>
#include <QIcon>

class AVFSFileInfoPrivate : public DAbstractFileInfoPrivate
{
public:
    AVFSFileInfoPrivate(const DUrl &url, AVFSFileInfo *qq)
        : DAbstractFileInfoPrivate(url, qq, true) {
    }
};

AVFSFileInfo::AVFSFileInfo(const DUrl &avfsUrl):
    DAbstractFileInfo(*new AVFSFileInfoPrivate(avfsUrl, this))
{
    Q_D(AVFSFileInfo);

    setProxy(DAbstractFileInfoPointer(new DFileInfo(realFileUrl(avfsUrl))));
}

bool AVFSFileInfo::canRename() const
{
    return false;
}

bool AVFSFileInfo::isWritable() const
{
    return false;
}

bool AVFSFileInfo::canShare() const
{
    return false;
}

QVector<MenuAction> AVFSFileInfo::menuActionList(DAbstractFileInfo::MenuType type) const
{

    QVector<MenuAction> actions;
    if(type == DAbstractFileInfo::SingleFile){
        actions << MenuAction::Open;
        if(!isDir())
            actions << MenuAction::OpenWith;
        actions << MenuAction::Separator;
        actions << MenuAction::Copy
                << MenuAction::Property;

    } else if(type == DAbstractFileInfo::SpaceArea){
        actions << MenuAction::SortBy
                << MenuAction::DisplayAs
                << MenuAction::Property;
    } else{
        actions << MenuAction::Open
                << MenuAction::Separator
                << MenuAction::Copy
                << MenuAction::Property;
    }
    return actions;
}

bool AVFSFileInfo::canIteratorDir() const
{
    return true;
}

bool AVFSFileInfo::isDir() const
{
    Q_D(const AVFSFileInfo);
    //Temporarily just support one lay arch file parser
    QString realFilePath = realFileUrl(fileUrl()).toLocalFile();
    if(FileUtils::isArchive(realFilePath)){
        realFilePath += "#/";
        return QFileInfo(realFilePath).isDir();
    }
    return d->proxy->isDir();
}

AVFSFileInfo::AVFSFileInfo(AVFSFileInfoPrivate &dd):
    DAbstractFileInfo(dd)
{
}

DUrl AVFSFileInfo::realFileUrl(const DUrl &avfsUrl)
{
    QString avfsPath = avfsUrl.path();

    if (avfsPath.endsWith("/"))
        avfsPath.chop(1);

    QString mountPath = QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first();
    mountPath = mountPath + "/.avfs";

    QString virtualPath = mountPath + avfsPath;
    QStringList pathItems = virtualPath.split("/");
    QString iterPath = "/";
    int archLength = 0;
    foreach (QString item, pathItems) {
        if(item == "")
            continue;
        iterPath += item;
        if(FileUtils::isArchive(iterPath)){
            archLength++;
            iterPath += "#/";
        }
        else
            iterPath += "/";
    }

    if(archLength > 1){
        if(iterPath.endsWith("#/"))
            iterPath.chop(2);
        else
            iterPath.chop(1);
    } else
        iterPath.chop(1);

    return DUrl::fromLocalFile(iterPath);
}

DUrl AVFSFileInfo::realDirUrl(const DUrl &avfsUrl)
{
    QString avfsPath = avfsUrl.path();

    if (avfsPath.endsWith("/"))
        avfsPath.chop(1);

    QString mountPath = QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first();
    mountPath = mountPath + "/.avfs";

    QString virtualPath = mountPath + avfsPath;
    QStringList pathItems = virtualPath.split("/");
    QString iterPath = "/";
    foreach (QString item, pathItems) {
        if(item == "")
            continue;
        iterPath += item;
        if(FileUtils::isArchive(iterPath))
            iterPath += "#/";
        else
            iterPath += "/";
    }

    iterPath.chop(1);

    return DUrl::fromLocalFile(iterPath);
}
