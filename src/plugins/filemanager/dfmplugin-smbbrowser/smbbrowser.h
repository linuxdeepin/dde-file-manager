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
#ifndef SMBBROWSER_H
#define SMBBROWSER_H

#include "dfmplugin_smbbrowser_global.h"

#include <dfm-framework/dpf.h>

namespace dfmplugin_smbbrowser {

using Prehandler = std::function<void(quint64 winId, const QUrl &url, std::function<void()> after)>;

class SmbBrowser : public dpf::Plugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.deepin.plugin.filemanager" FILE "smbbrowser.json")

public:
    virtual void initialize() override;
    virtual bool start() override;

    static void contenxtMenuHandle(quint64 windowId, const QUrl &url, const QPoint &globalPos);

protected Q_SLOTS:
    void onWindowOpened(quint64 windd);

private:
    void addNeighborToSidebar();
    void registerNetworkAccessPrehandler();
    static void networkAccessPrehandler(quint64 winId, const QUrl &url, std::function<void()> after);
};

}

Q_DECLARE_METATYPE(dfmplugin_smbbrowser::Prehandler)

#endif   // SMBBROWSER_H
