/**
 * Copyright (C) 2017 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/
#include "dfmfilepreviewfactory.h"
#include "dfmfilepreviewplugin.h"
#include "dfmfactoryloader.h"
#include "dfmfilepreview.h"

DFM_BEGIN_NAMESPACE

#ifndef QT_NO_LIBRARY
Q_GLOBAL_STATIC_WITH_ARGS(DFMFactoryLoader, loader,
    (DFMFilePreviewFactoryInterface_iid, QLatin1String("/previews"), Qt::CaseInsensitive))
#endif

class DFMFilePreviewFactoryPrivate
{
public:
    static QMap<const DFMFilePreview*, int> previewToLoaderIndex;
};

QMap<const DFMFilePreview*, int> DFMFilePreviewFactoryPrivate::previewToLoaderIndex;

QStringList DFMFilePreviewFactory::keys()
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

DFMFilePreview *DFMFilePreviewFactory::create(const QString &key)
{
    if (DFMFilePreview *view = dLoadPlugin<DFMFilePreview, DFMFilePreviewPlugin>(loader(), key)) {
        DFMFilePreviewFactoryPrivate::previewToLoaderIndex[view] = loader()->indexOf(key);

        QObject::connect(view, &DFMFilePreview::destroyed, view, [view] {
            DFMFilePreviewFactoryPrivate::previewToLoaderIndex.remove(view);
        });

        return view;
    }

    return Q_NULLPTR;
}

bool DFMFilePreviewFactory::isSuitedWithKey(const DFMFilePreview *view, const QString &key)
{
    int index = DFMFilePreviewFactoryPrivate::previewToLoaderIndex.value(view, -1);

    if (index == -1)
        return false;

    return index == loader()->indexOf(key);
}

DFM_END_NAMESPACE
