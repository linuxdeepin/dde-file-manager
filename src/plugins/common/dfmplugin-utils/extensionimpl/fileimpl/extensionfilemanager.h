// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EXTENSIONFILEMANAGER_H
#define EXTENSIONFILEMANAGER_H

#include "dfmplugin_utils_global.h"

#include <dfm-extension/file/private/dfmextfileoperationhookprivate.h>

#include <QObject>
#include <QMap>

DPUTILS_BEGIN_NAMESPACE

class ExtensionFileManager : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(ExtensionFileManager)
    using Container = QMap<quint64, QPair<DFMEXT::DFMExtFileOperationHook *, DFMEXT::DFMExtFileOperationHookPrivate *>>;

public:
    static ExtensionFileManager &instance();
    void initialize();

private Q_SLOTS:
    void onAllPluginsInitialized();
    bool launch(const QString &desktopFile, const QStringList &filePaths);

private:
    explicit ExtensionFileManager(QObject *parent = nullptr);
    Container container;
};

DPUTILS_END_NAMESPACE

#endif   // EXTENSIONFILEMANAGER_H
