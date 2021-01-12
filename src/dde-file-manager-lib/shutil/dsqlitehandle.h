#ifndef DSQLITEHANDLE_H
#define DSQLITEHANDLE_H

#include "durl.h"
#include "app/define.h"
#include "deviceinfo/udisklistener.h"
#include "deviceinfo/udiskdeviceinfo.h"

#include <mutex>
#include <regex>
#include <memory>
#include <unordered_map>

#include <QDir>
#include <QMap>
#include <QObject>
#include <QDBusMetaType>
#include <QScopedPointer>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlDatabase>

#define DATABASE_PATH "/usr/share/dde-file-manager/database"

#if QT_VERSION < QT_VERSION_CHECK(5,14,0)
namespace std {

template<>
struct hash<QString> {
    inline std::size_t operator()(const QString &str)const noexcept
    {
        return (std::hash<std::string> {}(str.toStdString()));
    }
};

}
#endif


class DSqliteHandle : public QObject
{
    Q_OBJECT
public:

    ///###: some value of enum of SqlType has "2/3"  suffix.
    ///###: It shows that this operation increment/delete/modify/query  the 2 tables in current sqlite.
    ///###: so, if there is not "2/3" suffix show that this operation do work in a table of current sqlite.
    enum class SqlType : std::size_t {
        None = 0,

        BeforeTagFiles,
        TagFiles,
        TagFiles2,
        TagFiles3,

        DeleteTags,
        DeleteTags2,
        DeleteTags3,
        DeleteTags4,

        DeleteFiles,
        DeleteFiles2,

        ChangeTagsName,
        ChangeTagsName2,

        ChangeFilesName,
        ChangeFilesName2,

        TagFilesThroughColor,
        TagFilesThroughColor2,
        TagFilesThroughColor3,

        GetFilesThroughTag,

        GetTagsThroughFile,
        GetSameTagsOfDiffFiles,

        UntagDiffPartionFiles,

        UntagSamePartionFiles,
        UntagSamePartionFiles2,

        GetAllTags,
        GetTagColor,
        ChangeTagColor
    };

    enum class ReturnCode : std::size_t {
        Exist = 0,
        NoExist,
        NoThisDir,
        PlaceHolder,
        FailedExecSql
    };

    explicit DSqliteHandle(QObject *const parent = nullptr);
    virtual ~DSqliteHandle() = default;
    DSqliteHandle(const DSqliteHandle &other) = delete;
    DSqliteHandle &operator=(const DSqliteHandle &other) = delete;

    ///####:---------------------->  <url(with protocal header), <tagName>>
    QVariant disposeClientData(const QMap<QString, QList<QString>> &filesAndTags, const unsigned long long &type);

    static DSqliteHandle *instance();
    static std::map<QString, std::multimap<QString, QString>> queryPartionsInfoOfDevices();
    static QPair<QString, QString> getMountPointOfFile(DUrl url, std::unique_ptr<std::map<QString, std::multimap<QString, QString>>> &partionsAndMountPoints);

signals:
    void addNewTags(const QVariant &new_tags);
    void deleteTags(const QVariant &be_deleted_tags);
    void changeTagColor(const QVariantMap &old_and_new_color);
    void changeTagName(const QVariantMap &old_and_new_name);
    void filesWereTagged(const QVariantMap &files_were_tagged);
    void untagFiles(const QVariantMap &del_tags_of_file);

private slots:
    void onMountAdded(UDiskDeviceInfoPointer infoPointer);
    void onMountRemoved(UDiskDeviceInfoPointer infoPointer);

private:
    static QString restoreEscapedChar(const QString &value);

    inline void closeSqlDatabase()noexcept
    {
        if (m_sqlDatabasePtr && m_sqlDatabasePtr->isOpen()) {
            m_sqlDatabasePtr->close();
        }
    }

    inline QString remove_mount_point(const QString &file, const QString &mount_point) noexcept
    {
        int index{ file.indexOf(mount_point) };
        QString file_be_removed_mount_point{file};

        if (index == 0) {
            int size{ mount_point.size() };
            file_be_removed_mount_point = file_be_removed_mount_point.remove(0, size);
        }

        return file_be_removed_mount_point;
    }


    template<SqlType Ty = SqlType::None, typename T = void>
    inline T execSqlstr(const QMap<QString, QList<QString>> &filesAndTags)
    {
        (void)filesAndTags;
        return;
    }


    template<SqlType type = SqlType::None, typename Ty, typename T = void>
    inline T helpExecSql(const Ty &sqlStrs, const QString &mountPoint)
    {
        (void)sqlStrs;
        (void)mountPoint;
        return;
    }

