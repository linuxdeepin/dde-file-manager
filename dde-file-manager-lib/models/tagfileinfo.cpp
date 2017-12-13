

#include "tagfileinfo.h"
#include "dfileservices.h"
#include "dfilesystemmodel.h"
#include "dabstractfileinfo.h"
#include "../interfaces/durl.h"
#include "../private/dfileinfo_p.h"


TagFileInfo::TagFileInfo(const DUrl &url)
            :DAbstractFileInfo{ url } //###: hasCached = true.
{
    ///###: if the true url of file is put into fragment field of Uri. Then setup proxy.
    if(url.tagedFileUrl().isValid() == true){
        DUrl uri{ url.fragment() };
        uri.setTagedFileUrl( DUrl{} ); //###: In fact, invoke QUrl::setFragment();

        DAbstractFileInfoPointer infoPointer{ DFileService::instance()->createFileInfo(nullptr, url.tagedFileUrl()) };
        this->DAbstractFileInfo::setProxy(infoPointer);
    }
}

bool TagFileInfo::isDir() const
{
    const DAbstractFileInfoPrivate* const d{ d_func() };

    return  ((!static_cast<bool>(d->proxy)) || d->proxy->isDir());
}

bool TagFileInfo::makeAbsolute()
{
    return true;
}


bool TagFileInfo::exists() const
{
    const DAbstractFileInfoPrivate* const d{ d_func() };

    return ((!static_cast<bool>(d->proxy)) || d->proxy->exists());
}

bool TagFileInfo::isTaged() const
{
    return true;
}

bool TagFileInfo::isReadable() const
{
    return true;
}

bool TagFileInfo::isWritable() const
{
    return true;
}

bool TagFileInfo::canRedirectionFileUrl() const
{
    const DAbstractFileInfoPrivate* const d{ d_func() };

    return static_cast<bool>(d->proxy);
}


DUrl TagFileInfo::redirectedFileUrl() const
{
    const DAbstractFileInfoPrivate* const d{ d_func() };

    return d->proxy->fileUrl();
}

QString TagFileInfo::fileDisplayName() const
{
    return (this->DAbstractFileInfo::baseName());
}

Qt::ItemFlags TagFileInfo::fileItemDisableFlags() const
{
    return Qt::ItemIsDragEnabled;
}

QSet<MenuAction> TagFileInfo::disableMenuActionList() const
{
    QSet<MenuAction> list;
    return list;
}

QVector<MenuAction> TagFileInfo::menuActionList(DAbstractFileInfo::MenuType type) const
{
    QVector<MenuAction> actions;

    if(type == SpaceArea){
        actions.push_back(MenuAction::DisplayAs);
        actions.push_back(MenuAction::SortBy);

    }else if(type == SingleFile){
        if(this->isDir() == true){
            actions.push_back(MenuAction::Open);
            actions.push_back(MenuAction::OpenInNewWindow);
            actions.push_back(MenuAction::OpenInNewTab);
            actions.push_back(MenuAction::Separator);
            actions.push_back(MenuAction::Rename);
//            actions.push_back(MenuAction::UnTaged);
        }

        actions.push_back(MenuAction::Separator);
        actions.push_back(MenuAction::Property);

    }else if(type == MultiFiles){
        actions.push_back(MenuAction::Open);
        actions.push_back(MenuAction::Separator);
        actions.push_back(MenuAction::Property);
    }

    return actions;
}


//bool TagFileInfo::columnDefaultVisibleForRole(int role) const
//{
//    if(role == DFileSystemModel::FileLastModifiedRole){
//        return false;
//    }

//    return DAbstractFileInfo::columnDefaultVisibleForRole(role);
//}


//DUrl TagFileInfo::parentUrl() const
//{
//    const DAbstractFileInfoPrivate* const d{ d_func() };

//    if(static_cast<bool>(d->proxy) == true){
//        QString path{ d->proxy->path() };
//        std::size_t index{ path.lastIndexOf(QString{"/"}) };
//        path.remove(index, path.size() - index);

//        return DUrl::fromUserTagedFile(path);
//    }
//    return DUrl{};
//}

//DUrl TagFileInfo::mimeDataUrl() const
//{
//    return DUrl::fromLocalFile(this->DAbstractFileInfo::absoluteFilePath());
//}


///###: this will be invoked by DAbstractFileInfo::fileIcon;
QString TagFileInfo::iconName() const
{
    return QString{"folder"};
}





