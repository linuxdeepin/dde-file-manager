// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "commandparser.h"
#include "private/commandparser_p.h"
#include "sessionloader.h"

#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/standardpaths.h>
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>
#include <dfm-base/interfaces/abstractjobhandler.h>
#include <dfm-base/utils/systempathutil.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/widgets/filemanagerwindowsmanager.h>
#include <dfm-base/base/application/application.h>

#include <dfm-framework/event/event.h>
#include <dfm-framework/lifecycle/lifecycle.h>

#include <QCoreApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QRegularExpression>
#include <QDebug>

Q_DECLARE_LOGGING_CATEGORY(logAppFileManager)

DFMBASE_USE_NAMESPACE
using namespace GlobalDConfDefines::ConfigPath;
using namespace GlobalDConfDefines::BaseConfig;

CommandParserPrivate::CommandParserPrivate()
{
}

CommandParserPrivate::~CommandParserPrivate()
{
}

CommandParserPrivate::EventArgsInfo CommandParserPrivate::parseEventArgs(const QByteArray &eventArg)
{
    QJsonDocument doc = QJsonDocument::fromJson(eventArg);
    if (doc.isNull())
        return {};

    EventArgsInfo argsInfo;
    const auto &obj = doc.object();
    if (obj.contains("action"))
        argsInfo.action = obj["action"].toString();

    if (obj.contains("params")) {
        const auto &paramsObj = obj["params"].toObject();
        for (auto iter = paramsObj.constBegin(); iter != paramsObj.end(); ++iter) {
            const auto &key = iter.key();
            const auto &value = iter.value();

            if (key == "sources") {
                const auto &srcList = value.toArray().toVariantList();
                argsInfo.params.insert("sources", srcList);
            }

            if (key == "target")
                argsInfo.params.insert("target", value.toString());
        }
    }

    return argsInfo;
}

CommandParser &CommandParser::instance()
{
    static CommandParser ins;
    return ins;
}

void CommandParser::bindEvents()
{
    dpfSignalDispatcher->subscribe("dfmplugin_core", "signal_StartApp", this, &CommandParser::processCommand);
}

bool CommandParser::isSet(const QString &name) const
{
    return commandParser->isSet(name);
}

QString CommandParser::value(const QString &name) const
{
    return commandParser->value(name);
}

void CommandParser::processCommand()
{
    if (isSet("d")) {
        qCInfo(logAppFileManager) << "Starting file manager in headless mode";
        dpfSignalDispatcher->publish(GlobalEventType::kHeadlessStarted);
        // whether to add setQuitOnLastWindowClosed
        return;
    }

    if (isSet("e")) {
        qCDebug(logAppFileManager) << "Processing event command";
        processEvent();
        return;
    }

    if (isSet("p")) {
        qCDebug(logAppFileManager) << "Showing property dialog";
        showPropertyDialog();
        return;
    }
    if (isSet("o")) {
        qCDebug(logAppFileManager) << "Opening with dialog";
        openWithDialog();
        return;
    }
    if (isSet("O")) {
        qCDebug(logAppFileManager) << "Opening home directory";
        openInHomeDirectory();
        return;
    }
    if (isSet("s") || isSet("sessionfile")) {
        qCDebug(logAppFileManager) << "Opening session file";
        openSession();
        return;
    }

    qCDebug(logAppFileManager) << "Opening URLs from command line arguments";
    openInUrls();
}

void CommandParser::process()
{
    return process(qApp->arguments());
}

void CommandParser::process(const QStringList &arguments)
{
    qCInfo(logAppFileManager) << "Processing command line arguments:" << arguments.size() << "args";
    // 直接使用参数，不再进行 percent 解码，因为 FileManager1DBus 已经处理了 Base64 解码
    SessionBusiness::instance()->process(arguments);
    commandParser->process(arguments);
}

void CommandParser::initialize()
{
    commandParser->setApplicationDescription(QString("%1 helper").arg(QCoreApplication::applicationName()));
    initOptions();
    commandParser->addHelpOption();
    commandParser->addVersionOption();
}

