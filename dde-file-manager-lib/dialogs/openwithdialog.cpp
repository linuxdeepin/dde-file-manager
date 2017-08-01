#include "openwithdialog.h"
#include "dfileservices.h"

#include "shutil/mimesappsmanager.h"
#include "shutil/desktopfile.h"
#include "shutil/fileutils.h"

#include "app/define.h"

#include "singleton.h"

#include <dseparatorhorizontal.h>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QApplication>
#include <QDesktopWidget>
#include <QWindow>
#include <QDebug>
#include <QLabel>
#include <QScrollArea>
#include <QPainter>
#include <QCheckBox>
#include <QFileDialog>
#include <QStandardPaths>

class OpenWithDialogListItem : public QWidget
{
    Q_OBJECT

public:
    explicit OpenWithDialogListItem(const QIcon &icon, const QString &text, QWidget *parent = 0);

    void setChecked(bool checked);
    QString text() const;

protected:
    void resizeEvent(QResizeEvent *e) Q_DECL_OVERRIDE;
    void enterEvent(QEvent *e) Q_DECL_OVERRIDE;
    void leaveEvent(QEvent *e) Q_DECL_OVERRIDE;
    void paintEvent(QPaintEvent *e) Q_DECL_OVERRIDE;

private:
    QIcon m_icon;
    QLabel *m_checkLabel;
    QLabel *m_iconLabel;
    QLabel *m_label;
};

OpenWithDialogListItem::OpenWithDialogListItem(const QIcon &icon, const QString &text, QWidget *parent)
    : QWidget(parent)
    , m_icon(icon)
{
    if (m_icon.isNull())
        m_icon = QIcon::fromTheme("application-x-desktop");

    m_checkLabel = new QLabel(this);
    m_checkLabel->setFixedSize(10, 10);
    m_checkLabel->setAlignment(Qt::AlignCenter);

    m_label = new QLabel(this);
    m_label->setText(text);

    m_iconLabel = new QLabel(this);
    m_iconLabel->setAlignment(Qt::AlignCenter);

    QHBoxLayout *layout = new QHBoxLayout(this);

    layout->setContentsMargins(5, 0, 5, 0);
    layout->addWidget(m_checkLabel);
    layout->addWidget(m_iconLabel);
    layout->addWidget(m_label);

    setMouseTracking(true);
}

void OpenWithDialogListItem::setChecked(bool checked)
{
    if (checked) {
        m_checkLabel->setPixmap(QPixmap(":/icons/images/light/select.png"));
    } else {
        m_checkLabel->setPixmap(QPixmap());
    }
}

QString OpenWithDialogListItem::text() const
{
    return m_label->text();
}

void OpenWithDialogListItem::resizeEvent(QResizeEvent *e)
{
    QWidget::resizeEvent(e);

    m_iconLabel->setFixedSize(e->size().height() - 20, e->size().height() - 20);
    m_iconLabel->setPixmap(m_icon.pixmap(m_iconLabel->size()));
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
    pa.fillPath(path, QColor(0, 0, 0, 0.05 * 255));
}

class OpenWithDialogListSparerItem : public QWidget
{
public:
    explicit OpenWithDialogListSparerItem(const QString &title, QWidget *parent = 0);

private:
    DSeparatorHorizontal *m_separator;
    QLabel *m_title;
};

