

#include "tagfileinfo.h"
#include "dfileservices.h"
#include "dfilesystemmodel.h"
#include "dabstractfileinfo.h"
#include "../interfaces/durl.h"
#include "../private/dfileinfo_p.h"


TagFileInfo::TagFileInfo(const DUrl &url)
            :DAbstractFileInfo{ url, false } //###: Do not cache.
{
    ///###: if the true url of file is put into fragment field of Uri. Then setup proxy.
    if(!url.taggedLocalFilePath().isEmpty()){
        DAbstractFileInfoPointer infoPointer{ DFileService::instance()->createFileInfo(nullptr, DUrl::fromLocalFile(url.fragment())) };
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

    if (d->proxy) {
        return d->proxy->exists();
    }

    return fileUrl() == DUrl(TAG_ROOT);
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
    const DAbstractFileInfoPrivate* d{ d_func() };

    QVector<MenuAction> actions{};
    DUrl current_url{ this->fileUrl() };
    QString parent_url{ current_url.parentUrl().path()};

    ///###: if there current file-info do not have a proxy!
    ///###: it shows that current item is a tag-dir(tag:///tag-name).
    if(current_url.isTaggedFile() && parent_url == QString{"/"} && !d->proxy){
        actions.push_back(MenuAction::Open);
        actions.push_back(MenuAction::OpenInNewWindow);
        actions.push_back(MenuAction::OpenInNewTab);

    }else{
        actions = DAbstractFileInfo::menuActionList(type);
    }

    return actions;
}




DUrl TagFileInfo::parentUrl() const
{
    DUrl url = fileUrl();

    if(url.taggedLocalFilePath().isEmpty()){
        return DAbstractFileInfo::parentUrl();
    }

    return DUrl::fromUserTaggedFile(url.path(), QString{});
}

QString TagFileInfo::iconName() const
{
    DUrl current_url{ this->fileUrl() };

    if(current_url.isTaggedFile()){
        return QString{"folder"};
    }

    return DAbstractFileInfo::iconName();
}

DUrl TagFileInfo::goToUrlWhenDeleted() const
{
    const DAbstractFileInfoPrivate* d{ d_func() };

    DUrl current_url{ this->fileUrl() };
    QString parent_url{ current_url.parentUrl().path()};

    ///###: if there current file-info do not have a proxy!
    ///###: it shows that current item is a tag-dir(tag:///tag-name).
    if(current_url.isTaggedFile() && parent_url == QString{"/"} && !d->proxy){
        return DUrl::fromLocalFile(QDir::homePath());
    }

    return DAbstractFileInfo::goToUrlWhenDeleted();
}



//bool TagFileInfo::columnDefaultVisibleForRole(int role) const
//{
//    if(role == DFileSystemModel::FileLastModifiedRole){
//        return false;
//    }

//    return DAbstractFileInfo::columnDefaultVisibleForRole(role);
//}




