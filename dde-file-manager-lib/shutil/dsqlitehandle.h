#ifndef DSQLITEHANDLE_H
#define DSQLITEHANDLE_H

#include "durl.h"
#include "app/define.h"
#include "tag/tagmanager.h"
#include "deviceinfo/udisklistener.h"
#include "deviceinfo/udiskdeviceinfo.h"

#include <regex>
#include <memory>
#include <unordered_map>

#include <QDir>
#include <QMap>
#include <QObject>
#include <QDBusMetaType>
#include <QScopedPointer>
#include <QReadWriteLock>



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

        TagFiles = 1,
        TagFiles2,
        TagFiles3,

        DeleteTags,
        DeleteTags2,

        DeleteFiles,
        ChangeTagsName,
        ChangeFilesName,

        TagFilesThroughColor,
        TagFilesThroughColor2,

        GetFilesThroughTag,

        GetTagsThroughFile,
        GetSameTagsOfDiffFiles,

        UntagDiffPartionFiles,


        UntagSamePartionFiles,
        UntagSamePartionFiles2
    };

    enum class ReturnCode : std::size_t
    {
        Exist = 0,
        NoExist,
        NoThisDir,
        PlaceHolder,
        FailedExecSql
    };

    DSqliteHandle(QObject* const parent = nullptr);
    virtual ~DSqliteHandle()=default;
    DSqliteHandle(const DSqliteHandle& other)=delete;
    DSqliteHandle& operator=(const DSqliteHandle& other)=delete;

    ///####:---------------------->  <url(with protocal header), <tagName>>
    QVariant disposeClientData(const QMap<QString, QList<QString>>& filesAndTags, const QString& userName, const std::size_t& type);

    void lockBackend()noexcept;
    void unlockBackend()noexcept;

    static DSqliteHandle* instance();
    static std::map<QString, std::multimap<QString, QString>> queryPartionsInfoOfDevices();
    static QPair<QString, QString> getMountPointOfFile(const DUrl& url, std::unique_ptr<std::map<QString, std::multimap<QString, QString>>>& partionsAndMountPoints);


signals:
    void backendIsBlocked();

private slots:
    void onMountAdded(UDiskDeviceInfoPointer infoPointer);
    void onMountRemoved(UDiskDeviceInfoPointer infoPointer);

private:
    static QString getConnectionNameFromPartion(const QString& partion)noexcept;
    static QString restoreEscapedChar(const QString& value);

    inline void closeSqlDatabase()noexcept
    {
        if(m_sqlDatabasePtr && m_sqlDatabasePtr->isOpen()){
            m_sqlDatabasePtr->close();
        }
    }


    template<SqlType Ty = SqlType::None, typename T = void>
    inline T execSqlstr(const QMap<QString, QList<QString>>& filesAndTags, const QString& userName)
    {
        (void)filesAndTags;
        (void)userName;
        return;
    }


    template<DSqliteHandle::SqlType type, typename Ty, typename T = void>
    inline T helpExecSql(const Ty& sqlStrs, const QString& mountPoint, const QString& userName)
    {
        (void)sqlStrs;
        (void)mountPoint;
        (void)userName;
        return;
    }

    ReturnCode checkWhetherHasSqliteInPartion(const QString& mountPoint, const QString& userName);
    void initializeConnect();
    void connectToSqlite(const QString& mountPoint, const QString& userName);

    std::unique_ptr<std::map<QString, std::multimap<QString, QString>>> m_partionsOfDevices{ nullptr };
    std::unique_ptr<QSqlDatabase> m_sqlDatabasePtr{ nullptr };
    std::atomic<bool> m_flag{ false };
    std::mutex m_mutex{};

};

///###: increase
template<> ///###:-----------------------------------------------------------><file, [tagsName]>
bool DSqliteHandle::execSqlstr<DSqliteHandle::SqlType::TagFiles, bool>(const QMap<QString, QList<QString>>& filesAndTags, const QString& userName);

template<>
bool DSqliteHandle::execSqlstr<DSqliteHandle::SqlType::TagFilesThroughColor, bool>(const QMap<QString, QList<QString>>& filesAndTags, const QString& userName);

///###: query
template<>
QList<QString> DSqliteHandle::execSqlstr<DSqliteHandle::SqlType::GetTagsThroughFile, QList<QString>>(const QMap<QString, QList<QString>>& filesAndTags, const QString& userName);

template<>
QList<QString> DSqliteHandle::execSqlstr<DSqliteHandle::SqlType::GetSameTagsOfDiffFiles, QList<QString>>(const QMap<QString, QList<QString>>& filesAndTags, const QString& userName);

template<>
QList<QString> DSqliteHandle::execSqlstr<DSqliteHandle::SqlType::GetFilesThroughTag, QList<QString>>(const QMap<QString, QList<QString>>& filesAndTags, const QString& userName);

///###: modify
template<> ///###: -------------------------------------------------------------> <OldFileName, NewFileName>
bool DSqliteHandle::execSqlstr<DSqliteHandle::SqlType::ChangeFilesName, bool>(const QMap<QString, QList<QString>>& filesAndTags, const QString& userName);

