#ifndef DFMEXTMENULOADER_P_H
#define DFMEXTMENULOADER_P_H

#include <QString>
#include <QLibrary>

class DFMExtPluginLoader;
class DFMExtPluginLoaderPrivate
{
    friend class DFMExtPluginLoader;
    DFMExtPluginLoader *const q;
    QLibrary qlib;
    QString errorString;
    explicit DFMExtPluginLoaderPrivate(DFMExtPluginLoader *qq)
        : q(qq)
    {

    }
};

#endif // DFMEXTMENULOADER_P_H
