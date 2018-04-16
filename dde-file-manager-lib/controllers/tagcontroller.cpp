

#include <QDebug>

#include "tagcontroller.h"
#include "tag/tagmanager.h"
#include "../tag/taginfo.h"
#include "../tag/tagmanager.h"
#include "../models/tagfileinfo.h"
#include "../interfaces/dfileinfo.h"
#include "interfaces/dfileservices.h"
#include "controllers/appcontroller.h"
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
    DAbstractFileInfoPointer tagedFilesInfo{ new TagFileInfo{ event->url() } };

    return tagedFilesInfo;
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
                                                                                 DUrl::fromUserTagedFile(QString{"/"} + tagBeg.key() ))
                                                    };
                infoList.push_back(tagInfoPtr);
            }

        }else if(currentUrl.parentUrl().path() == QString{"/"}){
            path = currentUrl.path();
            QString tagName{ path.remove(0, 1) };
            QSharedPointer<DFMGetFilesThroughTag> event{ new DFMGetFilesThroughTag{this, tagName} };
            QVariant var{ AppController::instance()->actionGetFilesThroughTag(event) };
            QList<QString> files{ var.toStringList() };

            for(const QString& str : files){
                DUrl url{ DUrl::fromLocalFile(str) };
                DAbstractFileInfoPointer fileInfo{ DFileService::instance()->createFileInfo(this, url) };
                infoList.push_back(fileInfo);
            }
        }
    }

    return infoList;
}

bool TagController::makeFilesTags(const QSharedPointer<DFMMakeFilesTagsEvent> &event) const
{
    (void)event;
    return true;
}

bool TagController::removeTagsOfFiles(const QSharedPointer<DFMRemoveTagsOfFilesEvent>& event) const
{
    (void)event;
    return true;
}
