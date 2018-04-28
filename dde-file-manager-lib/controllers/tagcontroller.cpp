

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
    void onTaggedSubfileCreated(const QList<DUrl>& filePath);
    void onTaggedFileDeleted(const QList<DUrl>& filesPath);
    void onTaggedFileMoved(const QList<QPair<DUrl, DUrl>>& filePath);
    void onTaggedFileAttributeChanged(const DUrl& filePath);
    void onTaggedFileModified(const DUrl& filePath);


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

//    qDebug()<< "watched url: " << url;
//    qDebug()<< "watched path: " << url.path();
}

void TaggedFileWatcher::setEnabledSubfileWatcher(const DUrl& subfileUrl, bool enabled)
{
    DUrl currentWatchedDir{ this->fileUrl() };
//    qDebug()<< "subfileUrl: " << subfileUrl << "=============" << "fileUrl: " << this->fileUrl();

    if(subfileUrl == currentWatchedDir){
        return;
    }

    if(enabled){
        this->addWatcher(subfileUrl);

    }else{
        this->removeWatcher(subfileUrl);
    }
}


void TaggedFileWatcher::onTaggedSubfileCreated(const QList<DUrl>& filesPath)
{
    TaggedFileWatcherPrivate* d{ d_func() };

    for(const DUrl& path : filesPath){
        DUrl newFileUrl{ DUrl::fromUserTaggedFile(d->m_beWatchedPath, path.toLocalFile()) };

        emit subfileCreated(newFileUrl);
    }
}

void TaggedFileWatcher::onTaggedFileDeleted(const QList<DUrl>& filesPath)
{
    TaggedFileWatcherPrivate* d{ d_func() };

    for(const DUrl& path : filesPath){
        DUrl newFileUrl{ DUrl::fromUserTaggedFile(d->m_beWatchedPath, path.toLocalFile()) };

        emit fileDeleted(newFileUrl);
    }
}

void TaggedFileWatcher::onTaggedFileMoved(const QList<QPair<DUrl, DUrl>>& filePath)
{
    TaggedFileWatcherPrivate* d{ d_func() };

    for(const QPair<DUrl, DUrl>& oldAndNew : filePath){
        DUrl old{ DUrl::fromUserTaggedFile(d->m_beWatchedPath, oldAndNew.first.toLocalFile()) };
        DUrl theNew{ DUrl::fromUserTaggedFile(d->m_beWatchedPath, oldAndNew.second.toLocalFile())};

        emit fileMoved(old, theNew);
    }


}

void TaggedFileWatcher::onTaggedFileAttributeChanged(const DUrl& filePath)
{
    TaggedFileWatcherPrivate* d{ d_func() };
    DUrl newFileUrl{ DUrl::fromUserTaggedFile(d->m_beWatchedPath, filePath.toLocalFile()) };

    emit fileAttributeChanged(newFileUrl);
}

void TaggedFileWatcher::onTaggedFileModified(const DUrl& filePath)
{
    TaggedFileWatcherPrivate* d{ d_func() };
    DUrl new_file_url{ DUrl::fromUserTaggedFile(d->m_beWatchedPath, filePath.toLocalFile()) };

    emit fileModified(new_file_url);
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

    QObject::connect(watcher, &DAbstractFileWatcher::fileAttributeChanged, this, &TaggedFileWatcher::onTaggedFileAttributeChanged);
    QObject::connect(watcher, &DAbstractFileWatcher::fileDeleted, [this](const DUrl& fileForDeleting){
        this->onTaggedFileDeleted(QList<DUrl>{fileForDeleting});
    });
    QObject::connect(watcher, &DAbstractFileWatcher::fileModified, this, &TaggedFileWatcher::onTaggedFileModified);

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

    QMetaObject::Connection connection1{ QObject::connect(TagManager::instance(), &TagManager::taggedFileAdded, q, &TaggedFileWatcher::onTaggedSubfileCreated) };
    QMetaObject::Connection connection2{ QObject::connect(TagManager::instance(), &TagManager::taggedFileDeleted, q, &TaggedFileWatcher::onTaggedFileDeleted) };
    QMetaObject::Connection connection3{ QObject::connect(TagManager::instance(), &TagManager::taggedFileMoved, q, &TaggedFileWatcher::onTaggedFileMoved) };

    return (connection1 && connection2 && connection3);
}

bool TaggedFileWatcherPrivate::stop()
{
    TaggedFileWatcher* q{q_func()};
    bool value{ QObject::disconnect(TagManager::instance(), 0, q, 0) };

    return value;
}


DAbstractFileWatcher* TagController::createFileWatcher(const QSharedPointer<DFMCreateFileWatcherEvent>& event) const
{
//    qDebug()<< "be watched url: " << event->url();
    return (new TaggedFileWatcher{event->url()});
}

bool TagController::renameFile(const QSharedPointer<DFMRenameEvent> &event) const
{
    return TagManager::instance()->changeTagName(QPair<QString, QString>(event->fromUrl().fileName(), event->toUrl().fileName()));
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

bool TagController::makeFileTags(const QSharedPointer<DFMMakeFileTagsEvent> &event) const
{
    QList<QString> tags = event->tags();
    return DFileService::instance()->makeFileTags(this, DUrl::fromLocalFile(event->url().taggedLocalFilePath()), tags);
}

bool TagController::removeTagsOfFile(const QSharedPointer<DFMRemoveTagsOfFileEvent> &event) const
{
    QList<QString> tags = event->tags();
    return DFileService::instance()->removeTagsOfFile(this, DUrl::fromLocalFile(event->url().taggedLocalFilePath()), tags);
}

