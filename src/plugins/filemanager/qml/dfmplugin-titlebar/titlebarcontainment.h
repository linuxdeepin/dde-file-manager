// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TITLEBARCONTAINMENT_H
#define TITLEBARCONTAINMENT_H

#include <dfm-base/dfm_global_defines.h>

#include <dfm-gui/containment.h>

#include <QObject>

namespace dfmplugin_titlebar {

class CrumbInterface;
class QuickCrumbModel;

class TitlebarContainment : public dfmgui::Containment
{
    Q_OBJECT
    Q_PROPERTY(QuickCrumbModel *crumbModel READ crumbModel CONSTANT)
    Q_PROPERTY(dfmbase::Global::ViewMode viewMode READ viewMode WRITE setViewMode NOTIFY viewModeChanged FINAL)
    Q_PROPERTY(bool showDetail READ showDetail WRITE setShowDetail NOTIFY showDetailChanged FINAL)

public:
    explicit TitlebarContainment(QObject *parent = nullptr);

    QuickCrumbModel *crumbModel() const;
    Q_SLOT void onUrlChanged(const QUrl &url);

    dfmbase::Global::ViewMode viewMode() const;
    void setViewMode(dfmbase::Global::ViewMode mode);
    Q_SIGNAL void viewModeChanged();

    bool showDetail() const;
    void setShowDetail(bool b);
    Q_SIGNAL void showDetailChanged();

private:
    void updateController(const QUrl &url);
    Q_SLOT void onHideAddrAndUpdateCrumbs(const QUrl &url);

private:
    CrumbInterface *crumbController { nullptr };
    QuickCrumbModel *model { nullptr };

    bool showDetailInfo { false };
    dfmbase::Global::ViewMode internalViewMode { dfmbase::Global::ViewMode::kIconMode };
};

}

#endif   // TITLEBARCONTAINMENT_H
