// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fileviewhelper.h"
#include "models/fileviewmodel.h"
#include "views/fileview.h"
#include "views/baseitemdelegate.h"
#include "views/iconitemeditor.h"
#include "views/listitemeditor.h"
#include "utils/workspacehelper.h"
#include "utils/fileoperatorhelper.h"
#include "events/workspaceeventsequence.h"

#include <dfm-base/base/application/application.h>
#include <dfm-base/base/application/settings.h>
#include <dfm-base/utils/clipboard.h>
#include <dfm-base/utils/windowutils.h>
#include <dfm-base/utils/universalutils.h>

#include <dfm-framework/event/event.h>

#include <DApplication>
#ifdef DTKWIDGET_CLASS_DSizeMode
#    include <DSizeMode>
#endif

#include <QTextEdit>
#include <QAbstractItemView>
#include <QTimer>

Q_DECLARE_METATYPE(QList<QUrl> *)

namespace dfmplugin_workspace {
const char *const kEidtorShowSuffix = "_d_whether_show_suffix";
}

DWIDGET_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DFMGLOBAL_USE_NAMESPACE
using namespace dfmplugin_workspace;

FileViewHelper::FileViewHelper(FileView *parent)
    : QObject(parent)
{
    init();
    fmDebug() << "FileViewHelper initialization completed";
}

FileViewHelper::~FileViewHelper()
{
    fmDebug() << "FileViewHelper destructor called";
}

FileView *FileViewHelper::parent() const
{
    return qobject_cast<FileView *>(QObject::parent());
}

bool FileViewHelper::isTransparent(const QModelIndex &index) const
{
    FileInfoPointer file = fileInfo(index);
    if (!file.get()) {
        return false;
    }

    TransparentStatus status = TransparentStatus::kDefault;
    if (WorkspaceEventSequence::instance()->doCheckTransparent(file->urlOf(UrlInfoType::kUrl), &status)) {
        switch (status) {
        case TransparentStatus::kTransparent:
            return true;
        case TransparentStatus::kUntransparent:
            return false;
        default:
            break;
        }
    }

    //  cutting

    if (ClipBoard::instance()->clipboardAction() == ClipBoard::kCutAction) {
        QUrl localUrl = file->urlOf(UrlInfoType::kUrl);
        auto cutUrls = ClipBoard::instance()->clipboardFileUrlList();
        if (cutUrls.contains(localUrl)) {
            fmDebug() << "File is transparent due to cut operation:" << localUrl.toString();
            return true;
        }

        if (file->canAttributes(CanableInfoType::kCanRedirectionFileUrl))
            return cutUrls.contains(QUrl::fromLocalFile(file->pathOf(PathInfoType::kAbsoluteFilePath)));
    }

    return false;
}

const FileInfoPointer FileViewHelper::fileInfo(const QModelIndex &index) const
{
    if (!parent()->isVerticalScrollBarSliderDragging())
        index.data(kItemCreateFileInfoRole);

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
    // When using the FileView selection model
    // please be careful not to call it directly.
    // It needs to be strongly converted, otherwise it will call the QT native selection model
    // causing data errors and affecting performance
    return parent()->selectedIndexCount();
}

bool FileViewHelper::isSelected(const QModelIndex &index) const
{
    return parent()->isSelected(index);
}

bool FileViewHelper::isDropTarget(const QModelIndex &index) const
{
    return parent()->isDragTarget(index);
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
        setcolor2(option->palette, appPalette, QPalette::Current, QPalette::Window);
    } else {
        setcolor2(option->palette, appPalette, QPalette::Normal, QPalette::Window);
    }

    bool transp = isTransparent(index);
    option->backgroundBrush = appPalette.brush(transp ? QPalette::Inactive : QPalette::Current, QPalette::Window);

    option->textElideMode = Qt::ElideLeft;
}

void FileViewHelper::updateGeometries()
{
    parent()->updateGeometry();
}

void FileViewHelper::keyboardSearch(const QString &search)
{
    if (search.isEmpty()) {
        fmDebug() << "Empty search string provided for keyboard search";
        return;
    }

    fmDebug() << "Keyboard search initiated with string:" << search;

    const QByteArray &key = search.toLocal8Bit();

    keyboardSearchKeys.append(key.at(0));
    bool reverseOrder = WindowUtils::keyShiftIsPressed();
    const QModelIndex &currentIndex = parent()->currentIndex();

    fmDebug() << "Search keys:" << keyboardSearchKeys << "reverse order:" << reverseOrder << "current index:" << currentIndex.row();

    QModelIndex index = findIndex(keyboardSearchKeys, true, currentIndex.row(), reverseOrder, !keyboardSearchTimer->isActive());

    if (index.isValid()) {
        fmDebug() << "Search found match at index:" << index.row();
        parent()->setCurrentIndex(index);
        parent()->scrollTo(index, reverseOrder ? QAbstractItemView::PositionAtBottom : QAbstractItemView::PositionAtTop);
    } else {
        fmDebug() << "No match found for search keys:" << keyboardSearchKeys;
    }
    keyboardSearchTimer->start();
}

