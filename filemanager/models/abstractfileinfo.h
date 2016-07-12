#ifndef ABSTRACTFILEINFO_H
#define ABSTRACTFILEINFO_H

#include <functional>

#include <QSharedDataPointer>
#include <QFile>
#include <QFileInfo>
#include <QDateTime>
#include <QMap>
#include <QAbstractItemView>
#include <QMimeType>

#include "durl.h"
#include "menuactiontype.h"

#define SORT_FUN_DEFINE(Value, Name, Type) \
bool sortFileListBy##Name(const AbstractFileInfoPointer &info1, const AbstractFileInfoPointer &info2, Qt::SortOrder order)\
{\
    bool isDir1 = info1->isDir();\
    bool isDir2 = info2->isDir();\
    \
    bool isFile1 = info1->isFile();\
    bool isFile2 = info2->isFile();\
    \
    auto value1 = static_cast<const Type*>(info1.constData())->Value();\
    auto value2 = static_cast<const Type*>(info2.constData())->Value();\
    \
    if (isDir1) {\
        if (!isDir2) return true;\
    } else {\
        if (isDir2) return false;\
    }\
    \
    bool isStrType = typeid(value1) == typeid(QString);\
    if (isStrType) {\
        if (Global::startWithHanzi(value1)) {\
            if (!Global::startWithHanzi(value2)) return false;\
        } else if (Global::startWithHanzi(value2)) {\
            return true;\
        }\
    }\
    \
    if ((isDir1 && isDir2 && (value1 == value2)) || (isFile1 && isFile2 && (value1 == value2))) {\
        return sortByString(info1->displayName(), info2->displayName());\
    }\
    \
    if (isStrType)\
        return sortByString(value1, value2, order);\
    \
    return ((order == Qt::DescendingOrder) ^ (value1 < value2)) == 0x01;\
}

namespace FileSortFunction {
bool sortByString(const QString &str1, const QString &str2, Qt::SortOrder order = Qt::AscendingOrder);
template<typename T>
bool sortByString(T, T, Qt::SortOrder order = Qt::AscendingOrder)
{
    Q_UNUSED(order)

    return false;
}
}

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
    QString pinyinName() const;

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

    virtual QMimeType mimeType() const
    { return data->mimeType;}
    virtual QString mimeTypeName() const
    { return mimeType().name();}

    virtual QString lastReadDisplayName() const;
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
    virtual MenuAction menuActionByColumnRole(int role) const;

    /// return DFileView::ViewMode flags
    virtual quint8 supportViewMode() const;
    /// support selection mode
    virtual QAbstractItemView::SelectionMode supportSelectionMode() const;

    QList<int> userColumnRoles() const
    { return m_userColumnRoles;}

    virtual QVariant userColumnDisplayName(int userColumnRole) const;

    /// get custom column data
    virtual QVariant userColumnData(int userColumnRole) const;

    /// get custom column width
    virtual int userColumnWidth(int userColumnRole) const;

    /// user column default visible for role
    virtual bool columnDefaultVisibleForRole(int role) const;

    typedef std::function<bool(const AbstractFileInfoPointer&, const AbstractFileInfoPointer&, Qt::SortOrder)> sortFunction;
    virtual sortFunction sortFunByColumn(int columnRole) const;

    virtual void sortByColumnRole(QList<AbstractFileInfoPointer> &fileList, int columnRole,
                              Qt::SortOrder order = Qt::AscendingOrder) const;

    /// getFileInfoFun is get AbstractFileInfoPointer by index for caller. if return -1 then insert file list last
    virtual int getIndexByFileInfo(getFileInfoFun fun, const AbstractFileInfoPointer &info, int columnType,
                                   Qt::SortOrder order = Qt::AscendingOrder) const;

    virtual bool canRedirectionFileUrl() const;
    virtual DUrl redirectedFileUrl() const;

    virtual bool isEmptyFloder() const;

    virtual Qt::ItemFlags fileItemDisableFlags() const;

    virtual bool canIteratorDir() const;

    virtual DUrl getUrlByNewFileName(const QString &fileName) const;

    virtual DUrl mimeDataUrl() const;
    virtual Qt::DropActions supportedDragActions() const;
    virtual Qt::DropActions supportedDropActions() const;

protected:
    struct FileInfoData
    {
        DUrl url;
        mutable QMimeType mimeType;
        QFileInfo fileInfo;

        bool exists;
        QString filePath;
        QString absoluteFilePath;
        QString fileName;
        QString displayName;
        QString pinyinName;
        QString path;
        QString absolutePath;

        qint64 size = -1;
        QDateTime created;
        QDateTime lastModified;
    };

    FileInfoData *data;

    QList<int> m_userColumnRoles;

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
    void init();

    static QMap<DUrl, FileMetaData> metaDataCacheMap;
};

QT_BEGIN_NAMESPACE
QDebug operator<<(QDebug deg, const AbstractFileInfo &info);
QT_END_NAMESPACE

#endif // ABSTRACTFILEINFO_H
