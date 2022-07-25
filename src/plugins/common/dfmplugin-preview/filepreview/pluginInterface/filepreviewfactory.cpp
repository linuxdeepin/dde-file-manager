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
#include "filepreviewfactory.h"
#include "previewpluginloader.h"
#include "private/pluginloader_p.h"
#include "dfm-base/interfaces/abstractfilepreviewplugin.h"

#include <QStringList>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_filepreview;

#ifndef QT_NO_LIBRARY
Q_GLOBAL_STATIC_WITH_ARGS(PreviewPluginLoader, loader,
                          (FilePreviewFactoryInterface_iid, QLatin1String("/previews"), Qt::CaseInsensitive))
#endif

QMap<const AbstractBasePreview *, int> FilePreviewFactory::previewToLoaderIndex;

QStringList FilePreviewFactory::keys()
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

AbstractBasePreview *FilePreviewFactory::create(const QString &key)
{
    if (AbstractBasePreview *view = dLoadPlugin<AbstractBasePreview, AbstractFilePreviewPlugin>(loader(), key)) {
        FilePreviewFactory::previewToLoaderIndex[view] = loader()->indexOf(key);

        QObject::connect(view, &AbstractBasePreview::destroyed, view, [view] {
            FilePreviewFactory::previewToLoaderIndex.remove(view);
        });

        return view;
    }

    return Q_NULLPTR;
}

bool FilePreviewFactory::isSuitedWithKey(const AbstractBasePreview *view, const QString &key)
{
    int index = FilePreviewFactory::previewToLoaderIndex.value(view, -1);

    if (index == -1)
        return false;

    return index == loader()->indexOf(key);
}
