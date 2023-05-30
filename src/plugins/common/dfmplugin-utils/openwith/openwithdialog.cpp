// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "openwithdialog.h"
#include "dfileservices.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/utils/windowutils.h>
#include <dfm-base/mimetype/mimesappsmanager.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/utils/properties.h>
#include <dfm-base/dfm_event_defines.h>

#include <dfm-framework/dpf.h>

#include <QVBoxLayout>
#include <QHBoxLayout>

#include <QApplication>
#include <QDesktopWidget>
#include <QWindow>
#include <QDebug>
#include <QScrollArea>
#include <QPainter>
#include <QFileDialog>
#include <QStandardPaths>
#include <QScroller>
#include <QPainterPath>
#include <QResizeEvent>
#include <QScrollArea>

#include <DPushButton>
#include <DSuggestButton>
#include <DLabel>
#include <DCheckBox>
#include <DCommandLinkButton>
#include <DHorizontalLine>
#include <DStyle>

#include <dtkwidget_global.h>
#ifdef DTKWIDGET_CLASS_DSizeMode
#    include <DSizeMode>
#endif

DWIDGET_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DFMGLOBAL_USE_NAMESPACE
using namespace dfmplugin_utils;

OpenWithDialogListItem::OpenWithDialogListItem(const QIcon &icon, const QString &text, QWidget *parent)
    : QWidget(parent),
      icon(icon.isNull() ? QIcon::fromTheme("application-x-desktop") : icon),
      checkButton(new DIconButton(this)),
      iconLabel(new DLabel(this)),
      label(new DLabel(this))

{
    checkButton->setFixedSize(10, 10);
    checkButton->setFlat(true);
    label->setText(text);
    iconLabel->setAlignment(Qt::AlignCenter);

    QHBoxLayout *layout = new QHBoxLayout(this);

    layout->setContentsMargins(5, 0, 5, 0);
    layout->addWidget(checkButton);
    layout->addWidget(iconLabel);
    layout->addWidget(label);

    setMouseTracking(true);
    initUiForSizeMode();
#ifdef DTKWIDGET_CLASS_DSizeMode
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::sizeModeChanged, this, &OpenWithDialogListItem::initUiForSizeMode);
#endif
}

void OpenWithDialogListItem::setChecked(bool checked)
{
    if (checked) {
        checkButton->setIcon(DStyle::SP_MarkElement);
    } else {
        checkButton->setIcon(QIcon());
    }
}

void OpenWithDialogListItem::initUiForSizeMode()
{
#ifdef DTKWIDGET_CLASS_DSizeMode
    int size = DSizeModeHelper::element(25, 30);
    iconLabel->setFixedSize(size, size);
    iconLabel->setPixmap(icon.pixmap(iconLabel->size()));
    setFixedSize(220, DSizeModeHelper::element(40, 50));
#else
    int size = 30;
    iconLabel->setFixedSize(size, size);
    iconLabel->setPixmap(icon.pixmap(iconLabel->size()));
    setFixedSize(220, 50);
#endif
}

QString OpenWithDialogListItem::text() const
{
    return label->text();
}

void OpenWithDialogListItem::enterEvent(QEvent *e)
{
    Q_UNUSED(e)

    update();

    return QWidget::enterEvent(e);
}

void OpenWithDialogListItem::leaveEvent(QEvent *e)
{
    Q_UNUSED(e)

    update();

    return QWidget::leaveEvent(e);
}

void OpenWithDialogListItem::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e)

    if (!underMouse())
        return;

    QPainter pa(this);
    QPainterPath path;

    path.addRoundedRect(rect(), 6, 6);
    pa.setRenderHint(QPainter::Antialiasing);
    pa.fillPath(path, QColor(0, 0, 0, static_cast<int>(0.05 * 255)));
}

class OpenWithDialogListSparerItem : public QWidget
{
public:
    explicit OpenWithDialogListSparerItem(const QString &title, QWidget *parent = nullptr);

private slots:
    void initUiForSizeMode();

private:
    DHorizontalLine *separator { nullptr };
    QLabel *titleLabel { nullptr };
};

void OpenWithDialogListSparerItem::initUiForSizeMode()
{
#ifdef DTKWIDGET_CLASS_DSizeMode
    QFont font;
    font.setPixelSize(DSizeModeHelper::element(14, 18));
    font.setWeight(QFont::DemiBold);
    titleLabel->setFont(font);
#else
    QFont font;
    font.setPixelSize(18);
    titleLabel->setFont(font);
#endif
}

