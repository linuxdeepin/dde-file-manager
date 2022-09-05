// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include "dfmglobal.h"
#include <QObject>

DFM_BEGIN_NAMESPACE
class DFMMediaInfoPrivate;
class DFMMediaInfo : public QObject
{
    Q_OBJECT
public:
    enum MeidiaType{
        General,
        Video,
        Audio,
        Text,
        Other,
        Image,
        Menu,
        Max,
    };

    DFMMediaInfo(const QString &filename, QObject *parent=nullptr);
    ~DFMMediaInfo();
    QString Value(const QString &key, MeidiaType meidiaType = General);
    void startReadInfo();
    void stopReadInfo();
signals:
    void Finished();

private:
    QScopedPointer<DFMMediaInfoPrivate> d_private;
    Q_DECLARE_PRIVATE_D(d_private, DFMMediaInfo)
};

DFM_END_NAMESPACE
