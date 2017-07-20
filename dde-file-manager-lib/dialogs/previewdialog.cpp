#include "previewdialog.h"

#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QDebug>
#include <QDesktopWidget>
#include <QApplication>
#include <QResizeEvent>
#include <QGraphicsBlurEffect>

#include "dfileservices.h"
#include "singleton.h"
#include "app/define.h"
#include "dfileinfo.h"
#include "shutil/fileutils.h"

#include "dplatformwindowhandle.h"
#include "dthememanager.h"
#include "shutil/mimetypedisplaymanager.h"

#include "plugins/pluginmanager.h"
#include "../plugininterfaces/preview/previewinterface.h"
#include "controllers/appcontroller.h"
#include "../utils/utils.h"

DWIDGET_USE_NAMESPACE

class PreviewDialogPrivate{
public:
    PreviewDialogPrivate(PreviewDialog* qq):
        q_ptr(qq){}

    //For further interfaces manager, store them as third party interfaces and dde official interfaces
    QList<PreviewInterface*> previewInterfaces;
    QList<PreviewInterface*> ddePreviewInterfaces;

    DUrlList urls;
    DUrl currentUrl;
    int currentIndex;
    QPoint bottomLeftPos;

    //UI
    QPushButton* closeButton = NULL;
    QPushButton* popupCloseButton = NULL;
    QPushButton* forwardButton = NULL;
    QPushButton* backwardButton = NULL;
    QVBoxLayout* mainLayout = NULL;
    QHBoxLayout* previewLayout = NULL;
    QHBoxLayout* toolbarLayout = NULL;
    QFrame* toolBar = NULL;
    QPushButton* openButton = NULL;
    QLabel* line = NULL;
    BlurWidget* blurWidget = NULL;
    DPlatformWindowHandle* handle;

    //Widgets that interface registered
    QWidget* currentPreviewWidget = NULL;
    QWidget* currentToolBarItem = NULL;
    QSize currentPreviewWidgetDefaultSize;

    PreviewDialog* q_ptr = NULL;
    Q_DECLARE_PUBLIC(PreviewDialog)
};

PreviewDialog::PreviewDialog(const DUrlList &urls, QWidget *parent) :
    QDialog(parent),
    d_ptr(new PreviewDialogPrivate(this))
{
    Q_D(PreviewDialog);

    setAttribute(Qt::WA_DeleteOnClose);

    d->currentIndex = 0;
    d->urls = urls;
    d->currentUrl = urls.first();

    initInterfaces();

    initUI();
    initConnections();
}

void PreviewDialog::initInterfaces()
{
    Q_D(PreviewDialog);

    //Load deepin official plugins and third party plugins
    foreach (PreviewInterface* iface, PluginManager::instance()->getPreviewInterfaces()) {
        if(iface->pluginName().contains("dde-")){
            d->ddePreviewInterfaces.append(iface);
        } else {
            d->previewInterfaces.append(iface);
        }
    }
}