OpenWithDialogListSparerItem::OpenWithDialogListSparerItem(const QString &title, QWidget *parent)
    : QWidget(parent), separator(new DHorizontalLine(this)), titleLabel(new QLabel(title, this))
{
#ifdef DTKWIDGET_CLASS_DSizeMode
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::sizeModeChanged, this, &OpenWithDialogListSparerItem::initUiForSizeMode);
#endif
    QVBoxLayout *layout = new QVBoxLayout(this);
    initUiForSizeMode();
    layout->addWidget(separator);
    layout->addWidget(titleLabel);
    layout->setContentsMargins(20, 0, 20, 0);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

OpenWithDialog::OpenWithDialog(const QList<QUrl> &list, QWidget *parent)
    : BaseDialog(parent), urlList(list)
{
    setWindowFlags(windowFlags()
                   & ~Qt::WindowMaximizeButtonHint
                   & ~Qt::WindowMinimizeButtonHint
                   & ~Qt::WindowSystemMenuHint);
    MimesAppsManager::instance()->initMimeTypeApps();
    initUI();
    initConnect();
    initData();
    initUiForSizeMode();
}

void OpenWithDialog::initUiForSizeMode()
{
#ifdef DTKWIDGET_CLASS_DSizeMode
    QFont font;
    int size = DSizeModeHelper::element(16, 20);
    font.setPixelSize(size);
    font.setWeight(QFont::DemiBold);
    setTitleFont(font);
#else
    QFont font;
    font.setPixelSize(20);
    font.setWeight(QFont::DemiBold);
    setTitleFont(font);
#endif
}

OpenWithDialog::OpenWithDialog(const QUrl &url, QWidget *parent)
    : BaseDialog(parent), curUrl(url)
{
    setWindowFlags(windowFlags()
                   & ~Qt::WindowMaximizeButtonHint
                   & ~Qt::WindowMinimizeButtonHint
                   & ~Qt::WindowSystemMenuHint);
    MimesAppsManager::instance()->initMimeTypeApps();
    initUI();
    initConnect();
    initData();
    initUiForSizeMode();
}

OpenWithDialog::~OpenWithDialog()
{
}

void OpenWithDialog::initUI()
{
    if (WindowUtils::isWayLand()) {

        this->setWindowFlags(this->windowFlags() & ~Qt::WindowMinMaxButtonsHint);
        this->setAttribute(Qt::WA_NativeWindow);
        //this->windowHandle()->setProperty("_d_dwayland_window-type", "wallpaper");
        this->windowHandle()->setProperty("_d_dwayland_minimizable", false);
        this->windowHandle()->setProperty("_d_dwayland_maximizable", false);
        this->windowHandle()->setProperty("_d_dwayland_resizable", false);
    }
    setFixedSize(710, 450);
    setTitle(tr("Open with"));

    scrollArea = new QScrollArea(this);
    scrollArea->setObjectName("OpenWithDialog-QScrollArea");

    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setWidgetResizable(true);
    QScroller::grabGesture(scrollArea);
    scrollArea->installEventFilter(this);
    scrollArea->viewport()->setStyleSheet("background-color:transparent;");   //设置滚动区域与主窗体颜色一致

    QWidget *contentWidget = new QWidget;

    contentWidget->setObjectName("contentWidget");
    contentWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    scrollArea->setWidget(contentWidget);

    recommandLayout = new DFlowLayout;
    otherLayout = new DFlowLayout;

    openFileChooseButton = new DCommandLinkButton(tr("Add other programs"), this);
    setToDefaultCheckBox = new DCheckBox(tr("Set as default"), this);
    setToDefaultCheckBox->setChecked(true);
    cancelButton = new DPushButton(tr("Cancel", "button"));
    chooseButton = new DSuggestButton(tr("Confirm", "button"));
    cancelButton->setFixedWidth(78);
    chooseButton->setFixedWidth(78);
    chooseButton->setFocus();

    QVBoxLayout *contentLayout = new QVBoxLayout;
    contentLayout->setContentsMargins(10, 0, 10, 0);
    contentLayout->addWidget(new OpenWithDialogListSparerItem(tr("Recommended Applications"), this));
    contentLayout->addLayout(recommandLayout);
    contentLayout->addWidget(new OpenWithDialogListSparerItem(tr("Other Applications"), this));
    contentLayout->addLayout(otherLayout);
    contentLayout->addStretch();

    contentWidget->setLayout(contentLayout);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(openFileChooseButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(setToDefaultCheckBox);
    buttonLayout->addSpacing(20);
    buttonLayout->addWidget(cancelButton);
    buttonLayout->addSpacing(5);
    buttonLayout->addWidget(chooseButton);
    buttonLayout->setContentsMargins(10, 0, 10, 0);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    QVBoxLayout *bottomLayout = new QVBoxLayout;

    bottomLayout->addWidget(new DHorizontalLine(this));
    bottomLayout->addSpacing(5);
    bottomLayout->addLayout(buttonLayout);
    bottomLayout->setContentsMargins(10, 0, 10, 0);

    mainLayout->addWidget(scrollArea);
    mainLayout->addLayout(bottomLayout);
    mainLayout->setContentsMargins(0, 35, 0, 10);
}

void OpenWithDialog::initConnect()
{
#ifdef DTKWIDGET_CLASS_DSizeMode
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::sizeModeChanged, this, &OpenWithDialog::initUiForSizeMode);
#endif
    connect(cancelButton, &QPushButton::clicked, this, &OpenWithDialog::close);
    connect(chooseButton, &QPushButton::clicked, this, &OpenWithDialog::openFileByApp);
    connect(openFileChooseButton, &DCommandLinkButton::clicked, this, &OpenWithDialog::useOtherApplication);
}

