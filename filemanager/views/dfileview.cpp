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
    setEditTriggers(QListView::EditKeyPressed | QListView::SelectedClicked);
}

void DFileView::initDelegate()
{
    setItemDelegate(new DFileItemDelegate(this));
}

void DFileView::initModel()
{
    setModel(new DFileSystemModel(this));
    setRootIndex(model()->setRootPath(QDir::currentPath()));
}

void DFileView::initConnects()
{
    connect(this, &DFileView::doubleClicked,
            this, [this](const QModelIndex &index) {
        if(model()->hasChildren(index)){
            emit fileSignalManager->currentUrlChanged(model()->getUrlByIndex(index));
        } else {
            QDesktopServices::openUrl(QUrl::fromLocalFile(model()->getUrlByIndex(index)));
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

bool DFileView::isIconViewMode()
{
    return orientation() == Qt::Vertical && isWrapping();
}

int DFileView::columnWidth(int column) const
{
    return m_headerView ? m_headerView->sectionSize(logicalIndexs.value(column)) : 100;
}

int DFileView::columnCount() const
{
    return m_headerView ? m_headerView->count() : 1;
}

QList<int> DFileView::columnRoleList() const
{
    return columnRoles;
}

void DFileView::cd(const QString &url)
{
    if(currentUrl() == url)
        return;

    qDebug() << "cd: current url:" << currentUrl() << "to url:" << url;

    QModelIndex index = model()->index(url);

    if(!index.isValid())
        index = model()->setRootPath(url);

    setRootIndex(index);

    currentUrlChanged(url);
}

void DFileView::switchListMode()
{
    if(isIconViewMode()) {
        if(!m_headerView) {
            m_headerView = new QHeaderView(Qt::Horizontal);
            m_headerView->setModel(model());
            m_headerView->setSectionsMovable(true);
            m_headerView->setHighlightSections(true);
            m_headerView->setSectionsClickable(true);
            m_headerView->setSortIndicatorShown(true);
            m_headerView->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            m_headerView->setDefaultSectionSize(120);
            m_headerView->setMinimumSectionSize(120);

            if(selectionModel()) {
                m_headerView->setSelectionModel(selectionModel());
            }

            connect(m_headerView, &QHeaderView::sectionResized,
                    this, static_cast<void (DFileView::*)()>(&DFileView::update));
            connect(m_headerView, &QHeaderView::sectionMoved,
                    this, &DFileView::moveColumnRole);
        }

        for(int i = 0; i < m_headerView->count(); ++i) {
            columnRoles << model()->headerDataToRole(model()->headerData(i, m_headerView->orientation(), Qt::DisplayRole));
            logicalIndexs << i;
        }

        addHeaderWidget(m_headerView);

        setIconSize(QSize(30, 30));
        setOrientation(QListView::TopToBottom, false);
        setSpacing(0);
    } else {
        if(m_headerView) {
            removeHeaderWidget(0);

            m_headerView = Q_NULLPTR;
        }

        columnRoles.clear();

        setIconSize(QSize(60, 60));
        setOrientation(QListView::LeftToRight, true);
        setSpacing(10);
    }
}

void DFileView::moveColumnRole(int /*logicalIndex*/, int oldVisualIndex, int newVisualIndex)
{
    columnRoles.move(oldVisualIndex, newVisualIndex);
    logicalIndexs.move(oldVisualIndex, newVisualIndex);

    update();
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

    DFileView::contextMenuEvent(event);
}

void DFileView::wheelEvent(QWheelEvent *event)
{
    if(isIconViewMode() && ctrlIsPressed) {
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
