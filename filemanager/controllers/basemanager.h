#ifndef BASEMANAGER_H
#define BASEMANAGER_H

#include <QObject>

class BaseManager : public QObject
{
    Q_OBJECT
public:
    explicit BaseManager(QObject *parent = 0);
    ~BaseManager();

    virtual void load();
    virtual void save();

signals:

public slots:
};

#endif // BASEMANAGER_H
