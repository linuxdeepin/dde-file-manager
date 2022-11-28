/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuyangming<liuyangming@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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
#ifndef TAG_H
#define TAG_H

#include "dfmplugin_tag_global.h"

#include <dfm-framework/dpf.h>
class TagDBus;
class QDBusConnection;
namespace dfmplugin_tag {

class Tag : public dpf::Plugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.deepin.plugin.common" FILE "tag.json")

    DPF_EVENT_NAMESPACE(DPTAG_NAMESPACE)
    // slot events
    DPF_EVENT_REG_SLOT(slot_GetTags)

    // hook events
    DPF_EVENT_REG_HOOK(hook_CanTag)

public:
    virtual void initialize() override;
    virtual bool start() override;

private slots:
    void onWindowOpened(quint64 windId);
    void regTagCrumbToTitleBar();
    void installToSideBar();
    void onAllPluginsInitialized();

private:
    static QWidget *createTagWidget(const QUrl &url);
    void followEvents();
    void bindScene(const QString &parentScene);
    void onMenuSceneAdded(const QString &scene);
    void bindEvents();
    void initDbus();
    void initServiceDBusInterfaces(QDBusConnection *connection);

    QSet<QString> menuScenes;
    bool subscribedEvent { false };
    QScopedPointer<TagDBus> tagDBus;
};

}

#endif   // TAG_H
