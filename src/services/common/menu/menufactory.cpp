#include "menufactory.h"
DSC_BEGIN_NAMESPACE

MenuFactory::MenuFactory()
{

}

MenuFactory::~MenuFactory()
{

}

AbstractFileMenu *MenuFactory::create(const QString name, QString *errorString)
{
    auto topClass = MenuFactory::instance().dpf::QtClassManager<AbstractFileMenu>::value(name);
    if (topClass)
        return topClass;
    topClass = MenuFactory::instance().dpf::QtClassFactory<AbstractFileMenu>::create(name);
    MenuFactory::instance().dpf::QtClassManager<AbstractFileMenu>::append(name, topClass, errorString); //缓存
    return topClass;
}

DSC_END_NAMESPACE
