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
#ifndef DFMFILEEVENTHANDLER_H
#define DFMFILEEVENTHANDLER_H

#include "dfmabstracteventhandler.h"

#include "event/protocol/dfmfileevent.h"

class DFMFileEvent;

class DFMFileEventHandler :public DFMAbstractEventHandler
{

public:
    DFMFileEventHandler();

    virtual ~DFMFileEventHandler() override;

    virtual bool canAsynProcess() override;

    virtual QString scheme() override;

    virtual void event(const DFMEventPointer &event) override;

    virtual void createDir(const DFMFileEventPointer &event);

    virtual void createFile(const DFMFileEventPointer &event);

    virtual void deleteDir(const DFMFileEventPointer &event);

    virtual void deleteFile(const DFMFileEventPointer &event);

    virtual void moveDir(const DFMFileEventPointer &event);

    virtual void moveFile(const DFMFileEventPointer &event);

    virtual void pasteFile(const DFMFileEventPointer &event);

    virtual void pasteDir(const DFMFileEventPointer &event);

    virtual void renameDir(const DFMFileEventPointer &event);

    virtual void renameFile(const DFMFileEventPointer &event);

    virtual void createSymlink(const DFMFileEventPointer &event);

};

#endif // DFMFILEEVENTHANDLER_H
