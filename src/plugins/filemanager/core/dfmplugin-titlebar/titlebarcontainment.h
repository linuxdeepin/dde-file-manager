// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TITLEBARCONTAINMENT_H
#define TITLEBARCONTAINMENT_H

#include <dfm-gui/containment.h>

#include <QObject>

namespace dfmplugin_titlebar {

class CrumbInterface;
class QuickCrumbModel;

class TitlebarContainment : public dfmgui::Containment
{
    Q_OBJECT
    Q_PROPERTY(QuickCrumbModel *crumbModel READ crumbModel CONSTANT)

public:
    explicit TitlebarContainment(QObject *parent = nullptr);

    QuickCrumbModel *crumbModel() const;
    Q_SLOT void onUrlChanged(const QUrl &url);

private:
    void updateController(const QUrl &url);
    Q_SLOT void onHideAddrAndUpdateCrumbs(const QUrl &url);

private:
    CrumbInterface *crumbController = nullptr;
    QuickCrumbModel *model = nullptr;
};

}

#endif   // TITLEBARCONTAINMENT_H
