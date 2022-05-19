/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
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
#ifndef FRAMESERVICE_H
#define FRAMESERVICE_H

#include "services/desktop/dd_service_global.h"

#include <dfm-framework/framework.h>

class Core;

DSB_D_BEGIN_NAMESPACE
namespace FrameProperty {
// property for window
inline constexpr char kPropScreenName[] = "ScreenName";
inline constexpr char kPropIsPrimary[] = "IsPrimary";
inline constexpr char kPropScreenGeometry[] = "ScreenGeometry";
inline constexpr char kPropScreenAvailableGeometry[] = "ScreenAvailableGeometry";
inline constexpr char kPropScreenHandleGeometry[] = "ScreenHandleGeometry";

// property for window and all sub widgets
inline constexpr char kPropWidgetName[] = "WidgetName";
inline constexpr char kPropWidgetLevel[] = "WidgetLevel";
}

class FrameServicePrivate;
class FrameService : public dpf::PluginService, dpf::AutoServiceRegister<FrameService>
{
    Q_OBJECT
    Q_DISABLE_COPY(FrameService)
    friend class dpf::QtClassFactory<dpf::PluginService>;
    friend class ::Core;
    friend class FrameServicePrivate;

public:
    static QString name()
    {
        return "org.deepin.service.Desktop.FrameService";
    }

    QList<QWidget *> rootWindows() const;
    void layoutWidget() const;
signals:   //must connect with Qt::DirectConnection
    void windowAboutToBeBuilded();
    void windowBuilded();
    void windowShowed();
    void geometryChanged();
    void availableGeometryChanged();

protected:
    explicit FrameService(QObject *parent = nullptr);

private:
    FrameServicePrivate *d;
};

DSB_D_END_NAMESPACE

#endif   // FRAMESERVICE_H
