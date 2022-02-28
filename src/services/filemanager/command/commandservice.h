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
#ifndef COMMADNSERVICE_H
#define COMMADNSERVICE_H

#include "dfm_filemanager_service_global.h"

#include <dfm-framework/service/pluginservicecontext.h>

#include <QObject>

QT_BEGIN_NAMESPACE
class QCommandLineParser;
class QCoreApplication;
class QCommandLineOption;
QT_END_NAMESPACE

DSB_FM_BEGIN_NAMESPACE

class CommandService final : public dpf::PluginService, dpf::AutoServiceRegister<CommandService>
{
    Q_OBJECT
    Q_DISABLE_COPY(CommandService)
    friend class dpf::QtClassFactory<dpf::PluginService>;

public:
    static QString name()
    {
        return "org.deepin.service.CommandService";
    }

    explicit CommandService(QObject *parent = nullptr);
    virtual ~CommandService() override;
    static CommandService *instance();

    void process();
    bool isSet(const QString &name) const;
    QString value(const QString &name) const;
    void processCommand();

private:
    void init();
    void initOptions();
    void addOption(const QCommandLineOption &option);
    void process(const QStringList &arguments);
    QStringList positionalArguments() const;
    QStringList unknownOptionNames() const;
    void showPropertyDialog();
    void openWithDialog();
    void openInHomeDirectory();
    void openInUrls();

private:
    QCommandLineParser *commandParser;
};

DSB_FM_END_NAMESPACE

#define commandServIns ::dfm_service_filemanager::CommandService::instance()
#endif   // COMMADNSERVICE_H
