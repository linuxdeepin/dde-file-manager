/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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
#ifndef CONFIGSYNCHORNIZER_H
#define CONFIGSYNCHORNIZER_H

#include "dfm_base_global.h"
#include "configsyncdefs.h"

#include "dfm-base/base/application/application.h"

#include <QObject>

DFMBASE_BEGIN_NAMESPACE

class ConfigSynchronizerPrivate;
/*!
 * \brief The ConfigSynchornizer class
 * this is used to synchronize the DSettings and DConfigs
 */
class ConfigSynchronizer : public QObject
{
    Q_OBJECT

public:
    static ConfigSynchronizer *instance();

    bool watchChange(const SyncPair &pair);

signals:

private:
    explicit ConfigSynchronizer(QObject *parent = nullptr);
    ~ConfigSynchronizer();

private:
    QScopedPointer<ConfigSynchronizerPrivate> d;
};

DFMBASE_END_NAMESPACE

#endif   // CONFIGSYNCHORNIZER_H