void PreviewDialog::initUI()
{
    Q_D(PreviewDialog);

//    DPlatformWindowHandle handle(this);
//    Q_UNUSED(handle)
    d->handle = new DPlatformWindowHandle(this);

    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    setContentsMargins(0, 0, 0, 0);

    d->forwardButton = new QPushButton(this);
    d->forwardButton->setFixedSize(20, 20);
    d->forwardButton->setObjectName("ForwardButton");

    d->backwardButton = new QPushButton(this);
    d->backwardButton->setFixedSize(20, 20);
    d->backwardButton->setObjectName("BackwardButton");

    d->closeButton = new QPushButton(this);
    d->closeButton->setFixedSize(24, 24);
    d->closeButton->setObjectName("CloseButton");

//    d->popupCloseButton = new QPushButton;
//    d->popupCloseButton->setFixedSize(24, 24);
//    d->popupCloseButton->setObjectName("CloseButton");
//    d->popupCloseButton->setStyleSheet(getQssFromFile(":/light/PreviewDialogDefault.theme"));
//    d->popupCloseButton->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
//    d->popupCloseButton->setAttribute(Qt::WA_TranslucentBackground);
//    d->popupCloseButton->hide();

    d->openButton =  new QPushButton(this);
    d->openButton->setFixedSize(70, 20);
    d->openButton->setText(QObject::tr("Open"));
    d->openButton->setObjectName("OpenButton");

    d->currentToolBarItem = new QWidget(this);

    d->toolbarLayout = new QHBoxLayout;
    d->toolbarLayout->setContentsMargins(0, 0, 0, 0);
    d->toolbarLayout->setSpacing(5);
    d->toolbarLayout->addSpacing(5);
    d->toolbarLayout->addWidget(d->backwardButton, 0, Qt::AlignVCenter);
    d->toolbarLayout->addWidget(d->forwardButton, 0, Qt::AlignVCenter);
    d->toolbarLayout->addStretch(0);
    d->toolbarLayout->addWidget(d->currentToolBarItem);
    d->toolbarLayout->addStretch(0);
    d->toolbarLayout->addWidget(d->openButton);
    d->toolbarLayout->addSpacing(5);

    d->toolBar = new QFrame(this);
    d->toolBar->setObjectName("ToolBar");
    d->toolBar->setContentsMargins(0, 0, 0, 0);
    d->toolBar->setLayout(d->toolbarLayout);
    d->toolBar->setFixedHeight(36);

    d->mainLayout = new QVBoxLayout;
    d->mainLayout->setContentsMargins(0, 0, 0, 0);
    d->mainLayout->setSpacing(0);
    d->mainLayout->addStretch(0);
    d->mainLayout->addWidget(d->toolBar);

    //Line on tool bar
    d->line = new QLabel(d->toolBar);
    d->line->setStyleSheet("background: rgba(0, 0, 0, 0.1);");
    d->line->setFixedHeight(1);
    d->line->hide();
    d->line->move(10, 0);

    //Blur effect for toobar
    d->blurWidget = new BlurWidget(this);
    d->blurWidget->hide();

    setLayout(d->mainLayout);

    updatePreview(d->currentUrl);
    d->closeButton->raise();
    updateNavigateButtons();


    QRect rect = this->rect();
    QRect dRect = qApp->desktop()->rect();
    rect.moveCenter(dRect.center());
    move(rect.x(), rect.y());
}

void PreviewDialog::initConnections()
{
    Q_D(PreviewDialog);

    connect(d->backwardButton, &QPushButton::clicked, this, &PreviewDialog::showBackward);
    connect(d->forwardButton, &QPushButton::clicked, this, &PreviewDialog::showForward);
    connect(d->closeButton, &QPushButton::clicked, this, &PreviewDialog::close);
    connect(d->openButton, &QPushButton::clicked, this, &PreviewDialog::openFile);
    connect(d->popupCloseButton, &QPushButton::clicked, this, &PreviewDialog::close);
    connect(d->popupCloseButton, &QPushButton::clicked, d->popupCloseButton, &QPushButton::close);
}

void PreviewDialog::showForward()
{
    Q_D(PreviewDialog);

    if(d->currentIndex < d->urls.count() - 1){
        d->currentIndex ++;
        d->currentUrl = d->urls.at(d->currentIndex);
        d->bottomLeftPos = geometry().bottomLeft();
        updatePreview(d->currentUrl);
        updateNavigateButtons();
    }
}

void PreviewDialog::showBackward()
{
    Q_D(PreviewDialog);

    if(d->currentIndex > 0){
        d->currentIndex --;
        d->currentUrl = d->urls.at(d->currentIndex);
        d->bottomLeftPos = geometry().bottomLeft();
        updatePreview(d->currentUrl);
        updateNavigateButtons();
    }
}

void PreviewDialog::openFile()
{
    Q_D(PreviewDialog);

    fileService->openFile(this, d->currentUrl);
}

