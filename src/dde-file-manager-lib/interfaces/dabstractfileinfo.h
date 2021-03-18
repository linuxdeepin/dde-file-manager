/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
 *             hujianzhong<hujianzhong@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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

#define SUFFIX_USRDIR "userdir"
#define SUFFIX_GVFSMP "gvfsmp"
#define SUFFIX_UDISKS "localdisk"

#define COMPARE_FUN_DEFINE(Value, Name, Type) \
    bool compareFileListBy##Name(const DAbstractFileInfoPointer &info1, const DAbstractFileInfoPointer &info2, Qt::SortOrder order)\
    {\
        bool isDir1 = info1->isDir();\
        bool isDir2 = info2->isDir();\
        \
        bool isFile1 = info1->isFile();\
        bool isFile2 = info2->isFile();\
        \
        if (!static_cast<const Type*>(info1.data())) \
            return false; \
        if (!static_cast<const Type*>(info2.data())) \
            return false; \
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


#ifdef SW_LABEL

struct LabelMenuItemData {
    QString id;
    QString label;
    QString tip;
    QString icon;
};
static QStringList LabelMenuItemIds;
static QMap<QString, LabelMenuItemData> LabelMenuItemDatas;
#endif

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
        // base type
        Directory,
        CharDevice,
        BlockDevice,
        FIFOFile,
        SocketFile,
        RegularFile,
        //
        Documents,
        Images,
        Videos,
        Audios,
        Archives,
        DesktopApplication,
        Executable,
        Backups,
        Unknown,
        CustomType = 0x100
    };

    inline static QString dateTimeFormat()
    {
        return "yyyy/MM/dd HH:mm:ss";
    }

    explicit DAbstractFileInfo(const DUrl &url, bool hasCache = true);
    explicit DAbstractFileInfo(const DUrl &url, const QMimeType &mimetype, bool hasCache = true);
    virtual ~DAbstractFileInfo();

    static const DAbstractFileInfoPointer getFileInfo(const DUrl &fileUrl);

    virtual bool exists() const;
    virtual bool isPrivate() const;

    virtual QString path() const;
    virtual QString filePath() const;
    virtual QString absolutePath() const;
    virtual QString absoluteFilePath() const;
    virtual QString baseName() const;
    virtual QString baseNameOfRename() const;
    virtual QString fileName() const;
    virtual QString fileNameOfRename() const;
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
    virtual bool isTaged() const;
    virtual bool canTag() const;
    virtual bool isWritableShared() const;
    virtual bool isAllowGuestShared() const;
    virtual bool makeAbsolute();



    /**
     * @brief canManageAuth 是否可以管理权限
     * @return
     */
    virtual bool canManageAuth() const;
    /**
     * @brief canMoveOrCopy 是否可以复制或剪切
     * @return
     */
    virtual bool canMoveOrCopy() const;

    // only for base file type
    virtual FileType fileType() const;
    virtual bool isFile() const;
    virtual bool isDir() const;
    virtual bool isSymLink() const;
    virtual bool isDesktopFile() const;
    virtual bool isVirtualEntry() const;

    virtual QString symlinkTargetPath() const;
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
    virtual QString fileTypeDisplayName() const;

    virtual DUrl fileUrl() const;
    inline QString scheme() const
    {return fileUrl().scheme();}

    virtual QIcon fileIcon() const;
    virtual QList<QIcon> additionalIcon() const;

    virtual DUrl parentUrl() const;
    /// from parentUrl() to top level parent url
    virtual DUrlList parentUrlList() const;
    virtual bool isAncestorsUrl(const DUrl &url, QList<DUrl> *ancestors = nullptr) const;
    virtual QVector<MenuAction> menuActionList(MenuType type = SingleFile) const;
    virtual QMap<MenuAction, QVector<MenuAction> > subMenuActionList(MenuType type = SingleFile) const;
    virtual QSet<MenuAction> disableMenuActionList() const;
    virtual MenuAction menuActionByColumnRole(int role) const;
    virtual QList<int> sortSubMenuActionUserColumnRoles() const; /*sortby submenu contains column roles*/

    /// return DFileView::ViewMode flags
    virtual quint8 supportViewMode() const;
    /// support selection mode
    virtual QList<SelectionMode> supportSelectionModes() const;

    virtual void setColumnCompact(bool compact);
    bool columnIsCompact() const;
    virtual QList<int> userColumnRoles() const;

    virtual QVariant userColumnDisplayName(int userColumnRole) const;
    /// get custom column data
    virtual QVariant userColumnData(int userColumnRole) const;
    virtual QList<int> userColumnChildRoles(int column) const;
    /// get custom column width
    int userColumnWidth(int userColumnRole) const;
    virtual int userColumnWidth(int userColumnRole, const QFontMetrics &fontMetrics) const;
    /// get custom row height
    int userRowHeight() const;
    virtual int userRowHeight(const QFontMetrics &fontMetrics) const;

    /// user column default visible for role
    virtual bool columnDefaultVisibleForRole(int role) const;

    typedef std::function<bool(const DAbstractFileInfoPointer &, const DAbstractFileInfoPointer &, Qt::SortOrder)> CompareFunction;
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
    virtual QString suffixOfRename() const;
    virtual QString completeSuffix() const;

    /// Make to inactive. stop get icon; stop watcher file
    virtual void makeToInactive();
    virtual void makeToActive();
    bool isActive() const;
    //优化gvfs文件卡，其他文件不改变，只有gvfs文件，传入true为强制刷新
    virtual void refresh(const bool isForce = false);

    virtual DUrl goToUrlWhenDeleted() const;
    virtual QString toLocalFile() const;

    virtual bool canDrop() const;

    virtual QFileInfo toQFileInfo() const;
    virtual QIODevice *createIODevice() const;

    virtual QVariantHash extraProperties() const;

    virtual bool checkMpsStr(const QString &) const;

    //为recentInfo提供接口
    virtual const QDateTime getReadTime() const;
    virtual void updateReadTime(const QDateTime &);
    virtual bool isGvfsMountFile() const;
    virtual qint8 gvfsMountFile() const;
    virtual void checkMountFile();

    virtual quint64 inode() const;


    /**
     * @brief canDragCompress 是否支持拖拽压缩
     * @return
     */
    virtual bool canDragCompress() const;

    virtual bool needCompleteDelete() const;

    /**
    * @brief isDragCompressFile 判断是否是支持拖拽压缩的文件格式
    * @return
    */
    virtual bool isDragCompressFileFormat() const;

