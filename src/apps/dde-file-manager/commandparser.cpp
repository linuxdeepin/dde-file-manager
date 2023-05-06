// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "commandparser.h"

#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/standardpaths.h>
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>

#include <dfm-framework/event/event.h>

#include <QCoreApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QDebug>

DFMBASE_USE_NAMESPACE

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
        bool enableHeadless { DConfigManager::instance()->value(kDefaultCfgPath, "dfm.headless", false).toBool() };
        if (enableHeadless) {
            qInfo() << "Start headless";
            // whether to add setQuitOnLastWindowClosed
            return;
        }
        qWarning() << "Cannot start dde-file-manager in no window mode "
                      "unless you can set the value of `dfm.headless` in DConfig to `true`!";
        ::exit(0);
    }

    if (isSet("e")) {
        // TODO: event from json handle, the old filemanager does not seem to be used
        qWarning() << "Cannot supported now";
        exit(0);
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

    openInUrls();
}

void CommandParser::process()
{
    return process(qApp->arguments());
}

void CommandParser::process(const QStringList &arguments)
{
    qDebug() << "App start args: " << arguments;
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
    dpfSlotChannel->push("dfmplugin_utils", "slot_OpenWith_ShowDialog", urlList);
}

void CommandParser::openInHomeDirectory()
{
    QString homePath = StandardPaths::location(StandardPaths::StandardLocation::kHomePath);
    QUrl url = QUrl::fromUserInput(homePath);
    dpfSignalDispatcher->publish(GlobalEventType::kOpenNewWindow, url);
}

void CommandParser::openInUrls()
{
    QList<QUrl> argumentUrls;

    for (QString path : positionalArguments()) {
        if (!isSet("raw")) {
            //路径字符串在DUrl::fromUserInput中会处理编码，这里不处理
            if (!QDir().exists(path) && !path.startsWith("./") && !path.startsWith("../") && !path.startsWith("/")) {
                // 路径中包含特殊字符的全部uri编码
                QRegExp regexp("[#&@\\!\\?]");
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
    if (argumentUrls.isEmpty())
        dpfSignalDispatcher->publish(GlobalEventType::kOpenNewWindow, QUrl());
    for (const QUrl &url : argumentUrls)
        openWindowWithUrl(url);
}

void CommandParser::openWindowWithUrl(const QUrl &url)
{
    // Some args must require the plugin to be started in advance
    static const QMap<QString, QString> kSchemeMap {
        { Global::Scheme::kSmb, "dfmplugin-smbbrowser" },
        { Global::Scheme::kTrash, "dfmplugin-trash" }
    };
    if (Q_UNLIKELY(kSchemeMap.keys().contains(url.scheme()))) {
        static std::once_flag flag;
        std::call_once(flag, [url]() {
            const QString &name { kSchemeMap.value(url.scheme()) };
            dpfSignalDispatcher->publish(GlobalEventType::kLoadPlugins, QStringList() << name);
        });
    }
    dpfSignalDispatcher->publish(GlobalEventType::kOpenNewWindow, url, isSet("n"));
}

CommandParser::CommandParser(QObject *parent)
    : QObject(parent),
      commandParser(new QCommandLineParser)
{
    initialize();
}
