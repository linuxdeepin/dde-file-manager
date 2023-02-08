/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lixiang<lixianga@uniontech.com>
 *
 * Maintainer: lixiang<lixianga@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef EDITSTACKEDWIDGET_H
#define EDITSTACKEDWIDGET_H

#include "dfmplugin_propertydialog_global.h"

#include <DLabel>
#include <DIconButton>

#include <QStackedWidget>
#include <QTextEdit>
#include <QTextLayout>
#include <QTextDocument>

namespace dfmplugin_propertydialog {
class NameTextEdit : public QTextEdit
{
    Q_OBJECT

public:
    explicit NameTextEdit(const QString &text = "", QWidget *parent = nullptr);
    virtual ~NameTextEdit() override;

    bool isCanceled() const;
    void setIsCanceled(bool isCancele);
    inline void useCharCountLimit() { useCharCount = true; }

signals:
    void editFinished();

public slots:
    void setPlainText(const QString &text);
    void slotTextChanged();

protected:
    void focusOutEvent(QFocusEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    inline int textLength(const QString &text)
    {
        return useCharCount ? text.length() : text.toLocal8Bit().length();
    }

private:
    bool isCancel = false;
    bool useCharCount = false;
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
