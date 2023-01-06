// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "subscriber.h"


Subscriber::Subscriber()
{
}

Subscriber::~Subscriber()
{
}

int Subscriber::eventKey() const
{
    return m_eventKey;
}

void Subscriber::setEventKey(int eventKey)
{
    m_eventKey = eventKey;
}
