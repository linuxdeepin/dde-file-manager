#ifndef ABSTRACTFILEINFO_H
#define ABSTRACTFILEINFO_H

#include <functional>

#include <QFile>
#include <QFileInfo>
#include <QDateTime>
#include <QMap>
#include <QMimeType>
#include <QMimeDatabase>
#include <QDir>

#include "durl.h"
#include "dfmglobal.h"

#define COMPARE_FUN_DEFINE(Value, Name, Type) \
bool compareFileListBy##Name(const DAbstractFileInfoPointer &info1, const DAbstractFileInfoPointer &info2, Qt::SortOrder order)\
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
        return compareByString(info1->fileDisplayName(), info2->fileDisplayName());\
    }\
    \
    bool isStrType = typeid(value1) == typeid(QString);\
    if (isStrType)\
        return compareByString(value1, value2, order);\
    \
    return ((order == Qt::DescendingOrder) ^ (value1 < value2)) == 0x01;\
}

namespace FileSortFunction {
bool compareByString(const QString &str1, const QString &str2, Qt::SortOrder order = Qt::AscendingOrder);
template<typename T>
bool compareByString(T, T, Qt::SortOrder order = Qt::AscendingOrder)
{
    Q_UNUSED(order)

    return false;
}
}

class DAbstractFileInfo;
class DAbstractFileWatcher;
typedef QExplicitlySharedDataPointer<DAbstractFileInfo> DAbstractFileInfoPointer;
typedef std::function<const DAbstractFileInfoPointer(int)> getFileInfoFun;
typedef DFMGlobal::MenuAction MenuAction;
class DAbstractFileInfoPrivate;
class DAbstractFileInfo : public QSharedData
{
public:
    enum MenuType {
        SingleFile,
        MultiFiles,
        MultiFilesSystemPathIncluded,
        SpaceArea
    };

    enum SelectionMode {
        NoSelection,
        SingleSelection,
        MultiSelection,
        ExtendedSelection,
        ContiguousSelection
    };

    enum FileType {
        Directory,
        Documents,
        Images,
        Videos,
        Audios,
        Archives,
        DesktopApplication,
        Executable,
        Unknown
    };

    inline static QString dateTimeFormat() {
        return "yyyy/MM/dd HH:mm:ss";
    }

    explicit DAbstractFileInfo(const DUrl &url, bool hasCache = true);
    virtual ~DAbstractFileInfo();

    static const DAbstractFileInfoPointer getFileInfo(const DUrl &fileUrl);

    virtual bool exists() const;

    virtual QString path() const;
    virtual QString filePath() const;
    virtual QString absolutePath() const;
    virtual QString absoluteFilePath() const;
    virtual QString baseName() const;
    virtual QString fileName() const;
    virtual QString fileDisplayName() const;
    virtual QString fileSharedName() const;
    QString fileDisplayPinyinName() const;

    virtual bool canRename() const;
    virtual bool canShare() const;
    virtual bool canFetch() const;
    virtual bool isReadable() const;
    virtual bool isWritable() const;
    virtual bool isExecutable() const;
    virtual bool isHidden() const;
    virtual bool isRelative() const;
    virtual bool isAbsolute() const;
    virtual bool isShared() const;
    virtual bool isWritableShared() const;
    virtual bool isAllowGuestShared() const;
    virtual bool makeAbsolute();

    virtual bool isFile() const;
    virtual bool isDir() const;
    virtual bool isSymLink() const;
    virtual bool isDesktopFile() const;

    virtual DUrl symLinkTarget() const;
    virtual DUrl rootSymLinkTarget() const;

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

    virtual QMimeType mimeType(QMimeDatabase::MatchMode mode = QMimeDatabase::MatchDefault) const;
    virtual QString mimeTypeName(QMimeDatabase::MatchMode mode = QMimeDatabase::MatchDefault) const
    { return mimeType(mode).name();}
    virtual QString iconName() const;
    virtual QString genericIconName() const;

    virtual QString lastReadDisplayName() const;
    virtual QString lastModifiedDisplayName() const;
    virtual QString createdDisplayName() const;
    virtual QString sizeDisplayName() const;
    virtual QString mimeTypeDisplayName() const;
    /// Used for sorting
    virtual QString fileType() const;

    virtual DUrl fileUrl() const;
    inline QString scheme() const
    {return fileUrl().scheme();}

    virtual QIcon fileIcon() const;
    virtual QList<QIcon> additionalIcon() const;

    virtual DUrl parentUrl() const;
    /// from parentUrl() to top level parent url
    virtual DUrlList parentUrlList() const;
    virtual bool isAncestorsUrl(const DUrl &url, QList<DUrl> *ancestors = 0) const;
    virtual QVector<MenuAction> menuActionList(MenuType type = SingleFile) const;
    virtual QMap<MenuAction, QVector<MenuAction> > subMenuActionList() const;
    virtual QSet<MenuAction> disableMenuActionList() const;
    virtual MenuAction menuActionByColumnRole(int role) const;

    /// return DFileView::ViewMode flags
    virtual quint8 supportViewMode() const;
    /// support selection mode
    virtual QList<SelectionMode> supportSelectionModes() const;

    virtual QList<int> userColumnRoles() const;
    virtual QVariant userColumnDisplayName(int userColumnRole) const;
    /// get custom column data
    virtual QVariant userColumnData(int userColumnRole) const;
    /// get custom column width
    int userColumnWidth(int userColumnRole) const;
    virtual int userColumnWidth(int userColumnRole, const QFontMetrics &fontMetrics) const;

    /// user column default visible for role
    virtual bool columnDefaultVisibleForRole(int role) const;

    typedef std::function<bool(const DAbstractFileInfoPointer&, const DAbstractFileInfoPointer&, Qt::SortOrder)> CompareFunction;
    virtual CompareFunction compareFunByColumn(int columnRole) const;
    /// Whether the file should be inserted into a position according to the current sort type and order
    virtual bool hasOrderly() const;

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
    virtual void makeToActive();
    bool isActive() const;
    virtual void refresh();

    virtual DUrl goToUrlWhenDeleted() const;
    virtual QString toLocalFile() const;

    virtual bool canDrop() const;

protected:
    explicit DAbstractFileInfo(DAbstractFileInfoPrivate &dd);
    void setProxy(const DAbstractFileInfoPointer &proxy);
    Q_DECL_DEPRECATED_X("!!!!!!!!!!!!!!!!!!!!!!!") virtual void setUrl(const DUrl &url);

    QScopedPointer<DAbstractFileInfoPrivate> d_ptr;
    Q_DECLARE_PRIVATE(DAbstractFileInfo)

private:
    Q_DISABLE_COPY(DAbstractFileInfo)

#ifdef SW_LABEL
public:
    QString getLabelIcon() const;
    void updateLabelMenuItems();

protected:
    struct LabelMenuItemData
    {
        QString id;
        QString label;
        QString tip;
        QString icon;
    };
    QStringList m_labelMenuItemIds;
    QMap<QString, LabelMenuItemData> m_labelMenuItemData;
#endif
};

QT_BEGIN_NAMESPACE
QDebug operator<<(QDebug deg, const DAbstractFileInfo &info);
QT_END_NAMESPACE
Q_DECLARE_METATYPE(DAbstractFileInfoPointer)

#endif // ABSTRACTFILEINFO_H
