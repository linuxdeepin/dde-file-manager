// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SUBSCRIBER_H
#define SUBSCRIBER_H

class QString;

class Subscriber
{
public:
    Subscriber();
    virtual ~Subscriber();

    virtual void doSubscriberAction(const QString &path) = 0;

    int eventKey() const;
    void setEventKey(int eventKey);

private:
    int m_eventKey = 0;
};

#endif // SUBSCRIBER_H
