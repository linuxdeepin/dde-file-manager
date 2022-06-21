/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: huangyu<huangyub@uniontech.com>
 *             liuyangming<liuyangming@uniontech.com>
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

#include "commandservice.h"

#include <dfm-framework/framework.h>

#include "services/common/propertydialog/property_defines.h"

#include "dfm-base/dfm_event_defines.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/base/standardpaths.h"

#include <QCoreApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QDebug>

DSB_FM_USE_NAMESPACE
DSC_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

CommandService::CommandService(QObject *parent)
    : dpf::PluginService(parent),
      dpf::AutoServiceRegister<CommandService>(),
      commandParser(new QCommandLineParser)
{
    init();
}

CommandService::~CommandService()
{
}

CommandService *CommandService::instance()
{

    auto &ctx = dpfInstance.serviceContext();
    static std::once_flag onceFlag;
    std::call_once(onceFlag, [&ctx]() {
        if (!ctx.load(DSB_FM_NAMESPACE::CommandService::name()))
            abort();
    });

    return ctx.service<DSB_FM_NAMESPACE::CommandService>(DSB_FM_NAMESPACE::CommandService::name());
}

void CommandService::process()
{
    return process(qApp->arguments());
}

bool CommandService::isSet(const QString &name) const
{
    return commandParser->isSet(name);
}

QString CommandService::value(const QString &name) const
{
    return commandParser->value(name);
}

void CommandService::init()
{
    commandParser->setApplicationDescription(QString("%1 helper").arg(QCoreApplication::applicationName()));
    initOptions();
    commandParser->addHelpOption();
    commandParser->addVersionOption();
}

void CommandService::processCommand()
{
    // whether to add setQuitOnLastWindowClosed
    if (isSet("d"))
        return;

    if (isSet("e")) {
        // Todo(yanghao): event from json handle, the old filemanager does not seem to be used
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

    openInUrls();
}

void CommandService::initOptions()
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

void CommandService::addOption(const QCommandLineOption &option)
{
    commandParser->addOption(option);
}

void CommandService::process(const QStringList &arguments)
{
    commandParser->process(arguments);
}

QStringList CommandService::positionalArguments() const
{
    return commandParser->positionalArguments();
}

QStringList CommandService::unknownOptionNames() const
{
    return commandParser->unknownOptionNames();
}

void CommandService::showPropertyDialog()
{
    QStringList paths = positionalArguments();
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

    dpfSignalDispatcher->publish(DSC_NAMESPACE::Property::EventType::kEvokePropertyDialog, urlList);
}

void CommandService::openWithDialog()
{
    QStringList files = positionalArguments();

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
    dpfSlotChannel->push("dfmplugin_utils", "slot_ShowOpenWithDialog", urlList);
}

void CommandService::openInHomeDirectory()
{
    QString homePath = StandardPaths::location(StandardPaths::StandardLocation::kHomePath);
    QUrl url = QUrl::fromUserInput(homePath);
    dpfSignalDispatcher->publish(GlobalEventType::kOpenNewWindow, url);
    // Todo(yanghao):isSet("n")
}

void CommandService::openInUrls()
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

        //Todo(yanghao): 路径转换（回收站、保险箱）
        if (isSet("show-item")) {
            const AbstractFileInfoPointer &fileInfo = InfoFactory::create<AbstractFileInfo>(url);
            if (!fileInfo)
                continue;

            QUrl parentUrl = fileInfo->parentUrl();
            parentUrl.setQuery("selectUrl=" + url.toString());
            url = parentUrl;
        }
        argumentUrls.append(url);
    }
    if (argumentUrls.isEmpty())
        dpfSignalDispatcher->publish(GlobalEventType::kOpenNewWindow, QUrl());
    for (const QUrl &url : argumentUrls)
        dpfSignalDispatcher->publish(GlobalEventType::kOpenNewWindow, url);
}
