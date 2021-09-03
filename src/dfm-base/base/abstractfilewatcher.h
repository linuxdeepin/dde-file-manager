/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef ABSTRACTFILEWATCHER_H
#define ABSTRACTFILEWATCHER_H

#include "dfm-base/dfm_base_global.h"

#include <QObject>

#include <dfm-io/core/dfileinfo.h>

class QUrl;
DFMBASE_BEGIN_NAMESPACE
class AbstractFileWatcherPrivate;
class AbstractFileWatcher : public QObject
{
    Q_OBJECT
public:
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
    explicit AbstractFileWatcher(const QString &filePath, QObject *parent = nullptr);
    explicit AbstractFileWatcher(AbstractFileWatcherPrivate &dd, const QUrl &url, QObject *parent = nullptr);
    QScopedPointer<AbstractFileWatcherPrivate> d_ptr;

private:
    Q_DISABLE_COPY(AbstractFileWatcher)
    Q_DECLARE_PRIVATE(AbstractFileWatcher)
};
DFMBASE_END_NAMESPACE
typedef QSharedPointer<DFMBASE_NAMESPACE::AbstractFileWatcher> AbstractFileWatcherPointer;

#endif // ABSTRACTFILEWATCHER_H
