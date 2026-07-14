// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "multifilebasicinfowidget.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/utils/fileutils.h>

#include <DFontSizeManager>

#include <QGridLayout>

inline constexpr int kFrameWidth { 360 };
inline constexpr int kLeftWidgetWidth { 100 };
inline constexpr int kRightWidgetWidth { 225 };
inline constexpr int kLeftContentsMargins { 0 };
inline constexpr int kRightContentsMargins { 5 };
inline constexpr int kSpacingHeight { 2 };

using namespace dfmplugin_propertydialog;
DWIDGET_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

MultiFileBasicInfoWidget::MultiFileBasicInfoWidget(const QList<QUrl> &urls,
                                                   QWidget *parent)
    : DArrowLineDrawer(parent)
    , fileCalculationUtils(new FileScanner)
{
    initUI();
    loadData(urls);
}

MultiFileBasicInfoWidget::~MultiFileBasicInfoWidget()
{
    fileCalculationUtils->stop();
    fileCalculationUtils->deleteLater();
}

void MultiFileBasicInfoWidget::getOrgHideBoxState(FilePropertyState &states)
{
    states.hideState = hideFile->checkState();
}

void MultiFileBasicInfoWidget::filesHideStateChanged(int state)
{
    emit hideBoxStateChanged(state);
}

void MultiFileBasicInfoWidget::initUI()
{
    setExpandedSeparatorVisible(false);
    setSeparatorVisible(false);
    setFixedWidth(kFrameWidth);
    setExpand(true);

    setTitle(tr("Basic info"));
    DFontSizeManager::instance()->bind(this, DFontSizeManager::SizeType::T6, QFont::DemiBold);

    DFrame *frameMain = new DFrame(this);
    frameMain->setFrameShape(QFrame::Shape::NoFrame);
    frameMain->setFixedWidth(kFrameWidth);

    filesSize = createValueLabel(frameMain, tr("Total size"));
    filesSize->setRightValue("-", Qt::ElideNone, Qt::AlignVCenter, true);
    filesCount = createValueLabel(frameMain, tr("Number of files"));
    filesCount->setRightValue("-", Qt::ElideNone, Qt::AlignVCenter, true);
    accessTime = createValueLabel(frameMain, tr("Time accessed"));
    accessTime->setRightValue("-", Qt::ElideNone, Qt::AlignVCenter, true);
    modifyTime = createValueLabel(frameMain, tr("Time modified"));
    modifyTime->setRightValue("-", Qt::ElideNone, Qt::AlignVCenter, true);

    hideFile = new SkipPartiallyCheckBox(frameMain);
    DFontSizeManager::instance()->bind(hideFile, DFontSizeManager::SizeType::T7, QFont::Normal);
    hideFile->setText(tr("Hide this file"));
    hideFile->setToolTip(hideFile->text());

    QGridLayout *layoutMain = new QGridLayout;
    layoutMain->setContentsMargins(kLeftContentsMargins, 0, 0, kRightContentsMargins);
    layoutMain->setSpacing(kSpacingHeight);
    layoutMain->addWidget(filesSize, 0, 0, 1, 6);
    layoutMain->addWidget(filesCount, 1, 0, 1, 6);
    layoutMain->addWidget(accessTime, 2, 0, 1, 6);
    layoutMain->addWidget(modifyTime, 3, 0, 1, 6);
    layoutMain->addWidget(hideFile, 4, 0, 1, 6, Qt::AlignHCenter);

    frameMain->setLayout(layoutMain);
    setContent(frameMain);
}

void MultiFileBasicInfoWidget::loadData(const QList<QUrl> &urls)
{
    // 获取文件个数和大小
    setFilesCountAndSize(urls);

    // 获取所有文件的访问时间
    setAccessTime(urls);

    // 获取所有文件的修改时间
    setModifyTime(urls);

    // 获取文件的隐藏状态(需要判断出所有文件的隐藏状态是否一致，如果不一致，设置成中间状态)
    setHideState(urls);
}

void MultiFileBasicInfoWidget::updateFilesCountAndSizeLabel(const FileScanner::ScanResult &result)
{
    filesCount->setRightValue(tr("%1 file(s), %2 folder(s)")
                                      .arg(result.fileCount)
                                      .arg(result.directoryCount),
                              Qt::ElideNone, Qt::AlignVCenter, true);

    filesSize->setRightValue(FileUtils::formatSize(result.totalSize),
                             Qt::ElideNone, Qt::AlignVCenter, true);
}

void MultiFileBasicInfoWidget::setFilesCountAndSize(const QList<QUrl> &urls)
{
    connect(fileCalculationUtils, &FileScanner::progressChanged,
            this, &MultiFileBasicInfoWidget::updateFilesCountAndSizeLabel);
    QList<QUrl> targets;
    UniversalUtils::urlsTransformToLocal(urls, &targets);
    fileCalculationUtils->start(targets);
}

