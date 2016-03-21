#include "dfileview.h"
#include "dfilesystemmodel.h"
#include "../app/global.h"
#include "fileitem.h"
#include "filemenumanager.h"
#include "dfileitemdelegate.h"
#include "fileinfo.h"

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
    connect(fileSignalManager, &FileSignalManager::childrenChanged,
            model(), &DFileSystemModel::updateChildren);
    connect(fileSignalManager, &FileSignalManager::refreshFolder,
            model(), &DFileSystemModel::refresh);
}

DFileSystemModel *DFileView::model() const
{
    return qobject_cast<DFileSystemModel*>(DListView::model());
}

DFileItemDelegate *DFileView::itemDelegate() const
{
    return qobject_cast<DFileItemDelegate*>(DListView::itemDelegate());
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

void DFileView::setColumnWidth(int column, int width)
{
    if(!m_headerView)
        return;

    m_headerView->resizeSection(logicalIndexs.value(column), width);
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

    emit currentUrlChanged(url);
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
            connect(m_headerView, &QHeaderView::sortIndicatorChanged,
                    model(), &QAbstractItemModel::sort);
        }

        for(int i = 0; i < m_headerView->count(); ++i) {
            columnRoles << model()->headerDataToRole(model()->headerData(i, m_headerView->orientation(), Qt::DisplayRole));
            logicalIndexs << i;

            if(columnRoles.last() == DFileSystemModel::FileNameRole) {
                m_headerView->resizeSection(i, width() - (m_headerView->count() - 1) * m_headerView->defaultSectionSize());
            }

            QSignalBlocker blocker(m_headerView);
            Q_UNUSED(blocker)
            m_headerView->setSortIndicator(i, model()->sortOrder());
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
    QMenu *menu;

    if (isEmptyArea(event->pos())){
        menu = m_fileMenuManager->genereteMenuByFileType("Space");
    }else{
        menu = m_fileMenuManager->genereteMenuByFileType("File");
        menu->setProperty("url", "/home");
    }

    menu->exec(mapToGlobal(event->pos()));
    menu->deleteLater();

    event->accept();
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

void DFileView::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton) {
        setDragEnabled(!isEmptyArea(event->pos()));
    } else if(event->button() == Qt::RightButton) {
        QWidget::mousePressEvent(event);

        return;
    }

    DListView::mousePressEvent(event);
}

bool DFileView::isEmptyArea(const QPoint &pos) const
{
    QModelIndex index = indexAt(pos);

    if(selectionModel()->selectedIndexes().contains(index)) {
        return false;
    } else {
        QStyleOptionViewItem option = viewOptions();

        option.rect = visualRect(index);

        const QList<QRect> &geometry_list = itemDelegate()->paintGeomertyss(option, index);

        for(const QRect &rect : geometry_list) {
            if(rect.contains(pos)) {
                return false;
            }
        }
    }

    return true;
}
