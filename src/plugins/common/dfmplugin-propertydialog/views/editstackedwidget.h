// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EDITSTACKEDWIDGET_H
#define EDITSTACKEDWIDGET_H

#include "dfmplugin_propertydialog_global.h"

#include <DLabel>
#include <DIconButton>
#include <DArrowRectangle>
#include <DTextEdit>

#include <QStackedWidget>
#include <QTextLayout>
#include <QTextDocument>

namespace dfmplugin_propertydialog {
class NameTextEdit : public DTK_WIDGET_NAMESPACE::DTextEdit
{
    Q_OBJECT

public:
    explicit NameTextEdit(const QString &text = "", QWidget *parent = nullptr);
    virtual ~NameTextEdit() override;

    bool isCanceled() const;
    void setIsCanceled(bool isCancele);
    inline void setCharCountLimit() { useCharCount = true; }
    void setMaxLength(int len);

signals:
    void editFinished();

public slots:
    void setPlainText(const QString &text);
    void slotTextChanged();
    void showAlertMessage(const QString &text, int duration = 3000);

protected:
    void focusOutEvent(QFocusEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    inline int textLength(const QString &text)
    {
        return useCharCount ? text.length() : text.toLocal8Bit().length();
    }

private:
    DTK_WIDGET_NAMESPACE::DArrowRectangle *createTooltip();

    bool isCancel = false;
    bool useCharCount = false;
    int maxLengthVal = 0;

    DTK_WIDGET_NAMESPACE::DArrowRectangle *tooltip { nullptr };
};

class EditStackedWidget : public QStackedWidget
{
    Q_OBJECT
public:
    explicit EditStackedWidget(QWidget *parent = nullptr);
    virtual ~EditStackedWidget() override;

private:
    void initUI();
    void initTextShowFrame(QString fileName);

public slots:
    void renameFile();
    void showTextShowFrame();
    void selectFile(const QUrl &url);
    void mouseProcess(QMouseEvent *event);

Q_SIGNALS:
    void selectUrlRenamed(const QUrl &url);

private:
    NameTextEdit *fileNameEdit { nullptr };
    DTK_WIDGET_NAMESPACE::DIconButton *nameEditIcon { nullptr };
    QFrame *textShowFrame { nullptr };
    QUrl fileUrl {};
};
}
#endif   // EDITSTACKEDWIDGET_H