protected:
    explicit DAbstractFileInfo(DAbstractFileInfoPrivate &dd);
    void setProxy(const DAbstractFileInfoPointer &proxy);
    Q_DECL_DEPRECATED_X("!!!!!!!!!!!!!!!!!!!!!!!") virtual void setUrl(const DUrl &url);

    /**
     * @brief loadFileEmblems 加载文件的自定义徽标
     * @param iconList 角标列表
     * @return 是否成功加载
     */
    bool loadFileEmblems(QList<QIcon> &iconList) const;
    /**
     * @brief parseEmblemString 解析徽标设置的字符串
     * @param emblem 图标对象
     * @param pos 位置
     * @param emblemStr 徽标字符串
     * @return 是否解析成功
     */
    bool parseEmblemString(QIcon &emblem, QString &pos, const QString &emblemStr) const;
    /**
     * @brief setEmblemIntoIcons 设置徽标icon到要显示的iconlist
     * @param pos 徽标位置
     * @param emblem 图标对象
     * @param iconList 显示的角标列表
     */
    void setEmblemIntoIcons(const QString &pos, const QIcon &emblem, QList<QIcon> &iconList) const;

    QScopedPointer<DAbstractFileInfoPrivate> d_ptr;
    Q_DECLARE_PRIVATE(DAbstractFileInfo)

private:
    Q_DISABLE_COPY(DAbstractFileInfo)

#ifdef SW_LABEL
public:
    QString getLabelIcon() const;
    void updateLabelMenuItems();
#endif
};

QT_BEGIN_NAMESPACE
QDebug operator<<(QDebug deg, const DAbstractFileInfo &info);
QT_END_NAMESPACE
Q_DECLARE_METATYPE(DAbstractFileInfoPointer)

#endif // ABSTRACTFILEINFO_H
