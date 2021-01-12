/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
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

#include "showfileplugin.h"
#include "interfaces/dfilemenu.h"
#include <QUrl>
#include <QMenu>
#include <QFileInfo>
#include <QFile>
#include <QTextStream>
#include <QProcess>
#include <QVBoxLayout>
#include <QTextBrowser>

ShowFilePlugin::ShowFilePlugin(QObject *parent) :
    QObject(parent)
{

    aIcon[0] = QIcon(":/images/a-s0.svg");
    aIcon[1] = QIcon(":/images/a-s1.svg");
    aIcon[2] = QIcon(":/images/a-s2.svg");
    aIcon[3] = QIcon(":/images/a-s3.svg");
    aIcon[4] = QIcon(":/images/a-s4.svg");


    bIcon[0] = QIcon(":/images/b-s0.svg");
    bIcon[1] = QIcon(":/images/b-s1.svg");
    bIcon[2] = QIcon(":/images/b-s2.svg");
    bIcon[3] = QIcon(":/images/b-s3.svg");
    bIcon[4] = QIcon(":/images/b-s4.svg");


        //测试菜单, 一共是 6个
        // 一级菜单：测试4及测试，其中 测试有二级菜单
        // 二级菜单： 测试 下面有：测试0-测试3
    cmdTab << "测试"          // 有子菜单
           << "测试0"
           << "测试1"
           << "测试2"
           << "测试3"
           << "测试4";        // 一级菜单
}

QList<QIcon> ShowFilePlugin::additionalIcons(const QString &file)
{
    // 取回 自己格式中的a, b
    int a;
    int b;
    int ret = analyzeFile(QUrl(file).path(), a, b );
    if( ret ){
        return QIconList();
    }

    QIconList icons;
    icons << aIcon[ a ];
    icons << bIcon[ b ];

    return icons;
}

QList<QAction *> ShowFilePlugin::additionalMenu(const QStringList &files, const QString &currentDir)
{
    if(files.count() >= 2)
        return QActionList();

    QString filename = QUrl(files.first()).path();

    int a = 4;
    int b;

    int ret = analyzeFile( filename, a, b );
    if( ret )
    {
        return QActionList();
    }
    if( a == 4 ) //只显示 测试4
    {
        QAction* action = new QAction(cmdTab[ 5 ],this);
        action->setData(QUrl(files.first()).path());
        connect(action, &QAction::triggered, this, &ShowFilePlugin::onActionTriggered);
        return QActionList() << action;

    }
    else  // 要 显示二级菜单的第一级菜单
    {
        QAction* action = new QAction(cmdTab[ 0 ],this);
        action->setData(QUrl(files.first()).path());

        if( a == 4 ) //只显示 测试0-测试3
        {
            return QActionList();
        }
            // 显示二级菜单
        DFileMenu* subMenu = new DFileMenu;
        QActionList subActions;
        for( int i = 0; i <= a; i++ )
        {
            QAction* subAction = new QAction(cmdTab[ i+1 ], this);
            subAction->setData(filename);
            subActions << subAction;
            connect(subAction, &QAction::triggered, this, &ShowFilePlugin::onActionTriggered);
        }

        subMenu->addActions(subActions);
        action->setMenu(subMenu);

        return QActionList() << action;
    }

    return QActionList();
}

QList<QAction *> ShowFilePlugin::additionalEmptyMenu(const QString &currentDir)
{
    return QActionList();
}

QWidget *ShowFilePlugin::expandWidget(const QString &file)
{

    int a;
    int b;
    int ret = analyzeFile(QUrl(file).path(), a, b );
    if( ret )
    {
        return NULL;
    }

    QWidget*w = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout;
    QTextBrowser* tb = new QTextBrowser(w);
    tb->setText(QString("Property extend info test:%1 \n%2 \n%3 \n%4")
                .arg(file)
                .arg("bbbbb")
                .arg("ccccccccccc")
                .arg("ccccccccc")
                .arg("ddddddddddddd"));
    layout->addWidget(tb);
    w->setLayout(layout);
    w->setFixedSize(320,150);
    tb->setFixedSize(w->size());
    return w;
}

QString ShowFilePlugin::expandWidgetTitle(const QString &file)
{
    int a;
    int b;
    int ret = analyzeFile(QUrl(file).path(), a, b );
    if( ret )
    {
        return "";
    }
    return "title 1";
}

void ShowFilePlugin::onActionTriggered()
{
    QAction* action = qobject_cast<QAction*>(sender());
    if(!action)
        return;

    QString actionName = action->text();
    QString file = action->data().toString();
    QStringList args;

    args << actionName << file;

    QProcess::startDetached("/opt/test/testprog", args);
}

int ShowFilePlugin::analyzeFile(const QString &filename, int &a, int &b)
{
    //测试文件格式为，首行 test[01]开始的，数字0为0-4, 并将这2个位数字分别放在 a, b中
    QFileInfo info( filename );
    if(!info.isFile())   // 为目录
    {
        return 2;
    }

    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return 2;
    }

    QTextStream in(&file);
    while (!in.atEnd())
    {
        QString line = in.readLine();   //只处理第一行
        bool ok;
        QString str = line.mid( 5, 2 );
        ushort dec = str.toUShort(&ok);
        if( ok )
        {
            a = dec/10;
            b = dec%10;
            if( a > 4 || a < 0 )
            {
                return 1;
            }
            if( b > 4 || b < 0 )
            {
                return 1;
            }
            str = QString("test[%1%2]").arg( a ).arg( b );
            int posi = line.indexOf( str );
            if( posi == 0 )
            {
               return 0;
            }
            else
            {
                return 1;
            }
        }

        break;
    }
    return 1;
}