QModelIndex FileViewHelper::findIndex(const QByteArray &keys, bool matchStart, int current, bool reverseOrder, bool excludeCurrent) const
{
    int rowCount = parent()->model()->rowCount(parent()->rootIndex());

    if (rowCount == 0) {
        fmDebug() << "No rows available for search";
        return QModelIndex();
    }

    fmDebug() << "Finding index with keys:" << keys << "matchStart:" << matchStart
              << "current:" << current << "reverseOrder:" << reverseOrder
              << "excludeCurrent:" << excludeCurrent << "rowCount:" << rowCount;

    for (int i = excludeCurrent ? 1 : 0; i <= rowCount; ++i) {
        int row = reverseOrder ? rowCount + current - i : current + i;

        row = row % rowCount;

        if (excludeCurrent && row == current) {
            continue;
        }

        const QModelIndex &index = parent()->model()->index(row, 0, parent()->rootIndex());
        const QString &pinyinName = parent()->model()->data(index, kItemFilePinyinNameRole).toString();
        if (matchStart ? pinyinName.startsWith(keys, Qt::CaseInsensitive)
                       : pinyinName.contains(keys, Qt::CaseInsensitive)) {
            return index;
        }
    }

    fmDebug() << "No match found for keys:" << keys;
    return QModelIndex();
}

bool FileViewHelper::isEmptyArea(const QPoint &pos)
{
    const QModelIndex &index = parent()->indexAt(pos);

    if (!index.isValid())
        return true;

    if (isSelected(index)) {
        return false;
    } else {
        const QRect &rect = parent()->visualRect(index);

        if (!rect.contains(pos)) {
            fmDebug() << "Position" << pos << "is outside item rect" << rect << "at index:" << index.row();
            return true;
        }

        // if the item can not be selected, return true
        if (!(index.flags() & Qt::ItemIsSelectable)) {
            fmDebug() << "Item at index" << index.row() << "is not selectable - treating as empty area";
            return true;
        }

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        QStyleOptionViewItem option = parent()->viewOptions();
#else
        QStyleOptionViewItem option;
        parent()->initViewItemOption(&option);
#endif
        option.rect = rect;

        const QList<QRect> &geometryList = itemDelegate()->paintGeomertys(option, index);
        auto ret = std::any_of(geometryList.begin(), geometryList.end(), [pos](const QRect &geometry) {
            return geometry.contains(pos);
        });

        return !ret;
    }
}

QSize FileViewHelper::viewContentSize() const
{
    return parent()->contentsSize();
}

int FileViewHelper::verticalOffset() const
{
    return parent()->verticalOffset();
}

bool FileViewHelper::isLastIndex(const QModelIndex &index)
{
    int rowCount = parent()->model()->rowCount(parent()->rootIndex());
    return index.row() + 1 == rowCount;
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
    int iconViewSpacing = view->spacing();

    int itemHeight = itemSize.height() + iconViewSpacing * 2;
    if (pos.y() % itemHeight < iconViewSpacing
        || pos.y() % itemHeight > (itemHeight - iconViewSpacing))
        return -1;

    int itemWidth = itemSize.width() + iconViewSpacing * 2;
    if (pos.x() % itemWidth < iconViewSpacing
        || pos.x() % itemWidth > (itemWidth - iconViewSpacing))
        return -1;

    int columnIndex = pos.x() / itemWidth;
    int columnCount = view->itemCountForRow();

    if (columnIndex >= columnCount)
        return -1;

    int rowIndex = pos.y() / itemHeight;
    return rowIndex * columnCount + columnIndex;
}

bool FileViewHelper::canChangeListItemHeight()
{
    auto customViewProperty = WorkspaceHelper::instance()->findCustomViewProperty(parent()->rootUrl().scheme());
    return customViewProperty.allowChangeListHeight;
}

int FileViewHelper::customDefaultListItemHeightLevel()
{
    auto customViewProperty = WorkspaceHelper::instance()->findCustomViewProperty(parent()->rootUrl().scheme());
    return customViewProperty.defaultListHeight;
}

