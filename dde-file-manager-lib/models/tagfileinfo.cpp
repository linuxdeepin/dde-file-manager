

#include "tagfileinfo.h"
#include "dfileservices.h"
#include "dfilesystemmodel.h"
#include "dabstractfileinfo.h"
#include "durl.h"
#include "private/dfileinfo_p.h"
#include "tag/tagmanager.h"

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

    if (fileUrl() == DUrl(TAG_ROOT)) {
        return true;
    }

    const QMap<QString, QString> &tag_map = TagManager::instance()->getAllTags();

    return tag_map.contains(fileUrl().tagName());
}

bool TagFileInfo::canRename() const
{
    Q_D(const DAbstractFileInfo);

    if (d->proxy) {
        return d->proxy->canRename();
    }

    return fileUrl() != DUrl(TAG_ROOT);
}

bool TagFileInfo::isTaged() const
{
    return true;
}

bool TagFileInfo::isWritable() const
{
    Q_D(const DAbstractFileInfo);

    if (d->proxy)
        return d->proxy->isWritable();

    return true;
}

bool TagFileInfo::canRedirectionFileUrl() const
{
    const DAbstractFileInfoPrivate* const d{ d_func() };

    return static_cast<bool>(d->proxy);
}

QFileDevice::Permissions TagFileInfo::permissions() const
{
    Q_D(const DAbstractFileInfo);

    if (d->proxy)
        return d->proxy->permissions();

    return QFile::ReadGroup | QFile::ReadOwner | QFile::ReadUser | QFile::ReadOther
            | QFile::WriteGroup | QFile::WriteOwner | QFile::WriteUser | QFile::WriteOther;
}


DUrl TagFileInfo::redirectedFileUrl() const
{
    const DAbstractFileInfoPrivate* const d{ d_func() };

    return d->proxy->fileUrl();
}

Qt::ItemFlags TagFileInfo::fileItemDisableFlags() const
{
    Q_D(const DAbstractFileInfo);

    if (d->proxy)
        return d->proxy->fileItemDisableFlags();

    return fileUrl() != DUrl(TAG_ROOT) ? Qt::ItemIsDragEnabled : Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
}

QVector<MenuAction> TagFileInfo::menuActionList(DAbstractFileInfo::MenuType type) const
{
    Q_D(const DAbstractFileInfo);

    QVector<MenuAction> actions;

    if (!d->proxy) {
        if (type == SpaceArea) {
            actions << MenuAction::DisplayAs;
            actions << MenuAction::SortBy;
        } else {
            actions << MenuAction::Open;
            actions << MenuAction::OpenInNewWindow;
            actions << MenuAction::OpenInNewTab;
            actions << MenuAction::Rename;
            actions << MenuAction::ChangeTagColor;
        }

        return actions;
    }

    actions = d->proxy->menuActionList(type);
    actions.insert(1, MenuAction::OpenFileLocation);

    return actions;
}

DUrl TagFileInfo::getUrlByNewFileName(const QString &name) const
{
    DUrl new_url = fileUrl();
    const QString &local_file = new_url.taggedLocalFilePath();

    if (local_file.isEmpty())
        return DAbstractFileInfo::getUrlByNewFileName(name);

    QFileInfo file_info(local_file);

    new_url.setTaggedFileUrl(file_info.absoluteDir().absoluteFilePath(name));

    return new_url;
}

bool TagFileInfo::canIteratorDir() const
{
    Q_D(const DAbstractFileInfo);

    if (!d->proxy)
        return true;

    return d->proxy->canIteratorDir();
}

QVariantHash TagFileInfo::extraProperties() const
{
    Q_D(const DAbstractFileInfo);

    if (d->proxy)
        return d->proxy->extraProperties();

    QVariantHash hash;

    if (fileUrl() == DUrl(TAG_ROOT))
        return hash;

    const QString &tag_name = fileUrl().tagName();
    const QColor &tag_color = TagManager::instance()->getTagColor({tag_name}).value(tag_name);

    hash["tag_name_list"] = QStringList{tag_name};
    hash["colored"] = QVariant::fromValue(QList<QColor>{tag_color});

    return hash;
}

QList<int> TagFileInfo::userColumnRoles() const
{
    Q_D(const DAbstractFileInfo);

    if (d->proxy)
        return d->proxy->userColumnRoles();

    if (fileUrl() == DUrl(TAG_ROOT)) {
        return QList<int> {
              DFileSystemModel::FileDisplayNameRole,
              DFileSystemModel::FileSizeRole
        };
    }

    return DAbstractFileInfo::userColumnRoles();
}

DUrl TagFileInfo::mimeDataUrl() const
{
    Q_D(const DAbstractFileInfo);

    if (d->proxy)
        return d->proxy->mimeDataUrl();

    return DUrl();
}

Qt::DropActions TagFileInfo::supportedDragActions() const
{
    Q_D(const DAbstractFileInfo);

    if (d->proxy)
        return d->proxy->supportedDragActions();

    return DAbstractFileInfo::supportedDragActions();
}

Qt::DropActions TagFileInfo::supportedDropActions() const
{
    Q_D(const DAbstractFileInfo);

    if (d->proxy)
        return d->proxy->supportedDropActions();

    if (fileUrl().tagName().isEmpty())
        return Qt::IgnoreAction;

    return Qt::CopyAction;
}

bool TagFileInfo::canDrop() const
{
    Q_D(const DAbstractFileInfo);

    if (d->proxy)
        return d->proxy->canDrop();

    return !fileUrl().tagName().isEmpty();
}

DUrl TagFileInfo::parentUrl() const
{
    DUrl url = fileUrl();

    if (url.taggedLocalFilePath().isEmpty()) {
        return DAbstractFileInfo::parentUrl();
    }

    return DUrl::fromUserTaggedFile(url.tagName(), QString{});
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




