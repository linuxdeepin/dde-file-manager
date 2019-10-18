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

#ifndef DDE_ANYTHINGMONITOR
    ///###: query
    QMap<QString, QString> getAllTags();

    QList<QString> getTagsThroughFiles(const QList<DUrl>& files);

    QMap<QString, QColor> getTagColor(const QList<QString>& tags) const;
    QString getTagColorName(const QString &tag) const;
    QString getTagIconName(const QString &tag) const;
    QString getTagIconName(const QColor &color) const;

    QList<QString> getFilesThroughTag(const QString& tagName);

    QString getTagNameThroughColor(const QColor &color) const;
    QColor getColorByColorName(const QString &colorName) const;
    QString getColorByDisplayName(const QString &colorDisplayName) const;
    QString getColorNameByColor(const QColor &color) const;
    QSet<QString> allTagOfDefaultColors() const;

    ///###:modify
    bool makeFilesTags(const QList<QString>& tags, const QList<DUrl>& files);

    bool changeTagColor(const QString& tagName, const QString& new_tag_color);

    bool removeTagsOfFiles(const QList<QString>& tags, const QList<DUrl>& files);

    bool changeTagName(const QPair<QString, QString>& oldAndNewName);

    bool makeFilesTagThroughColor(const QString &color, const QList<DUrl>& files);
#endif
    static bool changeFilesName(const QList<QPair<QByteArray, QByteArray> > &oldAndNewFilesName);

#ifndef DDE_ANYTHINGMONITOR
    ///###:delete
    bool deleteTags(const QList<QString>& tags);
#endif
    static bool deleteFiles(const QList<DUrl>& urlList);

#ifndef DDE_ANYTHINGMONITOR
    static TagManager* instance()
    {
        static TagManager* tagManager{ new TagManager };
        return tagManager;
    }

signals:
    void addNewTag(const QList<QString>& new_tags);
    void changeTagColor(const QMap<QString, QString>& old_and_new_color);
    void changeTagName(const QMap<QString, QString>& old_and_new_name);
    void deleteTag(const QList<QString>& be_deleted_tags);
    void filesWereTagged(const QMap<QString, QList<QString>>& files_were_tagged);
    void untagFiles(const QMap<QString, QList<QString>>& tag_be_removed_files);

private:
    void init_connect()noexcept;
#endif
};

#endif // TAGMANAGER_H
