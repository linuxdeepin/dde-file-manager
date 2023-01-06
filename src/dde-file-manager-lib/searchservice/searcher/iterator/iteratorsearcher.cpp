// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "iteratorsearcher.h"
#include "utils/searchhelper.h"
#include "interfaces/dfileservices.h"

#include <QDebug>

namespace {
const int kEmitInterval = 50;   // 推送时间间隔（ms）
const char *const kFilterFolders = "^/(boot|dev|proc|sys|run|lib|usr).*$";
}

IteratorSearcher::IteratorSearcher(const DUrl &url, const QString &key, QObject *parent)
    : AbstractSearcher(url, RegularExpression::checkWildcardAndToRegularExpression(key), parent)
{
    searchPathList << url;
    regex = QRegularExpression(keyword, QRegularExpression::CaseInsensitiveOption);
}

bool IteratorSearcher::search()
{
    //准备状态切运行中，否则直接返回
    if (!status.testAndSetRelease(kReady, kRuning))
        return false;

    notifyTimer.start();
    // 遍历搜索
    doSearch();

    //检查是否还有数据
    if (status.testAndSetRelease(kRuning, kCompleted)) {
        //发送数据
        if (hasItem())
            emit unearthed(this);
    }

    return true;
}

void IteratorSearcher::stop()
{
    status.storeRelease(kTerminated);
}

bool IteratorSearcher::hasItem() const
{
    QMutexLocker lk(&mutex);
    return !allResults.isEmpty();
}

QList<DUrl> IteratorSearcher::takeAll()
{
    QMutexLocker lk(&mutex);
    return std::move(allResults);
}

void IteratorSearcher::tryNotify()
{
    int cur = notifyTimer.elapsed();
    if (hasItem() && (cur - lastEmit) > kEmitInterval) {
        lastEmit = cur;
        qDebug() << "IteratorSearcher unearthed, current spend:" << cur;
        emit unearthed(this);
    }
}

void IteratorSearcher::doSearch()
{
    forever {
        if (searchPathList.isEmpty() || status.loadAcquire() != kRuning)
            return;

        const auto &url = searchPathList.takeAt(0);
        const auto &filters = QDir::AllEntries | QDir::NoDotAndDotDot | QDir::System;
        auto iterator = DFileService::instance()->createDirIterator(nullptr, url, {}, filters);
        if (!iterator)
            continue;

        // 仅在过滤目录下进行搜索时，过滤目录下的内容才能被检索
        if (url.isLocalFile()) {
            static QRegExp reg(kFilterFolders);
            const auto &searchRootPath = searchUrl.toLocalFile();
            const auto &filePath = url.toLocalFile();
            if (!reg.exactMatch(searchRootPath) && reg.exactMatch(filePath))
                continue;
        }

        while (iterator->hasNext()) {
            //中断
            if (status.loadAcquire() != kRuning)
                return;

            iterator->next();
            auto info = iterator->fileInfo();
            if (!info || !info->exists())
                continue;

            // 将目录添加到待搜索目录中
            if (info->isDir() && !info->isSymLink()) {
                const auto &fileUrl = info->fileUrl();
                if (!searchPathList.contains(fileUrl))
                    searchPathList << fileUrl;
            }

            QRegularExpressionMatch match = regex.match(info->fileDisplayName());
            if (match.hasMatch()) {
                const auto &fileUrl = info->fileUrl();
                {
                    QMutexLocker lk(&mutex);
                    allResults << fileUrl;
                }

                //推送
                tryNotify();
            }
        }

        iterator.clear();
    }
}
