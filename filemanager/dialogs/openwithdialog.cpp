#include "openwithdialog.h"
#include <QListWidget>
#include <QListWidgetItem>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include "../shutil/mimesappsmanager.h"
#include "../app/global.h"
#include "../shutil/desktopfile.h"
#include "../shutil/iconprovider.h"
#include "../views/dscrollbar.h"

OpenWithDialog::OpenWithDialog(const QString &url, QWidget *parent) :
    DWidget(parent)
{
    m_url = url;
    setWindowFlags(windowFlags()
                           &~ Qt::WindowMaximizeButtonHint
                           &~ Qt::WindowMinimizeButtonHint
                           &~ Qt::WindowSystemMenuHint);
    initUI();
    initConnect();
}

OpenWithDialog::~OpenWithDialog()
{

}

void OpenWithDialog::initUI()
{

    setFixedSize(400, 400);
    setTitle(tr("Open with"));
    m_listWidget = new QListWidget;
    m_listWidget->setVerticalScrollBar(new DScrollBar);

    m_cancelButton = new QPushButton(tr("Cancel"));
    m_chooseButton = new QPushButton(tr("Choose"));

    QHBoxLayout* buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_cancelButton);
    buttonLayout->addWidget(m_chooseButton);
    buttonLayout->setContentsMargins(0, 0, 10, 0);

    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addWidget(m_listWidget);
    mainLayout->addLayout(buttonLayout);
    mainLayout->setContentsMargins(5, 5, 5, 5);
    setLayout(mainLayout);
    addItems();
}

void OpenWithDialog::initConnect()
{
    connect(m_cancelButton, &QPushButton::clicked, this, &OpenWithDialog::close);
    connect(m_chooseButton, &QPushButton::clicked, this, &OpenWithDialog::openFileByApp);
}


void OpenWithDialog::addItems()
{
    QFont font;
    font.setPixelSize(20);
    QListWidgetItem* recommendItem = new QListWidgetItem("Recommend applications");
    recommendItem->setSizeHint(QSize(200, 40));
    recommendItem->setFont(font);
    recommendItem->setFlags(Qt::NoItemFlags);
    m_listWidget->addItem(recommendItem);


    QStringList recommendApps = mimeAppsManager->MimeApps.value(MimesAppsManager::getMimeTypeByFileName(m_url));

    foreach (QString f, recommendApps){
        QString iconName = mimeAppsManager->DesktopObjs.value(f).getIcon();
        QIcon icon(iconProvider->getDesktopIcon(iconName, 48));
        QListWidgetItem* item = new QListWidgetItem(icon, mimeAppsManager->DesktopObjs.value(f).getName());
        m_listWidget->addItem(item);
    }

    QListWidgetItem* otherItem = new QListWidgetItem("Other applications");

    otherItem->setSizeHint(QSize(200, 40));
    otherItem->setFont(font);
    otherItem->setFlags(Qt::NoItemFlags);
    m_listWidget->addItem(otherItem);

    foreach (QString f, mimeAppsManager->DesktopObjs.keys()) {
        if (recommendApps.contains(f)){
            continue;
        }
        QString iconName = mimeAppsManager->DesktopObjs.value(f).getIcon();
        QIcon icon(iconProvider->getDesktopIcon(iconName, 48));
        QListWidgetItem* item = new QListWidgetItem(icon, mimeAppsManager->DesktopObjs.value(f).getName());
        m_listWidget->addItem(item);
    }
}

void OpenWithDialog::openFileByApp()
{

}

