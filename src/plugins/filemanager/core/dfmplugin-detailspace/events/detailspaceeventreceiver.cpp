// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "detailspaceeventreceiver.h"
#include "utils/detailspacehelper.h"
#include "utils/detailmanager.h"

#include <dfm-base/widgets/filemanagerwindowsmanager.h>

#include <dfm-framework/event/event.h>

#include <functional>

using namespace dfmplugin_detailspace;

DetailSpaceEventReceiver &DetailSpaceEventReceiver::instance()
{
    static DetailSpaceEventReceiver receiver;
    return receiver;
}

void DetailSpaceEventReceiver::connectService()
{
    // self
    dpfSlotChannel->connect(DPF_MACRO_TO_STR(DPDETAILSPACE_NAMESPACE), "slot_DetailView_Show",
                            this, &DetailSpaceEventReceiver::handleTileBarShowDetailView);
    dpfSlotChannel->connect(DPF_MACRO_TO_STR(DPDETAILSPACE_NAMESPACE), "slot_ViewExtension_Register",
                            this, &DetailSpaceEventReceiver::handleViewExtensionRegister);
    dpfSlotChannel->connect(DPF_MACRO_TO_STR(DPDETAILSPACE_NAMESPACE), "slot_BasicViewExtension_Register",
                            this, &DetailSpaceEventReceiver::handleBasicViewExtensionRegister);
    dpfSlotChannel->connect(DPF_MACRO_TO_STR(DPDETAILSPACE_NAMESPACE), "slot_BasicViewExtension_Root_Register",
                            this, &DetailSpaceEventReceiver::handleBasicViewExtensionRootRegister);
    dpfSlotChannel->connect(DPF_MACRO_TO_STR(DPDETAILSPACE_NAMESPACE), "slot_BasicFiledFilter_Add",
                            this, &DetailSpaceEventReceiver::handleBasicFiledFilterAdd);
    dpfSlotChannel->connect(DPF_MACRO_TO_STR(DPDETAILSPACE_NAMESPACE), "slot_BasicFiledFilter_Root_Add",
                            this, &DetailSpaceEventReceiver::handleBasicFiledFilterRootAdd);

    // workspace
    dpfSignalDispatcher->subscribe("dfmplugin_workspace", "signal_View_SelectionChanged",
                                   this, &DetailSpaceEventReceiver::handleViewSelectionChanged);
}

void DetailSpaceEventReceiver::handleTileBarShowDetailView(quint64 windowId, bool checked)
{
    DetailSpaceHelper::showDetailView(windowId, checked);
}

void DetailSpaceEventReceiver::handleSetSelect(quint64 windowId, const QUrl &url)
{
    DetailSpaceHelper::setDetailViewSelectFileUrl(windowId, url);
}

bool DetailSpaceEventReceiver::handleViewExtensionRegister(CustomViewExtensionView view, int index)
{
    return DetailManager::instance().registerExtensionView(view, index);
}

bool DetailSpaceEventReceiver::handleBasicViewExtensionRegister(BasicViewFieldFunc func, const QString &scheme)
{
    return DetailManager::instance().registerBasicViewExtension(scheme, func);
}

bool DetailSpaceEventReceiver::handleBasicViewExtensionRootRegister(BasicViewFieldFunc func, const QString &scheme)
{
    return DetailManager::instance().registerBasicViewExtensionRoot(scheme, func);
}

bool DetailSpaceEventReceiver::handleBasicFiledFilterAdd(const QString &scheme, const QStringList &enums)
{
    QMetaEnum &&metaState { QMetaEnum::fromType<DetailFilterType>() };
    QString &&join { enums.join("|") };
    bool ok { false };

    auto &&enumValues { static_cast<DetailFilterType>(metaState.keysToValue(join.toUtf8().constData(), &ok)) };
    if (ok)
        ok = DetailManager::instance().addBasicFiledFiltes(scheme, enumValues);

    return ok;
}

bool DetailSpaceEventReceiver::handleBasicFiledFilterRootAdd(const QString &scheme, const QStringList &enums)
{
    QMetaEnum &&metaState { QMetaEnum::fromType<DetailFilterType>() };
    QString &&join { enums.join("|") };
    bool ok { false };

    auto &&enumValues { static_cast<DetailFilterType>(metaState.keysToValue(join.toUtf8().constData(), &ok)) };
    if (ok)
        ok = DetailManager::instance().addRootBasicFiledFiltes(scheme, enumValues);

    return ok;
}

void DetailSpaceEventReceiver::handleViewSelectionChanged(const quint64 windowID, const QItemSelection &selected, const QItemSelection &deselected)
{
    Q_UNUSED(selected)
    Q_UNUSED(deselected)

    const QList<QUrl> &urls { dpfSlotChannel->push("dfmplugin_workspace", "slot_View_GetSelectedUrls", windowID).value<QList<QUrl>>() };
    if (urls.isEmpty()) {
        auto window { FMWindowsIns.findWindowById(windowID) };
        if (window)
            DetailSpaceHelper::setDetailViewSelectFileUrl(windowID, window->currentUrl());
    } else {
        DetailSpaceHelper::setDetailViewSelectFileUrl(windowID, urls.first());
    }
}

DetailSpaceEventReceiver::DetailSpaceEventReceiver(QObject *parent)
    : QObject(parent)
{
}
