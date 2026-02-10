// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "views/private/folderlistwidget_p.h"
#include "views/folderlistwidget.h"
#include "views/folderviewdelegate.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/utils/chinese2pinyin.h>

#include <QVBoxLayout>
#include <QStandardItemModel>
#include <QKeyEvent>
#include <QGuiApplication>
#include <QScreen>

DWIDGET_USE_NAMESPACE
using namespace dfmplugin_titlebar;
DFMBASE_USE_NAMESPACE

static constexpr int kFolderBatchSize = { 2000 };
static constexpr int kFolderListItemMargin { 6 };
static constexpr int kFloderListMargin = { 10 };
static constexpr int kMaxAvailableWidth { 800 };
static constexpr int kMinAvailableWidth { 173 };

FolderListWidgetPrivate::FolderListWidgetPrivate(FolderListWidget *qq)
    : QObject(qq), q(qq)
{
    q->resize(172, kFolderListItemMargin * 2 + kFolderItemHeight * 8);
    q->setMouseTracking(true);
    layout = new QVBoxLayout(qq);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    folderModel = new QStandardItemModel(this);
    folderView = new DListView(qq);
    folderView->setMouseTracking(true);
    folderView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    folderView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    folderView->setViewportMargins(kFolderListItemMargin, kFolderListItemMargin, kFolderListItemMargin, kFolderListItemMargin);
    folderView->setUniformItemSizes(true);
    folderView->setViewMode(QListView::ListMode);
    folderView->setResizeMode(QListView::Adjust);
    folderView->setMovement(QListView::Static);
    folderView->setLayoutMode(QListView::Batched);
    folderView->setBatchSize(kFolderBatchSize);

    folderView->setModel(folderModel);
    folderDelegate = new FolderViewDelegate(folderView);
    folderView->setItemDelegate(folderDelegate);

    layout->addWidget(folderView);
    q->setLayout(layout);

    initConnect();
}

FolderListWidgetPrivate::~FolderListWidgetPrivate()
{
}

void FolderListWidgetPrivate::initConnect()
{
    connect(folderView, &DListView::clicked, this, &FolderListWidgetPrivate::clicked);
}

void FolderListWidgetPrivate::clicked(const QModelIndex &index)
{
    q->hide();

    if (index.isValid()) {
        int row = index.data(Qt::UserRole).toInt();
        if (row >= 0 && row < crumbDatas.size()) {
            Q_EMIT q->urlButtonActivated(crumbDatas[row].url);
        }
    }
}

void FolderListWidgetPrivate::returnPressed()
{
    if (!folderView) {
        q->hide();
        return;
    }
    clicked(folderView->currentIndex());
}

QModelIndex FolderListWidgetPrivate::getStartIndexFromHover(bool isUp)
{
    const int rowCount = folderModel->rowCount();
    if (rowCount <= 0)
        return QModelIndex();

    int nextRow = 0;
    QModelIndex currentIndex = folderView->currentIndex();
    if (currentIndex.isValid()) {
        nextRow = currentIndex.row();
    } else {
        QPoint mousePos = q->mapFromGlobal(QCursor::pos());
        QModelIndex hoverIndex = folderView->indexAt(mousePos);
        nextRow = hoverIndex.isValid() ? hoverIndex.row() : (isUp ? rowCount - 1 : 0);
    }

    if (isUp) {
        nextRow = (nextRow - 1 + rowCount) % rowCount;
    } else {
        nextRow = (nextRow + 1) % rowCount;
    }

    return folderModel->index(nextRow, 0);
}

void FolderListWidgetPrivate::selectUp()
{
    if (!folderView) {
        q->hide();
        return;
    }

    auto curIndex = folderView->currentIndex();
    if (!curIndex.isValid()) {
        curIndex = getStartIndexFromHover(true);
    } else {
        int row = curIndex.row() - 1;
        if (row < 0)
            row = folderModel->rowCount() - 1;
        curIndex = folderModel->index(row, 0);
    }

    if (curIndex.isValid())
        folderView->setCurrentIndex(curIndex);
}

void FolderListWidgetPrivate::selectDown()
{
    if (!folderView) {
        q->hide();
        return;
    }

    auto curIndex = folderView->currentIndex();
    if (!curIndex.isValid()) {
        curIndex = getStartIndexFromHover(false);
    } else {
        int row = curIndex.row() + 1;
        if (row >= folderModel->rowCount())
            row = 0;
        curIndex = folderModel->index(row, 0);
    }

    if (curIndex.isValid())
        folderView->setCurrentIndex(curIndex);
}

void FolderListWidgetPrivate::handleKeyInput(const QString &pressedText)
{
    if (pressedText.isEmpty() || !pressedText[0].isPrint())
        return;

    QModelIndex currentIndex = folderView->currentIndex();
    int startRow = -1;
    if (!currentIndex.isValid()) {
        // 如果没有激活选中某一项的时候，就以鼠标悬浮所在项为基准去往后查找
        QPoint mousePos = q->mapFromGlobal(QCursor::pos());
        QModelIndex hoverIndex = folderView->indexAt(mousePos);
        startRow = hoverIndex.isValid() ? hoverIndex.row() : -1;
    } else {
        startRow = currentIndex.row();
    }

    findAndSelectMatch(pressedText, startRow);
}

FolderListWidget::FolderListWidget(QWidget *parent)
    : DBlurEffectWidget(parent), d(new FolderListWidgetPrivate(this))
{
    setWindowFlags(Qt::Popup | Qt::FramelessWindowHint);
    setBlurEnabled(true);
    setMode(DBlurEffectWidget::GaussianBlur);
}

FolderListWidget::~FolderListWidget() = default;

