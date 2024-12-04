// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
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
        qCInfo(logAppFileManager) << "Start headless";
        dpfSignalDispatcher->publish(GlobalEventType::kHeadlessStarted);
        // whether to add setQuitOnLastWindowClosed
        return;
    }

    if (isSet("e")) {
        processEvent();
        return;
    }

    if (isSet("p")) {
        showPropertyDialog();
        return;
    }
    if (isSet("o")) {
        openWithDialog();
        return;
    }
    if (isSet("O")) {
        openInHomeDirectory();
        return;
    }
    if (isSet("s") || isSet("sessionfile")) {
        openSession();
        return;
    }

    openInUrls();
}

void CommandParser::process()
{
    return process(qApp->arguments());
}

void CommandParser::process(const QStringList &arguments)
{
    qCInfo(logAppFileManager) << "App start args: " << arguments;
    QStringList args;
    for (const auto &arg : arguments) {
        if (!arg.startsWith("-")) {
            args.append(QUrl::fromPercentEncoding(arg.toStdString().c_str()));
        } else {
            args.append(arg);
        }
    }
    SessionBusiness::instance()->process(args);
    commandParser->process(args);
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
        const FileInfoPointer &fileInfo = InfoFactory::create<FileInfo>(url);
        if (!fileInfo || !fileInfo->exists()) {
            qCWarning(logAppFileManager) << "Input path is invalid or file not exists, can not show property dialog! path: " << path;
            continue;
        }

        QString uPath = url.path();
        if (uPath.endsWith(QDir::separator()) && uPath.size() > 1)
            uPath.chop(1);
        url.setPath(uPath);

        //Todo(yanghao): symlink , desktop files filters
        if (urlList.contains(url))
            continue;
        urlList << url;
    }
    if (urlList.isEmpty())
        return;

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

        //Todo(yanghao): symlink , desktop files filters
        if (urlList.contains(url))
            continue;
        urlList << url;
    }
    if (urlList.isEmpty())
        return;
    dpfSlotChannel->push("dfmplugin_utils", "slot_OpenWith_ShowDialog", 0, urlList);
}

void CommandParser::openInHomeDirectory()
{
    QString homePath = StandardPaths::location(StandardPaths::StandardLocation::kHomePath);
    QUrl url = QUrl::fromUserInput(homePath);
    auto flag = DConfigManager::instance()->value(kViewDConfName,
                                                  kOpenFolderWindowsInASeparateProcess, true)
                        .toBool();
    dpfSignalDispatcher->publish(GlobalEventType::kOpenNewWindow, url, flag);
}

void CommandParser::openInUrls()
{
    QList<QUrl> argumentUrls;
    for (QString path : positionalArguments()) {
        if (!isSet("raw")) {
            //路径字符串在DUrl::fromUserInput中会处理编码，这里不处理
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
    auto flag = !DConfigManager::instance()->value(kViewDConfName,
                                                   kOpenFolderWindowsInASeparateProcess, true)
                         .toBool();
    flag = flag ? false : isSet("n") || isSet("s") || isSet("sessionfile") || isSet("show-item");
    dpfSignalDispatcher->publish(GlobalEventType::kOpenNewWindow, url, flag);
}

void CommandParser::openSession()
{
    for (QString filename : positionalArguments()) {
        QString sessionFile = "";
        if (!SessionBusiness::instance()->readPath(filename, &sessionFile))
            qCWarning(logAppFileManager) << "no session path get";
        openWindowWithUrl(QUrl(sessionFile));
        break;   // current time only support one file.
    }
}

void CommandParser::processEvent()
{
    const QStringList &argumets = positionalArguments();
    if (argumets.isEmpty())
        return;

    const auto &&argsInfo = d->parseEventArgs(argumets.first().toLocal8Bit());
    const auto &srcUrls = UrlRoute::fromStringList(argsInfo.params.value("sources").toStringList());

    if (argsInfo.action == "refresh") {
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

    if (!eventMap.contains(argsInfo.action))
        return;

    switch (eventMap[argsInfo.action]) {
    case GlobalEventType::kCopy: {
        const auto &targetUrl = UrlRoute::fromUserInput(argsInfo.params.value("target").toString());
        dpfSignalDispatcher->publish(GlobalEventType::kCopy, 0, srcUrls, targetUrl,
                                     AbstractJobHandler::JobFlag::kNoHint, nullptr);
        break;
    }
    case GlobalEventType::kCutFile: {
        if (SystemPathUtil::instance()->checkContainsSystemPath(srcUrls))
            return;
        const auto &targetUrl = UrlRoute::fromUserInput(argsInfo.params.value("target").toString());
        dpfSignalDispatcher->publish(GlobalEventType::kCutFile, 0, srcUrls, targetUrl,
                                     AbstractJobHandler::JobFlag::kNoHint, nullptr);
        break;
    }
    case GlobalEventType::kDeleteFiles: {
        if (SystemPathUtil::instance()->checkContainsSystemPath(srcUrls))
            return;
        dpfSignalDispatcher->publish(GlobalEventType::kDeleteFiles, 0, srcUrls, AbstractJobHandler::JobFlag::kNoHint, nullptr);
        break;
    }
    case GlobalEventType::kMoveToTrash: {
        if (SystemPathUtil::instance()->checkContainsSystemPath(srcUrls)
            || FileUtils::isTrashFile(srcUrls.first()))
            return;
        dpfSignalDispatcher->publish(GlobalEventType::kMoveToTrash, 0, srcUrls, AbstractJobHandler::JobFlag::kNoHint, nullptr);
        break;
    }
    case GlobalEventType::kCleanTrash: {
        dpfSignalDispatcher->publish(GlobalEventType::kCleanTrash,
                                     0,
                                     QList<QUrl>(),
                                     AbstractJobHandler::DeleteDialogNoticeType::kEmptyTrash, nullptr);
        break;
    }
    default:
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
