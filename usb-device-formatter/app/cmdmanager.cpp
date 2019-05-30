/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
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

#include "cmdmanager.h"
#include <QStringList>

CMDManager *CMDManager::instance()
{
    static CMDManager * instance;
    if(!instance)
        instance = new CMDManager(0);

    return instance;
}

void CMDManager::process(const QApplication &app)
{
    init();
    m_parser.process(app);
}

void CMDManager::init()
{
    m_parser.addOption(m_modelModeOpt);
    m_parser.setApplicationDescription("Usb Device Formatter");
    m_parser.addPositionalArgument("device-path", "The external device path to format. (required)");
    m_parser.addHelpOption();
    m_parser.addVersionOption();
}

bool CMDManager::isSet(const QString &name) const
{
    return m_parser.isSet(name);
}

QString CMDManager::getPath()
{
    QStringList positionalArguments = m_parser.positionalArguments();
    if (positionalArguments.count() > 0) {
        return m_parser.positionalArguments().at(0);
    }

    return QString();
}

QStringList CMDManager::positionalArguments() const
{
    return m_parser.positionalArguments();
}

void CMDManager::showHelp(int exitCode)
{
    return m_parser.showHelp(exitCode);
}

int CMDManager::getWinId()
{
    QString winId = m_parser.value(m_modelModeOpt);
    if(winId.isEmpty())
        return -1;
    return winId.toInt();
}

CMDManager::CMDManager(QObject *parent) :
    QObject(parent),
    m_modelModeOpt(QStringList() << "m" << "model-mode",
                   "Enable model mode.",
                   "window ID")
{
}
