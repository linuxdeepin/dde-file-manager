#pragma once

#include <QWidget>
#include "dfmglobal.h"
#include "controllers/vaultcontroller.h"

QT_BEGIN_NAMESPACE
class QStackedLayout;
QT_END_NAMESPACE

DFM_BEGIN_NAMESPACE

class DFMVaultContentInterface : public QWidget
{
    Q_OBJECT
public:
    explicit DFMVaultContentInterface(QWidget *parent = nullptr);

    virtual QPair<DUrl, bool> requireRedirect(VaultController::VaultState state);
    virtual void setRootUrl(const DUrl & url);
};

// -------------------------------------------

class DFMVaultPages : public DFMVaultContentInterface
{
    Q_OBJECT
public:
    explicit DFMVaultPages(QWidget *parent = nullptr);

    void setRootUrl(const DUrl & url) override;

    virtual QString pageString(const DUrl & url) = 0;
    void insertPage(const QString &pageName, QWidget * widget);

private:
    QMap<QString, QWidget*> m_pages;
    QStackedLayout * m_layout;
};

DFM_END_NAMESPACE
