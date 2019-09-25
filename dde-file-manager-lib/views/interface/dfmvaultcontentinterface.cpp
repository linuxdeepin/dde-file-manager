#include "dfmvaultcontentinterface.h"

#include <QStackedLayout>

DFM_BEGIN_NAMESPACE

DFMVaultContentInterface::DFMVaultContentInterface(QWidget *parent) : QWidget(parent)
{

}

QPair<DUrl, bool> DFMVaultContentInterface::requireRedirect(VaultController::VaultState state)
{
    Q_UNUSED(state);
    return {DUrl(), false};
}

void DFMVaultContentInterface::setRootUrl(const DUrl &url)
{
    Q_UNUSED(url);
    return;
}

// ------------------------------------------------

DFMVaultPages::DFMVaultPages(QWidget *parent)
    : DFMVaultContentInterface(parent)
    , m_layout(new QStackedLayout(this))
{

}

void DFMVaultPages::setRootUrl(const DUrl &url)
{
    QString pageStr = pageString(url);
    m_layout->setCurrentWidget(m_pages[pageStr]);
    emit rootPageChanged(pageStr);
}

QWidget *DFMVaultPages::page(const QString &pageName)
{
    return m_pages.value(pageName, nullptr);
}

void DFMVaultPages::insertPage(const QString &pageName, QWidget *widget)
{
    m_pages.insert(pageName, widget);
    m_layout->addWidget(widget);
}

DFM_END_NAMESPACE
