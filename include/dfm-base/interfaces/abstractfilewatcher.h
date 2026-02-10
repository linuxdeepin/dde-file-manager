// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ABSTRACTFILEWATCHER_H
#define ABSTRACTFILEWATCHER_H

#include <dfm-base/dfm_base_global.h>

#include <QObject>

class QUrl;
namespace dfmbase {
class AbstractFileWatcherPrivate;
class AbstractFileWatcher : public QObject
{
    Q_OBJECT
    friend class AbstractFileWatcherPrivate;

public:
    QScopedPointer<AbstractFileWatcherPrivate> d;

protected:
    explicit AbstractFileWatcher(AbstractFileWatcherPrivate *dptr, QObject *parent = nullptr);

public:
    explicit AbstractFileWatcher() = delete;

    virtual ~AbstractFileWatcher();

    virtual QUrl url() const;
    virtual bool startWatcher();
    virtual bool stopWatcher();
    virtual bool restartWatcher();
    virtual void setEnabledSubfileWatcher(const QUrl &subfileUrl, bool enabled = true);
    int getCacheInfoConnectSize() const;
    void addCacheInfoConnectSize();
    void reduceCacheInfoConnectSize();
    //debug function
    static QStringList getMonitorFiles();

    virtual void notifyFileAdded(const QUrl &url);
    virtual void notifyFileChanged(const QUrl &url);
    virtual void notifyFileDeleted(const QUrl &url);

Q_SIGNALS:
    /*!
     * \brief fileDeleted 文件删除信号
     *
     * \param const QUrl &url 删除的文件的url
     */
    void fileDeleted(const QUrl &url);
    /*!
     * \brief fileAttributeChanged 文件的属性发生改变信号
     *
     * \param const QUrl &url 文件属性改变的url
     */
    void fileAttributeChanged(const QUrl &url);
    /*!
     * \brief fileDeleted 当前监视目录下的子文件创建信号
     *
     * \param const QUrl &url 当前监视目录下的子文件的url
     */
    void subfileCreated(const QUrl &url);
    /*!
     * \brief fileRename 当前监视目录文件重命名时发送此信号
     *
     * \param const QUrl &oldUrl 重命名前的文件url
     *
     * \param const DFileInfo &newUrl 重名后的文件url
     */
    void fileRename(const QUrl &oldUrl, const QUrl &newUrl);
};
}
typedef QSharedPointer<DFMBASE_NAMESPACE::AbstractFileWatcher> AbstractFileWatcherPointer;

#endif   // ABSTRACTFILEWATCHER_H
