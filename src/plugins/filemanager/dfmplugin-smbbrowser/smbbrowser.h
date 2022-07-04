/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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
#ifndef MYSHARESPLUGIN_H
#define MYSHARESPLUGIN_H

#include "dfmplugin_smbbrowser_global.h"

#include <dfm-framework/dpf.h>

namespace dfmplugin_smbbrowser {

class SmbBrowser : public dpf::Plugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.deepin.plugin.filemanager" FILE "smbbrowser.json")

public:
    virtual void initialize() override;
    virtual bool start() override;
    virtual ShutdownFlag stop() override;

protected Q_SLOTS:
    void onWindowCreated(quint64 winId);
    void onWindowOpened(quint64 windd);

private:
    void addNeighborToSidebar();
    void registerSambaPrehandler();
    static void sambaPrehandler(const QUrl &url, std::function<void()> after);
};

}
#endif   // MYSHARESPLUGIN_H
