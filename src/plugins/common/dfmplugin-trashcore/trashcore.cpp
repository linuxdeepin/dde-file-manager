// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "trashcore.h"
#include "trashfileinfo.h"
#include "utils/trashcorehelper.h"
#include "events/trashcoreeventreceiver.h"
#include "events/trashcoreeventsender.h"

#include <dfm-base/base/urlroute.h>
#include <dfm-base/base/schemefactory.h>

using CustomViewExtensionView = std::function<QWidget *(const QUrl &url)>;
Q_DECLARE_METATYPE(CustomViewExtensionView)

namespace dfmplugin_trashcore {
DFM_LOG_REGISTER_CATEGORY(DPTRASHCORE_NAMESPACE)

void TrashCore::initialize()
{
    DFMBASE_NAMESPACE::UrlRoute::regScheme(TrashCoreHelper::scheme(), "/", TrashCoreHelper::icon(), true, tr("Trash"));
    DFMBASE_NAMESPACE::InfoFactory::regClass<TrashFileInfo>(TrashCoreHelper::scheme(), DFMBASE_NAMESPACE::InfoFactory::kNoCache);

    TrashCoreEventSender::instance();

    dpfSlotChannel->connect(DPF_MACRO_TO_STR(DPTRASHCORE_NAMESPACE), "slot_TrashCore_EmptyTrash",
                            TrashCoreEventReceiver::instance(), &TrashCoreEventReceiver::handleEmptyTrash);

    followEvents();
}

bool TrashCore::start()
{
    auto propertyDialogPlugin { DPF_NAMESPACE::LifeCycle::pluginMetaObj("dfmplugin-propertydialog") };
    if (propertyDialogPlugin && (propertyDialogPlugin->pluginState() == DPF_NAMESPACE::PluginMetaObject::kInitialized || propertyDialogPlugin->pluginState() == DPF_NAMESPACE::PluginMetaObject::kStarted)) {
        regCustomPropertyDialog();
    } else {
        connect(DPF_NAMESPACE::Listener::instance(), &DPF_NAMESPACE::Listener::pluginInitialized,
                this, [this](const QString &iid, const QString &name) {
                    Q_UNUSED(iid)
                    if (name == "dfmplugin-propertydialog")
                        regCustomPropertyDialog();
                },
                Qt::DirectConnection);
    }

    return true;
}

void TrashCore::followEvents()
{
    dpfHookSequence->follow("dfmplugin_fileoperations", "hook_Operation_CopyFromFile", TrashCoreEventReceiver::instance(), &TrashCoreEventReceiver::copyFromFile);
    dpfHookSequence->follow("dfmplugin_fileoperations", "hook_Operation_CutFromFile", TrashCoreEventReceiver::instance(), &TrashCoreEventReceiver::cutFileFromTrash);
}

void TrashCore::regCustomPropertyDialog()
{
    CustomViewExtensionView func { TrashCoreHelper::createTrashPropertyDialog };
    dpfSlotChannel->push("dfmplugin_propertydialog", "slot_CustomView_Register",
                         func, TrashCoreHelper::scheme());
}
}   // namespace dfmplugin_trashcore
