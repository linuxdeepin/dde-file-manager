// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "app/global.h"
#include "appcontroller.h"
#include "fileoperation.h"
#include "tag/tagmanagerdaemon.h"
#include "usershare/usersharemanager.h"
#include "accesscontrol/accesscontrolmanager.h"
#include "vault/vaultmanager.h"
#include "revocation/revocationmanager.h"
#include "vault/vaultbruteforceprevention.h"

AppController::AppController(QObject *parent) : QObject(parent)
{
    initControllers();
    initConnect();
}

AppController::~AppController()
{

}

void AppController::initControllers()
{
    m_accessController = new AccessControlManager(this);
    m_userShareManager = new UserShareManager(this);
    m_tagManagerDaemon = new TagManagerDaemon{ this };
    m_vaultManager = new VaultManager(this);
    m_revocationManager = new RevocationManager(this);
    m_vaultForce = new VaultBruteForcePrevention(this);
}

void AppController::initConnect()
{
}

