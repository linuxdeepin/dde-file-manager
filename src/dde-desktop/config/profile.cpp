// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "profile.h"

class ProfilePrivate
{
public:
    explicit ProfilePrivate(Profile *parent) : q_ptr(parent){}

    Profile *q_ptr;
    Q_DECLARE_PUBLIC(Profile)
};

Profile::Profile(QObject *parent) :
    QObject(parent), d_ptr(new ProfilePrivate(this))
{

}

Profile::~Profile()
{

}
