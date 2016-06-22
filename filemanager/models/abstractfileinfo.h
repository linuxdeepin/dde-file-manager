#ifndef ABSTRACTFILEINFO_H
#define ABSTRACTFILEINFO_H

#include <functional>

#include <QSharedDataPointer>
#include <QFile>
#include <QFileInfo>
#include <QDateTime>
#include <QMap>

#include "durl.h"
#include "menuactiontype.h"

class AbstractFileInfo;
typedef QExplicitlySharedDataPointer<AbstractFileInfo> AbstractFileInfoPointer;
typedef std::function<const AbstractFileInfoPointer(int)> getFileInfoFun;
typedef MenuActionType::MenuAction MenuAction;

class AbstractFileInfo : public QSharedData
{

public:

    enum MenuType {
        SingleFile,
        MultiFiles,
        MultiFilesSystemPathIncluded,
        SpaceArea
    };

    enum ColumnType {
        DisplayNameType = 0,
        SizeType = 1,
        LastModifiedDateType = 2,
        CreatedDateType = 3,
        FileMimeType = 4,
        UserType = 5
    };

    inline static QString timeFormat(){
        return "yyyy/MM/dd HH:mm:ss";
    }

    AbstractFileInfo();
    AbstractFileInfo(const DUrl &url);
    AbstractFileInfo(const QString &url);

    virtual ~AbstractFileInfo();

    inline AbstractFileInfo &operator =(const AbstractFileInfo &other)
    {data = other.data; return *this;}

    virtual void setUrl(const DUrl &url);
    virtual bool exists() const;

    virtual QString filePath() const;
    virtual QString absoluteFilePath() const;
    virtual QString fileName() const;
    virtual QString displayName() const;

    virtual QString path() const;
    virtual QString absolutePath() const;

    virtual bool isCanRename() const = 0;
    virtual bool isReadable() const;
    virtual bool isWritable() const;
    virtual bool isExecutable() const;
    virtual bool isHidden() const;

    virtual bool isRelative() const;
    virtual bool isAbsolute() const;
    virtual bool makeAbsolute();

    virtual bool isFile() const;
    virtual bool isDir() const;
    virtual bool isSymLink() const;
    virtual bool isDesktopFile() const;


    virtual QString readLink() const;
    inline QString symLinkTarget() const { return readLink(); }

    virtual QString owner() const;
    virtual uint ownerId() const;
    virtual QString group() const;
    virtual uint groupId() const;

    inline bool permission(QFile::Permissions permissions) const
    {return (permissions & this->permissions()) == permissions;}

    virtual QFile::Permissions permissions() const;

    virtual qint64 size() const;
    virtual qint64 filesCount() const;

    virtual QDateTime created() const;
    virtual QDateTime lastModified() const;
    virtual QDateTime lastRead() const;

    virtual QString mimeTypeName() const
    {return data->mimeTypeName;}

    virtual QString lastModifiedDisplayName() const;
    virtual QString createdDisplayName() const;
    virtual QString sizeDisplayName() const;
    virtual QString mimeTypeDisplayName() const;
    virtual int mimeTypeDisplayNameOrder() const;

    inline DUrl fileUrl() const
    {return data->url;}

    inline QString scheme() const
    {return data->url.scheme();}

    virtual QIcon fileIcon() const = 0;

    virtual DUrl parentUrl() const;
    virtual QVector<MenuAction> menuActionList(MenuType type = SingleFile) const;
    virtual QMap<MenuAction, QVector<MenuAction> > subMenuActionList() const;
    virtual QSet<MenuAction> disableMenuActionList() const;

    /// return DFileView::ViewMode flags
    virtual quint8 supportViewMode() const;

    virtual quint8 userColumnCount() const;

    /// userColumnType = ColumnType::UserType + user column index
    virtual QVariant userColumnDisplayName(quint8 userColumnType) const;

    /// get custom column data
    virtual QVariant userColumnData(quint8 userColumnType) const;

    /// get custom column width
    virtual int userColumnWidth(quint8 userColumntype) const;

    virtual void sortByColumn(QList<AbstractFileInfoPointer> &fileList, quint8 columnType,
                              Qt::SortOrder order = Qt::AscendingOrder) const;

    /// getFileInfoFun is get AbstractFileInfoPointer by index for caller. if return -1 then insert file list last
    virtual int getIndexByFileInfo(getFileInfoFun fun, const AbstractFileInfoPointer &info, quint8 columnType,
                                   Qt::SortOrder order = Qt::AscendingOrder) const;

    virtual bool canRedirectionFileUrl() const;
    virtual DUrl redirectedFileUrl() const;

    virtual bool isEmptyFloder() const;

    static Qt::SortOrder sortOrderGlobal;

protected:
    virtual void sortByUserColumn(QList<AbstractFileInfoPointer> &fileList, quint8 columnType,
                                  Qt::SortOrder order = Qt::AscendingOrder) const;

    struct FileInfoData
    {
        DUrl url;
        mutable QString mimeTypeName;
        QFileInfo fileInfo;

        bool exists;
        QString filePath;
        QString absoluteFilePath;
        QString fileName;
        QString displayName;
        QString path;
        QString absolutePath;

        qint64 size = -1;
        QDateTime created;
        QDateTime lastModified;
    };

    FileInfoData *data;

private:
    struct FileMetaData
    {
        bool isReadable;
        bool isWritable;
        bool isExecutable;

        QFile::Permissions permissions;
    };

    inline const FileMetaData metaData() const
    {
        return metaDataCacheMap.value(data->url);
    }

    void updateFileMetaData();

    static QMap<DUrl, FileMetaData> metaDataCacheMap;
};

QT_BEGIN_NAMESPACE
QDebug operator<<(QDebug deg, const AbstractFileInfo &info);
QT_END_NAMESPACE

#endif // ABSTRACTFILEINFO_H
