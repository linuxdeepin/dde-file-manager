// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
#include "checkboxwidthtextindex.h"

#include "searchmanager/searchmanager.h"

#include <dfm-search/dsearch_global.h>

#include <QVBoxLayout>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QStandardPaths>
#include <QDir>

namespace dfmplugin_search {
DWIDGET_USE_NAMESPACE

TextIndexStatusBar::TextIndexStatusBar(QWidget *parent)
    : QWidget { parent }
{
    setContentsMargins(4, 0, 0, 0);

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
    iconLabel->setPixmap(QIcon::fromTheme("dialog-ok").pixmap(16, 16));

    msgLabel = new DTK_NAMESPACE::Widget::DTipLabel("", this);
    msgLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    msgLabel->setWordWrap(true);
    msgLabel->setContentsMargins(4, 0, 0, 0);

    updateBtn = new DCommandLinkButton("", this);
    updateBtn->setFocusPolicy(Qt::NoFocus);
    updateBtn->setContentsMargins(0, 0, 0, 0);
    QFont font = msgLabel->font();
    updateBtn->setFont(font);
    connect(updateBtn, &DCommandLinkButton::clicked, this, [this]() {
        emit resetIndex();
    });

    // 添加到布局
    boxLayout->setSpacing(0);
    boxLayout->setContentsMargins(0, 0, 0, 0);
    boxLayout->addWidget(spinner);
    boxLayout->addWidget(iconLabel);
    boxLayout->addWidget(msgLabel);
    boxLayout->addWidget(updateBtn);
    boxLayout->addStretch();

    // 初始状态
    updateBtn->hide();
    spinner->hide();
    iconLabel->show();
}

void TextIndexStatusBar::setRunning(bool running)
{
    if (running) {
        spinner->show();
        spinner->start();
        iconLabel->hide();
        updateBtn->hide();
    } else {
        spinner->hide();
        spinner->stop();
        iconLabel->show();
        updateBtn->show();
    }
}

void TextIndexStatusBar::updateUI(Status status)
{
    if (!boxLayout)
        return;
    int index = boxLayout->indexOf(msgLabel);
    if (status == Status::Inactive || status == Status::Indexing) {
        boxLayout->setStretch(index, 1);
    } else {
        boxLayout->setStretch(index, 0);
    }
    boxLayout->update();
}

void TextIndexStatusBar::setStatus(Status status, const QVariant &data)
{
    Q_UNUSED(data)

    currentStatus = status;
    updateUI(status);
    switch (status) {
    case Status::Indexing:
        setRunning(true);
        updateIndexingProgress(0, 0);
        break;
    case Status::Completed: {
        setRunning(false);
        msgLabel->clear();
        auto client = TextIndexClient::instance();
        client->getLastUpdateTime();
        updateBtn->setText(tr("Update index now"));
        iconLabel->setPixmap(QIcon::fromTheme("dialog-ok").pixmap(16, 16));
        break;
    }
    case Status::Failed:
        setRunning(false);
        msgLabel->setText(tr("Index update failed, please"));
        updateBtn->setText(tr("try updating again."));
        iconLabel->setPixmap(QIcon::fromTheme("dialog-error").pixmap(16, 16));
        break;
    case Status::Inactive:
        spinner->hide();
        spinner->stop();
        iconLabel->hide();
        updateBtn->hide();
        msgLabel->setText(tr("Enable to search file contents. Indexing may take a few minutes."));
        break;
    }
}

void TextIndexStatusBar::updateIndexingProgress(qlonglong count, qlonglong total)
{
    if (currentStatus != Status::Indexing) {
        return;
    }

    if (count == 0 && total == 0) {
        msgLabel->setText(tr("Building index"));
        return;
    }

    if (count != 0 && total == 0) {
        msgLabel->setText(tr("Building index, %1 files indexed").arg(count));
        return;
    }

    msgLabel->setText(tr("Building index, %1/%2 items indexed").arg(count).arg(total));
}

TextIndexStatusBar::Status TextIndexStatusBar::status() const
{
    return currentStatus;
}

CheckBoxWidthTextIndex::CheckBoxWidthTextIndex(QWidget *parent)
    : QWidget { parent }, currentIndexCheckContext(IndexCheckContext::None)
{
    setContentsMargins(0, 0, 0, 0);
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);

    checkBox = new QCheckBox(this);
    statusBar = new TextIndexStatusBar(this);
    layout->addWidget(checkBox);
    layout->addWidget(statusBar);

    connect(checkBox, &QCheckBox::checkStateChanged, this, [this](Qt::CheckState state) {
        if (checkBox->isChecked()) {
            statusBar->setStatus(TextIndexStatusBar::Status::Indexing);
        } else {
            statusBar->setStatus(TextIndexStatusBar::Status::Inactive);
        }
        emit checkStateChanged(state);
    });

    connect(SearchManager::instance(), &SearchManager::enableFullTextSearchChanged,
            this, [this](bool enable) {
                setChecked(enable);
            });

    // 修复resetIndex处理逻辑，将检查和后续动作分离
    connect(statusBar, &TextIndexStatusBar::resetIndex, this, [this]() {
        currentIndexCheckContext = IndexCheckContext::ResetIndex;
        auto client = TextIndexClient::instance();
        client->checkIndexExists();
    });

