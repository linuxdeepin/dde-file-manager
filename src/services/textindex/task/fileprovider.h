// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILEPROVIDER_H
#define FILEPROVIDER_H

#include "service_textindex_global.h"
#include "utils/taskstate.h"

#include <dfm-search/searchresult.h>

#include <QString>
#include <QStringList>
#include <functional>

SERVICETEXTINDEX_BEGIN_NAMESPACE

// 文件处理回调函数类型
using FileHandler = std::function<void(const QString &path)>;

// 文件提供者接口，用于抽象不同的文件获取方式
class FileProvider
{
public:
    virtual ~FileProvider() = default;

    // 遍历文件并处理
    virtual void traverse(TaskState &state, const FileHandler &handler) = 0;
};

// 文件系统遍历提供者
class FileSystemProvider : public FileProvider
{
public:
    explicit FileSystemProvider(const QString &rootPath);
    void traverse(TaskState &state, const FileHandler &handler) override;

private:
    QString m_rootPath;
};

// 直接文件列表提供者
class DirectFileListProvider : public FileProvider
{
public:
    explicit DirectFileListProvider(const DFMSEARCH::SearchResultList &files);
    void traverse(TaskState &state, const FileHandler &handler) override;

private:
    DFMSEARCH::SearchResultList m_fileList;
};

SERVICETEXTINDEX_END_NAMESPACE

#endif   // FILEPROVIDER_H
