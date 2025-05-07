// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef CHECKBOXWIDTHTEXTINDEX_H
#define CHECKBOXWIDTHTEXTINDEX_H

#include "utils/textindexclient.h"

#include <DTipLabel>
#include <DSpinner>
#include <DCommandLinkButton>

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
        Inactive   // 未开启
    };

    explicit TextIndexStatusBar(QWidget *parent = nullptr);
    void setStatus(Status status, const QVariant &data = QVariant());
    void updateIndexingProgress(qlonglong count);
    void setRunning(bool running);
    Status status() const;

Q_SIGNALS:
    void resetIndex();

private:
    QPixmap iconPixmap(const QString &iconName, int size);

    Status currentStatus { Status::Inactive };
    DTK_NAMESPACE::Widget::DSpinner *spinner { nullptr };
    DTK_NAMESPACE::Widget::DTipLabel *iconLabel { nullptr };
    DTK_NAMESPACE::Widget::DTipLabel *msgLabel { nullptr };
    DTK_WIDGET_NAMESPACE::DCommandLinkButton *updateBtn { nullptr };
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
    bool shouldHandleIndexEvent(const QString &path, TextIndexClient::TaskType type) const;
};
}   // namespace dfmplugin_search
#endif   // CHECKBOXWIDTHTEXTINDEX_H
