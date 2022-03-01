/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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
#ifndef BURNJOB_H
#define BURNJOB_H

#include "dfmplugin_burn_global.h"
#include "dfm-base/interfaces/abstractjobhandler.h"

#include <dfm-burn/opticaldiscmanager.h>

#include <QObject>

DPBURN_BEGIN_NAMESPACE

class BurnJob : public QObject
{
    Q_OBJECT

public:
    struct BurnConfig
    {
        QString volName;
        int speeds;
        DFMBURN::BurnOptions opts;
    };

    using WorkFunc = std::function<void(int, int)>;

public:
    explicit BurnJob(QObject *parent = nullptr);

    void doOpticalDiskBlank(const QString &dev, const JobHandlePointer handler);
    void doUDFDataBurn(const QString &dev, const QUrl &url, const BurnConfig &conf, const JobHandlePointer handler);
    void doISODataBurn(const QString &dev, const QUrl &url, const BurnConfig &conf, const JobHandlePointer handler);
    void doISOImageBurn(const QString &dev, const QUrl &imageUrl, const BurnConfig &conf, const JobHandlePointer handler);

    // TODO(zhangs): connect signals
signals:
    void reqShowErrorMessage(const QString &title, const QString &message);

private slots:
    void onJobUpdated(DFMBURN::JobStatus status, int progress, const QString &speed, const QStringList &message);
    void workInProcess(const WorkFunc &writeFunc, const WorkFunc &readFunc);

private:
    void initConnect();
    bool prepare(const QString &devId);
};

DPBURN_END_NAMESPACE

#endif   // BURNJOB_H
