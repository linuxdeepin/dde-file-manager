// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dfmfilepreviewfactory.h"
#include "private/dfmfilepreviewfactory_p.h"
#include "dfmfilepreviewplugin.h"
#include "dfmfactoryloader.h"
#include "dfmfilepreview.h"

DFM_BEGIN_NAMESPACE

#ifndef QT_NO_LIBRARY
Q_GLOBAL_STATIC_WITH_ARGS(DFMFactoryLoader, loader,
    (DFMFilePreviewFactoryInterface_iid, QLatin1String("/previews"), Qt::CaseInsensitive))
#endif

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
