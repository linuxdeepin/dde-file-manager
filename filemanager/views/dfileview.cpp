#include "dfileview.h"
#include "dfilesystemmodel.h"
#include "../app/global.h"
#include "fileitem.h"
#include "filemenumanager.h"
#include "dfileitemdelegate.h"

#include <QPushButton>
#include <QMenu>
#include <QWheelEvent>
#include <QDesktopServices>

DFileView::DFileView(QWidget *parent) : DListView(parent)
{
    initUI();
    initDelegate();
    initModel();
    initConnects();
}

DFileView::~DFileView()
{

}

void DFileView::initUI()
{
    m_fileMenuManager = new FileMenuManager(this);
    setSpacing(10);
    setResizeMode(QListView::Adjust);
    setOrientation(QListView::LeftToRight, true);
    setStyleSheet("background: white");
    setIconSize(QSize(60, 60));

    setTextElideMode(Qt::ElideMiddle);
    setDragEnabled(true);
    setDragDropMode(QAbstractItemView::DragDrop);
    setDefaultDropAction(Qt::MoveAction);
    setDropIndicatorShown(true);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setSelectionBehavior(QAbstractItemView::SelectItems);
    setSelectionRectVisible(true);
    setEditTriggers(QListView::EditKeyPressed);
}

void DFileView::initDelegate()
{
    setItemDelegate(new DFileItemDelegate(this));
}

void DFileView::initModel()
{
    setModel(new DFileSystemModel(this));
    setRootIndex(model()->setRootPath(QUrl::fromLocalFile("/").toString(QUrl::EncodeUnicode)));
}

void DFileView::initConnects()
{
    connect(this, &DFileView::doubleClicked,
            this, [this](const QModelIndex &index) {
        if(model()->hasChildren(index)){
            emit fileSignalManager->currentUrlChanged(model()->getUrlByIndex(index));
        } else {
            QDesktopServices::openUrl(QUrl(model()->getUrlByIndex(index)));
        }
    });
    connect(fileSignalManager, &FileSignalManager::currentUrlChanged,
            this, &DFileView::cd);
    connect(fileSignalManager, &FileSignalManager::iconChanged,
            model(), &DFileSystemModel::updateIcon);
    connect(fileSignalManager, &FileSignalManager::childrenChanged,
            model(), &DFileSystemModel::updateChildren);
    connect(fileSignalManager, &FileSignalManager::refreshFolder,
            model(), &DFileSystemModel::refresh);
}

DFileSystemModel *DFileView::model() const
{
    return qobject_cast<DFileSystemModel*>(DListView::model());
}

QString DFileView::currentUrl() const
{
    return model()->getUrlByIndex(rootIndex());
}

void DFileView::cd(const QString &url)
{
    qDebug() << "cd: current url:" << currentUrl() << "to url:" << url;

    QModelIndex index = model()->index(url);

    if(!index.isValid())
        index = model()->setRootPath(url);

    setRootIndex(index);
}

void DFileView::switchListMode()
{
    if(isWrapping()) {
        setOrientation(QListView::TopToBottom, false);
    } else {
        setOrientation(QListView::LeftToRight, true);
    }
}

void DFileView::contextMenuEvent(QContextMenuEvent *event)
{
    qDebug() << event << indexAt(event->pos());
    QModelIndex index = indexAt(event->pos());
    QMenu* menu;
    if (index.isValid()){
        menu = m_fileMenuManager->genereteMenuByFileType("File");
        menu->setProperty("url", "/home");
    }else{
        menu = m_fileMenuManager->genereteMenuByFileType("Space");
    }
    menu->exec(mapToGlobal(event->pos()));
    menu->deleteLater();
    menu->deleteLater();
}

void DFileView::wheelEvent(QWheelEvent *event)
{
    if(ctrlIsPressed) {
        if(event->angleDelta().y() > 0) {
            setIconSize(iconSize() * 1.1);
        } else {
            setIconSize(iconSize() * 0.9);
        }
    }

    DListView::wheelEvent(event);
}

void DFileView::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Control) {
        ctrlIsPressed = true;
    }

    DListView::keyPressEvent(event);
}

void DFileView::keyReleaseEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Control) {
        ctrlIsPressed = false;
    }

    DListView::keyReleaseEvent(event);
}

void DFileView::showEvent(QShowEvent *event)
{
    DListView::showEvent(event);

    setFocus();
}
