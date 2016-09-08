#ifndef SUBSCRIBER_H
#define SUBSCRIBER_H

class QString;

class Subscriber
{
public:

    virtual void doSubscriberAction(const QString& path) = 0;

    int eventKey() const;
    void setEventKey(int eventKey);

private:
    int m_eventKey;
};

#endif // SUBSCRIBER_H