void PreviewDialog::resizeEvent(QResizeEvent *event)
{
    Q_D(PreviewDialog);

    d->closeButton->move(event->size().width() - d->closeButton->width() - 5, 5);
    d->line->setFixedWidth(event->size().width() - 20);

    d->toolBar->move(0, event->size().height() - d->toolBar->height());

    if(d->currentToolBarItem){
        d->currentToolBarItem->setFixedWidth(event->size().width() - d->openButton->width() - d->backwardButton->width() - d->forwardButton->width() - 5*5);
    }


    const DAbstractFileInfoPointer& info = fileService->createFileInfo(this, d->currentUrl);
    if(info->mimeTypeName().startsWith("image") || info->mimeTypeName().startsWith("video")){

        d->currentPreviewWidget->setFixedSize(event->size());

        QPixmap pixmap = d->currentPreviewWidget->grab(QRect(QPoint(0, event->size().height() - d->toolBar->height()),
                                                             QSize(event->size().width(),d->toolBar->height())));

        d->blurWidget->setFixedSize(d->toolBar->size());
        d->blurWidget->move(0, event->size().height() - d->blurWidget->height());

        d->blurWidget->setFixedSize(pixmap.size());
        d->blurWidget->setPixmap(pixmap);
    } else{
        d->currentPreviewWidget->setFixedSize(event->size().width(), event->size().height() - d->toolBar->height());
    }

    //Handle for popup widgets
//    if(d->currentPreviewWidget->windowHandle()){
//        updatePopupWidgetsGeometry();
//    }

    QDialog::resizeEvent(event);
}

//bool PreviewDialog::event(QEvent *event)
//{
//    if(event->type() == QEvent::Move){
//        updatePopupWidgetsGeometry();
//    }
//    return QDialog::event(event);
//}

void PreviewDialog::updatePreview(const DUrl &url)
{
    Q_D(PreviewDialog);

    PreviewInterface* previewIface = NULL;

    //Get official iface by default, and use default setting in the future.
    foreach (PreviewInterface* iface, d->ddePreviewInterfaces) {
        iface->init(url.toString());
        if(iface->canPreview()){
            previewIface = iface;
            break;
        }
    }

    if(!previewIface){
        foreach (PreviewInterface* iface, d->previewInterfaces) {
            iface->init(url.toString());
            if(iface->canPreview()){
                previewIface = iface;
            }
        }
    }

    //Use unknown
    if(!previewIface){
        showUnknownPreview(url);
        setMinimumSize(DEFAULT_MIN_SIZE);
        updateDialogGeometry();
    } else {
        QWidget* w = previewIface->previewWidget();
        QWidget* item = previewIface->toolBarItem();

        registerPreviewWidget(w);
        registerToolbarItem(item);
        setMinimumSize(previewIface->previewWidgetMinSize());
        updateDialogGeometry();
    }

    const DAbstractFileInfoPointer& info = fileService->createFileInfo(this, url);

    if(info->mimeTypeName().startsWith("text") || info->mimeTypeName() == "application/pdf"){
        D_THEME_INIT_WIDGET(DocPreviewDialog)
        d->line->show();
    } else if(info->mimeTypeName().startsWith("audio")){
        D_THEME_INIT_WIDGET(MusicPreviewDialog)
        d->line->hide();
    } else if(info->mimeTypeName().startsWith("video") || mimeTypeDisplayManager->supportVideoMimeTypes().contains(info->mimeTypeName())){
        D_THEME_INIT_WIDGET(PreviewDialogDefault)
        d->line->hide();
    } else {
        D_THEME_INIT_WIDGET(PreviewDialogDefault)
        d->line->hide();
    }
}

void PreviewDialog::updateNavigateButtons()
{
    Q_D(PreviewDialog);

    if(d->urls.count() <= 1){
        hideNavigateButtons();
    } else {
        showNavigateButtons();
    }

    if(d->currentIndex <= 0){
        d->backwardButton->setEnabled(false);
    } else {
        d->backwardButton->setEnabled(true);
    }

    if(d->currentIndex >= d->urls.count() - 1){
        d->forwardButton->setEnabled(false);
    } else {
        d->forwardButton->setEnabled(true);
    }
}

void PreviewDialog::showNavigateButtons()
{
    Q_D(PreviewDialog);

    d->backwardButton->show();
    d->forwardButton->show();
}

