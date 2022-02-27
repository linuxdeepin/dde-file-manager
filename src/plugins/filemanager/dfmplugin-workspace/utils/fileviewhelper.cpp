/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: zhangsheng<zhangsheng@uniontech.com>
 *             liuyangming<liuyangming@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "fileviewhelper.h"
#include "models/fileviewmodel.h"
#include "models/filesortfilterproxymodel.h"
#include "views/fileview.h"
#include "views/baseitemdelegate.h"
#include "views/iconitemeditor.h"
#include "utils/workspacehelper.h"
#include "utils/fileoperaterhelper.h"

#include "services/common/delegate/delegateservice.h"

#include "dfm-base/base/application/application.h"
#include "dfm-base/base/application/settings.h"
#include "dfm-base/utils/clipboard.h"
#include "dfm-base/utils/windowutils.h"

#include <DApplication>

#include <QLineEdit>
#include <QTextEdit>
#include <QAbstractItemView>
#include <QTimer>

DWIDGET_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DPWORKSPACE_USE_NAMESPACE

FileViewHelper::FileViewHelper(FileView *parent)
    : QObject(parent)
{
    init();
}

FileViewHelper::~FileViewHelper()
{
}

FileView *FileViewHelper::parent() const
{
    return qobject_cast<FileView *>(QObject::parent());
}

bool FileViewHelper::isTransparent(const QModelIndex &index) const
{
    AbstractFileInfoPointer file = fileInfo(parent()->proxyModel()->mapToSource(index));
    if (!file.get())
        return false;

    //  cutting
    if (ClipBoard::instance()->clipboardAction() == ClipBoard::kCutAction) {

        if (ClipBoard::instance()->clipboardFileUrlList().contains(file->url())) {
            qDebug() << index << file->url();
            return true;
        }

        // the linked file only judges the URL, not the inode,
        // because the inode of the linked file is consistent with that of the source file
        if (!file->isSymLink()) {
            if (ClipBoard::instance()->clipboardFileInodeList().contains(file->inode()))
                return true;
        }
    }

    if (delegateServIns->isTransparent(file->url()))
        return true;

    // Todo(yanghao)

    return false;
}

const AbstractFileInfoPointer FileViewHelper::fileInfo(const QModelIndex &index) const
{
    return parent()->model()->fileInfo(index);
}

QMargins FileViewHelper::fileViewViewportMargins() const
{
    return parent()->viewportMargins();
}

QWidget *FileViewHelper::indexWidget(const QModelIndex &index) const
{
    return parent()->indexWidget(index);
}

int FileViewHelper::selectedIndexsCount() const
{
    return parent()->selectionModel()->selectedIndexes().count();
}

bool FileViewHelper::isSelected(const QModelIndex &index) const
{
    return parent()->isSelected(index);
}

bool FileViewHelper::isDropTarget(const QModelIndex &index) const
{
    // Todo(yanghao): isDropTarget
    return false;
}

void FileViewHelper::initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const
{
    if (isSelected(index) && index.flags().testFlag(Qt::ItemFlag::ItemIsSelectable)) {
        option->state |= QStyle::State_Selected;
    } else {
        option->state &= QStyle::StateFlag(~QStyle::State_Selected);
    }

    QPalette appPalette = QGuiApplication::palette();

    auto setcolor1 = [](QPalette &p1, QPalette &p2, QPalette::ColorRole role) {
        p1.setColor(role, p2.color(role));
    };

    auto setcolor2 = [](QPalette &p1, QPalette &p2, QPalette::ColorGroup group, QPalette::ColorRole role) {
        p1.setColor(group, role, p2.color(group, role));
    };

    setcolor1(option->palette, appPalette, QPalette::Text);
    setcolor2(option->palette, appPalette, QPalette::Disabled, QPalette::Text);

    if ((option->state & QStyle::State_Selected) && option->showDecorationSelected) {
        setcolor2(option->palette, appPalette, QPalette::Inactive, QPalette::Text);
    } else {
        setcolor2(option->palette, appPalette, QPalette::Inactive, QPalette::Text);
    }

    option->palette.setColor(QPalette::BrightText, Qt::white);

    setcolor1(option->palette, appPalette, QPalette::Shadow);

    if ((option->state & QStyle::State_HasFocus) && option->showDecorationSelected && selectedIndexsCount() > 1) {
        setcolor2(option->palette, appPalette, QPalette::Current, QPalette::Background);
    } else {
        setcolor2(option->palette, appPalette, QPalette::Normal, QPalette::Background);
    }

    bool transp = isTransparent(index);
    option->backgroundBrush = appPalette.brush(transp ? QPalette::Inactive : QPalette::Current, QPalette::Background);

    option->textElideMode = Qt::ElideLeft;
}

void FileViewHelper::updateGeometries()
{
    parent()->updateGeometry();
}

void FileViewHelper::keyboardSearch(const QString &search)
{
    if (search.isEmpty())
        return;
    auto key = search.toLocal8Bit().at(0);

    keyboardSearchKeys.append(key);
    bool reverseOrder = WindowUtils::keyShiftIsPressed();
    const QModelIndex &currentIndex = parent()->currentIndex();

    QModelIndex index = findIndex(keyboardSearchKeys, true, currentIndex.row(), reverseOrder, !keyboardSearchTimer->isActive());

    if (index.isValid()) {
        parent()->setCurrentIndex(index);
        parent()->scrollTo(index, reverseOrder ? QAbstractItemView::PositionAtBottom : QAbstractItemView::PositionAtTop);
    }
    keyboardSearchTimer->start();
}

