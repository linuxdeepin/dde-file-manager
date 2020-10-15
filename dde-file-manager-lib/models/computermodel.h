/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *               2019 ~ 2019 Chris Xiong
 *
 * Author:     Chris Xiong<chirs241097@gmail.com>
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

#ifndef COMPUTERMODEL_H
#define COMPUTERMODEL_H

#include "ddiskmanager.h"
#include "deviceinfo/udisklistener.h"

#include <QAbstractItemModel>
#include <QFuture>

#include "durl.h"
#include "interfaces/dabstractfileinfo.h"

//url schemes internal to ComputerModel
#define SPLITTER_SCHEME "splitter"
#define WIDGET_SCHEME "widget"

class ComputerView;

struct ComputerModelItemData
{
    enum Category
    {
        cat_user_directory,
        cat_internal_storage,
        cat_external_storage,
        cat_splitter,
        cat_widget
    };
    DAbstractFileInfoPointer fi;
    DUrl url;
    QString sptext;
    QWidget* widget = nullptr;
    Category cat;
};

class ComputerModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    enum DataRoles
    {
        SizeTotalRole = Qt::UserRole + 1,   //uint64_t
        SizeInUseRole = Qt::UserRole + 2,   //uint64_t
        FileSystemRole = Qt::UserRole + 3,  //QString
        UsgWidgetRole = Qt::UserRole + 4,   //No longer used
        ICategoryRole = Qt::UserRole + 5,   //ComputerModelItemData::Category
        OpenUrlRole = Qt::UserRole + 6,     //DUrl
        MountOpenUrlRole = Qt::UserRole + 7,//DUrl
        ActionVectorRole = Qt::UserRole + 8,//QVector<MenuAction>
        DFMRootUrlRole = Qt::UserRole + 9,  //DUrl
        VolumeTagRole = Qt::UserRole + 10,  //卷标 sr0
        IconNameRole = Qt::UserRole + 11,   //图标名称
        SchemeRole = Qt::UserRole + 12, //QString
        ProgressRole = Qt::UserRole + 13, // Bool(Progress visible)
        SizeRole = Qt::UserRole + 14, // Bool(Size visible)
        DiscUUIDRole = Qt::UserRole + 15,   //设备UUID
        DiscOpticalRole = Qt::UserRole + 16,//光盘是否是空盘
    };
    Q_ENUM(DataRoles)

    explicit ComputerModel(QObject* parent = nullptr);
    ~ComputerModel() override;

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    QModelIndex findIndex(const DUrl &url) const;
    int itemCount() const;
    void getRootFile();

public Q_SLOTS:
    void addItem(const DUrl &url, QWidget *w = nullptr);
    void insertAfter(const DUrl &url, const DUrl &ref, QWidget *w = nullptr);
    void insertBefore(const DUrl &url, const DUrl &ref, QWidget *w = nullptr);
    void removeItem(const DUrl &url);
    void onGetRootFile(const DAbstractFileInfoPointer &chi);
    void onOpticalChanged();

Q_SIGNALS:
    void itemCountChanged(int nitems);
    void opticalChanged();

private:
    ComputerView* par;
    QScopedPointer<DDiskManager> m_diskm;
    QList<ComputerModelItemData> m_items;
    DAbstractFileWatcher* m_watcher;
    int m_nitems;
    static bool m_isQueryRootFileFinshed; // 查询跟目录是否完成

#ifdef ENABLE_ASYNCINIT
    QPair<bool,QFuture<void>> m_initThread; //初始化线程，first为是否强制结束线程
#endif
    void initItemData(ComputerModelItemData &data, const DUrl &url, QWidget *w);
    int findItem(const DUrl &url);

    static DUrl makeSplitterUrl(QString text);
    int findNextSplitter(const int &index);
};


#endif // COMPUTERMODEL_H