void PreviewDialog::hideNavigateButtons()
{
    Q_D(PreviewDialog);

    d->backwardButton->hide();
    d->forwardButton->hide();
}

void PreviewDialog::showUnknownPreview(const DUrl &url)
{
    UnknownPreviewWidget* w = new UnknownPreviewWidget(url, this);
    registerPreviewWidget(w);
    registerToolbarItem(NULL);
}

void PreviewDialog::updateDialogGeometry()
{
    Q_D(PreviewDialog);

    QSize pSize = d->currentPreviewWidgetDefaultSize;
    QRect rect = QRect(0, 0, pSize.width(), pSize.height());
    QRect dRect = qApp->desktop()->geometry();

    const DAbstractFileInfoPointer& info = fileService->createFileInfo(this, d->currentUrl);

    if(info->mimeTypeName().startsWith("image") ||
            info->mimeTypeName().startsWith("video") ||
            mimeTypeDisplayManager->supportVideoMimeTypes().contains(info->mimeTypeName())){
        if(d->toolbarLayout->count() == 1)
            d->toolbarLayout->removeWidget(d->toolBar);
        d->toolBar->move(0, height() - d->toolBar->height());
        d->blurWidget->raise();
        d->toolBar->raise();
        d->blurWidget->show();
        rect.setSize(QSize(qMax(rect.width(), minimumSize().width()), qMax(rect.height(), minimumSize().height())));
        //Make sure that rect's size is same as current preview widget's size on image and vidoe preview
        d->currentPreviewWidget->setFixedSize(rect.size());
    } else{
        if(d->toolbarLayout->count() == 0)
            d->toolbarLayout->addWidget(d->toolBar);
        rect.setSize(QSize(qMax(rect.width(), minimumSize().width()), qMax(pSize.height() + d->toolBar->height(), minimumSize().height())));
        d->blurWidget->hide();
    }

    int gapx, gapy;

    gapx = dRect.width() - (d->bottomLeftPos.x() + rect.width());
    gapy = d->bottomLeftPos.y() - rect.height();

    gapx = gapx < 0 ? gapx : 0;
    gapy = gapy < 0 ? gapy : 0;

    resize(rect.size());

    /** For fixed size control **/
//    setFixedSize(rect.size());
//    if(info->mimeTypeName().startsWith("image") ||
//            info->mimeTypeName().startsWith("text") ||
//            mimeTypeDisplayManager->supportVideoMimeTypes().contains(info->mimeTypeName()) ||
//            info->mimeTypeName().startsWith("video") ||
//            info->mimeTypeName() == "application/pdf"){

//        setMaximumSize(qApp->desktop()->size());
//    } else {
//        setMaximumSize(rect.size());
//    }

    rect.moveBottomLeft(d->bottomLeftPos);

    move(rect.x() + gapx, rect.y() - gapy);

    if(d->currentToolBarItem){
        d->currentToolBarItem->setFixedWidth(rect.width() - d->openButton->width() - d->backwardButton->width() - d->forwardButton->width() - 5*5);
    }
    d->closeButton->raise();

//    if(d->currentPreviewWidget->windowHandle()){
//        d->popupCloseButton->show();
//    } else {
//        d->popupCloseButton->hide();
//    }

//    updatePopupWidgetsGeometry();
}

//void PreviewDialog::updatePopupWidgetsGeometry()
//{
//    Q_D(PreviewDialog);

//    QRect rect = geometry();
//    d->popupCloseButton->move(rect.x() + rect.width() - d->popupCloseButton->width() - 5,
//                              rect.y() + 5);
//}

void PreviewDialog::registerPreviewWidget(QWidget *w)
{
    if(!w){
        qDebug () << "This plugin has a invalid preview widget.";
        return;
    }

    Q_D(PreviewDialog);

    d->currentPreviewWidgetDefaultSize = w->size();
    d->currentPreviewWidget->deleteLater();
    d->currentPreviewWidget = NULL;

    w->setParent(this);
    w->show();
    w->raise();
    d->currentPreviewWidget = w;
    d->currentPreviewWidget->move(0, 0);

    if (w->windowHandle()) {
        connect(d->handle, &DPlatformWindowHandle::frameMarginsChanged, w, [w, d] {
            w->move(d->handle->frameMargins().left(), d->handle->frameMargins().top());
        });
        w->move(d->handle->frameMargins().left(), d->handle->frameMargins().top());
    }
}

