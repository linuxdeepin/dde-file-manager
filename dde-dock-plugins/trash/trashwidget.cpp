
#include "constants.h"
#include "trashwidget.h"

#include <QPainter>
#include <QIcon>
#include <QApplication>
#include <QDragEnterEvent>
#include <QJsonDocument>

DWIDGET_USE_NAMESPACE

TrashWidget::TrashWidget(QWidget *parent)
    : QWidget(parent),

      m_popupApplet(new PopupControlWidget(this))
{
//    QIcon::setThemeName("deepin");

    m_popupApplet->setVisible(false);

    connect(m_popupApplet, &PopupControlWidget::emptyChanged, this, &TrashWidget::updateIcon);

    updateIcon();
    setAcceptDrops(true);
}

QWidget *TrashWidget::popupApplet()
{
    return m_popupApplet;
}

QSize TrashWidget::sizeHint() const
{
    return QSize(26, 26);
}

const QString TrashWidget::contextMenu() const
{
    QList<QVariant> items;
    items.reserve(2);

    QMap<QString, QVariant> open;
    open["itemId"] = "open";
    open["itemText"] = tr("Open");
    open["isActive"] = true;
    items.push_back(open);

    if (!m_popupApplet->empty())
    {
        QMap<QString, QVariant> empty;
        empty["itemId"] = "empty";
        empty["itemText"] = tr("Empty");
        empty["isActive"] = true;
        items.push_back(empty);
    }

    QMap<QString, QVariant> menu;
    menu["items"] = items;
    menu["checkableMenu"] = false;
    menu["singleCheck"] = false;

    return QJsonDocument::fromVariant(menu).toJson();
}

int TrashWidget::trashItemCount() const
{
    return m_popupApplet->trashItems();
}

void TrashWidget::invokeMenuItem(const QString &menuId, const bool checked)
{
    Q_UNUSED(checked);

    if (menuId == "open")
        m_popupApplet->openTrashFloder();
    else if (menuId == "empty")
        m_popupApplet->clearTrashFloder();
}

void TrashWidget::dragEnterEvent(QDragEnterEvent *e)
{
    if (e->mimeData()->hasFormat("RequestDock")) {
        // accept prevent the event from being propgated to the dock main panel
        // which also takes drag event;
        e->accept();

        if (!e->mimeData()->hasFormat("Removable")) {
            // show the forbit dropping cursor.
            e->setDropAction(Qt::IgnoreAction);
        }

        return;
    }

    if (e->mimeData()->hasFormat("text/uri-list"))
        return e->accept();
}

void TrashWidget::dropEvent(QDropEvent *e)
{
    if (e->mimeData()->hasFormat("RequestDock"))
        return removeApp(e->mimeData()->data("AppKey"));

    if (e->mimeData()->hasFormat("text/uri-list"))
    {
        const QMimeData *mime = e->mimeData();
        for (auto url : mime->urls())
            moveToTrash(url);
    }
}

void TrashWidget::paintEvent(QPaintEvent *e)
{
    QWidget::paintEvent(e);

    QPainter painter(this);
    painter.drawPixmap(rect().center() - m_icon.rect().center(), m_icon);
}

void TrashWidget::resizeEvent(QResizeEvent *e)
{
    QWidget::resizeEvent(e);

    updateIcon();
}

void TrashWidget::mousePressEvent(QMouseEvent *e)
{
    const QPoint dis = e->pos() - rect().center();
    if (e->button() != Qt::RightButton || dis.manhattanLength() > std::min(width(), height()) * 0.8 * 0.5)
        return QWidget::mousePressEvent(e);

    emit requestContextMenu();
}

void TrashWidget::updateIcon()
{
    const Dock::DisplayMode displayMode = qApp->property(PROP_DISPLAY_MODE).value<Dock::DisplayMode>();

    QString iconString = "user-trash";
    if (!m_popupApplet->empty())
        iconString.append("-full");
    if (displayMode == Dock::Efficient)
        iconString.append("-symbolic");

    const int size = displayMode == Dock::Fashion ? std::min(width(), height()) * 0.8 : 16;
    QIcon icon = QIcon::fromTheme(iconString);
    m_icon = icon.pixmap(size, size);

    update();
}

void TrashWidget::removeApp(const QString &appKey)
{
    const QString cmd("dbus-send --print-reply --dest=com.deepin.dde.Launcher /com/deepin/dde/Launcher com.deepin.dde.Launcher.UninstallApp string:\"" + appKey + "\"");

    QProcess *proc = new QProcess;
    proc->start(cmd);
    proc->waitForFinished();

    proc->deleteLater();
}

void TrashWidget::moveToTrash(const QUrl &url)
{
    const QFileInfo info = url.toLocalFile();

    QProcess::startDetached("gvfs-trash", QStringList() << "-f" << info.absoluteFilePath());

//    QDir trashDir(m_popupApplet->trashDir() + "/files");
//    if (!trashDir.exists())
//        trashDir.mkpath(".");

//    QDir().rename(info.absoluteFilePath(), trashDir.absoluteFilePath(info.fileName()));
}
