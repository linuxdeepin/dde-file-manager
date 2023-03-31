// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SUBSCRIBER_H
#define SUBSCRIBER_H

#include <dfm-base/dfm_base_global.h>

class QString;
namespace dfmbase {
class Subscriber
{
public:
    explicit Subscriber();
    virtual ~Subscriber();

    virtual void doSubscriberAction(const QString &path) = 0;
    int eventKey() const;
    void setEventKey(int eventKey);

private:
    int eventKeys { 0 };
};
}

#endif   // SUBSCRIBER_H
