// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DETAILSPACECONTAINMENT_H
#define DETAILSPACECONTAINMENT_H

#include <QObject>

#include <dfm-gui/containment.h>

namespace dfmplugin_detailspace {

class DetailSpaceContainment : public dfmgui::Containment
{
    Q_OBJECT

    Q_PROPERTY(bool detailVisible READ detailVisible WRITE setDetailVisible NOTIFY detailVisibleChanged FINAL)

public:
    explicit DetailSpaceContainment(QObject *parent = nullptr);

    bool detailVisible() const;
    void setDetailVisible(bool b);
    Q_SIGNAL void detailVisibleChanged(bool b);

private:
    bool visibleFlag { false };
    QString curIconName;
};

}

#endif   // DETAILSPACECONTAINMENT_H
