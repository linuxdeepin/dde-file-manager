// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dfmtagcrumbcontroller.h"
#include "tag/tagmanager.h"

DFM_BEGIN_NAMESPACE

DFMTagCrumbController::DFMTagCrumbController(QObject *parent)
    : DFMCrumbInterface(parent)
{

}

DFMTagCrumbController::~DFMTagCrumbController()
{

}

bool DFMTagCrumbController::supportedUrl(DUrl url)
{
    return (url.scheme() == TAG_SCHEME);
}

QList<CrumbData> DFMTagCrumbController::seprateUrl(const DUrl &url)
{
    QString colorName;
    QString displayText;

    if (url != DUrl(TAG_ROOT)) {
        displayText = url.fileName();
        colorName = TagManager::instance()->getTagIconName(url.fileName());
    } else {
        displayText = QCoreApplication::translate("DFMTagCrumbController", "Tag information");
        colorName = QStringLiteral("CrumbIconButton.Tags");
    }

    return { CrumbData(url, displayText, colorName) };
}

DFM_END_NAMESPACE
