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

#include <interfaces/durl.h>

#include <QMap>
#include <QList>
#include <QDebug>




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



class TagManager final : public QObject
{
    Q_OBJECT
public:

    enum class SqlType : std::size_t
    {
        GetAllTags = 1,
        MakeFilesTags,
        GetTagsThroughFile,
        GetFilesThroughTag,
        MakeFilesTagThroughColor
    };


    TagManager();
    virtual ~TagManager()=default;

    TagManager(const TagManager& other)=delete;
    TagManager& operator=(const TagManager& other)=delete;

    ///###: query
    QMap<QString, QString> getAllTags();

    QList<QString> getTagsThroughFiles(const QList<DUrl>& files);

    QMap<QString, QColor> getTagColor(const QList<QString>& tags);

    QList<QString> getFilesThroughTag(const QString& tagName);

    QString getTagNameThroughColor(const QString &colorName) const;

    ///###:modify
    bool makeFilesTags(const QList<QString>& tags, const QList<DUrl>& files);

    bool changeTagColor(const QString& tagName, const QPair<QString, QString>& oldAndNewTagColor);

    bool removeTagsOfFiles(const QList<QString>& tags, const QList<DUrl>& files);

    bool changeTagName(const QPair<QString, QString>& oldAndNewName);

    bool makeFilesTagThroughColor(const QString &color, const QList<DUrl>& files);

    bool changeFilesName(const QList<QPair<DUrl, DUrl>>& oldAndNewFilesName);


    ///###:delete
    bool deleteTags(const QList<QString>& tags);
    bool deleteFiles(const QList<DUrl>& urlList);


    static TagManager* instance()
    {
        static TagManager* tagManager{ new TagManager };
        return tagManager;
    }


signals:
    void taggedFileAdded(const QMap<QString, QList<DUrl>>& tag_and_url);
    void taggedFileDeleted(const QList<DUrl>& url);
    void taggedFileMoved(const QList<QPair<DUrl, DUrl>>& url);
    void tagAdded(const QList<QString>& tagNames);
    void tagDeleted(const QList<QString>& tagNames);
    void tagRenamed(const QPair<QString, QString>& oldAndNew);
};

#endif // TAGMANAGER_H
