// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "subscriber.h"

namespace dfmbase {
Subscriber::Subscriber()
{
}

Subscriber::~Subscriber()
{
}

int Subscriber::eventKey() const
{
    return eventKeys;
}

void Subscriber::setEventKey(int eventKey)
{
    eventKeys = eventKey;
}

}