void CommandParser::initOptions()
{
    QCommandLineOption newWindowOption(QStringList() << "n"
                                                     << "new-window",
                                       "show new window");
    QCommandLineOption backendOption(QStringList() << "d"
                                                   << "none-window-process",
                                     "start dde-file-manager in no window mode");
    QCommandLineOption openPropertyDialogOption(QStringList() << "p"
                                                              << "property",
                                                "show property dialog");
    QCommandLineOption rootOption(QStringList() << "r"
                                                << "root",
                                  "exec dde-file-manager in root mode");
    QCommandLineOption showFileItem(QStringList() << "show-item", "show a file item in a new window");
    QCommandLineOption raw(QStringList() << "R"
                                         << "raw",
                           "process file item url as raw QUrl，will not encode special characters(e.g. '#' '&' '@' '!' '?')");
    QCommandLineOption event(QStringList() << "e"
                                           << "event",
                             "process the event by json data");

    QCommandLineOption getMonitorFiles(QStringList() << "get-monitor-files", "Get all the files that have been monitored");
    // blumia: about -w and -r: -r will exec `dde-file-manager-pkexec` (it use `pkexec` command) which won't pass the currect
    //         working dir, so we need to manually set the working dir via -w. that's why we add a -w arg.
    QCommandLineOption workingDirOption(QStringList() << "w"
                                                      << "working-dir",
                                        "set the file manager working directory (won't work with -r argument)",
                                        "directory");
    QCommandLineOption openWithDialog(QStringList() << "o"
                                                    << "open",
                                      "open with dialog");
    QCommandLineOption openHomeOption(QStringList() << "O"
                                                    << "open-home",
                                      "open home");
    QCommandLineOption sessionOption(QStringList() << "s"
                                                   << "sessionfile",
                                     "support session loader");

    addOption(newWindowOption);
    addOption(backendOption);
    addOption(openPropertyDialogOption);
    addOption(rootOption);
    addOption(showFileItem);
    addOption(raw);
    addOption(event);
    addOption(getMonitorFiles);
    addOption(workingDirOption);
    addOption(openWithDialog);
    addOption(openHomeOption);
    addOption(sessionOption);
}

void CommandParser::addOption(const QCommandLineOption &option)
{
    commandParser->addOption(option);
}

QStringList CommandParser::positionalArguments() const
{
    return commandParser->positionalArguments();
}

QStringList CommandParser::unknownOptionNames() const
{
    return commandParser->unknownOptionNames();
}

void CommandParser::showPropertyDialog()
{
    const QStringList &paths = positionalArguments();
    QList<QUrl> urlList;
    for (const QString &path : paths) {
        QUrl url = QUrl::fromUserInput(path);
        if (url.isLocalFile()) {
            const FileInfoPointer &fileInfo = InfoFactory::create<FileInfo>(url);
            if (!fileInfo || !fileInfo->exists()) {
                qCWarning(logAppFileManager) << "Failed to show property dialog: invalid path or file not exists -" << path;
                continue;
            }
        }

        QString uPath = url.path();
        if (uPath.endsWith(QDir::separator()) && uPath.size() > 1)
            uPath.chop(1);
        url.setPath(uPath);

        // Todo(yanghao): symlink , desktop files filters
        if (urlList.contains(url))
            continue;
        urlList << url;
    }
    if (urlList.isEmpty()) {
        qCWarning(logAppFileManager) << "No valid files found for property dialog";
        return;
    }

    qCInfo(logAppFileManager) << "Showing property dialog for" << urlList.size() << "files";
    dpfSlotChannel->push("dfmplugin_propertydialog", "slot_PropertyDialog_Show", urlList, QVariantHash());
}

void CommandParser::openWithDialog()
{
    const QStringList &files = positionalArguments();

    QList<QUrl> urlList;
    for (const QString &path : files) {
        QUrl url = QUrl::fromUserInput(path);
        QString uPath = url.path();
        if (uPath.endsWith(QDir::separator()) && uPath.size() > 1)
            uPath.chop(1);
        url.setPath(uPath);

        // Todo(yanghao): symlink , desktop files filters
        if (urlList.contains(url))
            continue;
        urlList << url;
    }
    if (urlList.isEmpty()) {
        qCWarning(logAppFileManager) << "No valid files found for open with dialog";
        return;
    }

    qCInfo(logAppFileManager) << "Opening 'Open With' dialog for" << urlList.size() << "files";
    dpfSlotChannel->push("dfmplugin_utils", "slot_OpenWith_ShowDialog", 0, urlList);
}

void CommandParser::openInHomeDirectory()
{
    QString homePath = StandardPaths::location(StandardPaths::StandardLocation::kHomePath);
    QUrl url = QUrl::fromUserInput(homePath);
    auto flag = DConfigManager::instance()->value(kViewDConfName,
                                                  kOpenFolderWindowsInASeparateProcess, true)
                        .toBool();
    qCInfo(logAppFileManager) << "Opening home directory:" << homePath << "in separate process:" << flag;
    dpfSignalDispatcher->publish(GlobalEventType::kOpenNewWindow, url, flag);
}

