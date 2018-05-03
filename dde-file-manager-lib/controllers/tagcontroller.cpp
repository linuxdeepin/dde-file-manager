

#include <QDebug>


#include "dfileservices.h"
#include "tagcontroller.h"
#include "tag/tagmanager.h"
#include "../tag/tagmanager.h"
#include "../models/tagfileinfo.h"
#include "../interfaces/dfileinfo.h"
#include "interfaces/dfileservices.h"
#include "controllers/appcontroller.h"
#include "private/dabstractfilewatcher_p.h"
#include "controllers/tagmanagerdaemoncontroller.h"


template<typename Ty>
using citerator = typename QList<Ty>::const_iterator;

template<typename Ty>
using itetrator = typename QList<Ty>::iterator;

TagController::TagController(QObject* const parent)
              :DAbstractFileController{ parent }
{
       //constructor!
}

const DAbstractFileInfoPointer TagController::createFileInfo(const QSharedPointer<DFMCreateFileInfoEvnet>& event) const
{
    DAbstractFileInfoPointer TaggedFilesInfo{ new TagFileInfo{ event->url() } };

    return TaggedFilesInfo;
}

const QList<DAbstractFileInfoPointer> TagController::getChildren(const QSharedPointer<DFMGetChildrensEvent>& event) const
{
    DUrl currentUrl{ event->url() };
    QList<DAbstractFileInfoPointer> infoList;


    if(currentUrl.isTaggedFile()){
        QString path{ currentUrl.path() };

        if(path == QString{"/"}){
            QMap<QString, QString> tags{ TagManager::instance()->getAllTags() };
            QMap<QString, QString>::const_iterator tagBeg{ tags.cbegin() };
            QMap<QString, QString>::const_iterator tagEnd{ tags.cend() };

            for(; tagBeg != tagEnd; ++tagBeg){
                DAbstractFileInfoPointer tagInfoPtr{
                                                       DFileService::instance()->createFileInfo(this,
                                                                                                DUrl::fromUserTaggedFile(QString{"/"} + tagBeg.key(), QString{}))
                                                    };
                infoList.push_back(tagInfoPtr);
            }

        }else if(currentUrl.parentUrl().path() == QString{"/"}){
            path = currentUrl.path();
            QString tagName{ path.remove(0, 1) };
            QList<QString> files{ TagManager::instance()->getFilesThroughTag(tagName) };

            for(const QString& localFilePath : files){
                DUrl url{ currentUrl };
                url.setTaggedFileUrl(localFilePath);
                DAbstractFileInfoPointer fileInfo{ new TagFileInfo(url) };

                infoList.push_back(fileInfo);
            }
        }
    }

    return infoList;
}

class TaggedFileWatcherPrivate;
class TaggedFileWatcher final : public DAbstractFileWatcher
{
public:
    explicit TaggedFileWatcher(const DUrl& url, QObject* const parent = nullptr);

    void setEnabledSubfileWatcher(const DUrl &subfileUrl, bool enabled = true);

private slots:
    void onFilesWereTagged();
    void onUntagFiles();

    void onTagAdded(const QList<QString>& tagNames);
    void onTagDeleted(const QList<QString>& tagNames);
    void onTagRenamed(const QPair<QString, QString>& oldAndNewName);


private:
    Q_DECLARE_PRIVATE(TaggedFileWatcher)

    void addWatcher(const DUrl& url) noexcept;
    void removeWatcher(const DUrl& url) noexcept;
};


class TaggedFileWatcherPrivate final : public DAbstractFileWatcherPrivate
{
public:
    TaggedFileWatcherPrivate(TaggedFileWatcher* qq)
        :DAbstractFileWatcherPrivate{qq}{}

    virtual bool start() override;
    virtual bool stop() override;

    Q_DECLARE_PUBLIC(TaggedFileWatcher)

    DUrl m_urlBak{};
    QString m_beWatchedPath{};
    QMap<DUrl, DAbstractFileWatcher*> m_watchers{};
};


TaggedFileWatcher::TaggedFileWatcher(const DUrl& url, QObject* const parent)
    :DAbstractFileWatcher{*(new TaggedFileWatcherPrivate{this}), url, parent}
{
    TaggedFileWatcherPrivate* d{ d_func() };
    d->m_beWatchedPath =  url.path();

#ifdef QT_DEBUG
    qDebug()<< "watched url: " << url;
    qDebug()<< "watched path: " << url.path();
#endif

}

void TaggedFileWatcher::setEnabledSubfileWatcher(const DUrl& subfileUrl, bool enabled)
{
    DUrl currentWatchedDir{ this->fileUrl() };

#ifdef QT_DEBUG
    qDebug()<< "subfileUrl: " << subfileUrl << "=============" << "fileUrl: " << this->fileUrl();
#endif

    if(subfileUrl == currentWatchedDir){
        return;
    }

    if(enabled){
        this->addWatcher(subfileUrl);

    }else{
        this->removeWatcher(subfileUrl);
    }
}

