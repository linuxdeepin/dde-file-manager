// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
#include "checkboxwidthtextindex.h"

#include "searchmanager/searchmanager.h"

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
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(5);
    setLayout(layout);

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
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(spinner);
    layout->addWidget(iconLabel);
    layout->addWidget(msgLabel);
    layout->addWidget(updateBtn);
    layout->addStretch();

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

void TextIndexStatusBar::setStatus(Status status, const QVariant &data)
{
    currentStatus = status;

    switch (status) {
    case Status::Indexing:
        setRunning(true);
        updateIndexingProgress(data.toLongLong());
        break;
    case Status::Completed: {
        setRunning(false);
        QString lastTime = TextIndexClient::instance()->getLastUpdateTime();
        msgLabel->setText(tr("Index update completed, last update time: %1").arg(lastTime));
        updateBtn->setText(tr("Update Index Now"));
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
        setRunning(false);
        iconLabel->hide();
        msgLabel->setText(tr("Enable to search file contents. Indexing may take a few minutes."));
        break;
    }
}

void TextIndexStatusBar::updateIndexingProgress(qlonglong count)
{
    if (currentStatus == Status::Indexing) {
        msgLabel->setText(tr("Building index, %1 files indexed").arg(count));
    }
}

TextIndexStatusBar::Status TextIndexStatusBar::status() const
{
    return currentStatus;
}

CheckBoxWidthTextIndex::CheckBoxWidthTextIndex(QWidget *parent)
    : QWidget { parent }
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

    connect(statusBar, &TextIndexStatusBar::resetIndex, this, [this]() {
        auto client = TextIndexClient::instance();
        bool exitsts = client->indexExists().has_value() && client->indexExists().value();
        // TODO(search): dfm-search
        if (exitsts) {
            client->startTask(TextIndexClient::TaskType::Update, QDir::homePath());
        } else {
            client->startTask(TextIndexClient::TaskType::Create, QDir::homePath());
        }
        statusBar->setStatus(TextIndexStatusBar::Status::Indexing);
    });
}

void CheckBoxWidthTextIndex::connectToBackend()
{
    auto client = TextIndexClient::instance();

    // Note: checkService 非常重要！不激活后端无法正确的连接信号
    auto status = client->checkService();
    fmDebug() << "TextIndex backend status:" << status;
    connect(client, &TextIndexClient::taskProgressChanged,
            this, [this](TextIndexClient::TaskType type, const QString &path, qlonglong count) {
                fmDebug() << "Index task changed:" << type << path << count;
                if (shouldHandleIndexEvent(path, type)) {
                    if (statusBar->status() != TextIndexStatusBar::Status::Indexing) {
                        statusBar->setStatus(TextIndexStatusBar::Status::Indexing);
                    }
                    statusBar->updateIndexingProgress(count);
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
        auto running = client->hasRunningRootTask();
        if (running.has_value()) {
            if (running.value()) {
                statusBar->setStatus(TextIndexStatusBar::Status::Indexing);
            } else {
                QString lastTime = client->getLastUpdateTime();
                bool exitsts = client->indexExists().has_value() && client->indexExists().value();
                statusBar->setStatus(!exitsts ? TextIndexStatusBar::Status::Failed : TextIndexStatusBar::Status::Completed,
                                     lastTime);
            }
        }
    } else {
        statusBar->setStatus(TextIndexStatusBar::Status::Inactive);
    }
}

bool CheckBoxWidthTextIndex::shouldHandleIndexEvent(const QString &path, TextIndexClient::TaskType type) const
{
    // TODO(search): dfm-search
    return checkBox->isChecked() && path == QDir::homePath() && type != TextIndexClient::TaskType::Remove;
}

}   // namespace dfmplugin_search
