/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#pragma once

#include <QObject>
#include "../global/singleton.h"

class Presenter : public QObject, public Singleton<Presenter>
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
    void OnIconLevelChanged(int iconLevel);

private:
    Q_DISABLE_COPY(Presenter)
    explicit Presenter(QObject *parent = 0);
    friend class Singleton<Presenter>;
};

