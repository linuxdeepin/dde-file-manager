// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "extensionfilemanager.h"
#include "extensionimpl/pluginsload/extensionpluginmanager.h"

#include <dfm-base/utils/applaunchutils.h>

#include <dfm-extension/file/dfmextfileoperationhook.h>

#include <QTimer>

#include <mutex>

DPUTILS_BEGIN_NAMESPACE
USING_DFMEXT_NAMESPACE
DFMBASE_USE_NAMESPACE

static std::vector<std::string> convertQStringListToVector(const QStringList &filePaths)
{
    std::vector<std::string> result;
    result.reserve(filePaths.size());   // 预先分配空间，避免多次分配内存

    for (const QString &path : filePaths) {
        result.push_back(path.toStdString());   // 将每个 QString 转换为 std::string
    }

    return result;
}

static QStringList convertVectorToQStringList(const std::vector<std::string> &filePaths)
{
    QStringList result;

    for (const std::string &path : filePaths) {
        result.append(QString::fromStdString(path));   // 将 std::string 转换为 QString 并添加到 QStringList
    }

    return result;
}

ExtensionFileManager &ExtensionFileManager::instance()
{
    static ExtensionFileManager ins;
    return ins;
}

void ExtensionFileManager::initialize()
{
    connect(&ExtensionPluginManager::instance(), &ExtensionPluginManager::allPluginsInitialized,
            this, &ExtensionFileManager::onAllPluginsInitialized);

    if (ExtensionPluginManager::instance().initialized()) {
        onAllPluginsInitialized();
    } else {
        QTimer::singleShot(200, this, []() {
            emit ExtensionPluginManager::instance().requestInitlaizePlugins();
        });
    }
}

void ExtensionFileManager::onAllPluginsInitialized()
{
    static std::once_flag flag;
    std::call_once(flag, [this]() {
        const auto &filePlugins { ExtensionPluginManager::instance().filePlugins() };
        std::for_each(filePlugins.begin(), filePlugins.end(), [this](DFMExtFilePlugin *plugin) {
            Q_ASSERT(plugin);
            quint64 pluginAddr { reinterpret_cast<quint64>(plugin) };
            auto hookPrivate = new DFMExtFileOperationHookPrivate;
            DFMExtFileOperationHook *hook = new DFMExtFileOperationHook(hookPrivate);
            plugin->fileOperationHooks(hook);
            container.insert(pluginAddr, { hook, hookPrivate });
        });

        if (container.isEmpty())
            return;

        // hook open file
        AppLaunchUtils::instance().addStrategy([this](const QString &desktopFile, const QStringList &filePaths) {
            return launch(desktopFile, filePaths);
        },
                                               0);
    });
}

bool ExtensionFileManager::launch(const QString &desktopFile, const QStringList &filePaths)
{
    Q_UNUSED(desktopFile);

    auto values = container.values();

    for (auto pair : values) {
        if (!pair.second->openFilesFunc)
            continue;

        const auto &srcPaths = convertQStringListToVector(filePaths);
        std::vector<std::string> ignorePaths;

        // 扩展接口不处理相关文件，则跳出使用下一个打开策略
        if (!pair.second->openFilesFunc(srcPaths, &ignorePaths))
            continue;

        if (!ignorePaths.empty()) {
            auto reOpenFiles = convertVectorToQStringList(ignorePaths);
            AppLaunchUtils::instance().defaultLaunchApp(desktopFile, reOpenFiles);
        }

        return true;
    }

    return false;
}

ExtensionFileManager::ExtensionFileManager(QObject *parent)
    : QObject { parent }
{
}

DPUTILS_END_NAMESPACE
