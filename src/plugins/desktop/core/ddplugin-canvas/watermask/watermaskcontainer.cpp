// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "watermaskcontainer.h"
#include "watermasksystem.h"
#include "watermaskframe.h"
#include "customwatermasklabel.h"

#include <QFile>
#include <QJsonParseError>
#include <QJsonDocument>
#include <QJsonObject>
#include <QVariantMap>

using namespace ddplugin_canvas;

static constexpr char kConfFile[] = "/usr/share/deepin/dde-desktop-watermask.json";

WatermaskContainer::WatermaskContainer(QWidget *parent) : QObject(parent)
{
    custom = new CustomWaterMaskLabel(parent);
    if (WatermaskSystem::isEnable()) {
        fmInfo() << "use WatermaskSystem.";
        control = new WatermaskSystem(parent);

        custom->lower();
        control->stackUnder(custom);

        connect(control, &WatermaskSystem::showedOn, custom, &CustomWaterMaskLabel::onSystemMaskShow);
    } else {
        fmInfo() << "use WaterMaskFrame.";
        frame = new WaterMaskFrame(kConfFile, parent);

        custom->lower();
        frame->stackUnder(custom);

        connect(frame, &WaterMaskFrame::showMask, custom, &CustomWaterMaskLabel::onSystemMaskShow);
    }
}

bool WatermaskContainer::isEnable()
{
    static int on = -1;
    if (on > -1) {
        return on > 0;
    }

    QFile file(kConfFile);
    if (!file.open(QFile::ReadOnly)) {
        fmWarning() << "WaterMask config file doesn't exist!";
        on = 0;
        return on;
    }

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);
    if (error.error == QJsonParseError::NoError) {
        auto json = doc.toVariant().toMap();
        if (json.contains("isMaskAlwaysOn"))
            on = json.value("isMaskAlwaysOn", false).toBool() ? 1 : 0;
        else
            on = 0;
    } else {
        fmCritical() << "config file is invailid" << kConfFile << error.errorString();
        on = 0;
    }

    return on;
}


void WatermaskContainer::refresh()
{
    if (control)
        control->refresh();
    else if (frame)
        frame->refresh();

    custom->refresh();
}

void WatermaskContainer::updatePosition()
{
    if (control)
        control->updatePosition();
    else if (frame)
        frame->updatePosition();
}
