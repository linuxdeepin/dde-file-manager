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
#include "dfmlaucheventhandler.h"

DFMLauchEventHandler::DFMLauchEventHandler()
{

}

DFMLauchEventHandler::~DFMLauchEventHandler()
{

}

bool DFMLauchEventHandler::canAsynProcess() {
    return true;
}

void DFMLauchEventHandler::event(const DFMEventPointer &event)
{
    if (DFMEvent::LauchEvent > event->type()
            || event->type() >= DFMEvent::WindowEvent) {
        return;
    }

    DFMLauchEventPointer e = qSharedPointerDynamicCast<DFMLauchEvent>(event);
    if (!e) return;

    switch (e->type()) {

    case DFMLauchEvent::OpenFile:
        openFile(e);
        break;

    case DFMLauchEvent::OpenFileByApp:
        openFileByApp(e);
        break;

    case DFMLauchEvent::OpenUrlByTerminal:
        openFileByApp(e);
        break;

    case DFMLauchEvent::CompressFile:
        compressFile(e);
        break;

    case DFMLauchEvent::DecompressFile:
        decompressFile(e);
        break;

    case DFMLauchEvent::DecompressFileHere:
        decompressFileHere(e);
        break;

    case DFMLauchEvent::WriteUrlToClipboard:
        writeUrlToClipboard(e);
        break;

    default:
        qWarning() << "DFMLauchEvent to process event is unknown type";
        break;
    }
}

void DFMLauchEventHandler::openFile(const DFMLauchEventPointer &event) {
    qInfo() << Q_FUNC_INFO;
}

void DFMLauchEventHandler::openFileByApp(const DFMLauchEventPointer &event) {
    qInfo() << Q_FUNC_INFO;
}

void DFMLauchEventHandler::openUrlByTerminal(const DFMLauchEventPointer &event) {
    qInfo() << Q_FUNC_INFO;
}

void DFMLauchEventHandler::compressFile(const DFMLauchEventPointer &event) {
    qInfo() << Q_FUNC_INFO;
}

void DFMLauchEventHandler::decompressFile(const DFMLauchEventPointer &event) {
    qInfo() << Q_FUNC_INFO;
}

void DFMLauchEventHandler::decompressFileHere(const DFMLauchEventPointer &event) {
    qInfo() << Q_FUNC_INFO;
}

void DFMLauchEventHandler::writeUrlToClipboard(const DFMLauchEventPointer &event) {
    qInfo() << Q_FUNC_INFO;
}


