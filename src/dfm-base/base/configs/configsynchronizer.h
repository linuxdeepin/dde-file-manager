// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CONFIGSYNCHORNIZER_H
#define CONFIGSYNCHORNIZER_H

#include <dfm-base/dfm_base_global.h>
#include <dfm-base/base/configs/configsyncdefs.h>
#include <dfm-base/base/application/application.h>

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

Q_SIGNALS:

private:
    explicit ConfigSynchronizer(QObject *parent = nullptr);
    ~ConfigSynchronizer();

private:
    QScopedPointer<ConfigSynchronizerPrivate> d;
};

DFMBASE_END_NAMESPACE

#endif   // CONFIGSYNCHORNIZER_H
