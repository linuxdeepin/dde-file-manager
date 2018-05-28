
#include "taghandle.h"

#include <../dde-file-manager-lib/tag/tagmanager.h>


void TagHandle::onFileCreate(const QByteArrayList& files)
{
    (void)files;
}



void TagHandle::onFileDelete(const QByteArrayList& files)
{
   if (!files.isEmpty()) {
       QList<DUrl> url_list{};

       for (const QByteArray& array : files) {
           url_list.push_back(DUrl::fromLocalFile(array));
       }

       TagManager::instance()->deleteFiles(url_list);
   }
}


void TagHandle::onFileRename(const QList<QPair<QByteArray, QByteArray>>& files)
{
    if (!files.isEmpty()) {
        QList<QPair<DUrl, DUrl>> old_and_new_name{};

        for (const QPair<QByteArray, QByteArray>& name : files) {
            DUrl old_name{ DUrl::fromLocalFile(name.first) };
            DUrl new_name{ DUrl::fromLocalFile(name.second) };

            old_and_new_name.push_back(QPair<DUrl, DUrl>{old_name, new_name});

            TagManager::instance()->changeFilesName(old_and_new_name);
        }
    }
}
