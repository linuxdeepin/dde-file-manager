/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lixiang<lixianga@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             lixiang<lixianga@uniontech.com>
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

#ifndef TEXTPREVIEW_H
#define TEXTPREVIEW_H

#include <QObject>
#include <QWidget>
#include <QPointer>

#include "dfmfilepreview.h"
#include "durl.h"

QT_BEGIN_NAMESPACE
class QPlainTextEdit;
QT_END_NAMESPACE

class TextPreview : public DFM_NAMESPACE::DFMFilePreview
{
    Q_OBJECT

public:
    explicit TextPreview(QObject *parent = nullptr);
    ~TextPreview() override;

    bool setFileUrl(const DUrl &url) override;
    DUrl fileUrl() const override;

    QWidget *contentWidget() const override;

    QString title() const override;
    bool showStatusBarSeparator() const override;

    QWidget *previewWidget();

public slots:
    /**
     * @brief valueChanged 用于文本内容分段加载使用
     * @param index 当前滚动条的值，这里使用的是垂直滚动条
     */
    void valueChanged(int index);

private:
    DUrl m_url;
    QString m_title;

    QPointer<QPlainTextEdit> m_textBrowser;

    //! 操作文件的对象
    QPointer<QIODevice> m_device;

    //! 用来控制m_textBrowser中使用appendPlainText追加数据后会发送valueChanged信号导致在
    //! 重复调用曹函数valueChange导致程序崩溃，所以m_flg为true时valueChanged曹函数中的逻辑
    //! 可以执行，否则不执行
    bool m_flg = false;
};

#endif // TEXTPREVIEW_H
