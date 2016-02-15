
#include "dbustype.h"

FileItemInfo::FileItemInfo(){

}

FileItemInfo::~FileItemInfo(){

}

void FileItemInfo::registerMetaType()
{
    qRegisterMetaType<FileItemInfo>("DesktoItemInfo");
    qDBusRegisterMetaType<FileItemInfo>();
}


QDBusArgument &operator<<(QDBusArgument &argument, const FileItemInfo &obj)
{
    argument.beginStructure();
    argument << obj.DisplayName << obj.BaseName;
    argument << obj.URI << obj.MIME;
    //argument << obj.Icon <<  obj.IconName << obj.thumbnail;
    argument << obj.Size << obj.FileType << obj.IsBackup << obj.IsHidden;
    argument << obj.IsReadOnly << obj.IsSymlink;
    argument << obj.CanDelete << obj.CanExecute;
    argument << obj.CanRead << obj.CanRename;
    argument << obj.CanTrash << obj.CanWrite;
    argument.endStructure();
    return argument;
}


const QDBusArgument &operator>>(const QDBusArgument &argument, FileItemInfo &obj)
{
    argument.beginStructure();
    argument >> obj.DisplayName >> obj.BaseName;
    argument >> obj.URI >> obj.MIME;
//    argument >> obj.Icon >> obj.IconName >> obj.thumbnail;
    argument >> obj.Size >> obj.FileType >> obj.IsBackup >> obj.IsHidden;
    argument >> obj.IsReadOnly >> obj.IsSymlink;
    argument >> obj.CanDelete >> obj.CanExecute;
    argument >> obj.CanRead >> obj.CanRename;
    argument >> obj.CanTrash >> obj.CanWrite;
    argument.endStructure();
    return argument;
}


ConflictInfo::ConflictInfo(){

}

ConflictInfo::~ConflictInfo(){

}

void ConflictInfo::registerMetaType()
{
    qRegisterMetaType<ConflictInfo>("ConflictInfo");
    qDBusRegisterMetaType<ConflictInfo>();
}


QDBusArgument &operator<<(QDBusArgument &argument, const ConflictInfo &obj)
{
    argument.beginStructure();
    argument << obj.code << obj.applyToAll;
    argument << obj.userData;
    argument.endStructure();
    return argument;
}


const QDBusArgument &operator>>(const QDBusArgument &argument, ConflictInfo &obj)
{
    argument.beginStructure();
    argument >> obj.code >> obj.applyToAll;
    argument >> obj.userData;
    argument.endStructure();
    return argument;
}

QDebug operator<<(QDebug deg, const FileItemInfo &info)
{
    deg << "name:" << info.DisplayName << "url:" << info.URI << "size:"
        << info.Size << "type:" << info.FileType;

    return deg;
}