void CommandParser::openInUrls()
{
    QList<QUrl> argumentUrls;
    for (QString path : positionalArguments()) {
        if (!isSet("raw")) {
            // 路径字符串在DUrl::fromUserInput中会处理编码，这里不处理
            if (!QDir().exists(path) && !path.startsWith("./") && !path.startsWith("../") && !path.startsWith("/")) {
                // 路径中包含特殊字符的全部uri编码
                QRegularExpression regexp("[#&@\\!\\?]");
                if (path.contains(regexp)) {
                    QString left, right, encode;
                    int idx = path.indexOf(regexp);
                    while (idx != -1) {
                        left = path.left(idx);
                        right = path.mid(idx + 1);
                        encode = QUrl::toPercentEncoding(path.mid(idx, 1));
                        path = left + encode + right;
                        idx = path.indexOf(regexp);
                    }
                }
            }
        }

        QUrl url = UrlRoute::fromUserInput(path);

        if (isSet("show-item")) {
            const FileInfoPointer &fileInfo = InfoFactory::create<FileInfo>(url);
            if (!fileInfo)
                continue;

            QUrl parentUrl = fileInfo->urlOf(UrlInfoType::kParentUrl);
            parentUrl.setQuery("selectUrl=" + url.toString());
            url = parentUrl;
        }
        argumentUrls.append(url);
    }
    if (argumentUrls.isEmpty()) {
        auto flag = DConfigManager::instance()->value(kViewDConfName,
                                                      kOpenFolderWindowsInASeparateProcess, true)
                            .toBool();
        dpfSignalDispatcher->publish(GlobalEventType::kOpenNewWindow, QUrl(), flag);
    }
    for (const QUrl &url : argumentUrls)
        openWindowWithUrl(url);
}

void CommandParser::openWindowWithUrl(const QUrl &url)
{
    // Some args must require the plugin to be started in advance
    QMap<QString, QString> schemeMap {};
    QMap<QString, dpf::PluginMetaObjectPointer> schemePlugins;
    dpf::LifeCycle::pluginMetaObjs([&schemeMap, &schemePlugins](dpf::PluginMetaObjectPointer ptr) {
        Q_ASSERT(ptr);
        const auto &data { ptr->customData() };
        if (data.isEmpty())
            return false;
        auto schemes = ptr->customData().value("PluginSchemes").toStringList();
        if (schemes.isEmpty())
            return false;
        for (auto scheme : schemes) {
            schemeMap.insert(scheme, ptr->name());
            schemePlugins.insert(scheme, ptr);
        }
        return true;
    });

    if (Q_UNLIKELY(schemeMap.keys().contains(url.scheme())
                   && schemePlugins.value(url.scheme())->pluginState() != dpf::PluginMetaObject::State::kLoaded)) {
        static std::once_flag flag;
        std::call_once(flag, [url, schemeMap]() {
            const QString &name { schemeMap.value(url.scheme()) };
            dpfSignalDispatcher->publish(GlobalEventType::kLoadPlugins, QStringList() << name);
        });
    }

    auto flag = !DConfigManager::instance()->value(kViewDConfName, kOpenFolderWindowsInASeparateProcess, true).toBool();
    auto actWinId = FMWindowsIns.lastActivedWindowId();
    if (Application::appAttribute(Application::kOpenInNewTab).toBool() && actWinId > 0) {
        auto activeWindow = [](FileManagerWindow *w) {
            if (w->isMinimized())
                w->setWindowState(w->windowState() & ~Qt::WindowMinimized);
            w->activateWindow();
        };

        if (flag) {
            auto window = FMWindowsIns.createWindow(url, false);
            if (window) {
                activeWindow(window);
                return;
            }
        }

        dpfSignalDispatcher->publish(GlobalEventType::kOpenNewTab, actWinId, url);
        auto window = FMWindowsIns.findWindowById(actWinId);
        if (window)
            activeWindow(window);
    } else {
        flag = flag ? false : isSet("n") || isSet("s") || isSet("sessionfile") || isSet("show-item");
        dpfSignalDispatcher->publish(GlobalEventType::kOpenNewWindow, url, flag);
    }
}

void CommandParser::openSession()
{
    for (QString filename : positionalArguments()) {
        QString sessionFile = "";
        if (!SessionBusiness::instance()->readPath(filename, &sessionFile)) {
            qCWarning(logAppFileManager) << "Failed to read session path from file:" << filename;
        } else {
            qCInfo(logAppFileManager) << "Opening session file:" << sessionFile;
            openWindowWithUrl(QUrl(sessionFile));
        }
        break;   // current time only support one file.
    }
}

