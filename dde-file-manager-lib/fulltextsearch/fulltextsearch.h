/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 lawrence
 *
 * Author:     lawrence<hujianzhong@deepin.com>
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

#ifndef FULLTEXTSEARCH_H
#define FULLTEXTSEARCH_H

#include <QObject>
#include <dfmglobal.h>
#include <lucene++/LuceneHeaders.h>
#include <QQueue>
#include <QPair>
#include <QMutex>
#include "controllers/jobcontroller.h"

using namespace Lucene;
DFM_BEGIN_NAMESPACE
class DFMFullTextSearchManager : public QThread
{
    Q_OBJECT

public:
    static DFMFullTextSearchManager *getInstance();

    /**
     * @brief fullTextSearch 全文搜索接口
     * @param keyword 搜索关键字
     * @return 搜索结果
     */
    QStringList fullTextSearch(const QString &keyword);

    /**
     * @brief fulltextIndex 创建索引接口
     * @param sourceDir 索引目录
     * @return
     */
    int fulltextIndex(const QString &sourceDir);

    void updateIndex(const QString &filePath);

    inline void setSearchState(JobController::State state)
    {
        m_state = state;
    }

private:
    explicit DFMFullTextSearchManager(QObject *parent = nullptr);

    void indexDocs(const IndexWriterPtr &writer, const QString &sourceDir);

    DocumentPtr getFileDocument(const QString &filename);

    /**
     * @brief createFileIndex 创建文件索引
     * @param sourcePath 文件路径
     * @return
     */
    bool createFileIndex(const QString &sourcePath);

    /**
     * @brief getFileContents 获取文件内容
     * @param filePath 文件路径
     * @return
     */
    QString getFileContents(const QString &filePath);

    /**
     * @brief searchByKeyworld 搜索
     * @param keyword 搜索关键字
     * @return
     */
    bool searchByKeyworld(const QString &keyword);

    void doPagingSearch(const SearcherPtr &searcher, const QueryPtr &query, int32_t hitsPerPage, bool raw, bool interactive);

    void readFileName(const char *filePath, QStringList &result);
private:
    bool status;

    /*!
     * \brief searchResults 搜索结果
     */
    QStringList searchResults;

    /**
     * @brief indexStorePath 索引存储目录
     */
    QString indexStorePath;

    JobController::State m_state = JobController::Stoped;

    QMutex mutex;
};

DFM_END_NAMESPACE
#endif // FULLTEXTSEARCH_H