void PreviewDialog::registerToolbarItem(QWidget *item)
{
    Q_D(PreviewDialog);

    if(!item){
        d->currentToolBarItem->hide();
        return;
    }

    d->toolbarLayout->removeWidget(d->currentToolBarItem);
    d->currentToolBarItem->deleteLater();
    d->currentToolBarItem = NULL;
    d->currentToolBarItem = item;

    d->toolbarLayout->insertWidget(3, d->currentToolBarItem, 0, Qt::AlignVCenter);
}

BlurWidget::BlurWidget(QWidget *parent):
    QLabel(parent)
{
    m_effect = new QGraphicsBlurEffect(this);
    setGraphicsEffect(m_effect);
}

UnknownPreviewWidget::UnknownPreviewWidget(const DUrl &url, QWidget *parent):
    QFrame(parent),
    m_url(url)
{
    initUI();
}

void UnknownPreviewWidget::initUI()
{
    const DAbstractFileInfoPointer& info = fileService->createFileInfo(this, m_url);
    if(!info){
        return;
    }

    QIcon icon = QIcon::fromTheme("unknown");
    m_imgLabel  = new QLabel(this);
    m_imgLabel->setPixmap(icon.pixmap(QSize(256, 256)));

    m_nameLabel = new QLabel(this);
    m_nameLabel->setObjectName("FileName");
    m_fileName = info->fileName();
    m_nameLabel->setText(m_fileName);

    QLabel* sizeLabel = new QLabel(this);
    sizeLabel->setText(FileUtils::formatSize(info->size()));

    QLabel* typeLabel = new QLabel(this);
    typeLabel->setText(QString("%1: %2").arg(tr("Type"), info->mimeTypeDisplayName()));

    m_messageLayout = new QVBoxLayout;
    m_messageLayout->addSpacing(28);
    m_messageLayout->setContentsMargins(0, 0, 0, 0);
    m_messageLayout->setSpacing(0);
    m_messageLayout->addWidget(m_nameLabel, 0, Qt::AlignLeft);
    m_messageLayout->addSpacing(5);
    m_messageLayout->addWidget(sizeLabel, 0, Qt::AlignLeft);
    m_messageLayout->addWidget(typeLabel, 0, Qt::AlignLeft);
    m_messageLayout->addStretch();

    m_mainLayout = new QHBoxLayout;
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);
    m_mainLayout->addWidget(m_imgLabel, 0, Qt::AlignTop);
    m_mainLayout->addSpacing(30);
    m_mainLayout->addLayout(m_messageLayout, 0);
    m_mainLayout->addStretch();

    setLayout(m_mainLayout);
    setContentsMargins(0, 0, 0, 0);
    setStyleSheet("QLabel{"
                        "font-size: 12px;"
                        "color: #5b5b5b;"
                    "}"
                    "QLabel#FileName{"
                        "font-size: 16px;"
                        "color: black;"
                    "}");

    setFixedSize(600, 300);
}

void UnknownPreviewWidget::resizeEvent(QResizeEvent *event)
{
    QFrame::resizeEvent(event);

    QFont f;
    f.setPixelSize(16);
    QFontMetrics fm(f);
//    m_messageLayout->setContentsMargins(0, m_imgLabel->geometry().y(), 0 , 0);
    int margins = (event->size().height() - 256) / 2;
    if((event->size().width() - margins - 150) < 256){
        margins = event->size().width() - 150 - 256;
    }
    setContentsMargins(margins, margins, 0, margins);
    m_nameLabel->setText(fm.elidedText(m_fileName, Qt::ElideRight, event->size().width() - (m_imgLabel->width() + 50 + margins)));

}
