// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef CHECKBOXWIDTHTEXTINDEX_H
#define CHECKBOXWIDTHTEXTINDEX_H

#include "utils/textindexclient.h"

#include <DTipLabel>
#include <DSpinner>

#include <QCheckBox>

namespace dfmplugin_search {

class TextIndexStatusBar : public QWidget
{
    Q_OBJECT

public:
    enum class Status {
        Indexing,   // 正在更新
        Completed,   // 更新完成
        Failed,   // 更新失败
        Hidden   // 隐藏状态
    };

    explicit TextIndexStatusBar(QWidget *parent = nullptr);
    void setStatus(Status status, const QVariant &data = QVariant());
    void updateIndexingProgress(qlonglong count);
    void setRunning(bool running);
    Status status() const;

private:
    Status currentStatus { Status::Hidden };
    DTK_NAMESPACE::Widget::DSpinner *spinner { nullptr };
    DTK_NAMESPACE::Widget::DTipLabel *iconLabel { nullptr };
    DTK_NAMESPACE::Widget::DTipLabel *msgLabel { nullptr };
};

class CheckBoxWidthTextIndex : public QWidget
{
    Q_OBJECT
public:
    explicit CheckBoxWidthTextIndex(QWidget *parent = nullptr);
    void setDisplayText(const QString &text);
    void setChecked(bool checked);
    void initStatusBar();

Q_SIGNALS:
    void stateChanged(int);

private:
    QCheckBox *checkBox { nullptr };
    TextIndexStatusBar *statusBar { nullptr };
    bool shouldHandleIndexEvent(const QString &path, TextIndexClient::TaskType type) const;
};
}   // namespace dfmplugin_search
#endif   // CHECKBOXWIDTHTEXTINDEX_H