    /**
     * @brief checkDBFileExist 检查db文件是否存在
     * @param path 所在目录
     * @param db_name db文件名
     * @return 文件存在　Exist = 0, 文件不存在　NoExist = 1,　所在目录不存在　NoThisDir = 2
     */
    ReturnCode checkDBFileExist(const QString &path, const QString &db_name = ".__deepin.db");

    void initializeConnect();

    /**
     * @brief connectToSqlite 创建数据库连接
     * 旧逻辑，正常连接数据库的逻辑
     * @param path　所在目录
     * @param db_name　db文件名
     */
    void connectToSqlite(const QString &path, const QString &db_name = QString{".__deepin.db"});

    /**
     * @brief connectToShareSqlite 创建连接到share目录的数据库
     * 判断数据库位置，如果之前版本在/home目录下创建过数据库，将数据库移动到/usr/share/dde-file-manager/database目录下
     * 在确保数据库在share目录后，调用connectToSqlite正常连接数据库
     * @param path　所在目录
     * @param db_name　db文件名
     */
    void connectToShareSqlite(const QString &path, const QString &db_name = ".__deepin.db");

    std::unique_ptr<std::map<QString, std::multimap<QString, QString>>> m_partionsOfDevices{ nullptr };
    std::unique_ptr<QSqlDatabase> m_sqlDatabasePtr{ nullptr };
    std::atomic<bool> m_flag{ false };
    std::mutex m_mutex{};

    QList<QString> m_newAddedTags{};
};

///###: increase
///
///###: there function will be invoked by TagFiles/TagFilesThroughColor in CLIENT only.
template<>
bool DSqliteHandle::execSqlstr<DSqliteHandle::SqlType::BeforeTagFiles, bool>(const QMap<QString, QList<QString>> &filesAndTags);

template<>
bool DSqliteHandle::execSqlstr<DSqliteHandle::SqlType::TagFiles, bool>(const QMap<QString, QList<QString>> &filesAndTags);

template<>
bool DSqliteHandle::execSqlstr<DSqliteHandle::SqlType::TagFilesThroughColor, bool>(const QMap<QString, QList<QString>> &filesAndTags);

///###: query
template<>
QList<QString> DSqliteHandle::execSqlstr<DSqliteHandle::SqlType::GetTagsThroughFile, QList<QString>>(const QMap<QString, QList<QString>> &filesAndTags);

template<>
QList<QString> DSqliteHandle::execSqlstr<DSqliteHandle::SqlType::GetSameTagsOfDiffFiles, QList<QString>>(const QMap<QString, QList<QString>> &filesAndTags);

template<>
QList<QString> DSqliteHandle::execSqlstr<DSqliteHandle::SqlType::GetFilesThroughTag, QList<QString>>(const QMap<QString, QList<QString>> &filesAndTags);

template<>
QMap<QString, QVariant> DSqliteHandle::execSqlstr<DSqliteHandle::SqlType::GetAllTags, QMap<QString, QVariant>>(const QMap<QString, QList<QString>> &filesAndTags);

template<>
QMap<QString, QVariant> DSqliteHandle::execSqlstr<DSqliteHandle::SqlType::GetTagColor, QMap<QString, QVariant>>(const QMap<QString, QList<QString>> &fileAndTags);

///###: modify
template<> ///###: -------------------------------------------------------------> <OldFileName, NewFileName>
bool DSqliteHandle::execSqlstr<DSqliteHandle::SqlType::ChangeFilesName, bool>(const QMap<QString, QList<QString>> &filesAndTags);

template<>///###: --------------------------------------------------------------><OldTagName, NewTagName>
bool DSqliteHandle::execSqlstr<DSqliteHandle::SqlType::ChangeTagsName, bool>(const QMap<QString, QList<QString>> &filesAndTags);

template<>
bool DSqliteHandle::execSqlstr<DSqliteHandle::SqlType::ChangeTagColor, bool>(const QMap<QString, QList<QString>> &filesAndTags);


///###: delete
template<>
bool DSqliteHandle::execSqlstr<DSqliteHandle::SqlType::UntagSamePartionFiles, bool>(const QMap<QString, QList<QString>> &filesAndTags);

template<>
bool DSqliteHandle::execSqlstr<DSqliteHandle::SqlType::UntagDiffPartionFiles, bool>(const QMap<QString, QList<QString>> &filesAndTags);

template<>
bool DSqliteHandle::execSqlstr<DSqliteHandle::SqlType::DeleteFiles, bool>(const QMap<QString, QList<QString>> &filesAndTags);

template<>
bool DSqliteHandle::execSqlstr<DSqliteHandle::SqlType::DeleteTags, bool>(const QMap<QString, QList<QString>> &filesAndTags);


///###:auxiliary function.
///###: tag files
template<>
bool DSqliteHandle::helpExecSql<DSqliteHandle::SqlType::TagFiles, QMap<QString, QList<QString>>, bool>(const QMap<QString, QList<QString>> &forDecreasing,
                                                                                                       const QString &mountPoint);