void OpenWithDialog::initData()
{
    //在选择默认程序时，有多个url，要传多个url
    if (curUrl.isValid() && urlList.isEmpty()) {
        const FileInfoPointer &fileInfo = InfoFactory::create<FileInfo>(curUrl);

        if (!fileInfo)
            return;
        mimeType = fileInfo->fileMimeType();

        if (FileUtils::isDesktopFile(curUrl))
            setToDefaultCheckBox->hide();
    } else if (!curUrl.isValid() && !urlList.isEmpty()) {
        QList<QUrl> openlist;
        bool bhide = true;
        for (auto url : urlList) {
            const FileInfoPointer &fileInfo = InfoFactory::create<FileInfo>(url);

            if (!fileInfo) {
                continue;
            }
            mimeType = fileInfo->fileMimeType();
            if (!FileUtils::isDesktopFile(url)) {
                bhide = false;
            }
            openlist.push_back(url);
        }

        if (openlist.isEmpty())
            return;

        if (bhide)
            setToDefaultCheckBox->hide();
    }

    const QString &defaultApp = MimesAppsManager::instance()->getDefaultAppByMimeType(mimeType);
    const QStringList &recommendApps = MimesAppsManager::instance()->getRecommendedAppsByQio(mimeType);

    for (int i = 0; i < recommendApps.count(); ++i) {
        const DesktopFile &desktopInfo = MimesAppsManager::instance()->DesktopObjs.value(recommendApps.at(i));

        OpenWithDialogListItem *item = createItem(QIcon::fromTheme(desktopInfo.desktopIcon()), desktopInfo.desktopDisplayName(), recommendApps.at(i));
        recommandLayout->addWidget(item);

        if (!defaultApp.isEmpty() && recommendApps.at(i).endsWith(defaultApp))
            checkItem(item);
    }

    QList<DesktopFile> otherAppList;

    foreach (const QString &f, MimesAppsManager::instance()->DesktopObjs.keys()) {
        //filter recommend apps , no show apps and no mime support apps
        const DesktopFile &app = MimesAppsManager::instance()->DesktopObjs.value(f);
        if (recommendApps.contains(f))
            continue;

        if (MimesAppsManager::instance()->DesktopObjs.value(f).isNoShow())
            continue;

        if (MimesAppsManager::instance()->DesktopObjs.value(f).desktopMimeType().isEmpty())
            continue;

        bool isSameDesktop = false;
        foreach (const DesktopFile &otherApp, otherAppList) {
            if (otherApp.desktopExec() == app.desktopExec() && otherApp.desktopLocalName() == app.desktopLocalName())
                isSameDesktop = true;
        }

        Properties desktopInfo(f, "Desktop Entry");

        const QString &customOpenDesktop = desktopInfo.value("X-DDE-File-Manager-Custom-Open").toString();

        // Filter self own desktop files for opening other types of files
        if (!customOpenDesktop.isEmpty() && customOpenDesktop != mimeType.name())
            continue;

        if (isSameDesktop)
            continue;

        otherAppList << MimesAppsManager::instance()->DesktopObjs.value(f);
        QString iconName = otherAppList.last().desktopIcon();
        OpenWithDialogListItem *item = createItem(QIcon::fromTheme(iconName), otherAppList.last().desktopDisplayName(), f);
        otherLayout->addWidget(item);

        if (!defaultApp.isEmpty() && f.endsWith(defaultApp))
            checkItem(item);
    }
}

void OpenWithDialog::checkItem(OpenWithDialogListItem *item)
{
    if (checkedItem)
        checkedItem->setChecked(false);

    item->setChecked(true);
    checkedItem = item;
}

