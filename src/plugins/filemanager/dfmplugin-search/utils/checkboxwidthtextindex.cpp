// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
#include "checkboxwidthtextindex.h"

#include "searchmanager/searchmanager.h"

#include <dfm-search/dsearch_global.h>

#include <DDciIcon>

#include <QVBoxLayout>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QStandardPaths>
#include <QDir>

DGUI_USE_NAMESPACE
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
    iconLabel->setPixmap(iconPixmap("dialog-ok", 16));

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
        QString lastTime = TextIndexClient::instance()->getLastUpdateTime();
        msgLabel->setText(tr("Index update completed, last update time: %1").arg(lastTime));
        iconLabel->setPixmap(iconPixmap("dialog-ok", 16));
        updateBtn->setText(tr("Update index now"));
        break;
    }
    case Status::Failed:
        setRunning(false);
        iconLabel->setPixmap(iconPixmap("dialog-error", 16));
        msgLabel->setText(tr("Index update failed, please"));
        updateBtn->setText(tr("try updating again."));
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
        if (exitsts) {
            client->startTask(TextIndexClient::TaskType::Update, DFMSEARCH::Global::defaultIndexedDirectory());
        } else {
            client->startTask(TextIndexClient::TaskType::Create, DFMSEARCH::Global::defaultIndexedDirectory());
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
        auto running = client->hasRunningRootTask();
        if (running.has_value()) {
            if (running.value()) {
                statusBar->setStatus(TextIndexStatusBar::Status::Indexing);
            } else {
                bool exitsts = client->indexExists().has_value() && client->indexExists().value();
                statusBar->setStatus(!exitsts ? TextIndexStatusBar::Status::Failed : TextIndexStatusBar::Status::Completed);
            }
        }
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
