// SPDX-FileCopyrightText: 2021 - 2024 UnionTech Software Technology Co., Ltd.
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

static constexpr int kFolderBatchSize = 2000;
static constexpr int kFolderListItemMargin { 6 };
constexpr int KFloderListMargin = 10;

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

void FolderListWidgetPrivate::selectUp()
{
    if (!folderView) {
        q->hide();
        return;
    }
    auto curIndex = folderView->currentIndex();
    if (!curIndex.isValid()) {
        if (folderModel->rowCount() > 0)
            curIndex = folderModel->index(folderModel->rowCount() - 1, 0);
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
        if (folderModel->rowCount() > 0)
            curIndex = folderModel->index(0, 0);
    } else {
        int row = curIndex.row() + 1;
        if (row >= folderModel->rowCount())
            row = 0;
        curIndex = folderModel->index(row, 0);
    }
    if (curIndex.isValid())
        folderView->setCurrentIndex(curIndex);
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
    int dataNum = 0;
    d->crumbDatas = datas;
    bool isShowedHiddenFiles = true;
    if (!stacked)
        isShowedHiddenFiles = Application::instance()->genericAttribute(Application::kShowedHiddenFiles).toBool();

    // Calculate max text width
    int maxTextWidth = 0;
    QFontMetrics fm(font());
    for (int i = 0; i < datas.size(); ++i) {
        auto info = InfoFactory::create<FileInfo>(datas[i].url);
        if (!info.isNull() && (isShowedHiddenFiles || !info->isAttributes(FileInfo::FileIsType::kIsHidden))) {
            QStandardItem *item = new QStandardItem(info->fileIcon(), datas[i].displayText);
            item->setData(i, Qt::UserRole);
            d->folderModel->insertRow(dataNum, item);
            dataNum++;

            // Calculate text width
            int textWidth = fm.horizontalAdvance(datas[i].displayText);
            maxTextWidth = qMax(maxTextWidth, textWidth);
        }
    }

    QRect availableRect = this->availableGeometry();
    // 整体的高度为屏幕高度减去任务栏高度，和上下各10单位的边距
    int maxAvailableHeight = availableRect.height() - KFloderListMargin * 2;

    // 计算实际需要的总高度
    int totalHeight;
    if (dataNum > 1) {
        d->folderView->setViewportMargins(kItemMargin, kItemMargin, kItemMargin, kItemMargin);
        totalHeight = kItemMargin * 2 + kFolderItemHeight * dataNum;
    } else {
        d->folderView->setViewportMargins(kItemMargin, kItemMargin * 3 / 2, kItemMargin, kItemMargin * 3 / 2);
        totalHeight = kItemMargin * 3 + kFolderItemHeight * dataNum;
    }
    setFixedHeight(qMin(totalHeight, maxAvailableHeight));
}

void FolderListWidget::popUp(const QPoint& popupPos)
{
    QPoint adjustedPos = popupPos;  // 初始位置
    QSize popupSize = size();
    const QRect availableRect = this->availableGeometry();

    const int maxHeight = availableRect.height() - 2 * KFloderListMargin;

    // 限制弹窗最大高度
    if (popupSize.height() > maxHeight) {
        resize(popupSize.width(), maxHeight);
        popupSize = size();
    }

    if (adjustedPos.y() + popupSize.height() > availableRect.bottom() - KFloderListMargin) {
        adjustedPos.setY(availableRect.bottom() - KFloderListMargin - popupSize.height());
    }

    // 确保不低于顶部边距
    if (adjustedPos.y() < availableRect.top() + KFloderListMargin) {
        adjustedPos.setY(availableRect.top() + KFloderListMargin);
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
    }
    else {
        QString pressedText = event->text();
        if (!pressedText.isEmpty() && pressedText[0].isPrint()) {
            QModelIndex currentIndex = d->folderView->currentIndex();
            int startRow = currentIndex.isValid() ? currentIndex.row() : -1;
            findAndSelectMatch(pressedText, startRow);
        }
    }
    DBlurEffectWidget::keyPressEvent(event);
}

void FolderListWidget::hideEvent(QHideEvent *event)
{
    emit hidden();
    DBlurEffectWidget::hideEvent(event);
}

bool FolderListWidget::matchText(const QString &source, const QString &input) const
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

bool FolderListWidget::findAndSelectMatch(const QString &text, int startRow) const
{
    bool foundCurrent = false;

    // 实现列表的循环查找，比如当前选中的满足输入的字符，则去匹配下一个
    for (int round = 0; round < 2; ++round) {
        for (int i = 0; i < d->folderModel->rowCount(); ++i) {
            int currentRow = (startRow + i + 1) % d->folderModel->rowCount();
            QString itemText = d->folderModel->item(currentRow)->text();

            if (matchText(itemText, text)) {
                if (!foundCurrent && currentRow == startRow) {
                    foundCurrent = true;
                    continue;
                }

                QModelIndex index = d->folderModel->index(currentRow, 0);
                d->folderView->setCurrentIndex(index);
                d->folderView->scrollTo(index);
                return true;
            }
        }
    }
    return false;
}

QRect FolderListWidget::availableGeometry() const
{
    QScreen *screen = QGuiApplication::primaryScreen();
    if (!screen)
        return QRect();
    return screen->availableGeometry();
}
