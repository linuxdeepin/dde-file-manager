// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef CHECKBOXWIDTHTEXTINDEX_H
#define CHECKBOXWIDTHTEXTINDEX_H

#include "utils/textindexclient.h"

#include <DTipLabel>
#include <DSpinner>

#include <QCheckBox>
#include <QHBoxLayout>
#include <QLabel>

namespace dfmplugin_search {

class TextIndexStatusBar : public QWidget
{
    Q_OBJECT

public:
    enum class Status {
        Indexing,   // 正在更新
        Completed,   // 更新完成
        Failed,   // 更新失败
        Inactive   // 未开启
    };

    explicit TextIndexStatusBar(QWidget *parent = nullptr);
    void setStatus(Status status, const QVariant &data = QVariant());
    void updateIndexingProgress(qlonglong count, qlonglong total);
    void setRunning(bool running);
    Status status() const;
    void updateUI(Status status);

    DTK_NAMESPACE::Widget::DTipLabel *msgLabel { nullptr };

    // 格式化带无下划线链接的文本
    void setFormattedTextWithLink(const QString &mainText, const QString &linkText, const QString &href);

Q_SIGNALS:
    void resetIndex();

private:
    QPixmap iconPixmap(const QString &iconName, int size);
    // 获取链接样式定义
    QString getLinkStyle() const;

    Status currentStatus { Status::Inactive };
    DTK_NAMESPACE::Widget::DSpinner *spinner { nullptr };
    DTK_NAMESPACE::Widget::DTipLabel *iconLabel { nullptr };
    QHBoxLayout *boxLayout { nullptr };
};

// 用于跟踪当前检查索引存在的上下文
enum class IndexCheckContext {
    None,   // 无特定上下文
    ResetIndex,   // 重置索引按钮触发
    InitStatus   // 初始化状态检查
};

class CheckBoxWidthTextIndex : public QWidget
{
    Q_OBJECT
public:
    explicit CheckBoxWidthTextIndex(QWidget *parent = nullptr);
    void connectToBackend();
    void setDisplayText(const QString &text);
    void setChecked(bool checked);
    void initStatusBar();

Q_SIGNALS:
    void checkStateChanged(Qt::CheckState);

private:
    QCheckBox *checkBox { nullptr };
    TextIndexStatusBar *statusBar { nullptr };
    IndexCheckContext currentIndexCheckContext { IndexCheckContext::None };
    bool shouldHandleIndexEvent(const QString &path, TextIndexClient::TaskType type) const;
};
}   // namespace dfmplugin_search
#endif   // CHECKBOXWIDTHTEXTINDEX_H
