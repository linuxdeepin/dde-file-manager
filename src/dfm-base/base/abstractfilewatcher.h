/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyu@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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
#ifndef ABSTRACTFILEWATCHER_H
#define ABSTRACTFILEWATCHER_H

#include "dfm-base/dfm_base_global.h"

#include <dfm-io/core/dfileinfo.h>

#include <QObject>

class QUrl;
DFMBASE_BEGIN_NAMESPACE
class AbstractFileWatcherPrivate;
class AbstractFileWatcher : public QObject
{
    Q_OBJECT
public:
    explicit AbstractFileWatcher() = delete;
    explicit AbstractFileWatcher(const QUrl &url, QObject *parent = nullptr);
    virtual ~AbstractFileWatcher();

    virtual QUrl url() const;
    virtual bool startWatcher();
    virtual bool stopWatcher();
    virtual bool restartWatcher();    
    virtual void setEnabledSubfileWatcher(const QUrl &subfileUrl, bool enabled = true);
    //debug function
    static QStringList getMonitorFiles();

Q_SIGNALS:
    /*!
     * \brief fileDeleted 文件删除信号
     *
     * \param const QUrl &url 删除的文件的url
     *
     * \param const DFileInfo &fileInfo 文件的fileinfo
     */
    void fileDeleted(const QUrl &url, const DFMIO::DFileInfo &fileInfo);
    /*!
     * \brief fileAttributeChanged 文件的属性发生改变信号
     *
     * \param const QUrl &url 文件属性改变的url
     *
     * \param const DFileInfo &fileInfo 文件的fileinfo
     */
    void fileAttributeChanged(const QUrl &url, const DFMIO::DFileInfo &fileInfo);
    /*!
     * \brief fileDeleted 当前监视目录下的子文件创建信号
     *
     * \param const QUrl &url 当前监视目录下的子文件的url
     *
     * \param const DFileInfo &fileInfo 文件的fileinfo
     */
    void subfileCreated(const QUrl &url, const DFMIO::DFileInfo &fileInfo);
protected:
    explicit AbstractFileWatcher(QObject* parent = nullptr);
    explicit AbstractFileWatcher(AbstractFileWatcherPrivate &dd, const QUrl &url, QObject *parent = nullptr);
    QScopedPointer<AbstractFileWatcherPrivate> d_ptr;

private:
    void initFileWatcher();
    void initConnect();

private:
    Q_DISABLE_COPY(AbstractFileWatcher)
    Q_DECLARE_PRIVATE(AbstractFileWatcher)
};
DFMBASE_END_NAMESPACE
typedef QSharedPointer<DFMBASE_NAMESPACE::AbstractFileWatcher> AbstractFileWatcherPointer;

#endif // ABSTRACTFILEWATCHER_H
