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



namespace std
{

template<>
struct hash<QString>
{
    inline std::size_t operator()(const QString& str)const noexcept
    {
        return (std::hash<std::string>{}(str.toStdString()));
    }
};

}



class DSqliteHandle : public QObject
{
    Q_OBJECT
public:

    ///###: some value of enum of SqlType has "2/3"  suffix.
    ///###: It shows that this operation increment/delete/modify/query  the 2 tables in current sqlite.
    ///###: so, if there is not "2/3" suffix show that this operation do work in a table of current sqlite.
    enum class SqlType : std::size_t
    {
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

    enum class ReturnCode : std::size_t
    {
        Exist = 0,
        NoExist,
        NoThisDir,
        PlaceHolder,
        FailedExecSql
    };

    explicit DSqliteHandle(QObject* const parent = nullptr);
    virtual ~DSqliteHandle()=default;
    DSqliteHandle(const DSqliteHandle& other)=delete;
    DSqliteHandle& operator=(const DSqliteHandle& other)=delete;

    ///####:---------------------->  <url(with protocal header), <tagName>>
    QVariant disposeClientData(const QMap<QString, QList<QString>>& filesAndTags, const unsigned long long& type);

    static DSqliteHandle* instance();
    static std::map<QString, std::multimap<QString, QString>> queryPartionsInfoOfDevices();
    static QPair<QString, QString> getMountPointOfFile(DUrl url, std::unique_ptr<std::map<QString, std::multimap<QString, QString>>>& partionsAndMountPoints);

signals:
    void addNewTags(const QVariant& new_tags);
    void deleteTags(const QVariant& be_deleted_tags);
    void changeTagColor(const QVariantMap& old_and_new_color);
    void changeTagName(const QVariantMap& old_and_new_name);
    void filesWereTagged(const QVariantMap& files_were_tagged);
    void untagFiles(const QVariantMap& tag_be_removed_files);

private slots:
    void onMountAdded(UDiskDeviceInfoPointer infoPointer);
    void onMountRemoved(UDiskDeviceInfoPointer infoPointer);

private:
    static QString restoreEscapedChar(const QString& value);

    inline void closeSqlDatabase()noexcept
    {
        if(m_sqlDatabasePtr && m_sqlDatabasePtr->isOpen()){
            m_sqlDatabasePtr->close();
        }
    }


    template<SqlType Ty = SqlType::None, typename T = void>
    inline T execSqlstr(const QMap<QString, QList<QString>>& filesAndTags)
    {
        (void)filesAndTags;
        return;
    }


    template<SqlType type = SqlType::None, typename Ty, typename T = void>
    inline T helpExecSql(const Ty& sqlStrs, const QString& mountPoint)
    {
        (void)sqlStrs;
        (void)mountPoint;
        return;
    }