QModelIndex FileViewHelper::findIndex(const QByteArray &keys, bool matchStart, int current, bool reverseOrder, bool excludeCurrent) const
{
    int rowCount = parent()->proxyModel()->rowCount(parent()->rootIndex());

    if (rowCount == 0)
        return QModelIndex();

    for (int i = excludeCurrent ? 1 : 0; i <= rowCount; ++i) {
        int row = reverseOrder ? rowCount + current - i : current + i;

        row = row % rowCount;

        if (excludeCurrent && row == current) {
            continue;
        }

        const QModelIndex &index = parent()->proxyModel()->index(row, 0, parent()->rootIndex());
        const QString &pinyinName = parent()->proxyModel()->data(index, FileViewItem::kItemFilePinyinNameRole).toString();
        if (matchStart ? pinyinName.startsWith(keys, Qt::CaseInsensitive)
                       : pinyinName.contains(keys, Qt::CaseInsensitive)) {
            return index;
        }
    }

    return QModelIndex();
}

bool FileViewHelper::isEmptyArea(const QPoint &pos)
{
    const QModelIndex &index = parent()->indexAt(pos);

    return !index.isValid();
}

int FileViewHelper::caculateListItemIndex(const QSize &itemSize, const QPoint &pos)
{
    if (pos.y() % (itemSize.height() + kListViewSpacing * 2) < kListViewSpacing)
        return -1;

    int itemHeight = itemSize.height() + kListViewSpacing * 2;
    return pos.y() / itemHeight;
}

int FileViewHelper::caculateIconItemIndex(const FileView *view, const QSize &itemSize, const QPoint &pos)
{
    int itemHeight = itemSize.height() + kIconViewSpacing * 2;

    if (pos.y() % itemHeight < (kIconViewSpacing + kIconModeColumnPadding)
        || pos.y() % itemHeight > (itemHeight - kIconModeColumnPadding))
        return -1;

    int itemWidth = itemSize.width() + kIconViewSpacing * 2;

    if (pos.x() % itemWidth <= (kIconViewSpacing + kIconModeColumnPadding)
        || pos.x() % itemWidth > (itemHeight - kIconModeColumnPadding))
        return -1;

    int columnIndex = pos.x() / itemWidth;
    int contentWidth = view->maximumViewportSize().width();
    int columnCount = qMax((contentWidth - 1) / itemWidth, 1);

    if (columnIndex >= columnCount)
        return -1;

    int rowIndex = pos.y() / (itemSize.height() + kIconViewSpacing * 2);
    return rowIndex * columnCount + columnIndex;
}

void FileViewHelper::handleCommitData(QWidget *editor) const
{
    if (!editor) {
        return;
    }

    const auto &index = parent()->proxyModel()->mapToSource(itemDelegate()->editingIndex());
    const AbstractFileInfoPointer &fileInfo = model()->fileInfo(index);

    if (!fileInfo) {
        return;
    }

    QLineEdit *lineEdit = qobject_cast<QLineEdit *>(editor);
    IconItemEditor *iconEdit = qobject_cast<IconItemEditor *>(editor);

    QString newFileName = lineEdit ? lineEdit->text() : iconEdit ? iconEdit->getTextEdit()->toPlainText() : "";

    if (newFileName.isEmpty()) {
        return;
    }

    QString suffix { editor->property("_d_whether_show_suffix").toString() };

    if (!suffix.isEmpty()) {
        newFileName += QString { "." };
        newFileName += suffix;
    } else if (Application::genericObtuselySetting()->value("FileName", "non-allowableEmptyCharactersOfEnd").toBool()) {
        //保留文件名称中的空格符号
        //newFileName = newFileName.trimmed();
        if (newFileName.isEmpty()) {
            return;
        }
    }

    if (fileInfo->fileName() == newFileName) {
        return;
    }

    QUrl oldUrl = fileInfo->url();
    QUrl newUrl = fileInfo->getUrlByNewFileName(newFileName);
    FileOperaterHelperIns->renameFile(this->parent(), oldUrl, newUrl);
}

void FileViewHelper::clipboardDataChanged()
{
    if (itemDelegate()) {
        for (const QModelIndex &index : itemDelegate()->hasWidgetIndexs()) {
            QWidget *item = indexWidget(index);

            if (item) {
                item->setProperty("opacity", isTransparent(index) ? 0.3 : 1);
            }
        }
    }

    parent()->update();
}

void FileViewHelper::clearSearchKey()
{
    keyboardSearchKeys.clear();
}

void FileViewHelper::init()
{
    keyboardSearchTimer = new QTimer(this);
    keyboardSearchTimer->setSingleShot(true);
    keyboardSearchTimer->setInterval(200);
    connect(keyboardSearchTimer, &QTimer::timeout, this, &FileViewHelper::clearSearchKey);

    connect(qApp, &DApplication::iconThemeChanged, parent(), static_cast<void (QWidget::*)()>(&QWidget::update));
    connect(ClipBoard::instance(), &ClipBoard::clipboardDataChanged, this, &FileViewHelper::clipboardDataChanged);
    connect(parent(), &FileView::triggerEdit, this, &FileViewHelper::triggerEdit);
}

FileViewModel *FileViewHelper::model() const
{
    return parent()->model();
}

BaseItemDelegate *FileViewHelper::itemDelegate() const
{
    return parent()->itemDelegate();
}