void OpenWithDialog::useOtherApplication()
{
    const QString &filePath = QFileDialog::getOpenFileName(this);

    if (filePath.isEmpty())
        return;

    QFileInfo info(filePath);
    QString targetDesktopFileName("%1/%2-custom-open-%3.desktop");

    targetDesktopFileName = targetDesktopFileName.arg(QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation)).arg(qApp->applicationName()).arg(mimeType.name().replace("/", "-"));

    if (filePath.endsWith(".desktop")) {
        auto list = recommandLayout->parentWidget()->findChildren<OpenWithDialog *>();
        auto ret = std::any_of(list.begin(), list.end(), [filePath](const OpenWithDialog *w) {
            return w->property("app").toString() == filePath;
        });

        if (ret)
            return;

        Properties desktop(filePath, "Desktop Entry");

        if (desktop.value("MimeType").toString().isEmpty())
            return;

        if (!QFile::link(filePath, targetDesktopFileName))
            return;
    } else if (info.isExecutable()) {
        Properties desktop;

        desktop.set("Type", "Application");
        desktop.set("Name", info.fileName());
        desktop.set("Icon", "application-x-desktop");
        desktop.set("Exec", filePath);
        desktop.set("MimeType", "*/*");
        desktop.set("X-DDE-File-Manager-Custom-Open", mimeType.name());
        desktop.set("NoDisplay", "true");

        if (QFile::exists(targetDesktopFileName))
            QFile(targetDesktopFileName).remove();

        if (!desktop.save(targetDesktopFileName, "Desktop Entry"))
            return;
    }

    // remove old custom item
    for (int i = 0; i < otherLayout->count(); ++i) {
        QWidget *w = otherLayout->itemAt(i)->widget();

        if (!w)
            continue;

        if (w->property("app").toString() == targetDesktopFileName) {
            otherLayout->removeWidget(w);
            w->deleteLater();
        }
    }

    OpenWithDialogListItem *item = createItem(QIcon::fromTheme("application-x-desktop"), info.fileName(), targetDesktopFileName);

    int otherLayoutSizeHintHeight = otherLayout->sizeHint().height();
    otherLayout->addWidget(item);
    item->show();
    otherLayout->parentWidget()->setFixedHeight(otherLayout->parentWidget()->height() + otherLayout->sizeHint().height() - otherLayoutSizeHintHeight);
    checkItem(item);
}

OpenWithDialogListItem *OpenWithDialog::createItem(const QIcon &icon, const QString &name, const QString &filePath)
{
    OpenWithDialogListItem *item = new OpenWithDialogListItem(icon, name, this);

    item->setProperty("app", filePath);
    item->installEventFilter(this);

    return item;
}

void OpenWithDialog::openFileByApp()
{
    if (!checkedItem)
        return;

    const QString &app = checkedItem->property("app").toString();

    if (setToDefaultCheckBox->isChecked())
        MimesAppsManager::instance()->setDefautlAppForTypeByGio(mimeType.name(), app);
    //Todo(yanghao): open file by app

    QList<QString> apps;
    apps << app;

    if (curUrl.isValid()) {
        QList<QUrl> urlList;
        urlList << curUrl;
        if (dpfSignalDispatcher->publish(GlobalEventType::kOpenFilesByApp, 0, urlList, apps)) {
            close();
            return;
        }
    }

    if (urlList.isEmpty()) {
        close();
        return;
    }

    if (dpfSignalDispatcher->publish(GlobalEventType::kOpenFilesByApp, 0, this->urlList, apps)) {
        close();
        return;
    }
}

void OpenWithDialog::showEvent(QShowEvent *event)
{
    BaseDialog::showEvent(event);

    recommandLayout->parentWidget()->setFixedWidth(scrollArea->width());
    recommandLayout->parentWidget()->resize(recommandLayout->parentWidget()->layout()->sizeHint());
}

bool OpenWithDialog::eventFilter(QObject *obj, QEvent *event)
{
    // blumia: for scrollArea, to avoid touch screen scrolling cause window move
    if (event->type() == QEvent::MouseMove) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        if (mouseEvent->source() == Qt::MouseEventSynthesizedByQt) {
            return true;
        }
    }

    if (event->type() == QEvent::MouseButtonPress) {

        if (static_cast<QMouseEvent *>(event)->button() == Qt::LeftButton) {
            if (OpenWithDialogListItem *item = qobject_cast<OpenWithDialogListItem *>(obj))
                checkItem(item);

            return true;
        }
    }

    return false;
}

//#include "openwithdialog.moc"
