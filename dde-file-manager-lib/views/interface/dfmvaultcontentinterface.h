#pragma once

#include <QWidget>
#include "dfmglobal.h"
#include "controllers/vaultcontroller.h"

DFM_BEGIN_NAMESPACE

class DFMVaultContentInterface : public QWidget
{
    Q_OBJECT
public:
    explicit DFMVaultContentInterface(QWidget *parent = nullptr);

    virtual QPair<DUrl, bool> requireRedirect(VaultController::VaultState state);
};

DFM_END_NAMESPACE
