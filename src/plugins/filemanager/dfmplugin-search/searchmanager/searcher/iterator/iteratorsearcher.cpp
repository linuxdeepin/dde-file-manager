// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "iteratorsearcher.h"
#include "utils/searchhelper.h"

#include <dfm-base/utils/fileutils.h>
#include <dfm-base/base/schemefactory.h>

#include <QDebug>

static int kEmitInterval = 50;   // 推送时间间隔（ms
static constexpr char kFilterFolders[] = "^/(dev|proc|sys|run|tmpfs).*$";

DFMBASE_USE_NAMESPACE
DPSEARCH_USE_NAMESPACE

IteratorSearcher::IteratorSearcher(const QUrl &url, const QString &key, QObject *parent)
    : AbstractSearcher(url, SearchHelper::instance()->checkWildcardAndToRegularExpression(key), parent)
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
        fmDebug() << "IteratorSearcher unearthed, current spend:" << cur;
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

        iterator->setProperty("QueryAttributes","standard::name,standard::type,standard::size,\
                                     standard::size,standard::is-symlink,standard::symlink-target,access::*,time::*");

        // 仅在过滤目录下进行搜索时，过滤目录下的内容才能被检索
        if (dfmbase::FileUtils::isLocalFile(url)) {
            QRegularExpression reg(kFilterFolders);
            const auto &searchRootPath = searchUrl.toLocalFile();
            const auto &filePath = url.toLocalFile();
            if (!reg.match(searchRootPath).hasMatch() && reg.match(filePath).hasMatch())
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
            if (info->isAttributes(OptInfoType::kIsDir) && !info->isAttributes(OptInfoType::kIsSymLink)) {
                const auto &fileUrl = info->urlOf(UrlInfoType::kUrl);
                if (!searchPathList.contains(fileUrl) || !fileUrl.path().startsWith("/sys/"))
                    searchPathList << fileUrl;
            }

            QRegularExpressionMatch match = regex.match(info->displayOf(DisPlayInfoType::kFileDisplayName));
            if (match.hasMatch()) {
                const auto &fileUrl = info->urlOf(UrlInfoType::kUrl);
                {
                    info->updateAttributes();
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
