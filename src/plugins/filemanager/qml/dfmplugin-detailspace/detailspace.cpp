// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "detailspace.h"
#include "utils/detailspacehelper.h"
#include "events/detailspaceeventreceiver.h"
#include "detailspacecontainment.h"
#include "model/quickfilebaseinfomodel.h"

#include <dfm-base/widgets/filemanagerwindowsmanager.h>

#include <dfm-gui/windowmanager.h>
#include <dfm-gui/appletfactory.h>

#include <QQmlEngine>

namespace dfmplugin_detailspace {
DFM_LOG_REISGER_CATEGORY(DPDETAILSPACE_NAMESPACE)
DFMBASE_USE_NAMESPACE

static constexpr char kAppletUrl[] { "org.deepin.filemanager.detailspace" };

static dfmgui::Applet *createDetailSpaceApplet(const QString &id, dfmgui::Containment *parent, QString *errorString)
{
    if (kAppletUrl == id) {
        Q_ASSERT_X(parent && parent->flags().testFlag(dfmgui::Applet::kPanel),
                   "Create detailspace applet", "Parent must based on panel");

        auto detailSpace = new DetailSpaceContainment(parent);
        // 追加到缓存记录
        DetailSpaceHelper::addDetailSpace(detailSpace);
        return detailSpace;
    }
    return nullptr;
}

void DetailSpace::initialize()
{
    connect(&FMWindowsIns, &FileManagerWindowsManager::windowClosed, this, &DetailSpace::onWindowClosed, Qt::DirectConnection);
    DetailSpaceEventReceiver::instance().connectService();

    // 注册插件使用模块组件
    // @uri org.deepin.filemanager.detailspace
    const char *uri = "org.deepin.filemanager.detailspace";
    qmlRegisterType<QuickFileBaseInfoModel>(uri, 1, 0, "BaseFileInfoModel");

    QString errorString;
    bool ret = dfmgui::AppletFactory::instance()->regCreator(
            kAppletUrl, &createDetailSpaceApplet, &errorString);
    if (!ret) {
        fmWarning() << QString("Register applet %1 failed.").arg(kAppletUrl) << errorString;
    }
}

bool DetailSpace::start()
{
    return true;
}

void DetailSpace::onWindowClosed(quint64 windId)
{
    DetailSpaceHelper::removeDetailSpace(windId);
}

}   // namespace dfmplugin_detailspace
