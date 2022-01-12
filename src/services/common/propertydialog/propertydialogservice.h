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
#ifndef PROPERTYDIALOGSERVICE_H
#define PROPERTYDIALOGSERVICE_H

#include "property_defines.h"
#include <dfm-framework/framework.h>

DSC_BEGIN_NAMESPACE
class PropertyDialogService final : public dpf::PluginService, dpf::AutoServiceRegister<PropertyDialogService>
{
    Q_OBJECT
    Q_DISABLE_COPY(PropertyDialogService)
    friend class dpf::QtClassFactory<dpf::PluginService>;

private:
    explicit PropertyDialogService(QObject *parent = nullptr);

public:
    static QString name()
    {
        return "org.deepin.service.PropertyDialogService";
    }

    /*!
     * \brief Used to register functions for creating extended control objects.
     * \param[in] view A function to create an extended control object.(The function returns the object type, see class in extendedcontrolview.h.)
     * \param[in] index control insert subscript
     * \param[out] error error get error message
     * \return true if the registration is successful, otherwise it fails.
     * \example:
     * class A :public ExtendedControlView
     * {
     *      Q_OBJECT
     *      Q_DISABLE_COPY(A)
     *  public:
     *      explicit A(QWidget *parent = nullptr):ExtendedControlView(parent){}
     *      virtual void setSelectFileUrl(const QUrl &url)
     *      {
     *          .......
     *      }
     * }
     *
     * A * fun(const QUrl &url)
     * {
     *      根据url判断是否创建控件对象
     *      创建
     *      A *a = new A();
     *      a->setSelectFileUrl(url);
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
     * bool flg = service->registerMethod<A>(fun, 1, &error);
     */
    template<typename T>
    bool registerMethod(RegisterCreateMethod::createControlView view, int index = -1, QString *error = nullptr)
    {
        return RegisterCreateMethod::ins()->registerFunction<T>(view, index, error);
    }

    void addFileProperty(const QList<QUrl> &url);

    void addDeviceProperty(const DeviceInfo &info);

    void showTrashProperty(const QUrl &url);

    void showComputerProperty(const QUrl &url);
};
DSC_END_NAMESPACE
#endif   // PROPERTYDIALOGSERVICE_H
