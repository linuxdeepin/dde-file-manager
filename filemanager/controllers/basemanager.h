#ifndef BASEMANAGER_H
#define BASEMANAGER_H

class BaseManager
{
public:
    explicit BaseManager();
    ~BaseManager();

    virtual void load();
    virtual void save();
};

#endif // BASEMANAGER_H