OpenWithDialogListSparerItem::OpenWithDialogListSparerItem(const QString &title, QWidget *parent)
    : QWidget(parent)
    , m_separator(new DSeparatorHorizontal(this))
    , m_title(new QLabel(title, this))
{
    m_title->setStyleSheet("QLabel{color: rgba(32, 32, 32, 0.4); font-size: 18px;}");

    QVBoxLayout *layout = new QVBoxLayout(this);

    layout->addWidget(m_separator);
    layout->addWidget(m_title);
    layout->setContentsMargins(20, 0, 20, 0);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

OpenWithDialog::OpenWithDialog(const DUrl &url, QWidget *parent) :
    BaseDialog(parent)
{
    m_url = url;
    setWindowFlags(windowFlags()
                           &~ Qt::WindowMaximizeButtonHint
                           &~ Qt::WindowMinimizeButtonHint
                           &~ Qt::WindowSystemMenuHint);
    initUI();
    initConnect();
    initData();
}

OpenWithDialog::~OpenWithDialog()
{

}

void OpenWithDialog::initUI()
{
    setFixedSize(710, 450);
    setTitle(tr("Open with"));

    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setStyleSheet("QScrollArea{background: transparent;} QWidget#contentWidget{background: transparent;}");
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setFrameShape(QFrame::NoFrame);

    QWidget *content_widget = new QWidget;

    content_widget->setObjectName("contentWidget");
    content_widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    m_scrollArea->setWidget(content_widget);

    m_recommandLayout = new DFlowLayout;
    m_otherLayout = new DFlowLayout;

    m_openFileChooseButton = new DLinkButton(tr("Add other programs"));
    m_setToDefaultCheckBox = new QCheckBox(tr("Set as default"));
    m_setToDefaultCheckBox->setChecked(true);
    m_cancelButton = new QPushButton(tr("Cancel"));
    m_chooseButton = new QPushButton(tr("Choose"));

    QVBoxLayout* content_layout = new QVBoxLayout;
    content_layout->setContentsMargins(10, 0, 10, 0);
    content_layout->addWidget(new OpenWithDialogListSparerItem(tr("Recommended Applications"), this));
    content_layout->addLayout(m_recommandLayout);
    content_layout->addWidget(new OpenWithDialogListSparerItem(tr("Other Applications"), this));
    content_layout->addLayout(m_otherLayout);
    content_layout->addStretch();

    content_widget->setLayout(content_layout);

    QHBoxLayout* buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(m_openFileChooseButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_setToDefaultCheckBox);
    buttonLayout->addSpacing(20);
    buttonLayout->addWidget(m_cancelButton);
    buttonLayout->addWidget(m_chooseButton);
    buttonLayout->setContentsMargins(10, 0, 10, 0);

    QVBoxLayout *main_layout= new QVBoxLayout(this);
    QVBoxLayout *bottom_layout = new QVBoxLayout;

    bottom_layout->addWidget(new DSeparatorHorizontal(this));
    bottom_layout->addSpacing(5);
    bottom_layout->addLayout(buttonLayout);
    bottom_layout->setContentsMargins(10, 0, 10, 0);

    main_layout->addWidget(m_scrollArea);
    main_layout->addLayout(bottom_layout);
    main_layout->setContentsMargins(0, 35, 0, 10);
}

void OpenWithDialog::initConnect()
{
    connect(m_cancelButton, &QPushButton::clicked, this, &OpenWithDialog::close);
    connect(m_chooseButton, &QPushButton::clicked, this, &OpenWithDialog::openFileByApp);
    connect(m_openFileChooseButton, &DLinkButton::clicked, this, &OpenWithDialog::useOtherApplication);
}

void OpenWithDialog::initData()
{
    const DAbstractFileInfoPointer &file_info = DFileService::instance()->createFileInfo(this, m_url);

    if (!file_info)
        return;

    QMimeDatabase db;

    m_mimeType = db.mimeTypeForName(FileUtils::getMimeTypeByGIO(file_info->redirectedFileUrl().toString()));

    if (file_info->isDesktopFile())
        m_setToDefaultCheckBox->hide();

    const QString &default_app = mimeAppsManager->getDefaultAppByMimeType(m_mimeType);
    const QStringList &recommendApps = mimeAppsManager->getRecommendedAppsByQio(m_mimeType);

    for (int i = 0; i < recommendApps.count(); ++i) {
        const DesktopFile &desktop_info = mimeAppsManager->DesktopObjs.value(recommendApps.at(i));

        OpenWithDialogListItem *item = createItem(QIcon::fromTheme(desktop_info.getIcon()), desktop_info.getLocalName(), recommendApps.at(i));
        m_recommandLayout->addWidget(item);

        if (!default_app.isEmpty() && recommendApps.at(i).endsWith(default_app))
            checkItem(item);
    }

    QList<DesktopFile> other_app_list;

    foreach (const QString& f, mimeAppsManager->DesktopObjs.keys()) {
        //filter recommend apps , no show apps and no mime support apps
        const DesktopFile& app = mimeAppsManager->DesktopObjs.value(f);
        if(recommendApps.contains(f))
            continue;

        if(mimeAppsManager->DesktopObjs.value(f).getNoShow())
            continue;

        if(mimeAppsManager->DesktopObjs.value(f).getMimeType().isEmpty())
            continue;

        bool isSameDesktop = false;
        foreach (const DesktopFile& otherApp, other_app_list) {
            if(otherApp.getExec() == app.getExec() && otherApp.getLocalName() == app.getLocalName())
                isSameDesktop = true;
        }

        Properties desktop_info(f, "Desktop Entry");

        const QString &custom_open_desktop = desktop_info.value("X-DDE-File-Manager-Custom-Open").toString();

        // Filter self own desktop files for opening other types of files
        if (!custom_open_desktop.isEmpty() && custom_open_desktop != m_mimeType.name())
            continue;

        if (isSameDesktop)
            continue;

        other_app_list << mimeAppsManager->DesktopObjs.value(f);
        QString iconName = other_app_list.last().getIcon();
        OpenWithDialogListItem *item = createItem(QIcon::fromTheme(iconName), other_app_list.last().getLocalName(), f);
        m_otherLayout->addWidget(item);

        if (!default_app.isEmpty() && f.endsWith(default_app))
            checkItem(item);
    }
}

void OpenWithDialog::checkItem(OpenWithDialogListItem *item)
{
    if (m_checkedItem)
        m_checkedItem->setChecked(false);

    item->setChecked(true);
    m_checkedItem = item;
}

void OpenWithDialog::useOtherApplication()
{
    const QString &file_path = QFileDialog::getOpenFileName(this);

    if (file_path.isEmpty())
        return;

    QFileInfo info(file_path);
    QString target_desktop_file_name("%1/%2-custom-open-%3.desktop");

    target_desktop_file_name = target_desktop_file_name.arg(QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation)).arg(qApp->applicationName()).arg(m_mimeType.name().replace("/", "-"));

    if (file_path.endsWith(".desktop")) {
        for (const OpenWithDialog *w : m_recommandLayout->parentWidget()->findChildren<OpenWithDialog*>()) {
            if (w->property("app").toString() == file_path)
                return;
        }

        Properties desktop(file_path, "Desktop Entry");

        if (desktop.value("MimeType").toString().isEmpty())
            return;

        if (!QFile::link(file_path, target_desktop_file_name))
            return;
    } else if (info.isExecutable()) {
        Properties desktop;

        desktop.set("Type", "Application");
        desktop.set("Name", info.fileName());
        desktop.set("Icon", "application-x-desktop");
        desktop.set("Exec", file_path);
        desktop.set("MimeType", "*/*");
        desktop.set("X-DDE-File-Manager-Custom-Open", m_mimeType.name());

        if (QFile::exists(target_desktop_file_name))
            QFile(target_desktop_file_name).remove();

        if (!desktop.save(target_desktop_file_name, "Desktop Entry"))
            return;
    }

    // remove old custom item
    for (int i = 0; i < m_otherLayout->count(); ++i) {
        QWidget *w = m_otherLayout->itemAt(i)->widget();

        if (!w)
            continue;

        if (w->property("app").toString() == target_desktop_file_name) {
            m_otherLayout->removeWidget(w);
            w->deleteLater();
        }
    }

    OpenWithDialogListItem *item = createItem(QIcon::fromTheme("application-x-desktop"), info.fileName(), target_desktop_file_name);

    int other_layout_sizeHint_height = m_otherLayout->sizeHint().height();
    m_otherLayout->addWidget(item);
    item->show();
    m_otherLayout->parentWidget()->setFixedHeight(m_otherLayout->parentWidget()->height() + m_otherLayout->sizeHint().height() - other_layout_sizeHint_height);
    checkItem(item);
}

