#include "openwithotherdialog.h"

#include <QListWidget>
#include <QListWidgetItem>
#include <QButtonGroup>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>

#include <dscrollbar.h>
#include <dthememanager.h>

#include "../app/global.h"
#include "../controllers/fileservices.h"
#include "../shutil/mimesappsmanager.h"
#include "../app/global.h"
#include "../shutil/desktopfile.h"
#include "../shutil/iconprovider.h"
#include "../shutil/fileutils.h"


OpenWithOtherDialog::OpenWithOtherDialog(const DUrl &url, QWidget *parent) :
    DDialog(parent)
{
    m_url = url;
    setModal(true);
    initUI();
    initConnect();
}

OpenWithOtherDialog::~OpenWithOtherDialog()
{

}

void OpenWithOtherDialog::initUI()
{
    setFixedSize(300, 400);
    const AbstractFileInfoPointer &fileInfo = fileService->createFileInfo(m_url);
    m_appListWidget = createOpenWithListWidget(fileInfo);


    QLabel* m_titleLabel = new QLabel(tr("Open with"), this);
    m_titleLabel->setFixedSize(200, 40);
    m_titleLabel->move(50, 0);
    m_titleLabel->setAlignment(Qt::AlignCenter);

    QStringList buttonTexts;
    buttonTexts << tr("Cancel") << tr("Choose");

    addContent(m_appListWidget);
    addButtons(buttonTexts);
    setSpacing(0);
    setDefaultButton(1);

}

void OpenWithOtherDialog::initConnect()
{
    connect(this, SIGNAL(buttonClicked(int,QString)), this, SLOT(handleButtonClicked(int,QString)));
}

QListWidget *OpenWithOtherDialog::createOpenWithListWidget(const AbstractFileInfoPointer &info)
{
    QListWidget* listWidget = new QListWidget;
//    listWidget->setStyleSheet("border:1px solid red");
    m_OpenWithButtonGroup = new QButtonGroup(listWidget);
    listWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    listWidget->setVerticalScrollBar(new DScrollBar);

    QString path = info->absoluteFilePath();
    QMimeType mimeType = mimeAppsManager->getMimeType(path);
    qDebug() << MimesAppsManager::getMimeTypeByFileName(path) << mimeType.aliases();
    QStringList recommendApps = mimeAppsManager->MimeApps.value(MimesAppsManager::getMimeTypeByFileName(path));;
    foreach (QString name, mimeType.aliases()) {
        QStringList apps = mimeAppsManager->MimeApps.value(name);
        foreach (QString app, apps) {
            if (!recommendApps.contains(app)){
                recommendApps.append(app);
            }
        }
    }
    QString defaultApp = mimeAppsManager->getDefaultAppByMimeType(mimeType);

    foreach (QString f, mimeAppsManager->DesktopObjs.keys()) {
        if (recommendApps.contains(f)){
            continue;
        }
        QString iconName = mimeAppsManager->DesktopObjs.value(f).getIcon();
        QIcon icon(fileIconProvider->getDesktopIcon(iconName, 48));
        QListWidgetItem* item = new QListWidgetItem;

        QCheckBox* itemBox = new QCheckBox(mimeAppsManager->DesktopObjs.value(f).getLocalName());
        itemBox->setStyleSheet(getQssFromFile(":/light/OpenWithOtherDialog.theme"));
        itemBox->setIcon(icon);
        itemBox->setFixedHeight(36);
        itemBox->setIconSize(QSize(16, 16));
        m_OpenWithButtonGroup->addButton(itemBox);
        itemBox->setProperty("app", f);
        listWidget->addItem(item);
        listWidget->setItemWidget(item, itemBox);

        if (QFileInfo(f).fileName() == defaultApp){
            itemBox->setChecked(true);
        }
    }

    int listHeight = 2;
    for(int i=0; i < listWidget->count(); i++){
        QListWidgetItem* item = listWidget->item(i);
        item->setFlags(Qt::NoItemFlags);
        int h = listWidget->itemWidget(item)->height();
        item->setSizeHint(QSize(item->sizeHint().width(), h));
        listHeight += h;
    }

    if (listHeight >= 36 * 9){
        listWidget->setFixedHeight(36 * 9);
    }else{
        listWidget->setFixedHeight(listHeight);
    }


    return listWidget;
}

void OpenWithOtherDialog::handleButtonClicked(int index, QString text)
{
    Q_UNUSED(text)
    if (index == 1){
        if (m_OpenWithButtonGroup->checkedButton()){
            QString app = m_OpenWithButtonGroup->checkedButton()->property("app").toString();
            QString file = m_url.toString();
            FileUtils::openFileByApp(file, app);
        }
    }
}