template<>
bool DSqliteHandle::helpExecSql<DSqliteHandle::SqlType::TagFiles2, QMap<QString, QList<QString>>, bool>(
                                                                                                  const QMap<QString, QList<QString>>  &forIncreasing, const QString &mountPoint);

template<>
bool DSqliteHandle::helpExecSql<DSqliteHandle::SqlType::TagFiles3, QList<QString>, bool>(const QList<QString> &forUpdating,
                                                                                         const QString &mountPoint);



template<>
bool DSqliteHandle::helpExecSql<DSqliteHandle::SqlType::TagFilesThroughColor,
     std::list<std::tuple<QString, QString, QString, QString, QString, QString>>, bool>(const std::list<std::tuple<QString, QString, QString, QString, QString, QString>> &sqlStrs,
                                                                                        const QString &mountPoint);
template<>
bool DSqliteHandle::helpExecSql<DSqliteHandle::SqlType::TagFilesThroughColor3, QString, bool>(const QString &tag_name, const QString &mountPoint);


///###: untag files in same/diff partion.
template<>
bool DSqliteHandle::helpExecSql<DSqliteHandle::SqlType::UntagSamePartionFiles, std::list<QString>, bool>(
    const std::list<QString> &sqlStrs, const QString &mountPoint);
template<>
bool DSqliteHandle::helpExecSql<DSqliteHandle::SqlType::UntagSamePartionFiles2, QMap<QString, QList<QString>>, bool>(const QMap<QString, QList<QString>> &fileNameAndTagNames,
                                                                                                                     const QString &mountPoint);


///### delete files and delete row(s) in tag_with_file and file_property.
template<>
bool DSqliteHandle::helpExecSql<DSqliteHandle::SqlType::DeleteFiles,
     std::list<QString>, bool>(const std::list<QString> &files, const QString &mount_point);

template<>
QMap<QString, QList<QString>> DSqliteHandle::helpExecSql<DSqliteHandle::SqlType::DeleteFiles2, std::list<QString>,
     QMap<QString, QList<QString>>>(const std::list<QString> &files, const QString &mount_point);

///###: delete tag(s)
template<>
QMap<QString, QList<QString>> DSqliteHandle::helpExecSql<DSqliteHandle::SqlType::DeleteTags4,
     QList<QString>, QMap<QString, QList<QString>>>(const QList<QString> &tag_names, const QString &mount_point);

template<>
bool DSqliteHandle::helpExecSql<DSqliteHandle::SqlType::DeleteTags,
     std::list<QString>, bool>(const std::list<QString> &sqlStrs, const QString &mountPoint);
template<>
bool DSqliteHandle::helpExecSql<DSqliteHandle::SqlType::DeleteTags2,
     QMap<QString, QList<QString>>, bool>(const QMap<QString, QList<QString>> &fileNameAndTagNames, const QString &mountPoint);

template<>
bool DSqliteHandle::helpExecSql<DSqliteHandle::SqlType::DeleteTags3, QList<QString>, bool>(const QList<QString> &tag_name, const QString &mountPoint);



///###: change file(s) name.
template<>
bool DSqliteHandle::helpExecSql<DSqliteHandle::SqlType::ChangeFilesName,
     std::map<QString, QString>, bool>(const std::map<QString, QString> &sqlStrs, const QString &mountPoint);

template<>
QMap<QString, QList<QString>> DSqliteHandle::helpExecSql<DSqliteHandle::SqlType::ChangeFilesName2, std::map<QString, QString>,
     QMap<QString, QList<QString>>>(const std::map<QString, QString> &files, const QString &mount_point);

///###: change tag(s) name.
template<>
bool DSqliteHandle::helpExecSql<DSqliteHandle::SqlType::ChangeTagsName,
     std::list<std::tuple<QString, QString, QString, QString>>, bool>(const std::list<std::tuple<QString, QString, QString, QString>> &sqlStrs,
                                                                      const QString &mountPoint);
template<>
bool DSqliteHandle::helpExecSql<DSqliteHandle::SqlType::ChangeTagsName2, QMap<QString, QList<QString>>, bool>(const QMap<QString, QList<QString>> &old_and_new,
                                                                                                              const QString &mountPoint);


///###: get tags through file.
template<>
QList<QString> DSqliteHandle::helpExecSql<DSqliteHandle::SqlType::GetTagsThroughFile,
      QString, QList<QString>>(const QString &sqlStr, const QString &mountPoint);


///###: get files which was tagged by appointed tag.
template<>
QList<QString> DSqliteHandle::helpExecSql<DSqliteHandle::SqlType::GetFilesThroughTag,
      QString, QList<QString>>(const QString &sqlStr, const QString &mountPoint);



#endif // DSQLITEHANDLE_H
