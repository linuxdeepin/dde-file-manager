// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "filepreviewfactory.h"
#include "previewpluginloader.h"
#include "private/pluginloader_p.h"
#include <dfm-base/interfaces/abstractfilepreviewplugin.h>

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
        if (!view)
            return nullptr;

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