template<>///###: --------------------------------------------------------------><OldTagName, NewTagName>
bool DSqliteHandle::execSqlstr<DSqliteHandle::SqlType::ChangeTagsName, bool>(const QMap<QString, QList<QString>>& filesAndTags, const QString& userName);


///###: delete
template<>
bool DSqliteHandle::execSqlstr<DSqliteHandle::SqlType::UntagSamePartionFiles, bool>(const QMap<QString, QList<QString>>& filesAndTags, const QString& userName);

template<>
bool DSqliteHandle::execSqlstr<DSqliteHandle::SqlType::UntagDiffPartionFiles, bool>(const QMap<QString, QList<QString>>& filesAndTags, const QString& userName);

template<>
bool DSqliteHandle::execSqlstr<DSqliteHandle::SqlType::DeleteFiles, bool>(const QMap<QString, QList<QString>>& filesAndTags, const QString& userName);

template<>
bool DSqliteHandle::execSqlstr<DSqliteHandle::SqlType::DeleteTags, bool>(const QMap<QString, QList<QString>>& filesAndTags, const QString& userName);


///###:auxiliary function.
///###: tag files
template<>
bool DSqliteHandle::helpExecSql<DSqliteHandle::SqlType::TagFiles, QMap<QString, QList<QString>>, bool>(const QMap<QString, QList<QString>>& forDecreasing,
                                                                                                       const QString& mountPoint, const QString& userName);

template<>
bool DSqliteHandle::helpExecSql<DSqliteHandle::SqlType::TagFiles2, QMap<QString, QList<QString>>, bool>(
                                                               const QMap<QString, QList<QString>>&  forIncreasing, const QString& mountPoint, const QString& userName);

template<>
bool DSqliteHandle::helpExecSql<DSqliteHandle::SqlType::TagFiles3, QList<QString>, bool>(const QList<QString>& forUpdating,
                                                                                                        const QString& mountPoint, const QString& userName);


template<>
bool DSqliteHandle::helpExecSql<DSqliteHandle::SqlType::TagFilesThroughColor,
                    std::list<std::tuple<QString, QString, QString, QString, QString, QString>>, bool>(const std::list<std::tuple<QString, QString, QString, QString, QString, QString>>& sqlStrs,
                                                                                                       const QString& mountPoint, const QString& userName);


///###: untag files in same/diff partion.
template<>
bool DSqliteHandle::helpExecSql<DSqliteHandle::SqlType::UntagSamePartionFiles, std::list<QString>, bool>(
                                                    const std::list<QString>& sqlStrs, const QString& mountPoint, const QString& userName);
template<>
bool DSqliteHandle::helpExecSql<DSqliteHandle::SqlType::UntagSamePartionFiles2, QMap<QString, QList<QString>>, bool>(const QMap<QString, QList<QString>>& fileNameAndTagNames, const QString& mountPoint,
                                                                                                               const QString& userName);


///### delete files and delete row(s) in tag_with_file and file_property.
template<>
bool DSqliteHandle::helpExecSql<DSqliteHandle::SqlType::DeleteFiles, std::map<QString, std::pair<QString, QString>>, bool>(const std::map<QString, std::pair<QString, QString>>& sqlStrs,
                                                                                                                            const QString& mountPoint, const QString& userName);

///###: delete tag(s)
template<>
bool DSqliteHandle::helpExecSql<DSqliteHandle::SqlType::DeleteTags,
                                std::list<QString>, bool>(const std::list<QString>& sqlStrs,
                                                                                  const QString& mountPoint, const QString& userName);
template<>
bool DSqliteHandle::helpExecSql<DSqliteHandle::SqlType::DeleteTags2,
                                QMap<QString, QList<QString>>, bool>(const QMap<QString, QList<QString>>& fileNameAndTagNames, const QString& mountPoint, const QString& userNamke);

///###: change file(s) name.
template<>
bool DSqliteHandle::helpExecSql<DSqliteHandle::SqlType::ChangeFilesName,
                                std::map<QString, QString>, bool>(const std::map<QString, QString>& sqlStrs, const QString& mountPoint, const QString& userName);

///###: change tag(s) name.
template<>
bool DSqliteHandle::helpExecSql<DSqliteHandle::SqlType::ChangeTagsName,
                                std::list<std::tuple<QString, QString, QString, QString>>, bool>(const std::list<std::tuple<QString, QString, QString, QString>>& sqlStrs,
                                                                                           const QString& mountPoint, const QString& userName);

///###: get tags through file.
template<>
QList<QString> DSqliteHandle::helpExecSql<DSqliteHandle::SqlType::GetTagsThroughFile,
                                QString, QList<QString>>(const QString& sqlStr, const QString& mountPoint, const QString& userName);


///###: get files which was tagged by appointed tag.
template<>
QList<QString> DSqliteHandle::helpExecSql<DSqliteHandle::SqlType::GetFilesThroughTag,
                                          QString, QList<QString>>(const QString& sqlStr, const QString& mountPoint, const QString& userName);


#endif // DSQLITEHANDLE_H
