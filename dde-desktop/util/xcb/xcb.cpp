/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include <QDebug>
#include <QX11Info>

#include <xcb/xcb.h>
#include <xcb/xcb_ewmh.h>

#include "xcb.h"


namespace Xcb
{

XcbMisc::XcbMisc()
{
    xcb_intern_atom_cookie_t *cookie = xcb_ewmh_init_atoms(QX11Info::connection(), &m_ewmh_connection);
    xcb_ewmh_init_atoms_replies(&m_ewmh_connection, cookie, NULL);
}

XcbMisc::~XcbMisc()
{

}

XcbMisc &XcbMisc::instance()
{
    static XcbMisc *_xcb_misc_instance = new XcbMisc;
    return *_xcb_misc_instance;
}

void XcbMisc::set_window_type(xcb_window_t winId, WindowType winType)
{
    xcb_atom_t atoms[1];

    switch (winType) {
    case WindowType::Desktop:
        atoms[0] = m_ewmh_connection._NET_WM_WINDOW_TYPE_DESKTOP;
        break;
    case WindowType::Dock:
        atoms[0] = m_ewmh_connection._NET_WM_WINDOW_TYPE_DOCK;
        break;
    default:
        break;
    }

    xcb_ewmh_set_wm_window_type(&m_ewmh_connection, winId, 1, atoms);
}


xcb_ewmh_wm_strut_partial_t XcbMisc::get_strut_partial(xcb_window_t winId)
{
    xcb_get_property_cookie_t cookie = xcb_ewmh_get_wm_strut_partial(&m_ewmh_connection, winId);

    xcb_ewmh_wm_strut_partial_t strut;
    xcb_generic_error_t *e;

    xcb_ewmh_get_wm_strut_partial_reply(
        &m_ewmh_connection,
        cookie,
        &strut,
        &e);
    return strut;
}

bool XcbMisc::is_dock_window(xcb_window_t winId)
{
    bool is_dock = false;
    xcb_ewmh_get_atoms_reply_t name;
    xcb_generic_error_t *e;
    xcb_get_property_cookie_t cookie = xcb_ewmh_get_wm_window_type(&m_ewmh_connection, winId);
    if (1 == xcb_ewmh_get_wm_window_type_reply(&m_ewmh_connection,
            cookie,
            &name,
            &e)) {

        for (uint32_t i = 0; i < name.atoms_len; ++i) {
            xcb_atom_t t = name.atoms[i];
            if (t == m_ewmh_connection._NET_WM_WINDOW_TYPE_DOCK) {
                is_dock = true;
            }
        }
    }
    return is_dock;
}

xcb_window_t XcbMisc::find_dock_window(int screen_nbr)
{
    QList<xcb_window_t> docks;
    for (int i = 0; i < screen_nbr; ++i) {
        xcb_ewmh_get_windows_reply_t clients;
        xcb_generic_error_t *e;
        xcb_get_property_cookie_t cookie = xcb_ewmh_get_client_list(&m_ewmh_connection, i);
        if (1 == xcb_ewmh_get_client_list_reply(&m_ewmh_connection, cookie, &clients, &e)) {
            for (uint32_t wi = 0; wi < clients.windows_len; ++wi) {
                xcb_window_t winId = clients.windows[wi];
                if (is_dock_window(winId)) {
                    docks << winId;
                }
            }
        }
    }

    return docks.length() > 0 ? docks.value(0) : 0;
}

}
