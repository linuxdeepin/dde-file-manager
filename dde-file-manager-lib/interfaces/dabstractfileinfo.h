#ifndef ABSTRACTFILEINFO_H
#define ABSTRACTFILEINFO_H

#include <functional>

#include <QFile>
#include <QFileInfo>
#include <QDateTime>
#include <QMap>
#include <QAbstractItemView>
#include <QMimeType>
#include <QDir>

#include "durl.h"
#include "dfmglobal.h"

#define SORT_FUN_DEFINE(Value, Name, Type) \
bool sortFileListBy##Name(const DAbstractFileInfoPointer &info1, const DAbstractFileInfoPointer &info2, Qt::SortOrder order)\
{\
    bool isDir1 = info1->isDir();\
    bool isDir2 = info2->isDir();\
    \
    bool isFile1 = info1->isFile();\
    bool isFile2 = info2->isFile();\
    \
    auto value1 = static_cast<const Type*>(info1.data())->Value();\
    auto value2 = static_cast<const Type*>(info2.data())->Value();\
    \
    if (isDir1) {\
        if (!isDir2) return true;\
    } else {\
        if (isDir2) return false;\
    }\
    \
    if ((isDir1 && isDir2 && (value1 == value2)) || (isFile1 && isFile2 && (value1 == value2))) {\
        return sortByString(info1->fileDisplayName(), info2->fileDisplayName());\
    }\
    \
    bool isStrType = typeid(value1) == typeid(QString);\
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

class DAbstractFileInfo;
typedef QSharedPointer<DAbstractFileInfo> DAbstractFileInfoPointer;
typedef std::function<const DAbstractFileInfoPointer(int)> getFileInfoFun;
typedef DFMGlobal::MenuAction MenuAction;
class DAbstractFileInfoPrivateBase : public QSharedData {public: virtual ~DAbstractFileInfoPrivateBase() {}};
class DAbstractFileInfoPrivate;
class DAbstractFileInfo
{
public:
    enum MenuType {
        SingleFile,
        MultiFiles,
        MultiFilesSystemPathIncluded,
        SpaceArea
    };

    inline static QString dateTimeFormat() {
        return "yyyy/MM/dd HH:mm:ss";
    }

    explicit DAbstractFileInfo(const DUrl &url);

    virtual ~DAbstractFileInfo();

    virtual void setUrl(const DUrl &url);
    virtual bool exists() const;

    virtual QString path() const;
    virtual QString filePath() const;
    virtual QString absolutePath() const;
    virtual QString absoluteFilePath() const;
    virtual QString baseName() const;
    virtual QString fileName() const;
    virtual QString fileDisplayName() const;
    QString fileDisplayPinyinName() const;

    virtual bool isCanRename() const;
    virtual bool isCanShare() const;
    virtual bool isReadable() const;
    virtual bool isWritable() const;
    virtual bool isExecutable() const;
    virtual bool isHidden() const;
    virtual bool isRelative() const;
    virtual bool isAbsolute() const;
    virtual bool isShared() const;
    virtual bool makeAbsolute();

    virtual bool isFile() const;
    virtual bool isDir() const;
    virtual bool isSymLink() const;
    virtual bool isDesktopFile() const;

    virtual QString symLinkTarget();

    virtual QString owner() const;
    virtual uint ownerId() const;
    virtual QString group() const;
    virtual uint groupId() const;

    virtual bool permission(QFile::Permissions permissions) const;
    virtual QFile::Permissions permissions() const;

    virtual qint64 size() const;
    virtual int filesCount() const;
    // Return fileCounts() if file is folder; otherwise return size()
    qint64 fileSize() const;

    virtual QDateTime created() const;
    virtual QDateTime lastModified() const;
    virtual QDateTime lastRead() const;

    virtual QMimeType mimeType() const;
    virtual QString mimeTypeName() const
    { return mimeType().name();}

    virtual QString lastReadDisplayName() const;
    virtual QString lastModifiedDisplayName() const;
    virtual QString createdDisplayName() const;
    virtual QString sizeDisplayName() const;
    virtual QString mimeTypeDisplayName() const;

    virtual DUrl fileUrl() const;
    inline QString scheme() const
    {return fileUrl().scheme();}

    inline bool hasThumbnail() const
    { return hasThumbnail(mimeType());}
    virtual bool hasThumbnail(const QMimeType &mimeTypeName) const;
    virtual QIcon fileIcon() const;
    virtual QList<QIcon> additionalIcon() const;

    virtual DUrl parentUrl() const;
    virtual bool isAncestorsUrl(const DUrl &url, QList<DUrl> *ancestors = 0) const;
    virtual QVector<MenuAction> menuActionList(MenuType type = SingleFile) const;
    virtual QMap<MenuAction, QVector<MenuAction> > subMenuActionList() const;
    virtual QSet<MenuAction> disableMenuActionList() const;
    virtual MenuAction menuActionByColumnRole(int role) const;

    /// return DFileView::ViewMode flags
    virtual quint8 supportViewMode() const;
    /// support selection mode
    virtual QAbstractItemView::SelectionMode supportSelectionMode() const;

    virtual QList<int> userColumnRoles() const;
    virtual QVariant userColumnDisplayName(int userColumnRole) const;
    /// get custom column data
    virtual QVariant userColumnData(int userColumnRole) const;
    /// get custom column width
    int userColumnWidth(int userColumnRole) const;
    virtual int userColumnWidth(int userColumnRole, const QFontMetrics &fontMetrics) const;

    /// user column default visible for role
    virtual bool columnDefaultVisibleForRole(int role) const;

    typedef std::function<bool(const DAbstractFileInfoPointer&, const DAbstractFileInfoPointer&, Qt::SortOrder)> sortFunction;
    virtual sortFunction sortFunByColumn(int columnRole) const;

    virtual void sortByColumnRole(QList<DAbstractFileInfoPointer> &fileList, int columnRole,
                              Qt::SortOrder order = Qt::AscendingOrder) const;

    /// getFileInfoFun is get AbstractFileInfoPointer by index for caller. if return -1 then insert file list last
    virtual int getIndexByFileInfo(getFileInfoFun fun, const DAbstractFileInfoPointer &info, int columnType,
                                   Qt::SortOrder order = Qt::AscendingOrder) const;

    virtual bool canRedirectionFileUrl() const;
    virtual DUrl redirectedFileUrl() const;

    virtual bool isEmptyFloder(const QDir::Filters &filters = QDir::AllEntries | QDir::NoDotAndDotDot | QDir::System) const;

    virtual Qt::ItemFlags fileItemDisableFlags() const;

    virtual bool canIteratorDir() const;

    virtual DUrl getUrlByNewFileName(const QString &fileName) const;
    /// Return absoluteFilePath + "/" + fileName if is dir; otherwise return DUrl()
    virtual DUrl getUrlByChildFileName(const QString &fileName) const;

    virtual DUrl mimeDataUrl() const;
    virtual Qt::DropActions supportedDragActions() const;
    virtual Qt::DropActions supportedDropActions() const;

    virtual QString loadingTip() const;
    virtual QString subtitleForEmptyFloder() const;

    virtual QString suffix() const;
    virtual QString completeSuffix() const;

    /// Make to inactive. stop get icon; stop watcher file
    virtual void makeToInactive();

protected:
    QExplicitlySharedDataPointer<DAbstractFileInfoPrivateBase> d_ptr;
    Q_DECLARE_PRIVATE(DAbstractFileInfo)

    Q_DECL_DEPRECATED_X("Wraning: The DAbstractFileInfo::DAbstractFileInfo() is not deprecated. \
                         But d_ptr is not initialize if use the constructor")
    explicit DAbstractFileInfo();

    virtual DAbstractFileInfoPrivate *createPrivateByUrl(const DUrl &url) const;
    void setProxy(const DAbstractFileInfoPointer &proxy);
    DAbstractFileInfoPrivate *getPrivateByUrl(const DUrl &url) const;

private:
    Q_DISABLE_COPY(DAbstractFileInfo)
};

QT_BEGIN_NAMESPACE
QDebug operator<<(QDebug deg, const DAbstractFileInfo &info);
QT_END_NAMESPACE

#endif // ABSTRACTFILEINFO_H
