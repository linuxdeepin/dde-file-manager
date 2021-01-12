#include "taghandleplugin.h"

TagHandlePlugin::TagHandlePlugin(QObject * const parent)
: DASPlugin{ parent }
{

}

DASInterface *TagHandlePlugin::create(const QString &key)
{
    (void)key;
    return (new TagHandle{});
}
