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
#include <QScopedPointer>
#include "../global/singleton.h"

class DFMSocketInterfacePrivate;
class DFMSocketInterface : public QObject, public Singleton<DFMSocketInterface>
{
    Q_OBJECT
    friend class Singleton<DFMSocketInterface>;
public:
    explicit DFMSocketInterface(QObject *parent = 0);
    ~DFMSocketInterface();

signals:

public slots:
    void showProperty(const QStringList &paths);

private:
    QScopedPointer<DFMSocketInterfacePrivate> d_ptr;
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), DFMSocketInterface)
};

