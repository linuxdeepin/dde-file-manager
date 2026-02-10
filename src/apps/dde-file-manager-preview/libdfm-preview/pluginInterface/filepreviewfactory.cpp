// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "filepreviewfactory.h"
#include "previewpluginloader.h"
#include "private/pluginloader_p.h"
#include <dfm-base/interfaces/abstractfilepreviewplugin.h>

#include <QStringList>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_filepreview;

Q_DECLARE_LOGGING_CATEGORY(logLibFilePreview)

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
    
    qCDebug(logLibFilePreview) << "FilePreviewFactory: available preview keys:" << list.size() << "keys";
#endif
    return list;
}

AbstractBasePreview *FilePreviewFactory::create(const QString &key)
{
    qCDebug(logLibFilePreview) << "FilePreviewFactory: attempting to create preview for key:" << key;
    
    if (AbstractBasePreview *view = dLoadPlugin<AbstractBasePreview, AbstractFilePreviewPlugin>(loader(), key)) {
        if (!view) {
            qCWarning(logLibFilePreview) << "FilePreviewFactory: plugin loaded but failed to create preview instance for key:" << key;
            return nullptr;
        }

        FilePreviewFactory::previewToLoaderIndex[view] = loader()->indexOf(key);
        qCInfo(logLibFilePreview) << "FilePreviewFactory: successfully created preview instance for key:" << key;

        QObject::connect(view, &AbstractBasePreview::destroyed, view, [view, key] {
            FilePreviewFactory::previewToLoaderIndex.remove(view);
            qCDebug(logLibFilePreview) << "FilePreviewFactory: preview instance destroyed for key:" << key;
        });

        return view;
    }

    qCWarning(logLibFilePreview) << "FilePreviewFactory: failed to create preview for key:" << key << "- no suitable plugin found";
    return Q_NULLPTR;
}

bool FilePreviewFactory::isSuitedWithKey(const AbstractBasePreview *view, const QString &key)
{
    if (!view) {
        qCWarning(logLibFilePreview) << "FilePreviewFactory: null preview instance provided for key compatibility check:" << key;
        return false;
    }

    int index = FilePreviewFactory::previewToLoaderIndex.value(view, -1);

    if (index == -1) {
        qCDebug(logLibFilePreview) << "FilePreviewFactory: preview instance not found in loader index for key:" << key;
        return false;
    }

    bool suited = index == loader()->indexOf(key);
    qCDebug(logLibFilePreview) << "FilePreviewFactory: preview compatibility check for key:" << key << "result:" << suited;
    return suited;
}

bool FilePreviewFactory::hasPluginForKey(const QString &key)
{
#ifndef QT_NO_LIBRARY
    int index = loader()->indexOf(key);
    bool exists = (index != -1);
    qCDebug(logLibFilePreview) << "FilePreviewFactory: plugin existence check for key:" << key << "result:" << exists;
    return exists;
#else
    return false;
#endif
}
