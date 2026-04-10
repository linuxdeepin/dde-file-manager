// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
#include "checkboxwidthocrindex.h"

#include "searchmanager/searchmanager.h"

#include <dfm-search/dsearch_global.h>

#include <DDciIcon>

#include <QVBoxLayout>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QStandardPaths>
#include <QDir>
#include <QSizePolicy>

DGUI_USE_NAMESPACE
namespace dfmplugin_search {
DWIDGET_USE_NAMESPACE

OcrIndexStatusBar::OcrIndexStatusBar(QWidget *parent)
    : QWidget { parent }
{
    setContentsMargins(4, 0, 0, 0);

    // 设置整体控件的尺寸策略，使其能够水平扩展
    QSizePolicy policy = sizePolicy();
    policy.setHorizontalPolicy(QSizePolicy::Expanding);
    setSizePolicy(policy);

    // 创建水平布局
    boxLayout = new QHBoxLayout(this);
    boxLayout->setContentsMargins(0, 0, 0, 0);
    boxLayout->setSpacing(5);
    setLayout(boxLayout);

    // 创建控件
    spinner = new DTK_NAMESPACE::Widget::DSpinner(this);
    spinner->setFixedSize(16, 16);

    iconLabel = new DTK_NAMESPACE::Widget::DTipLabel("", this);
    iconLabel->setFixedSize(16, 16);
    iconLabel->setPixmap(iconPixmap("dialog-ok", 16));

    msgLabel = new DTK_NAMESPACE::Widget::DTipLabel("", this);
    msgLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    msgLabel->setWordWrap(true);
    msgLabel->setContentsMargins(4, 0, 0, 0);
    msgLabel->setOpenExternalLinks(false);   // 不自动打开外部链接，而是发出linkActivated信号
    msgLabel->setTextFormat(Qt::RichText);   // 设置为富文本格式，确保HTML标签能够正确解析

    // 设置水平尺寸策略为扩展，确保能尽可能在一行显示
    QSizePolicy policyMsgLabel = msgLabel->sizePolicy();
    policyMsgLabel.setHorizontalPolicy(QSizePolicy::Expanding);
    policyMsgLabel.setHorizontalStretch(1);
    msgLabel->setSizePolicy(policyMsgLabel);

    // 连接linkActivated信号来处理点击事件
    connect(msgLabel, &QLabel::linkActivated, this, [this](const QString &) {
        emit resetIndex();
    });

    // 添加到布局
    boxLayout->setSpacing(0);
    boxLayout->setContentsMargins(0, 0, 0, 0);
    boxLayout->addWidget(spinner);
    boxLayout->addWidget(iconLabel);
    boxLayout->addWidget(msgLabel, 1);   // 给标签添加伸展因子

    // 初始状态
    spinner->hide();
    iconLabel->show();
}

void OcrIndexStatusBar::setRunning(bool running)
{
    if (running) {
        spinner->show();
        spinner->start();
        iconLabel->hide();
    } else {
        spinner->hide();
        spinner->stop();
        iconLabel->show();
    }
}

QPixmap OcrIndexStatusBar::iconPixmap(const QString &iconName, int size)
{
    const auto ratio = qApp->devicePixelRatio();
    const auto &dciIcon = DDciIcon::fromTheme(iconName);
    QPixmap px;
    if (!dciIcon.isNull()) {
        auto theme = DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType
                ? DDciIcon::Light
                : DDciIcon::Dark;
        px = dciIcon.pixmap(ratio, size, theme);
    } else {
        const auto &qicon = QIcon::fromTheme(iconName);
        px = qicon.pixmap(size);
        px.setDevicePixelRatio(ratio);
    }

    return px;
}

void OcrIndexStatusBar::updateUI(Status status)
{
    if (!boxLayout) {
        fmWarning() << "Cannot update OcrIndex status bar UI: boxLayout is null";
        return;
    }

    int index = boxLayout->indexOf(msgLabel);
    if (status == Status::Inactive || status == Status::Indexing) {
        boxLayout->setStretch(index, 1);
    } else {
        boxLayout->setStretch(index, 0);
    }
    boxLayout->update();
}

QString OcrIndexStatusBar::getLinkStyle() const
{
    // 定义全局链接样式以保持一致性
    return "<style> a {text-decoration: none; color: #0081FF;} a:hover {color: #40A9FF;} </style>";
}

void OcrIndexStatusBar::setFormattedTextWithLink(const QString &mainText, const QString &linkText, const QString &href)
{
    // 使用内嵌样式去除链接下划线并添加悬停效果
    const QString styleHtml = getLinkStyle();

    // 确保设置为富文本格式，才能正确解析HTML标签
    msgLabel->setTextFormat(Qt::RichText);

    // 组合成最终显示文本
    QString format = "%1 %2 <a href=\"%3\">%4</a>";
    msgLabel->setText(QString(format).arg(styleHtml, mainText, href, linkText));
}

void OcrIndexStatusBar::setStatus(Status status, const QVariant &data)
{
    Q_UNUSED(data)

    currentStatus = status;
    updateUI(status);
    switch (status) {
    case Status::Indexing:
        fmDebug() << "Setting OcrIndex status to Indexing";
        setRunning(true);
        updateIndexingProgress(0, 0);
        break;
    case Status::Completed: {
        fmDebug() << "OcrIndex completed successfully";
        setRunning(false);
        iconLabel->setPixmap(iconPixmap("dialog-ok", 16));

        // 再次获取最后更新时间，以便回调更新具体时间
        auto client = OcrIndexClient::instance();
        client->getLastUpdateTime();
        break;
    }
    case Status::Failed: {
        fmWarning() << "OcrIndex failed";
        setRunning(false);
        setFormattedTextWithLink(
                tr("OCR index update failed, please"),
                tr("try updating again"),
                "update");
        iconLabel->setPixmap(iconPixmap("dialog-error", 16));
        break;
    }
    case Status::Inactive:
        fmDebug() << "Setting OcrIndex status to Inactive";
        spinner->hide();
        spinner->stop();
        iconLabel->hide();
        msgLabel->setTextFormat(Qt::PlainText);
        msgLabel->setText(tr("Enable to search text in images. Indexing may take a few minutes"));
        break;
    }
}

void OcrIndexStatusBar::updateIndexingProgress(qlonglong count, qlonglong total)
{
    if (currentStatus != Status::Indexing) {
        fmDebug() << "Ignoring OcrIndex progress update: status is not Indexing";
        return;
    }

    if (count == 0 && total == 0) {
        msgLabel->setTextFormat(Qt::PlainText);
        msgLabel->setText(tr("Building OCR index"));
        return;
    }

    if (count != 0 && total == 0) {
        msgLabel->setTextFormat(Qt::PlainText);
        msgLabel->setText(tr("Building OCR index, %1 files indexed").arg(count));
        return;
    }

    msgLabel->setTextFormat(Qt::PlainText);
    msgLabel->setText(tr("Building OCR index, %1/%2 items indexed").arg(count).arg(total));
}

OcrIndexStatusBar::Status OcrIndexStatusBar::status() const
{
    return currentStatus;
}

CheckBoxWidthOcrIndex::CheckBoxWidthOcrIndex(QWidget *parent)
    : QWidget { parent }, currentOcrIndexCheckContext(OcrIndexCheckContext::None)
{
    setContentsMargins(0, 0, 0, 0);

    // 设置控件尺寸策略，使其能水平扩展
    QSizePolicy policy = sizePolicy();
    policy.setHorizontalPolicy(QSizePolicy::Expanding);
    setSizePolicy(policy);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(4);   // 添加适当的垂直间距
    setLayout(layout);

    checkBox = new QCheckBox(this);
    statusBar = new OcrIndexStatusBar(this);
    layout->addWidget(checkBox);
    layout->addWidget(statusBar);

    connect(checkBox, &QCheckBox::checkStateChanged, this, [this](Qt::CheckState state) {
        if (checkBox->isChecked()) {
            statusBar->setStatus(OcrIndexStatusBar::Status::Indexing);
        } else {
            statusBar->setStatus(OcrIndexStatusBar::Status::Inactive);
        }
        emit checkStateChanged(state);
    });

    connect(SearchManager::instance(), &SearchManager::enableOcrTextSearchChanged,
            this, [this](bool enable) {
                fmInfo() << "OCR text search enabled state changed to:" << enable;
                setChecked(enable);
            });

    // 修复resetIndex处理逻辑，将检查和后续动作分离
    connect(statusBar, &OcrIndexStatusBar::resetIndex, this, [this]() {
        currentOcrIndexCheckContext = OcrIndexCheckContext::ResetIndex;
        auto client = OcrIndexClient::instance();
        client->checkIndexExists();
    });

    // 连接异步信号处理
    connect(OcrIndexClient::instance(), &OcrIndexClient::indexExistsResult, this,
            [this](bool exists, bool success) {
                if (!success) {
                    fmWarning() << "[OcrIndex] Failed to check if index exists";
                    return;
                }

                // 根据不同的检查上下文执行不同的操作
                if (currentOcrIndexCheckContext == OcrIndexCheckContext::ResetIndex) {
                    // 重置索引按钮的特定处理逻辑
                    auto client = OcrIndexClient::instance();
                    // 预防在首次自动更新前执行reset时，目录还未监控
                    client->setEnable(true);
                    if (exists) {
                        fmDebug() << "Starting OcrIndex update task";
                        client->startTask(OcrIndexClient::TaskType::Update, DFMSEARCH::Global::defaultIndexedDirectory());
                    } else {
                        fmDebug() << "Starting OcrIndex create task";
                        client->startTask(OcrIndexClient::TaskType::Create, DFMSEARCH::Global::defaultIndexedDirectory());
                    }
                    statusBar->setStatus(OcrIndexStatusBar::Status::Indexing);
                } else if (currentOcrIndexCheckContext == OcrIndexCheckContext::InitStatus) {
                    // 初始化状态检查的特定处理逻辑
                    if (checkBox->isChecked()) {
                        statusBar->setStatus(exists ? OcrIndexStatusBar::Status::Completed : OcrIndexStatusBar::Status::Failed);
                    }
                }

                // 重置上下文
                currentOcrIndexCheckContext = OcrIndexCheckContext::None;
            });

    connect(OcrIndexClient::instance(), &OcrIndexClient::lastUpdateTimeResult, this, [this](const QString &time, bool success) {
        if (success && !time.isEmpty()) {
            // 使用辅助方法设置带有样式的链接文本
            statusBar->setFormattedTextWithLink(
                    tr("OCR index update completed, last update time: %1").arg(time),
                    tr("Update index now"),
                    "update");
        } else {
            statusBar->msgLabel->clear();
            fmWarning() << "Failed to get OcrIndex last update time, success:" << success;
        }
    });

    // 修复hasRunningRootTaskResult处理逻辑，将检查和后续动作分离
    connect(OcrIndexClient::instance(), &OcrIndexClient::hasRunningRootTaskResult, this,
            [this](bool running, bool success) {
                if (!success) {
                    fmWarning() << "[OcrIndex] Failed to check if root task is running";
                    return;
                }

                // 初始化状态栏特定处理逻辑
                if (checkBox->isChecked()) {
                    if (running) {
                        fmDebug() << "OcrIndex task is running, setting status to Indexing";
                        statusBar->setStatus(OcrIndexStatusBar::Status::Indexing);
                    } else {
                        fmDebug() << "No running OcrIndex task, checking if index exists";
                        // 异步检查索引是否存在，并设置上下文以指示这是初始化检查
                        currentOcrIndexCheckContext = OcrIndexCheckContext::InitStatus;
                        OcrIndexClient::instance()->checkIndexExists();
                    }
                }
            });
}

void CheckBoxWidthOcrIndex::connectToBackend()
{
    auto client = OcrIndexClient::instance();

    // 使用异步方式检查服务状态
    client->checkServiceStatus();

    // 连接服务状态结果信号
    connect(client, &OcrIndexClient::serviceStatusResult, this, [](OcrIndexClient::ServiceStatus status) {
        fmDebug() << "OcrIndex backend status:" << status;
    });

    connect(client, &OcrIndexClient::taskProgressChanged,
            this, [this](OcrIndexClient::TaskType type, const QString &path, qlonglong count, qlonglong total) {
                fmDebug() << "OcrIndex task progress changed - type:" << static_cast<int>(type)
                          << "path:" << path << "progress:" << count << "/" << total;
                if (shouldHandleIndexEvent(path, type)) {
                    if (statusBar->status() != OcrIndexStatusBar::Status::Indexing) {
                        statusBar->setStatus(OcrIndexStatusBar::Status::Indexing);
                    }
                    statusBar->updateIndexingProgress(count, total);
                }
            });

    connect(client, &OcrIndexClient::taskFinished,
            this, [this](OcrIndexClient::TaskType type, const QString &path, bool success) {
                fmDebug() << "OcrIndex task finished - type:" << static_cast<int>(type)
                          << "path:" << path << "success:" << success;
                if (shouldHandleIndexEvent(path, type)) {
                    statusBar->setStatus(success ? OcrIndexStatusBar::Status::Completed : OcrIndexStatusBar::Status::Failed);
                }
            });

    connect(client, &OcrIndexClient::taskFailed,
            this, [this](OcrIndexClient::TaskType type, const QString &path, const QString &error) {
                fmWarning() << "OcrIndex task failed - type:" << static_cast<int>(type)
                            << "path:" << path << "error:" << error;
                if (shouldHandleIndexEvent(path, type)) {
                    statusBar->setStatus(OcrIndexStatusBar::Status::Failed);
                }
            });
}

void CheckBoxWidthOcrIndex::setDisplayText(const QString &text)
{
    if (checkBox)
        checkBox->setText(text);
}

void CheckBoxWidthOcrIndex::setChecked(bool checked)
{
    if (checkBox)
        checkBox->setChecked(checked);
}

void CheckBoxWidthOcrIndex::initStatusBar()
{
    if (checkBox->isChecked()) {
        auto client = OcrIndexClient::instance();
        // 使用异步API检查根目录索引任务是否正在运行
        client->checkHasRunningRootTask();
        // 处理逻辑已移至hasRunningRootTaskResult信号处理器
    } else {
        statusBar->setStatus(OcrIndexStatusBar::Status::Inactive);
    }
}

bool CheckBoxWidthOcrIndex::shouldHandleIndexEvent(const QString &path, OcrIndexClient::TaskType type) const
{
    Q_UNUSED(path)
    Q_UNUSED(type)

    return checkBox->isChecked();
}

}   // namespace dfmplugin_search
