// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "detailspacecontainment.h"

#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/utils/thumbnail/thumbnailhelper.h>

#include <dfm-framework/event/event.h>

DFMBASE_USE_NAMESPACE

namespace dfmplugin_detailspace {

DetailSpaceContainment::DetailSpaceContainment(QObject *parent)
    : dfmgui::Containment(parent)
{
}

bool DetailSpaceContainment::detailVisible() const
{
    return visibleFlag;
}

void DetailSpaceContainment::setDetailVisible(bool b)
{
    if (visibleFlag != b) {
        visibleFlag = b;
        Q_EMIT detailVisibleChanged(b);
    }
}

}
