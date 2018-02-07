/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
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

#ifndef FILESSIZEWORKER_H
#define FILESSIZEWORKER_H

#include <QObject>
#include "durl.h"

class FilesSizeWorker : public QObject
{
    Q_OBJECT
public:
    explicit FilesSizeWorker(const DUrlList& urls={}, QObject *parent = 0);
    ~FilesSizeWorker();

    DUrlList urls() const;
    void setUrls(const DUrlList &urls);

    bool stopped() const;
    void setStopped(bool stopped);

    qint64 size() const;
    void setSize(const qint64 &size);

signals:
    void sizeUpdated(qint64 size);

public slots:
    void coumpueteSize();
    void updateSize();
    void stop();

private:
    DUrlList m_urls = {};
    qint64 m_size = 0;
    bool m_stopped = false;

};

#endif // FILESSIZEWORKER_H
