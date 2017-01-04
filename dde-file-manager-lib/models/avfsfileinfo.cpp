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

    QString archRootPath;
};

AVFSFileInfo::AVFSFileInfo(const QString &archRootPath, const DUrl &avfsUrl):
    DAbstractFileInfo(*new AVFSFileInfoPrivate(avfsUrl, this))
{
    Q_D(AVFSFileInfo);

    d->archRootPath = archRootPath;
    setProxy(DAbstractFileInfoPointer(new DFileInfo(realFileUrl(archRootPath, avfsUrl))));
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
    QString realFilePath = realFileUrl(d->archRootPath, fileUrl()).toLocalFile();
    if(FileUtils::isArchive(realFilePath)){
        if(d->archRootPath != fileUrl().path())
            return false;
    }
    return d->proxy->isDir();
}

AVFSFileInfo::AVFSFileInfo(AVFSFileInfoPrivate &dd):
    DAbstractFileInfo(dd)
{
}

DUrl AVFSFileInfo::realFileUrl(QString archRootPath, const DUrl &avfsUrl)
{
    if (archRootPath.endsWith("/"))
        archRootPath.chop(1);

    QString avfsPath = avfsUrl.path();

    if (avfsPath.endsWith("/"))
        avfsPath.chop(1);

    QString realPath;

    QString mountPath = QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first();
    mountPath = mountPath + "/.avfs";

    //Check if is the url of an arch's root path
    if (archRootPath == avfsPath)
        realPath = mountPath + avfsPath.replace(archRootPath, (archRootPath+ "#/"));
    else
        realPath = mountPath + avfsPath.replace(archRootPath, (archRootPath+ "#"));

    return DUrl::fromLocalFile(realPath);
}
