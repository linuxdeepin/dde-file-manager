#ifndef DFMFACTORYLOADER_P_H
#define DFMFACTORYLOADER_P_H

#include <QString>
#include <private/qobject_p.h>
#include <private/qcoreapplication_p.h>
#include <QMutex>
#include <QPluginLoader>

#include "dfmglobal.h"
#include "dfmfactoryloader.h"

DFM_BEGIN_NAMESPACE

namespace {

// avoid duplicate QStringLiteral data:
inline QString iidKeyLiteral() { return QStringLiteral("IID"); }
#ifdef QT_SHARED
inline QString versionKeyLiteral() { return QStringLiteral("version"); }
#endif
inline QString metaDataKeyLiteral() { return QStringLiteral("MetaData"); }
inline QString keysKeyLiteral() { return QStringLiteral("Keys"); }

}

class DFMFactoryLoaderPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(DFMFactoryLoader)
public:
    DFMFactoryLoaderPrivate();
    ~DFMFactoryLoaderPrivate();
    mutable QMutex mutex;
    QByteArray iid;
    QList<QPluginLoader *> pluginLoaderList;
    QMultiMap<QString, QPluginLoader *> keyMap;
    QString suffix;
    Qt::CaseSensitivity cs;
    bool rki = false;
    QStringList loadedPaths;

    static QStringList pluginPaths;
};


DFM_END_NAMESPACE

#endif // DFMFACTORYLOADER_P_H
