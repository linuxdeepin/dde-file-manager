/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/
#pragma once

#include <xcb/xcb_ewmh.h>

namespace Xcb {

class XcbMisc
{
public:
    enum Orientation {
        OrientationLeft,
        OrientationRight,
        OrientationTop,
        OrientationBottom
    };

    enum WindowType {
        Dock,
        Desktop
    };

    virtual ~XcbMisc();

    static XcbMisc &instance();

    void set_window_type(xcb_window_t winId, WindowType winType);
    void set_strut_partial(xcb_window_t winId, Orientation orientation, uint strut, uint start, uint end);

    xcb_window_t find_dock_window(int screen_nbr);
    xcb_ewmh_wm_strut_partial_t get_strut_partial(xcb_window_t winId);
private:
    XcbMisc();

    bool is_dock_window(xcb_window_t winId);

    xcb_ewmh_connection_t m_ewmh_connection;
};

}
