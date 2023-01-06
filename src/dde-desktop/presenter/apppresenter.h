// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include "../global/singleton.h"

class Presenter : public QObject, public DDEDesktop::Singleton<Presenter>
{
    Q_OBJECT
public:

    void init();

signals:
    void setConfig(const QString &group, const QString &key, const QVariant &value);
    void removeConfig(const QString &group, const QString &key);
    void setConfigList(const QString &group, const QStringList &keys, const QVariantList &values);
    void removeConfigList(const QString &group, const QStringList &keys);

public slots:
    void onSortRoleChanged(int role, Qt::SortOrder order);
    void onAutoAlignToggled();
    void onAutoMergeToggled();
    void OnIconLevelChanged(int iconLevel);

private:
    Q_DISABLE_COPY(Presenter)
    explicit Presenter(QObject *parent = nullptr);
    friend class DDEDesktop::Singleton<Presenter>;
};

