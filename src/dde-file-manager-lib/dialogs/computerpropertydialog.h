/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     gongheng<gongheng@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             gongheng<gongheng@uniontech.com>
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

#ifndef COMPUTERPROPERTYDIALOG_H
#define COMPUTERPROPERTYDIALOG_H

#include <ddialog.h>

#include <QWidget>
#include <QThread>

QT_BEGIN_HEADER
class QFrame;
QT_END_NAMESPACE

DWIDGET_USE_NAMESPACE

class DBusSystemInfo;

// 添加子线程，获取计算机配置信息，以及发送配置信息更新信号
class GetInfoWork : public QThread
{
    Q_OBJECT
public:
    explicit GetInfoWork(QObject *parent = nullptr);
    // 设置初始数据(未获得信息的项目)
    void setInitData(QList<QString> datas);
    // 开始线程
    void startWork();
    // 结束线程
    void stopWork();

signals:
    // 发送该信号，刷新最新属性值
    void sigSendInfo(QMap<QString, QString> mapNewDatas);

protected:
    void run() override;

private:
    QList<QString> m_datas = {};
    bool m_bStop = false;
};

class ComputerPropertyDialog : public DDialog
{
    Q_OBJECT
public:
    explicit ComputerPropertyDialog(QWidget *parent = nullptr);
    ~ComputerPropertyDialog() override;
    void initUI();
    QHash<QString, QString> getMessage(const QStringList& data);
    // 开启子线程，更新计算机信息
    void updateComputerInfo();

signals:
    void closed();

protected:
    // 重写隐藏事件，实现窗口隐藏时，关闭属性更新线程
    void hideEvent(QHideEvent *event) Q_DECL_OVERRIDE;

private slots:
    // 更新计算机信息
    void slotSetInfo(QMap<QString, QString> mapNewDatas);

private:
    DBusSystemInfo *m_systemInfo = nullptr;
    // 缓存计算机属性值项
    QHash<QString, QLabel*> m_mapItems = {};
    // 缓存未完成的计算机属性等待项
    QHash<QString, QFrame*> m_mapNotFinish = {};
    // 创建属性更新线程对象
    GetInfoWork *m_getInfoWork = nullptr;
};

#endif // COMPUTERPROPERTYDIALOG_H
