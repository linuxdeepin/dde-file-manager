#ifndef TAGMANAGER_H
#define TAGMANAGER_H


#include <map>
#include <ctime>
#include <regex>
#include <future>
#include <string>
#include <functional>
#include <type_traits>

#include "glob.h"
#include "taginfo.h"

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QReadWriteLock>
#include <QDebug>


#ifdef __cplusplus
extern "C"
{
#endif /*__cplusplus*/

#include <pwd.h>
#include <unistd.h>

#ifdef __cplusplus
}
#endif /*__cplusplus*/




namespace std {

template<>
struct hash<QString>
{
    inline std::size_t operator()(const QString& qstr) const noexcept
    {
        return (std::hash<std::string>{}(qstr.toStdString()));
    }
};

template<>
struct less<TagProperty>
{
    inline bool operator()(const TagProperty& lh, const TagProperty& rh)const noexcept
    {
        return lh.operator<(rh);
    }
};


template<>
struct less<FileProperty>
{
    inline bool operator()(const FileProperty& lh, const FileProperty& rh)const noexcept
    {
        return lh.operator<(rh);
    }
};

}


namespace impl {


///###: When in C++14/17/20 or later.
///###: You should use std::invoke_result to replace std::result_of.
template<typename Lock, typename Func1 = typename std::result_of<decltype(&Lock::lockForWrite)(Lock)>::type,
                        typename Func2 = typename std::result_of<decltype(&Lock::lockForRead)(Lock)>::type>
struct shared_mutex
{

public:
    enum class Options{Read, Write};

    shared_mutex(Lock& lck, shared_mutex::Options option = shared_mutex::Options::Read):lock{ &lck }
    {
        if(option == shared_mutex::Options::Read){
            lock->lockForRead();
        }else{
            lock->lockForWrite();
        }
    }


    ~shared_mutex()
    {
        lock->unlock();
    }

    shared_mutex(shared_mutex<Lock>& other)=delete;
    shared_mutex& operator=(const shared_mutex<Lock>& other)=delete;

private:
    Lock* lock;
};

}


static constexpr const char* const USERNAME{"username"};
static constexpr const char* const PASSWORD{"password"};


class TagManager
{
public:
    enum class SqlType : std::size_t
    {
        GetAllTags = 1,
        MakeFilesTags,
        GetTagsThroughFile,
        GetFilesThroughTag
    };


    TagManager()
    {
        QString mainDBLocation{ TagManager::getMainDBLocation() };
        impl::shared_mutex<QReadWriteLock> sharedLck{ TagManager::mutex,
                                                      impl::shared_mutex<QReadWriteLock>::Options::Write};
                if(QSqlDatabase::contains(R"foo(deep)foo")){
                    TagManager::sqlDataBase = QSqlDatabase::database(R"foo(deep)foo");

                }else{
                    TagManager::sqlDataBase = QSqlDatabase::addDatabase(R"foo(QSQLITE)foo", R"foo(deep)foo");
                    TagManager::sqlDataBase.setDatabaseName(mainDBLocation);
                    TagManager::sqlDataBase.setUserName(USERNAME);
                    TagManager::sqlDataBase.setPassword(PASSWORD);
                }


                if(TagManager::sqlDataBase.open()){
                    QSqlQuery sqlQuery{ TagManager::sqlDataBase };


                        QString createTagProperty{
                            "CREATE TABLE IF NOT EXISTS \"tag_property\" "
                            " ("
                            "`tag_index` INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE,"
                            "`tag_name`  TEXT NOT NULL UNIQUE,"
                            "`tag_color` TEXT NOT NULL"
                            ")"
                        };

                        if(!sqlQuery.exec(createTagProperty)){
                            qDebug() << sqlQuery.lastError();
                        }
                }
    }

    ~TagManager()
    {
        ///###: close database connection.
        if(TagManager::sqlDataBase.isOpen()){
            TagManager::sqlDataBase.close();
        }
    }

    TagManager(const TagManager& other)=delete;
    TagManager& operator=(const TagManager& other)=delete;

    inline static QString getCurrentUserName()
    {
        passwd* pwd = getpwuid(getuid());
        return QString::fromStdString(pwd->pw_name);
    }

    ///###: query
    QMap<QString, QString> getAllTags();

    QList<QString> getSameTagsOfDiffFiles(const QList<DUrl>& files);

    QMap<QString, QString> getTagColor(const QList<QString>& tags);

    QList<QString> getFilesThroughTag(const QString& tagName);



    ///###:modify
    bool makeFilesTags(const QList<QString>& tags, const QList<DUrl>& files);

    bool changeTagColor(const QString& oldColorName, const QString& newColorName);

    bool remveTagsOfFiles(const QList<QString>& tags, const QList<DUrl>& files);


    bool changeTagName(const QPair<QString, QString>& oldAndNewName);

    ///###:delete
    bool deleteTags(const QList<QString>& tags);

    bool deleteFiles(const QList<QString>& fileList);
    bool deleteFiles(const QList<DUrl>& urlList);


    static QSharedPointer<TagManager> instance()
    {
        static QSharedPointer<TagManager> tagManager{ nullptr };
        std::call_once(TagManager::onceFlag,
                       [&]{ tagManager = QSharedPointer<TagManager>{ new TagManager }; });

        return tagManager;
    }

private:
    static QString getMainDBLocation();

    QSqlDatabase sqlDataBase{};
    QReadWriteLock mutex{};
    static std::once_flag onceFlag;
};

#endif // TAGMANAGER_H