ViewMode FileViewHelper::customDefaultViewMode()
{
    auto customProperty = WorkspaceHelper::instance()->findCustomViewProperty(parent()->rootUrl().scheme());
    return customProperty.defaultViewMode;
}

void FileViewHelper::handleCommitData(QWidget *editor) const
{
    if (!editor) {
        fmWarning() << "No editor provided for commit data operation";
        return;
    }

    const auto &index = itemDelegate()->editingIndex();
    const FileInfoPointer &fileInfo = parent()->model()->fileInfo(index);

    if (!fileInfo) {
        fmWarning() << "No file info available for commit data at index:" << index.row();
        return;
    }

    fmDebug() << "Handling commit data for file:" << fileInfo->urlOf(UrlInfoType::kUrl).toString();

    ListItemEditor *lineEdit = qobject_cast<ListItemEditor *>(editor);
    IconItemEditor *iconEdit = qobject_cast<IconItemEditor *>(editor);

    QString newFileName = lineEdit ? lineEdit->text() : iconEdit ? iconEdit->getTextEdit()->toPlainText()
                                                                 : "";

    if (newFileName.isEmpty()) {
        fmWarning() << "Empty filename provided for rename operation";
        return;
    }

    fmDebug() << "New filename from editor:" << newFileName;

    QString suffix { editor->property(kEidtorShowSuffix).toString() };

    if (!suffix.isEmpty()) {
        newFileName += QStringLiteral(".");
        newFileName += suffix;
        fmDebug() << "Added suffix to filename:" << newFileName;
    } else if (Application::genericObtuselySetting()->value("FileName", "non-allowableEmptyCharactersOfEnd").toBool()) {
        newFileName = newFileName.trimmed();
        if (newFileName.isEmpty()) {
            return;
        }
    }

    QString originalName = fileInfo->nameOf(NameInfoType::kFileName);
    if (originalName == newFileName) {
        fmDebug() << "Filename unchanged - no rename needed:" << originalName;
        return;
    }

    QUrl oldUrl = fileInfo->getUrlByType(UrlInfoType::kGetUrlByNewFileName, originalName);
    QUrl newUrl = fileInfo->getUrlByType(UrlInfoType::kGetUrlByNewFileName, newFileName);

    fmDebug() << "Committing file rename from:" << originalName << "to:" << newFileName;
    fmDebug() << "Old URL:" << oldUrl.toString() << "New URL:" << newUrl.toString();

    // Todo(yanghao): tag
    FileOperatorHelperIns->renameFile(this->parent(), oldUrl, newUrl);
}

void FileViewHelper::selectFiles(const QList<QUrl> &files)
{
    QList<QUrl> vitualFiles;
    bool ok = dpfHookSequence->run("dfmplugin_workspace", "hook_Url_FetchPathtoVirtual", files, &vitualFiles);
    if (ok && !vitualFiles.isEmpty()) {
        fmDebug() << "Using virtual files for selection - count:" << vitualFiles.size();
        parent()->selectFiles(vitualFiles);
        return;
    }
    if (files.count() > 0) {
        fmDebug() << "Using original files for selection";
        parent()->selectFiles(files);
    }
}

void FileViewHelper::handleTrashStateChanged()
{
    parent()->trashStateChanged();
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
    fmDebug() << "Initializing FileViewHelper components";

    keyboardSearchTimer = new QTimer(this);
    keyboardSearchTimer->setSingleShot(true);
    keyboardSearchTimer->setInterval(200);
    connect(keyboardSearchTimer, &QTimer::timeout, this, &FileViewHelper::clearSearchKey);
    fmDebug() << "Keyboard search timer initialized with 200ms interval";

    connect(qApp, &DApplication::iconThemeChanged, parent(), static_cast<void (QWidget::*)()>(&QWidget::update));
    connect(ClipBoard::instance(), &ClipBoard::clipboardDataChanged, this, &FileViewHelper::clipboardDataChanged);
    connect(parent(), &FileView::triggerEdit, this, &FileViewHelper::triggerEdit);
    connect(WorkspaceHelper::instance(), &WorkspaceHelper::requestSelectFiles, this, &FileViewHelper::selectFiles);
    connect(WorkspaceHelper::instance(), &WorkspaceHelper::trashStateChanged, this, &FileViewHelper::handleTrashStateChanged);

    fmDebug() << "Signal connections established for FileViewHelper";
}

BaseItemDelegate *FileViewHelper::itemDelegate() const
{
    return parent()->itemDelegate();
}
