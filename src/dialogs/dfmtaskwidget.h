/*
 * Copyright (C) 2019 Deepin Technology Co., Ltd.
 *
 * Author:     Mike Chen <kegechen@gmail.com>
 *
 * Maintainer: Mike Chen <chenke_cm@deepin.com>
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
#ifndef DFMTASKWIDGET_H
#define DFMTASKWIDGET_H
#include <QAbstractButton>
#include <QLabel>
#include <QWidget>
#include <durl.h>

class DFMElidedLable : public QLabel
{
    Q_OBJECT
public:
    explicit DFMElidedLable(QWidget *parent = nullptr);
    virtual ~DFMElidedLable();
    void setText(const QString &text);
};

class DFMTaskWidgetPrivate;
class DFMTaskWidget : public QWidget
{
    Q_OBJECT
public:
    enum BUTTON {
        PAUSE,
        STOP,
        SKIP,
        REPLACE,
        COEXIST,
        CHECKBOX_NOASK
    };
    explicit DFMTaskWidget(QWidget *parent = nullptr);
    virtual ~DFMTaskWidget();
    QString taskId();
    void setTaskId(const QString &taskId);
    void setMsgText(const QString &srcMsg, const QString &dstMsg);
    void setSpeedText(const QString &speed, const QString &rmtime);
    void setProgressValue(int value); // <0 stop, >=0 start
    void setErrorMsg(const QString &err);
    void setConflictMsg(const DUrl &src, const DUrl &dst);
    void setButtonText(BUTTON bt, const QString &text);
    void setHoverEnable(bool enable);
    void hideButton(BUTTON bt, bool hidden = true);
    void setHandleingError(const bool &handleing);
    bool isHandleingError() const;
    QAbstractButton *getButton(BUTTON bt);

    void progressStart();

protected:
    void onMouseHover(bool hover);
    void showConflictButtons(bool showBtns = true, bool showConflict = true);

    virtual void enterEvent(QEvent *event);
    virtual void leaveEvent(QEvent *event);
    virtual void paintEvent(QPaintEvent *event);

signals:
    void butonClicked(BUTTON btn);
    void heightChanged();
    void hoverChanged(bool);

public slots:
    void onProgressChanged(qreal progress, qint64 writeData);
    void onBurnProgressChanged(qreal progress, qint64 writeData);
    void onSpeedUpdated(qint64 speed);
    void onStateChanged(int state);

private:
    QScopedPointer<DFMTaskWidgetPrivate> d_private;
    Q_DECLARE_PRIVATE_D(d_private, DFMTaskWidget)
};

#endif // DFMTASKWIDGET_H
