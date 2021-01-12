#ifndef TAGHANDLEPLUGIN_H
#define TAGHANDLEPLUGIN_H

#include <dasplugin.h>

#include "taghandle.h"

using namespace DAS_NAMESPACE;

class TagHandlePlugin : public DASPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID DASFactoryInterface_iid FILE "dde-anythingmonitor.json")

public:
    explicit TagHandlePlugin(QObject *const parent = nullptr);
    virtual ~TagHandlePlugin() = default;

    TagHandlePlugin(const TagHandlePlugin &other) = delete;
    TagHandlePlugin &operator=(const TagHandlePlugin &other) = delete;

    DASInterface *create(const QString &key);
};

#endif // TAGHANDLEPLUGIN_H
