// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
#include "checkboxwidthtextindex.h"

#include "searchmanager/searchmanager.h"

#include <DDciIcon>

#include <QVBoxLayout>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QStandardPaths>
#include <QDir>

DGUI_USE_NAMESPACE
namespace dfmplugin_search {

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
    iconLabel->setPixmap(iconPixmap("dialog-ok", 16));

    msgLabel = new DTK_NAMESPACE::Widget::DTipLabel("", this);
    msgLabel->setAlignment(Qt::AlignLeft);
    msgLabel->setWordWrap(true);

    // 添加到布局
    layout->addWidget(spinner);
    layout->addWidget(iconLabel);
    layout->addWidget(msgLabel, 1);
    layout->addStretch();

    // 初始状态
    spinner->hide();
    iconLabel->show();
    hide();
}

void TextIndexStatusBar::setRunning(bool running)
{
    msgLabel->show();
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

QPixmap TextIndexStatusBar::iconPixmap(const QString &iconName, int size)
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
    }

    return px;
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
        iconLabel->setPixmap(iconPixmap("dialog-ok", 16));
        break;
    }
    case Status::Failed:
        setRunning(false);
        msgLabel->setText(tr("Index update failed, please turn on the \"Full-Text search\" switch again"));
        iconLabel->setPixmap(iconPixmap("dialog-error", 16));
        break;
    case Status::Hidden:
        iconLabel->hide();
        msgLabel->hide();
        hide();
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

    connect(checkBox, &QCheckBox::stateChanged, this, [this](int state) {
        if (checkBox->isChecked()) {
            statusBar->show();
            statusBar->setStatus(TextIndexStatusBar::Status::Indexing);
        } else {
            statusBar->setStatus(TextIndexStatusBar::Status::Hidden);
        }
        emit stateChanged(state);
    });

    auto client = TextIndexClient::instance();
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

    connect(SearchManager::instance(), &SearchManager::enableFullTextSearchChanged,
            this, [this](bool enable) {
                setChecked(enable);
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
        statusBar->show();
        auto client = TextIndexClient::instance();
        auto running = client->hasRunningRootTask();
        if (running.has_value()) {
            if (running.value()) {
                statusBar->setStatus(TextIndexStatusBar::Status::Indexing);
            } else {
                QString lastTime = client->getLastUpdateTime();
                statusBar->setStatus(lastTime.isEmpty() ? TextIndexStatusBar::Status::Failed : TextIndexStatusBar::Status::Completed,
                                     lastTime);
            }
        }
    } else {
        statusBar->setStatus(TextIndexStatusBar::Status::Hidden);
    }
}

bool CheckBoxWidthTextIndex::shouldHandleIndexEvent(const QString &path, TextIndexClient::TaskType type) const
{
    // TODO(search): dfm-search
    return checkBox->isChecked() && path == QDir::homePath() && type != TextIndexClient::TaskType::Remove;
}

}   // namespace dfmplugin_search
