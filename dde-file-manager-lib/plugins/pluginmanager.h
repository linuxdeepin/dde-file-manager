#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include <QObject>
#include <QScopedPointer>

class MenuInterface;
class PluginManager;


class PluginManagerPrivate {

public:
    PluginManagerPrivate(PluginManager* parent);

    QList<MenuInterface*> menuInterfaces;

private:
    PluginManager* q_ptr=NULL;
    Q_DECLARE_PUBLIC(PluginManager)
};


class PluginManager : public QObject
{
    Q_OBJECT

public:
    inline static PluginManager* instance(){
        static PluginManager* instance = new PluginManager();
        return instance;
    }

    inline static QString PluginDir(){
        return PLUGINDIR;
    }

    void loadPlugin();
    QList<MenuInterface*> getMenuInterfaces();

signals:

public slots:

private:
    explicit PluginManager(QObject *parent = 0);
    ~PluginManager();

    QScopedPointer<PluginManagerPrivate> d_ptr;
    Q_DECLARE_PRIVATE(PluginManager)
};

#endif // PLUGINMANAGER_H