    // 连接异步信号处理
    connect(TextIndexClient::instance(), &TextIndexClient::indexExistsResult, this,
            [this](bool exists, bool success) {
                if (!success) {
                    fmWarning() << "[TextIndex] Failed to check if index exists";
                    return;
                }

                // 根据不同的检查上下文执行不同的操作
                if (currentIndexCheckContext == IndexCheckContext::ResetIndex) {
                    // 重置索引按钮的特定处理逻辑
                    auto client = TextIndexClient::instance();
                    // 预防在首次自动更新前执行reset时，目录还未监控
                    client->setEnable(true);
                    if (exists) {
                        client->startTask(TextIndexClient::TaskType::Update, DFMSEARCH::Global::defaultIndexedDirectory());
                    } else {
                        client->startTask(TextIndexClient::TaskType::Create, DFMSEARCH::Global::defaultIndexedDirectory());
                    }
                    statusBar->setStatus(TextIndexStatusBar::Status::Indexing);
                } else if (currentIndexCheckContext == IndexCheckContext::InitStatus) {
                    // 初始化状态检查的特定处理逻辑
                    if (checkBox->isChecked()) {
                        statusBar->setStatus(exists ? TextIndexStatusBar::Status::Completed : TextIndexStatusBar::Status::Failed);
                    }
                }

                // 重置上下文
                currentIndexCheckContext = IndexCheckContext::None;
            });

    connect(TextIndexClient::instance(), &TextIndexClient::lastUpdateTimeResult, this, [this](const QString &time, bool success) {
        if (success && !time.isEmpty()) {
            statusBar->msgLabel->setText(tr("Index update completed, last update time: %1").arg(time));
        }
    });

    // 修复hasRunningRootTaskResult处理逻辑，将检查和后续动作分离
    connect(TextIndexClient::instance(), &TextIndexClient::hasRunningRootTaskResult, this,
            [this](bool running, bool success) {
                if (!success) {
                    fmWarning() << "[TextIndex] Failed to check if root task is running";
                    return;
                }

                // 初始化状态栏特定处理逻辑
                if (checkBox->isChecked()) {
                    if (running) {
                        statusBar->setStatus(TextIndexStatusBar::Status::Indexing);
                    } else {
                        // 异步检查索引是否存在，并设置上下文以指示这是初始化检查
                        currentIndexCheckContext = IndexCheckContext::InitStatus;
                        TextIndexClient::instance()->checkIndexExists();
                    }
                }
            });
}

void CheckBoxWidthTextIndex::connectToBackend()
{
    auto client = TextIndexClient::instance();

    // 使用异步方式检查服务状态
    client->checkServiceStatus();

    // 连接服务状态结果信号
    connect(client, &TextIndexClient::serviceStatusResult, this, [](TextIndexClient::ServiceStatus status) {
        fmDebug() << "TextIndex backend status:" << status;
    });

    connect(client, &TextIndexClient::taskProgressChanged,
            this, [this](TextIndexClient::TaskType type, const QString &path, qlonglong count, qlonglong total) {
                fmDebug() << "Index task changed:" << type << path << count << total;
                if (shouldHandleIndexEvent(path, type)) {
                    if (statusBar->status() != TextIndexStatusBar::Status::Indexing) {
                        statusBar->setStatus(TextIndexStatusBar::Status::Indexing);
                    }
                    statusBar->updateIndexingProgress(count, total);
                }
            });

    connect(client, &TextIndexClient::taskFinished,
            this, [this](TextIndexClient::TaskType type, const QString &path, bool success) {
                if (shouldHandleIndexEvent(path, type)) {
                    statusBar->setStatus(success ? TextIndexStatusBar::Status::Completed : TextIndexStatusBar::Status::Failed);
                }
            });

    connect(client, &TextIndexClient::taskFailed,
            this, [this](TextIndexClient::TaskType type, const QString &path, const QString &error) {
                if (shouldHandleIndexEvent(path, type)) {
                    statusBar->setStatus(TextIndexStatusBar::Status::Failed);
                }
            });
}

void CheckBoxWidthTextIndex::setDisplayText(const QString &text)
{
    if (checkBox)
        checkBox->setText(text);
}

void CheckBoxWidthTextIndex::setChecked(bool checked)
{
    if (checkBox)
        checkBox->setChecked(checked);
}

void CheckBoxWidthTextIndex::initStatusBar()
{
    if (checkBox->isChecked()) {
        auto client = TextIndexClient::instance();
        // 使用异步API检查根目录索引任务是否正在运行
        client->checkHasRunningRootTask();
        // 处理逻辑已移至hasRunningRootTaskResult信号处理器
    } else {
        statusBar->setStatus(TextIndexStatusBar::Status::Inactive);
    }
}

bool CheckBoxWidthTextIndex::shouldHandleIndexEvent(const QString &path, TextIndexClient::TaskType type) const
{
    bool isIndexedPath = DFMSEARCH::Global::defaultIndexedDirectory().contains(path);
    return checkBox->isChecked() && isIndexedPath && type != TextIndexClient::TaskType::Remove;
}

}   // namespace dfmplugin_search
