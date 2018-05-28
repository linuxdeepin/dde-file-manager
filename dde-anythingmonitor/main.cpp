#include <dasplugin.h>

#include "taghandle.h"

using namespace DAS_NAMESPACE;

class TagHandlePlugin : public DASPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID DASFactoryInterface_iid FILE "taghandle_as_plugin.json")

public:
    TagHandlePlugin(QObject* const parent = nullptr)
        :DASPlugin{ parent }{}
    virtual ~TagHandlePlugin()=default;

    TagHandlePlugin(const TagHandlePlugin& other)=delete;
    TagHandlePlugin& operator=(const TagHandlePlugin& other)=delete;

    DASInterface* create(const QString& key)
    {
        (void)key;
        return (new TagHandle{});
    }

};

#include "main.moc"