void FolderListWidget::setFolderList(const QList<CrumbData> &datas, bool stacked)
{
    d->folderModel->clear();
    d->crumbDatas = datas;
    bool isShowedHiddenFiles = true;
    if (!stacked)
        isShowedHiddenFiles = Application::instance()->genericAttribute(Application::kShowedHiddenFiles).toBool();

    // Calculate max text width
    int itemMaxWidth = kMinAvailableWidth;
    for (int i = 0; i < datas.size(); ++i) {
        auto info = InfoFactory::create<FileInfo>(datas[i].url);
        if (!info.isNull() && (isShowedHiddenFiles || !info->isAttributes(FileInfo::FileIsType::kIsHidden))) {
            QStandardItem *item = new QStandardItem(info->fileIcon(), datas[i].displayText);
            item->setData(i, Qt::UserRole);
            d->folderModel->appendRow(item);

            // Calculate text width
            int textWidth = d->folderView->sizeHintForIndex(d->folderModel->index(i, 0)).width();
            itemMaxWidth = qMax(itemMaxWidth, textWidth);
        }
    }

    QRect availableRect = this->availableGeometry(QCursor::pos());
    // 整体的高度为屏幕高度减去任务栏高度，和上下各10单位的边距
    int maxAvailableHeight = availableRect.height() - kFloderListMargin * 2;

    // 计算实际需要的总高度
    int totalHeight;
    int rowCount = d->folderModel->rowCount();
    if (rowCount > 1) {
        d->folderView->setViewportMargins(kItemMargin, kItemMargin, kItemMargin, kItemMargin);
        totalHeight = kItemMargin * 2 + kFolderItemHeight * rowCount;
    } else {
        d->folderView->setViewportMargins(kItemMargin, kItemMargin * 3 / 2, kItemMargin, kItemMargin * 3 / 2);
        totalHeight = kItemMargin * 3 + kFolderItemHeight * rowCount;
    }
    setFixedSize(qMin(itemMaxWidth, kMaxAvailableWidth), qMin(totalHeight, maxAvailableHeight));
}

void FolderListWidget::popUp(const QPoint &popupPos)
{
    QPoint adjustedPos = popupPos;   // 初始位置
    QSize popupSize = size();
    const QRect availableRect = this->availableGeometry(popupPos);

    const int maxHeight = availableRect.height() - 2 * kFloderListMargin;

    // 限制弹窗最大高度
    if (popupSize.height() > maxHeight) {
        resize(popupSize.width(), maxHeight);
        popupSize = size();
    }

    if (adjustedPos.y() + popupSize.height() > availableRect.bottom() - kFloderListMargin) {
        adjustedPos.setY(availableRect.bottom() - kFloderListMargin - popupSize.height());
    }

    // 确保不低于顶部边距
    if (adjustedPos.y() < availableRect.top() + kFloderListMargin) {
        adjustedPos.setY(availableRect.top() + kFloderListMargin);
    }

    // 超出屏幕边界时,调整水平位置
    if (adjustedPos.x() + popupSize.width() > availableRect.right()) {
        adjustedPos.setX(availableRect.right() - popupSize.width());
    }
    if (adjustedPos.x() < availableRect.left()) {
        adjustedPos.setX(availableRect.left());
    }

    move(adjustedPos);
    show();
}

void FolderListWidget::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Up) {
        d->selectUp();
    } else if (event->key() == Qt::Key_Down) {
        d->selectDown();
    } else if (event->key() == Qt::Key_Return) {
        d->returnPressed();
    } else {
        d->handleKeyInput(event->text());
    }
    DBlurEffectWidget::keyPressEvent(event);
}

void FolderListWidget::hideEvent(QHideEvent *event)
{
    emit hidden();
    DBlurEffectWidget::hideEvent(event);
}

bool FolderListWidgetPrivate::matchText(const QString &source, const QString &input) const
{
    if (input.isEmpty() || source.isEmpty())
        return false;

    QString sourceLower = source.toLower();
    QString inputLower = input.toLower();

    if (sourceLower.startsWith(inputLower))
        return true;
    // 上面的英文未匹配，使用拼音再匹配一次
    if (input[0].isLetter()) {
        QString pinyinText = Pinyin::Chinese2Pinyin(source);
        return pinyinText.toLower().startsWith(inputLower);
    }

    return false;
}

bool FolderListWidgetPrivate::findAndSelectMatch(const QString &text, int startRow) const
{
    bool foundCurrent = false;

    // 实现列表的循环查找，比如当前选中的满足输入的字符，则去匹配下一个
    for (int round = 0; round < 2; ++round) {
        for (int i = 0; i < folderModel->rowCount(); ++i) {
            int currentRow = (startRow + i + 1) % folderModel->rowCount();
            QString itemText = folderModel->item(currentRow)->text();

            if (matchText(itemText, text)) {
                if (!foundCurrent && currentRow == startRow) {
                    foundCurrent = true;
                    continue;
                }

                QModelIndex index = folderModel->index(currentRow, 0);
                folderView->setCurrentIndex(index);
                folderView->scrollTo(index);
                return true;
            }
        }
    }
    return false;
}

QRect FolderListWidget::availableGeometry(const QPoint &popUpPos) const
{
    QScreen *screen = nullptr;

    // 先通过弹出位置来计算所在屏幕
    if (!popUpPos.isNull())
        screen = QGuiApplication::screenAt(popUpPos);

    // 如果根据传递值没有找到屏幕则使用全局鼠标所在位置
    if (!screen)
        screen = QGuiApplication::screenAt(QCursor::pos());
    // 找到窗口所在屏幕
    if (!screen && window())
        screen = window()->screen();

    // 使用主屏幕
    if (!screen) {
        screen = QGuiApplication::primaryScreen();
    }

    return screen ? screen->availableGeometry() : QRect();
}
