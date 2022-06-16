/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lixiang<lixianga@uniontech.com>
 *
 * Maintainer: lixiang<lixianga@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "propertydialogservice.h"
#include "property_defines.h"
#include "utils/computerpropertyhelper.h"
#include "dfm-base/utils/universalutils.h"

#include <mutex>

DSC_BEGIN_NAMESPACE
CPY_BEGIN_NAMESPACE
namespace EventType {
const int kEvokePropertyDialog = DFMBASE_NAMESPACE::UniversalUtils::registerEventType();
}

CPY_END_NAMESPACE
DSC_END_NAMESPACE

DFMBASE_USE_NAMESPACE
CPY_USE_NAMESPACE
DSC_USE_NAMESPACE
PropertyDialogService::PropertyDialogService(QObject *parent)
    : dpf::PluginService(parent),
      dpf::AutoServiceRegister<PropertyDialogService>()
{
}

PropertyDialogService *PropertyDialogService::service()
{
    auto &ctx = dpfInstance.serviceContext();
    static std::once_flag onceFlag;
    std::call_once(onceFlag, [&ctx]() {
        if (!ctx.load(DSC_NAMESPACE::PropertyDialogService::name()))
            abort();
    });

    return ctx.service<DSC_NAMESPACE::PropertyDialogService>(DSC_NAMESPACE::PropertyDialogService::name());
}

/*!
 * \brief Used to register functions for creating extended control objects.
 * \param[in] view A function to create an extended control object.(The function returns the object type, see class in extendedcontrolview.h.)
 * \param[in] index control insert subscript
 * \param[out] error error get error message
 * \return true if the registration is successful, otherwise it fails.
 * \example:
 * class A :public QWidget
 * {
 *      Q_OBJECT
 *      Q_DISABLE_COPY(A)
 *  public:
 *      explicit A(QWidget *parent = nullptr):QWidget(parent){}
 * }
 *
 * QWidget * fun(const QUrl &url)
 * {
 *      根据url判断是否创建控件对象
 *      创建
 *      A *a = new A();
 *      return a;
 *      不创建
 *      return nullptr;
 * }
 *
 * QString errStr;
 * auto &ctx = dpfInstance.serviceContext();
 * if (!ctx.load(WindowsService::name(), &errStr)) {
 * qCritical() << errStr;
 * abort();
 * }
 * PropertyDialogService *service = ctx.service<PropertyDialogService>(PropertyDialogService::name());
 * QString error;
 * bool flg = service->registerMethod(fun, 1, &error);
 */
bool PropertyDialogService::registerControlExpand(CPY_NAMESPACE::createControlViewFunc view, int index)
{
    return CPY_NAMESPACE::RegisterCreateProcess::instance()->registerControlExpand(view, index);
}

/*!
 * \brief Registers functions for creating custom properties dialog objects.
 * \param[in] view  Create custom properties dialog function.
 * \param[in] scheme corresponding to the creation function.
 * \return true if the registration is successful, otherwise it fails.
 * \example:
 * class A :public QWidget
 * {
 *      Q_OBJECT
 *      Q_DISABLE_COPY(A)
 *  public:
 *      explicit A(QWidget *parent = nullptr):QWidget(parent){}
 * }
 *
 * QWidget * fun(const QUrl &url)
 * {
 *      根据url判断是否创建控件对象
 *      创建
 *      A *a = new A();
 *      return a;
 *      不创建
 *      return nullptr;
 * }
 *
 * QString errStr;
 * auto &ctx = dpfInstance.serviceContext();
 * if (!ctx.load(WindowsService::name(), &errStr)) {
 * qCritical() << errStr;
 * abort();
 * }
 * PropertyDialogService *service = ctx.service<PropertyDialogService>(PropertyDialogService::name());
 * QString error;
 *
 * bool flg = service->registerMethod(fun, scheme);
 */
bool PropertyDialogService::registerCustomizePropertyView(CPY_NAMESPACE::createControlViewFunc view, QString scheme)
{
    return CPY_NAMESPACE::RegisterCreateProcess::instance()->registerCustomizePropertyView(view, scheme);
}

bool PropertyDialogService::registerBasicViewFiledExpand(CPY_NAMESPACE::basicViewFieldFunc func, const QString &scheme)
{
    return CPY_NAMESPACE::RegisterCreateProcess::instance()->registerBasicViewFiledExpand(func, scheme);
}

bool PropertyDialogService::registerFilterControlField(const QString &scheme, CPY_NAMESPACE::FilePropertyControlFilter filter)
{
    return CPY_NAMESPACE::RegisterCreateProcess::instance()->registerFilterControlField(scheme, filter);
}

QWidget *PropertyDialogService::createWidget(const QUrl &url)
{
    return CPY_NAMESPACE::RegisterCreateProcess::instance()->createCustomizePropertyWidget(url);
}

QMap<int, QWidget *> PropertyDialogService::createControlView(const QUrl &url)
{
    return CPY_NAMESPACE::RegisterCreateProcess::instance()->createControlView(url);
}

QMap<dfm_service_common::Property::BasicExpandType, dfm_service_common::Property::BasicExpand> PropertyDialogService::basicExpandField(const QUrl &url)
{
    return CPY_NAMESPACE::RegisterCreateProcess::instance()->basicExpandField(url);
}

dfm_service_common::Property::FilePropertyControlFilter PropertyDialogService::contorlFieldFilter(const QUrl &url)
{
    return CPY_NAMESPACE::RegisterCreateProcess::instance()->contorlFieldFilter(url);
}

bool PropertyDialogService::isContains(const QUrl &url)
{
    return CPY_NAMESPACE::RegisterCreateProcess::instance()->isContains(url);
}

void PropertyDialogService::addComputerPropertyToPropertyService()
{
    registerCustomizePropertyView(ComputerPropertyHelper::createComputerProperty, ComputerPropertyHelper::scheme());
}