void TaggedFileWatcher::onFilesWereTagged()
{

}


void TaggedFileWatcher::onTagAdded(const QList<QString>& tagNames)
{
    for(const QString& tag_name : tagNames){
        DUrl new_file_url{ DUrl::fromUserTaggedFile(QString{"/"} + tag_name, QString{}) };

        emit subfileCreated(new_file_url);
    }
}


void TaggedFileWatcher::onTagDeleted(const QList<QString>& tagNames)
{
    for(const QString& tag_name : tagNames){
        DUrl new_file_url{ DUrl::fromUserTaggedFile(QString{"/"}+tag_name, QString{}) };

        emit fileDeleted(new_file_url);
    }
}

void TaggedFileWatcher::onTagRenamed(const QPair<QString, QString>& oldAndNewName)
{
    DUrl old_name{ DUrl::fromUserTaggedFile(QString{"/"} + oldAndNewName.first, QString{}) };
    DUrl new_name{ DUrl::fromUserTaggedFile(QString{"/"} + oldAndNewName.second, QString{}) };

    emit fileMoved(old_name, new_name);
}

void TaggedFileWatcher::addWatcher(const DUrl& url)noexcept
{
    TaggedFileWatcherPrivate* d{ d_func() };
    DUrl local_file_url{ DUrl::fromLocalFile(url.taggedLocalFilePath()) };

    if(!local_file_url.isValid() || d->m_watchers.contains(local_file_url)){
        return;
    }

    DAbstractFileWatcher* watcher{ DFileService::instance()->createFileWatcher(this, local_file_url) };

    if(!watcher){
        return;
    }

    watcher->setParent(this);
    watcher->moveToThread(this->thread());
    d->m_watchers[url] = watcher;

    if(d->started){
        watcher->startWatcher();
    }

}

void TaggedFileWatcher::removeWatcher(const DUrl& url)noexcept
{
    TaggedFileWatcherPrivate* d{ d_func() };
    DAbstractFileWatcher *watcher = d->m_watchers.take(url);

    if (!watcher){
        return;
    }

    watcher->deleteLater();
}


bool TaggedFileWatcherPrivate::start()
{
    TaggedFileWatcher* q{q_func()};
//    QObject::connect(TagManager::instance(), &TagManager::addNewTag, q, &TaggedFileWatcher::onTagAdded);
//    QObject::connect(TagManager::instance(), &TagManager::deleteTag, q, &TaggedFileWatcher::onTagDeleted);

    return false;
}

bool TaggedFileWatcherPrivate::stop()
{
    TaggedFileWatcher* q{q_func()};
    bool value{ QObject::disconnect(TagManager::instance(), 0, q, 0) };

    return value;
}


DAbstractFileWatcher* TagController::createFileWatcher(const QSharedPointer<DFMCreateFileWatcherEvent>& event) const
{
#ifdef QT_DEBUG
    qDebug()<< "be watched url: " << event->url();
#endif

    return (new TaggedFileWatcher{event->url()});
}

bool TagController::renameFile(const QSharedPointer<DFMRenameEvent> &event) const
{
    const QString &old_name = event->fromUrl().fileName();
    const QString &new_name = event->toUrl().fileName();;

    return TagManager::instance()->changeTagName(qMakePair(old_name, new_name));
}

bool TagController::deleteFiles(const QSharedPointer<DFMDeleteEvent> &event) const
{
    QStringList tagNames;

    for(auto oneUrl : event->urlList()) {
        QString oneName = oneUrl.fileName();
        tagNames.append(oneName);
    }

    return TagManager::instance()->deleteTags(tagNames);
}

bool TagController::setFileTags(const QSharedPointer<DFMSetFileTagsEvent> &event) const
{
    QList<QString> tags = event->tags();
    return DFileService::instance()->setFileTags(this, DUrl::fromLocalFile(event->url().taggedLocalFilePath()), tags);
}

bool TagController::removeTagsOfFile(const QSharedPointer<DFMRemoveTagsOfFileEvent> &event) const
{
    QList<QString> tags = event->tags();
    return DFileService::instance()->removeTagsOfFile(this, DUrl::fromLocalFile(event->url().taggedLocalFilePath()), tags);
}

QList<QString> TagController::getTagsThroughFiles(const QSharedPointer<DFMGetTagsThroughFilesEvent> &event) const
{
    DUrlList new_list;

    for (const DUrl &tag_url : event->urlList()) {
        const QString &file = tag_url.taggedLocalFilePath();

        if (!file.isEmpty())
            new_list << DUrl::fromLocalFile(file);
    }

    return DFileService::instance()->getTagsThroughFiles(this, new_list);
}

