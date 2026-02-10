// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PACKETWRITINGMENUSCENE_H
#define PACKETWRITINGMENUSCENE_H

#include "dfmplugin_optical_global.h"

#include <dfm-base/interfaces/abstractmenuscene.h>
#include <dfm-base/interfaces/abstractscenecreator.h>

DPOPTICAL_BEGIN_NAMESPACE

class PacketWritingMenuScenePrivate;
class PacketWritingMenuCreator : public DFMBASE_NAMESPACE::AbstractSceneCreator
{
public:
    static QString name()
    {
        return "PacketWritingMenu";
    }
    DFMBASE_NAMESPACE::AbstractMenuScene *create() override;
};

class PacketWritingMenuScene : public DFMBASE_NAMESPACE::AbstractMenuScene
{
    Q_OBJECT
public:
    explicit PacketWritingMenuScene(QObject *parent = nullptr);
    ~PacketWritingMenuScene() override;
    QString name() const override;
    bool initialize(const QVariantHash &params) override;
    void updateState(QMenu *parent) override;

private:
    QScopedPointer<PacketWritingMenuScenePrivate> d;
};

DPOPTICAL_END_NAMESPACE

#endif   // PACKETWRITINGMENUSCENE_H
