/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuzhangjian<liuzhangjian@uniontech.com>
 *
 * Maintainer: liuzhangjian<liuzhangjian@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "iteratorsearcher.h"
#include "search/utils/searchhelper.h"

#include "dfm-base/base/schemefactory.h"

#include <QDebug>

namespace {
static int kEmitInterval = 50;   // 推送时间间隔（ms
const char *const kFilterFolders = "^/(dev|proc|sys|run|tmpfs).*$";
}

DFMBASE_USE_NAMESPACE
DSB_FM_BEGIN_NAMESPACE

IteratorSearcher::IteratorSearcher(const QUrl &url, const QString &key, QObject *parent)
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

QList<QUrl> IteratorSearcher::takeAll()
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
        auto iterator = DirIteratorFactory::create(url, QStringList(), QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files);
        if (!iterator)
            continue;

        // 仅在过滤目录下进行搜索时，过滤目录下的内容才能被检索
        if (url.isLocalFile()) {
            QRegExp reg(kFilterFolders);
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
                const auto &fileUrl = info->url();
                if (!searchPathList.contains(fileUrl) || !fileUrl.path().startsWith("/sys/"))
                    searchPathList << fileUrl;
            }

            QRegularExpressionMatch match = regex.match(info->fileDisplayName());
            if (match.hasMatch()) {
                const auto &fileUrl = info->url();
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

DSB_FM_END_NAMESPACE
