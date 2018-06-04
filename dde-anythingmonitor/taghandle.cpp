
#include "taghandle.h"

#include <tag/tagmanager.h>
#include <shutil/danythingmonitorfilter.h>



void TagHandle::onFileCreate(const QByteArrayList &files)
{
    (void)files;
}



void TagHandle::onFileDelete(const QByteArrayList &files)
{
    if (!files.isEmpty()) {
        QList<DUrl> url_list{};

        for (const QByteArray &byte_array : files) {
            DUrl url{ DUrl::fromLocalFile(byte_array) };
            bool result{ DAnythingMonitorFilter::instance()->whetherFilterCurrentPath(url) };

            if (result) {
                url_list.push_back(url);
            }

        }

        TagManager::instance()->deleteFiles(url_list);
    }
}


void TagHandle::onFileRename(const QList<QPair<QByteArray, QByteArray>> &files)
{
    if (!files.isEmpty()) {
        QList<QPair<DUrl, DUrl>> old_and_new_name{};

        for (const QPair<QByteArray, QByteArray> &name : files) {
            DUrl old{ DUrl::fromLocalFile(name.first) };
            bool result{ DAnythingMonitorFilter::instance()->whetherFilterCurrentPath(old) };

            if (result) {
                DUrl new_name{ DUrl::fromLocalFile(name.second) };
                old_and_new_name.push_back(QPair<DUrl, DUrl> {old, new_name});
                TagManager::instance()->changeFilesName(old_and_new_name);
            }
        }
    }
}
