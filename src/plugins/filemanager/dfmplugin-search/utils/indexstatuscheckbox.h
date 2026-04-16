// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef INDEXSTATUSCHECKBOX_H
#define INDEXSTATUSCHECKBOX_H

#include <DTipLabel>
#include <DSpinner>

#include <QCheckBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>

namespace dfmplugin_search {

class IndexStatusCheckBox : public QWidget
{
    Q_OBJECT

public:
    enum class Status {
        Indexing,
        Completed,
        Failed,
        Inactive
    };

    explicit IndexStatusCheckBox(QWidget *parent = nullptr);

    void setDisplayText(const QString &text);
    void setChecked(bool checked);
    bool isChecked() const;

    void setInactiveText(const QString &text);
    void setIndexingTexts(const QString &initialText,
                          const QString &filesText,
                          const QString &itemsText);
    void setCompletedText(const QString &mainText,
                          const QString &linkText,
                          const QString &href = QStringLiteral("update"));
    void setFailedText(const QString &mainText,
                       const QString &linkText,
                       const QString &href = QStringLiteral("update"));

    void setStatus(Status status);
    Status status() const;
    void updateIndexingProgress(qlonglong count, qlonglong total);

Q_SIGNALS:
    void checkStateChanged(Qt::CheckState state);
    void resetRequested();

private:
    void setRunning(bool running);
    void updateUI(Status status);
    QPixmap iconPixmap(const QString &iconName, int size);
    void setFormattedTextWithLink(const QString &mainText, const QString &linkText, const QString &href);
    QString linkStyle() const;

private:
    QCheckBox *m_checkBox { nullptr };
    DTK_NAMESPACE::Widget::DSpinner *m_spinner { nullptr };
    DTK_NAMESPACE::Widget::DTipLabel *m_iconLabel { nullptr };
    DTK_NAMESPACE::Widget::DTipLabel *m_msgLabel { nullptr };
    QHBoxLayout *m_statusLayout { nullptr };
    Status m_status { Status::Inactive };
    QString m_inactiveText;
    QString m_indexingInitialText;
    QString m_indexingFilesText;
    QString m_indexingItemsText;
};

}   // namespace dfmplugin_search

#endif   // INDEXSTATUSCHECKBOX_H