    ReturnCode checkWhetherHasSqliteInPartion(const QString& mountPoint, const QString& db_name = QString{".__deepin.db"});
    void initializeConnect();
    void connectToSqlite(const QString& mountPoint, const QString& db_name = QString{".__deepin.db"});

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
bool DSqliteHandle::execSqlstr<DSqliteHandle::SqlType::BeforeTagFiles, bool>(const QMap<QString, QList<QString>>& filesAndTags);

template<>
bool DSqliteHandle::execSqlstr<DSqliteHandle::SqlType::TagFiles, bool>(const QMap<QString, QList<QString>>& filesAndTags);

template<>
bool DSqliteHandle::execSqlstr<DSqliteHandle::SqlType::TagFilesThroughColor, bool>(const QMap<QString, QList<QString>>& filesAndTags);

///###: query
template<>
QList<QString> DSqliteHandle::execSqlstr<DSqliteHandle::SqlType::GetTagsThroughFile, QList<QString>>(const QMap<QString, QList<QString>>& filesAndTags);

template<>
QList<QString> DSqliteHandle::execSqlstr<DSqliteHandle::SqlType::GetSameTagsOfDiffFiles, QList<QString>>(const QMap<QString, QList<QString>>& filesAndTags);

template<>
QList<QString> DSqliteHandle::execSqlstr<DSqliteHandle::SqlType::GetFilesThroughTag, QList<QString>>(const QMap<QString, QList<QString>>& filesAndTags);

template<>
QMap<QString, QVariant> DSqliteHandle::execSqlstr<DSqliteHandle::SqlType::GetAllTags, QMap<QString, QVariant>>(const QMap<QString, QList<QString>>& filesAndTags);

template<>
QMap<QString, QVariant> DSqliteHandle::execSqlstr<DSqliteHandle::SqlType::GetTagColor, QMap<QString, QVariant>>(const QMap<QString, QList<QString>>& fileAndTags);

///###: modify
template<> ///###: -------------------------------------------------------------> <OldFileName, NewFileName>
bool DSqliteHandle::execSqlstr<DSqliteHandle::SqlType::ChangeFilesName, bool>(const QMap<QString, QList<QString>>& filesAndTags);

template<>///###: --------------------------------------------------------------><OldTagName, NewTagName>
bool DSqliteHandle::execSqlstr<DSqliteHandle::SqlType::ChangeTagsName, bool>(const QMap<QString, QList<QString>>& filesAndTags);

template<>
bool DSqliteHandle::execSqlstr<DSqliteHandle::SqlType::ChangeTagColor, bool>(const QMap<QString, QList<QString>>& filesAndTags);


///###: delete
template<>
bool DSqliteHandle::execSqlstr<DSqliteHandle::SqlType::UntagSamePartionFiles, bool>(const QMap<QString, QList<QString>>& filesAndTags);

template<>
bool DSqliteHandle::execSqlstr<DSqliteHandle::SqlType::UntagDiffPartionFiles, bool>(const QMap<QString, QList<QString>>& filesAndTags);

template<>
bool DSqliteHandle::execSqlstr<DSqliteHandle::SqlType::DeleteFiles, bool>(const QMap<QString, QList<QString>>& filesAndTags);

template<>
bool DSqliteHandle::execSqlstr<DSqliteHandle::SqlType::DeleteTags, bool>(const QMap<QString, QList<QString>>& filesAndTags);


///###:auxiliary function.
///###: tag files
template<>
bool DSqliteHandle::helpExecSql<DSqliteHandle::SqlType::TagFiles, QMap<QString, QList<QString>>, bool>(const QMap<QString, QList<QString>>& forDecreasing,
                                                                                                       const QString& mountPoint);

template<>
bool DSqliteHandle::helpExecSql<DSqliteHandle::SqlType::TagFiles2, QMap<QString, QList<QString>>, bool>(
                                                               const QMap<QString, QList<QString>>&  forIncreasing, const QString& mountPoint);

template<>
bool DSqliteHandle::helpExecSql<DSqliteHandle::SqlType::TagFiles3, QList<QString>, bool>(const QList<QString>& forUpdating,
                                                                                                        const QString& mountPoint);



template<>
bool DSqliteHandle::helpExecSql<DSqliteHandle::SqlType::TagFilesThroughColor,
                    std::list<std::tuple<QString, QString, QString, QString, QString, QString>>, bool>(const std::list<std::tuple<QString, QString, QString, QString, QString, QString>>& sqlStrs,
                                                                                                       const QString& mountPoint);
template<>
bool DSqliteHandle::helpExecSql<DSqliteHandle::SqlType::TagFilesThroughColor3, QString, bool>(const QString& tag_name, const QString& mountPoint);


///###: untag files in same/diff partion.
template<>
bool DSqliteHandle::helpExecSql<DSqliteHandle::SqlType::UntagSamePartionFiles, std::list<QString>, bool>(
                                                    const std::list<QString>& sqlStrs, const QString& mountPoint);
template<>
bool DSqliteHandle::helpExecSql<DSqliteHandle::SqlType::UntagSamePartionFiles2, QMap<QString, QList<QString>>, bool>(const QMap<QString, QList<QString>>& fileNameAndTagNames,
                                                                                                                     const QString& mountPoint);


///### delete files and delete row(s) in tag_with_file and file_property.
template<>
bool DSqliteHandle::helpExecSql<DSqliteHandle::SqlType::DeleteFiles,
                                    std::list<QString>, bool>(const std::list<QString>& files, const QString& mount_point);

template<>
QMap<QString, QList<QString>> DSqliteHandle::helpExecSql<DSqliteHandle::SqlType::DeleteFiles2, std::list<QString>,
                                             QMap<QString, QList<QString>>>(const std::list<QString>& files, const QString& mount_point);

///###: delete tag(s)
template<>
QMap<QString, QList<QString>> DSqliteHandle::helpExecSql<DSqliteHandle::SqlType::DeleteTags4,
                                            QList<QString>, QMap<QString, QList<QString>>>(const QList<QString>& tag_names, const QString& mount_point);

template<>
bool DSqliteHandle::helpExecSql<DSqliteHandle::SqlType::DeleteTags,
                                std::list<QString>, bool>(const std::list<QString>& sqlStrs, const QString& mountPoint);
template<>
bool DSqliteHandle::helpExecSql<DSqliteHandle::SqlType::DeleteTags2,
                                QMap<QString, QList<QString>>, bool>(const QMap<QString, QList<QString>>& fileNameAndTagNames, const QString& mountPoint);

template<>
bool DSqliteHandle::helpExecSql<DSqliteHandle::SqlType::DeleteTags3, QList<QString>, bool>(const QList<QString>& tag_name, const QString& mountPoint);



///###: change file(s) name.
template<>
bool DSqliteHandle::helpExecSql<DSqliteHandle::SqlType::ChangeFilesName,
                                std::map<QString, QString>, bool>(const std::map<QString, QString>& sqlStrs, const QString& mountPoint);

///###: change tag(s) name.
template<>
bool DSqliteHandle::helpExecSql<DSqliteHandle::SqlType::ChangeTagsName,
                                std::list<std::tuple<QString, QString, QString, QString>>, bool>(const std::list<std::tuple<QString, QString, QString, QString>>& sqlStrs,
                                                                                           const QString& mountPoint);
template<>
bool DSqliteHandle::helpExecSql<DSqliteHandle::SqlType::ChangeTagsName2, QMap<QString, QList<QString>>, bool>(const QMap<QString, QList<QString>>& old_and_new,
                                                                                                                     const QString &mountPoint);


///###: get tags through file.
template<>
QList<QString> DSqliteHandle::helpExecSql<DSqliteHandle::SqlType::GetTagsThroughFile,
                                QString, QList<QString>>(const QString& sqlStr, const QString& mountPoint);


///###: get files which was tagged by appointed tag.
template<>
QList<QString> DSqliteHandle::helpExecSql<DSqliteHandle::SqlType::GetFilesThroughTag,
                                          QString, QList<QString>>(const QString& sqlStr, const QString& mountPoint);



#endif // DSQLITEHANDLE_H