void CommandParser::processEvent()
{
    const QStringList &argumets = positionalArguments();
    if (argumets.isEmpty()) {
        qCWarning(logAppFileManager) << "No event arguments provided";
        return;
    }

    const auto &&argsInfo = d->parseEventArgs(argumets.first().toLocal8Bit());
    const auto &srcUrls = UrlRoute::fromStringList(argsInfo.params.value("sources").toStringList());

    qCDebug(logAppFileManager) << "Processing event:" << argsInfo.action << "with" << srcUrls.size() << "source URLs";

    if (argsInfo.action == "refresh") {
        qCInfo(logAppFileManager) << "Refreshing directories for" << srcUrls.size() << "URLs";
        dpfSlotChannel->push("dfmplugin_workspace", "slot_RefreshDir", srcUrls);
        return;
    }

    static QMap<QString, GlobalEventType> eventMap {
        { "copy", GlobalEventType::kCopy },
        { "move", GlobalEventType::kCutFile },
        { "delete", GlobalEventType::kDeleteFiles },
        { "trash", GlobalEventType::kMoveToTrash },
        { "cleantrash", GlobalEventType::kCleanTrash },
    };

    if (!eventMap.contains(argsInfo.action)) {
        qCWarning(logAppFileManager) << "Unknown event action:" << argsInfo.action;
        return;
    }

    switch (eventMap[argsInfo.action]) {
    case GlobalEventType::kCopy: {
        const auto &targetUrl = UrlRoute::fromUserInput(argsInfo.params.value("target").toString());
        qCInfo(logAppFileManager) << "Copying" << srcUrls.size() << "files to:" << targetUrl.toString();
        dpfSignalDispatcher->publish(GlobalEventType::kCopy, 0, srcUrls, targetUrl,
                                     AbstractJobHandler::JobFlag::kNoHint, nullptr);
        break;
    }
    case GlobalEventType::kCutFile: {
        if (SystemPathUtil::instance()->checkContainsSystemPath(srcUrls)) {
            qCWarning(logAppFileManager) << "Cannot move system files";
            return;
        }
        const auto &targetUrl = UrlRoute::fromUserInput(argsInfo.params.value("target").toString());
        qCInfo(logAppFileManager) << "Moving" << srcUrls.size() << "files to:" << targetUrl.toString();
        dpfSignalDispatcher->publish(GlobalEventType::kCutFile, 0, srcUrls, targetUrl,
                                     AbstractJobHandler::JobFlag::kNoHint, nullptr);
        break;
    }
    case GlobalEventType::kDeleteFiles: {
        if (SystemPathUtil::instance()->checkContainsSystemPath(srcUrls)) {
            qCWarning(logAppFileManager) << "Cannot delete system files";
            return;
        }
        qCInfo(logAppFileManager) << "Deleting" << srcUrls.size() << "files";
        dpfSignalDispatcher->publish(GlobalEventType::kDeleteFiles, 0, srcUrls, AbstractJobHandler::JobFlag::kNoHint, nullptr);
        break;
    }
    case GlobalEventType::kMoveToTrash: {
        if (SystemPathUtil::instance()->checkContainsSystemPath(srcUrls)
            || FileUtils::isTrashFile(srcUrls.first())) {
            qCWarning(logAppFileManager) << "Cannot move system files or trash files to trash";
            return;
        }
        qCInfo(logAppFileManager) << "Moving" << srcUrls.size() << "files to trash";
        dpfSignalDispatcher->publish(GlobalEventType::kMoveToTrash, 0, srcUrls, AbstractJobHandler::JobFlag::kNoHint, nullptr);
        break;
    }
    case GlobalEventType::kCleanTrash: {
        qCInfo(logAppFileManager) << "Cleaning trash";
        dpfSignalDispatcher->publish(GlobalEventType::kCleanTrash,
                                     0,
                                     QList<QUrl>(),
                                     AbstractJobHandler::DeleteDialogNoticeType::kEmptyTrash, nullptr);
        break;
    }
    default:
        qCWarning(logAppFileManager) << "Unhandled event type for action:" << argsInfo.action;
        break;
    }
}

CommandParser::CommandParser(QObject *parent)
    : QObject(parent),
      commandParser(new QCommandLineParser),
      d(new CommandParserPrivate)
{
    initialize();
}

CommandParser::~CommandParser()
{
}
