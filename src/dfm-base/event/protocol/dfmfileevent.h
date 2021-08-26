/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liyigang<liyigang@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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
#ifndef DFMFILEEVENT_H
#define DFMFILEEVENT_H

#include "event/protocol/dfmevent.h"

#include <QUrl>

/* @class DFMFileEventData
 * @brief DFMFileEvent 使用的事件数据
 * @param[QUrl] source 操作的原Url，可以是QUrl路径与QUrl标识的文件路径
 * @param[QUrl] source 操作的目标Url，可以是QUrl路径与QUrl标识的文件路径
 */
class DFMFileEventData {
    QUrl m_source;
    QUrl m_target;
public:

    DFMFileEventData(QUrl source,QUrl target) {
        m_source = source;
        m_target = target;
    }

    DFMFileEventData(const DFMFileEventData &data)
    {
        m_source = data.m_source;
        m_target = data.m_target;
    }

    void setSource(const QUrl &source){m_source = source;}
    void setTarget(const QUrl &target){m_target = target;}

    QUrl source(){return m_source;}
    QUrl target(){return m_target;}
};

/* @class DFMFileEventData
 * @brief DFMFileEvent 使用的事件数据，该事件可以是一对多的关联关系
 * 比如创建多个文件加，需要带一个Type的ID，同时传递多个文件操作数据。
 * @member[QList<DFMFileEventData>] m_datas source 操作的原Url，可以是QUrl路径与QUrl标识的文件路径
 */
class DFMFileEvent: public DFMEvent
{
public:
    enum Type{
        CreateDir = DFMEvent::FileEvent + 1,  ///< 创建文件夹事件
        CreateFile,    ///< 创建文件事件
        DeleteDir,     ///< 删除文件夹事件
        DeleteFile,    ///< 删除文件事件
        MoveDir,       ///< 移动文件夹事件
        MoveFile,      ///< 移动文件事件
        PasteFile,     ///< 粘贴文件事假
        PasteDir,      ///< 粘贴文件夹事件
        RenameDir,     ///< 重名名文件夹事件
        RenameFile,    ///< 重命名文件事件
        CreateSymlink, ///< 创建链接事件
    }; Q_ENUM(Type)

    /* @method getDatas
     * @brief 获取事件附带的数据
     * @return QList<DFMFileEventData> 数据列表
     */

    DFMFileEvent(int type);

    QList<DFMFileEventData> getDatas() const{
        return m_datas;
    }

private:
    QList<DFMFileEventData> m_datas; //操作的数据列表
};

typedef QSharedPointer<DFMFileEvent> DFMFileEventPointer;

#endif // DFMFILEEVENT_H
