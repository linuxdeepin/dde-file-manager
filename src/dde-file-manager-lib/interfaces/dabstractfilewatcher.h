/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
 *             hujianzhong<hujianzhong@uniontech.com>
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

#ifndef DABSTRACTFILEWATCHER_H
#define DABSTRACTFILEWATCHER_H

#include <QObject>

class DUrl;
class DAbstractFileWatcherPrivate;
class DAbstractFileWatcher : public QObject
{
    Q_OBJECT

public:
    virtual ~DAbstractFileWatcher();

    DUrl fileUrl() const;

    bool startWatcher();
    bool stopWatcher();
    bool restartWatcher();

    virtual void setEnabledSubfileWatcher(const DUrl &subfileUrl, bool enabled = true);

    using SignalType1 = void(DAbstractFileWatcher::*)(const DUrl &);
    using SignalType2 = void(DAbstractFileWatcher::*)(const DUrl &, const DUrl &);
    using SignalType3 = void(DAbstractFileWatcher::*)(const DUrl &, const int &);
    static bool ghostSignal(const DUrl &targetUrl, SignalType1 signal, const DUrl &arg1);
    static bool ghostSignal(const DUrl &targetUrl, SignalType2 signal, const DUrl &arg1, const DUrl &arg2);
    static bool ghostSignal(const DUrl &targetUrl, SignalType3 signal, const DUrl &arg1, const int isExternalSource = 1);

signals:
    void fileDeleted(const DUrl &url);
    void fileAttributeChanged(const DUrl &url, const int &isExternalSource = 1);
    void fileMoved(const DUrl &fromUrl, const DUrl &toUrl);
    void subfileCreated(const DUrl &url);
    void fileModified(const DUrl &url);
    void fileClosed(const DUrl &url);

protected:
    explicit DAbstractFileWatcher(DAbstractFileWatcherPrivate &dd, const DUrl &url, QObject *parent = nullptr);

    QScopedPointer<DAbstractFileWatcherPrivate> d_ptr;

private:
    Q_DISABLE_COPY(DAbstractFileWatcher)
    Q_DECLARE_PRIVATE(DAbstractFileWatcher)
};

#endif // DABSTRACTFILEWATCHER_H
