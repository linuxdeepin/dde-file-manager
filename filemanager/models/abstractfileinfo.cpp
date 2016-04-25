#include "abstractfileinfo.h"

#include "../views/dfileview.h"

#include "../shutil/fileutils.h"

#include <QDateTime>
#include <QDebug>

AbstractFileInfo::AbstractFileInfo()
    : data(new FileInfoData)
{

}

AbstractFileInfo::AbstractFileInfo(const DUrl &url)
    : data(new FileInfoData)
{
    data->url = url;
    data->fileInfo.setFile(url.path());
}

AbstractFileInfo::AbstractFileInfo(const QString &url)
    : data(new FileInfoData)
{
    data->url = DUrl::fromUserInput(url);
    data->fileInfo.setFile(data->url.path());
}

AbstractFileInfo::~AbstractFileInfo()
{
    delete data;
}

void AbstractFileInfo::setUrl(const DUrl &url)
{
    data->url = url;
    data->fileInfo.setFile(url.path());
}

bool AbstractFileInfo::exists() const
{
    return data->fileInfo.exists();
}

QString AbstractFileInfo::filePath() const
{
    return data->fileInfo.filePath();
}

QString AbstractFileInfo::absoluteFilePath() const
{
    return data->fileInfo.absoluteFilePath();
}

QString AbstractFileInfo::fileName() const
{
    return data->fileInfo.fileName();
}

QString AbstractFileInfo::displayName() const
{
    return fileName();
}

QString AbstractFileInfo::path() const
{
    return data->fileInfo.path();
}

QString AbstractFileInfo::absolutePath() const
{
    return data->fileInfo.absolutePath();
}

bool AbstractFileInfo::isReadable() const
{
    return data->fileInfo.isReadable();
}

bool AbstractFileInfo::isWritable() const
{
    return data->fileInfo.isWritable();
}

bool AbstractFileInfo::isExecutable() const
{
    return data->fileInfo.isExecutable();
}

bool AbstractFileInfo::isHidden() const
{
    return data->fileInfo.isHidden();
}

bool AbstractFileInfo::isRelative() const
{
    return data->fileInfo.isRelative();
}

bool AbstractFileInfo::isAbsolute() const
{
    return data->fileInfo.isAbsolute();
}

bool AbstractFileInfo::makeAbsolute()
{
    bool ok = data->fileInfo.makeAbsolute();

    data->url.setPath(data->fileInfo.filePath());

    return ok;
}

bool AbstractFileInfo::isFile() const
{
    return data->fileInfo.isFile();
}

bool AbstractFileInfo::isDir() const
{
    return data->fileInfo.isDir();
}

bool AbstractFileInfo::isSymLink() const
{
    return data->fileInfo.isSymLink();
}

QString AbstractFileInfo::readLink() const
{
    return data->fileInfo.readLink();
}

QString AbstractFileInfo::owner() const
{
    return data->fileInfo.owner();
}

uint AbstractFileInfo::ownerId() const
{
    return data->fileInfo.ownerId();
}

QString AbstractFileInfo::group() const
{
    return data->fileInfo.group();
}

uint AbstractFileInfo::groupId() const
{
    return data->fileInfo.groupId();
}

QFileDevice::Permissions AbstractFileInfo::permissions() const
{
    return data->fileInfo.permissions();
}

qint64 AbstractFileInfo::size() const
{
    return data->fileInfo.size();
}

QDateTime AbstractFileInfo::created() const
{
    return data->fileInfo.created();
}

QDateTime AbstractFileInfo::lastModified() const
{
    return data->fileInfo.lastModified();
}

QDateTime AbstractFileInfo::lastRead() const
{
    return data->fileInfo.lastRead();
}

DUrl AbstractFileInfo::parentUrl() const
{
    DUrl url = data->url;
    const QString &path = url.path();

    url.setPath(path.left(path.lastIndexOf('/') + 1));

    return url;
}

QVector<AbstractFileInfo::MenuAction> AbstractFileInfo::menuActionList(AbstractFileInfo::MenuType type) const
{
    QVector<MenuAction> actionKeys;

    if(type == SpaceArea) {
        actionKeys.reserve(9);

        actionKeys << NewFolder
                   << NewDocument
                   << Separator
                   << DisplayAs
                   << SortBy
                   << OpenInTerminal
                   << Separator
                   << Property;
    } else {
        actionKeys.reserve(11);

        actionKeys << Open << (isDir() ? OpenInNewWindow : OpenWith)
                   << Separator;
        if (isDir()){
            actionKeys << Compress;
        }else if(isFile()){
            if (FileUtils::isArchive(absoluteFilePath())){
                actionKeys << Decompress << DecompressHere;
            }else{
                actionKeys << Compress;
            }
        }
        actionKeys << Separator
                   << Cut
                   << Copy
                   << CreateSoftLink
                   << SendToDesktop
                   << Rename;
        QPixmap tempPixmap;
        DUrl url = data->url;
        if (tempPixmap.load(url.toLocalFile())){
            actionKeys << SetAsWallpaper;
        }

        actionKeys << Separator
                   << Delete
                   << Separator
                   << Property;
    }

    return actionKeys;
}


quint8 AbstractFileInfo::supportViewMode() const
{
    return DFileView::AllViewMode;
}

QMap<AbstractFileInfo::MenuAction, QVector<AbstractFileInfo::MenuAction> > AbstractFileInfo::subMenuActionList() const
{
    QMap<MenuAction, QVector<MenuAction> > actions;

    QVector<MenuAction> openwithMenuActionKeys;
    openwithMenuActionKeys << OpenWithCustom;
    actions.insert(OpenWith, openwithMenuActionKeys);


    QVector<MenuAction> docmentMenuActionKeys;
    docmentMenuActionKeys << NewWord
                          << NewExcel
                          << NewPowerpoint
                          << NewText;
    actions.insert(NewDocument, docmentMenuActionKeys);

    QVector<MenuAction> displayAsMenuActionKeys;
    displayAsMenuActionKeys << IconView
                          << ListView
                          << ExtendView;

    actions.insert(DisplayAs, displayAsMenuActionKeys);


    QVector<MenuAction> sortByMenuActionKeys;
    sortByMenuActionKeys << Name
                          << Size
                          << Type
                          << CreatedDate
                          << LastModifiedDate;
    actions.insert(SortBy, sortByMenuActionKeys);

    return actions;
}

QT_BEGIN_NAMESPACE
QDebug operator<<(QDebug deg, const AbstractFileInfo &info)
{
    deg << "file url:" << info.fileUrl()
        << "mime type:" << info.mimeTypeName();

    return deg;
}
QT_END_NAMESPACE
