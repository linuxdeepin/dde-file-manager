/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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
#include "detailspaceeventreceiver.h"
#include "utils/detailspacehelper.h"
#include "utils/detailmanager.h"

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
    dpfSlotChannel->connect(DPF_MACRO_TO_STR(DPDETAILSPACE_NAMESPACE), "slot_DetailView_Show",
                            this, &DetailSpaceEventReceiver::handleTileBarShowDetailView);
    dpfSlotChannel->connect(DPF_MACRO_TO_STR(DPDETAILSPACE_NAMESPACE), "slot_DetailView_Select",
                            this, &DetailSpaceEventReceiver::handleSetSelect);

    dpfSlotChannel->connect(DPF_MACRO_TO_STR(DPDETAILSPACE_NAMESPACE), "slot_ViewExtension_Register",
                            this, &DetailSpaceEventReceiver::handleViewExtensionRegister);
    dpfSlotChannel->connect(DPF_MACRO_TO_STR(DPDETAILSPACE_NAMESPACE), "slot_ViewExtension_Unregister",
                            this, &DetailSpaceEventReceiver::handleViewExtensionUnregister);

    dpfSlotChannel->connect(DPF_MACRO_TO_STR(DPDETAILSPACE_NAMESPACE), "slot_BasicViewExtension_Register",
                            this, &DetailSpaceEventReceiver::handleBasicFiledFilterAdd);
    dpfSlotChannel->connect(DPF_MACRO_TO_STR(DPDETAILSPACE_NAMESPACE), "slot_BasicViewExtension_Unregister",
                            this, &DetailSpaceEventReceiver::handleBasicFiledFilterRemove);

    dpfSlotChannel->connect(DPF_MACRO_TO_STR(DPDETAILSPACE_NAMESPACE), "slot_BasicFiledFilter_Add",
                            this, &DetailSpaceEventReceiver::handleBasicFiledFilterAdd);
    dpfSlotChannel->connect(DPF_MACRO_TO_STR(DPDETAILSPACE_NAMESPACE), "slot_BasicFiledFilter_Remove",
                            this, &DetailSpaceEventReceiver::handleBasicFiledFilterRemove);
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

void DetailSpaceEventReceiver::handleViewExtensionUnregister(int index)
{
    DetailManager::instance().unregisterExtensionView(index);
}

bool DetailSpaceEventReceiver::handleBasicViewExtensionRegister(const QString &scheme, BasicViewFieldFunc func)
{
    return DetailManager::instance().registerBasicViewExtension(scheme, func);
}

void DetailSpaceEventReceiver::handleBasicViewExtensionUnregister(const QString &scheme)
{
    DetailManager::instance().unregisterBasicViewExtension(scheme);
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

void DetailSpaceEventReceiver::handleBasicFiledFilterRemove(const QString &scheme)
{
    DetailManager::instance().removeBasicFiledFilters(scheme);
}

DetailSpaceEventReceiver::DetailSpaceEventReceiver(QObject *parent)
    : QObject(parent)
{
}
