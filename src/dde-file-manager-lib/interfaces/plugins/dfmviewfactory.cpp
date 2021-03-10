/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     gongheng<gongheng@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             gongheng<gongheng@uniontech.com>
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

#include "dfmviewfactory.h"
#include "dfmviewplugin.h"
#include "dfmfactoryloader.h"

#include "durl.h"
#include "dfmbaseview.h"
#include "private/dfmviewfactory_p.h"

#include <QWidget>

DFM_BEGIN_NAMESPACE

#ifndef QT_NO_LIBRARY
Q_GLOBAL_STATIC_WITH_ARGS(DFMFactoryLoader, loader,
    (DFMViewFactoryInterface_iid, QLatin1String("/views"), Qt::CaseInsensitive))
#endif

QMap<const DFMBaseView*, int> DFMViewFactoryPrivate::viewToLoaderIndex;

QStringList DFMViewFactory::keys()
{
    QStringList list;
#ifndef QT_NO_LIBRARY
    typedef QMultiMap<int, QString> PluginKeyMap;

    const PluginKeyMap keyMap = loader()->keyMap();
    const PluginKeyMap::const_iterator cend = keyMap.constEnd();
    for (PluginKeyMap::const_iterator it = keyMap.constBegin(); it != cend; ++it)
        list.append(it.value());
#endif
    return list;
}

DFMBaseView *DFMViewFactory::create(const QString &key)
{
    if (DFMBaseView *view = dLoadPlugin<DFMBaseView, DFMViewPlugin>(loader(), key)) {
        DFMViewFactoryPrivate::viewToLoaderIndex[view] = loader()->indexOf(key);

        QObject *object = dynamic_cast<QObject*>(view);

        if (!object)
            object = view->widget();

        QObject::connect(object, &QObject::destroyed, object, [view] {
            DFMViewFactoryPrivate::viewToLoaderIndex.remove(view);
        });

        return view;
    }

    return Q_NULLPTR;
}

bool DFMViewFactory::viewIsSuitedWithUrl(const DFMBaseView *view, const DUrl &url)
{
    int index = DFMViewFactoryPrivate::viewToLoaderIndex.value(view, -1);

    if (index != -1) {
        QStringList key_list;

        key_list << url.scheme() + "://" + url.host();
        key_list << "://" + url.host();
        key_list << url.scheme() + "://";

        for (const QString &key : key_list) {
            int i = loader()->indexOf(key);

            if (i < 0)
                continue;

            return i == index;
        }
    }

    return false;
}

DFM_END_NAMESPACE