void MultiFileBasicInfoWidget::setAccessTime(const QList<QUrl> &urls)
{
    QDateTime firstAccessTime;
    bool accessTimeConsistent = true;
    for (int i = 0; i < urls.size(); ++i) {
        FileInfoPointer info = InfoFactory::create<FileInfo>(urls.at(i));
        if (info.isNull()) {
            accessTimeConsistent = false;
            break;
        }
        QDateTime lastRead = info->timeOf(TimeInfoType::kLastRead).value<QDateTime>();
        if (i == 0) {
            firstAccessTime = lastRead;
        } else if (lastRead != firstAccessTime) {
            accessTimeConsistent = false;
            break;
        }
    }
    if (accessTimeConsistent && firstAccessTime.isValid()) {
        accessTime->setRightValue(firstAccessTime.toString(FileUtils::dateTimeFormat()),
                                  Qt::ElideNone, Qt::AlignVCenter, true);
    }
}

void MultiFileBasicInfoWidget::setModifyTime(const QList<QUrl> &urls)
{
    QDateTime firstModifyTime;
    bool modifyTimeConsistent = true;
    for (int i = 0; i < urls.size(); ++i) {
        FileInfoPointer info = InfoFactory::create<FileInfo>(urls.at(i));
        if (info.isNull()) {
            modifyTimeConsistent = false;
            break;
        }
        QDateTime lastModified = info->timeOf(TimeInfoType::kLastModified).value<QDateTime>();
        if (i == 0) {
            firstModifyTime = lastModified;
        } else if (lastModified != firstModifyTime) {
            modifyTimeConsistent = false;
            break;
        }
    }
    if (modifyTimeConsistent && firstModifyTime.isValid()) {
        modifyTime->setRightValue(firstModifyTime.toString(FileUtils::dateTimeFormat()),
                                  Qt::ElideNone, Qt::AlignVCenter, true);
    }
}

void MultiFileBasicInfoWidget::setHideState(const QList<QUrl> &urls)
{
    bool firstHidden = false;
    for (int i = 0; i < urls.size(); ++i) {
        FileInfoPointer info = InfoFactory::create<FileInfo>(urls.at(i));
        if (info.isNull()) {
            hideFile->setEnabled(false);
            fmCritical() << "Failed to get file info: " << urls.at(i).path();
            return;
        }

        if (!info->canAttributes(CanableInfoType::kCanHidden)) {
            hideFile->setEnabled(false);
            fmWarning() << "Can not hide or visible the file: " << urls.at(i).path();
            return;
        }

        // 属性中的隐藏不管理以'.'开头的文件
        if (info->fileName().startsWith('.')) {
            hideFile->setEnabled(false);
            fmWarning() << "Can not hide or visible the file:" << urls.at(i).path();
            return;
        }

        bool isHidden = info->isAttributes(OptInfoType::kIsHidden);
        if (i == 0) {
            firstHidden = isHidden;
        } else if (isHidden != firstHidden) {
            // 状态不一致，设置为中间状态
            hideFile->setCheckState(Qt::PartiallyChecked);
            connect(hideFile, &DCheckBox::stateChanged,
                    this, &MultiFileBasicInfoWidget::filesHideStateChanged);
            return;
        }
    }

    // 所有文件状态一致
    hideFile->setCheckState(firstHidden ? Qt::Checked : Qt::Unchecked);
    connect(hideFile, &DCheckBox::stateChanged,
            this, &MultiFileBasicInfoWidget::filesHideStateChanged);
}

KeyValueLabel *MultiFileBasicInfoWidget::createValueLabel(QFrame *frame,
                                                          const QString &leftValue)
{
    KeyValueLabel *res = new KeyValueLabel(frame);
    res->setLeftFontSizeWeight(DFontSizeManager::SizeType::T7, QFont::Weight::Medium);
    res->setLeftValue(leftValue, Qt::ElideMiddle, Qt::AlignLeft, true);
    res->setRightFontSizeWeight(DFontSizeManager::SizeType::T8, QFont::Light);
    res->leftWidget()->setFixedWidth(kLeftWidgetWidth);
    res->rightWidget()->setFixedWidth(kRightWidgetWidth);
    return res;
}

void MultiFileBasicInfoWidget::calculateFileCount(const QList<QUrl> &urls,
                                                  int &dirCount,
                                                  int &fileCount)
{
    dirCount = 0;
    fileCount = 0;
    for (const QUrl &url : urls) {
        FileInfoPointer info = InfoFactory::create<FileInfo>(url);
        if (info.isNull())
            return;

        if (info->isAttributes(OptInfoType::kIsSymLink)) {
            if (info->isAttributes(OptInfoType::kIsDir))
                ++dirCount;
            else
                ++fileCount;
            continue;
        }

        if (info->isAttributes(OptInfoType::kIsDir)) {
            ++dirCount;
            continue;
        }

        if (info->isAttributes(OptInfoType::kIsFile)) {
            ++fileCount;
            continue;
        }
    }
}
