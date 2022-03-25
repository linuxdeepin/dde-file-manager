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

#include "utils/registercreateprocess.h"

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

    static PropertyDialogService *instance();

    bool registerMethod(CPY_NAMESPACE::RegisterCreateProcess::createControlViewFunc view, int index = -1, QString *error = nullptr);

    bool registerMethod(CPY_NAMESPACE::RegisterCreateProcess::createControlViewFunc view, QString scheme);

    bool registerBasicExpand(CPY_NAMESPACE::RegisterCreateProcess::basicViewFieldFunc func, QString scheme);

    bool registerPropertyPathShowStyle(QString scheme);

    QWidget *createWidget(const QUrl &url);

    QMap<int, QWidget *> createControlView(const QUrl &url);

    QList<QMap<QString, QString>> basicExpandField(const QUrl &url);

    bool isContains(const QUrl &url);

    void addComputerPropertyToPropertyService();
};
DSC_END_NAMESPACE
#define propertyServIns ::DSC_NAMESPACE::PropertyDialogService::instance()
#endif   // PROPERTYDIALOGSERVICE_H