OpenWithDialogListItem *OpenWithDialog::createItem(const QIcon &icon, const QString &name, const QString &filePath)
{
    OpenWithDialogListItem *item = new OpenWithDialogListItem(icon, name, this);

    item->setProperty("app", filePath);
    item->setFixedSize(220, 50);
    item->installEventFilter(this);

    return item;
}

void OpenWithDialog::openFileByApp()
{
    if (!m_checkedItem)
        return;

    const QString &app = m_checkedItem->property("app").toString();

    if (m_setToDefaultCheckBox->isChecked())
        mimeAppsManager->setDefautlAppForTypeByGio(m_mimeType.name(), app);

    if (DFileService::instance()->openFileByApp(this, app, m_url))
        close();
}

void OpenWithDialog::showEvent(QShowEvent *event)
{
    BaseDialog::showEvent(event);

    m_recommandLayout->parentWidget()->setFixedWidth(m_scrollArea->width());
    m_recommandLayout->parentWidget()->resize(m_recommandLayout->parentWidget()->layout()->sizeHint());
}

bool OpenWithDialog::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        if (static_cast<QMouseEvent*>(event)->button() == Qt::LeftButton) {
            if (OpenWithDialogListItem *item = qobject_cast<OpenWithDialogListItem*>(obj))
                checkItem(item);

            return true;
        }
    }

    return false;
}

#include "openwithdialog.moc"
