#ifndef MENUFACTORY_H
#define MENUFACTORY_H

#include "dfm_common_service_global.h"

#include <dfm-base/base/abstractfilemenu.h>
#include <dfm-framework/service/pluginservicecontext.h>

DSC_BEGIN_NAMESPACE
DFMBASE_USE_NAMESPACE

class MenuFactory final : dpf::QtClassFactory<AbstractFileMenu>,
        dpf::QtClassManager<AbstractFileMenu>
{
    Q_DISABLE_COPY(MenuFactory)
    friend class MenuService;

    MenuFactory();
    ~MenuFactory();

    template <class T>
    static bool regClass(const QString name, QString *errorString = nullptr)
    {
        return MenuFactory::instance().dpf::QtClassFactory<AbstractFileMenu>::regClass<T>(name,errorString);
    }

    static AbstractFileMenu *create(const QString name, QString *errorString = nullptr);

    static MenuFactory &instance()
    {
        static MenuFactory factory;
        return factory;
    }
};

DSC_END_NAMESPACE
#endif // MENUFACTORY_H
