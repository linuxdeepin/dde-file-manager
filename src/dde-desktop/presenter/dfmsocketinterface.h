// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QScopedPointer>
#include "../global/singleton.h"

class DFMSocketInterfacePrivate;
class DFMSocketInterface : public QObject, public DDEDesktop::Singleton<DFMSocketInterface>
{
    Q_OBJECT
    friend class DDEDesktop::Singleton<DFMSocketInterface>;
public:
    explicit DFMSocketInterface(QObject *parent = nullptr);
    virtual ~ DFMSocketInterface();

signals:

public slots:
    void showProperty(const QStringList &paths);

private:
    QScopedPointer<DFMSocketInterfacePrivate> d_ptr;
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), DFMSocketInterface)
};

