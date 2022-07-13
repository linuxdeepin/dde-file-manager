#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include <QApplication>
#include <QWidget>
#include <QX11Info>

#define private public
#include "util/xcb/xcb.h"

TEST(xcb,set_window_type_dock)
{
    QWidget w;
    w.winId();
    EXPECT_NE(nullptr,w.windowHandle());

    Xcb::XcbMisc::instance().set_window_type(w.winId(),Xcb::XcbMisc::Dock);

    {
        xcb_ewmh_connection_t ewmh_connection;
        xcb_intern_atom_cookie_t *cookie = xcb_ewmh_init_atoms(QX11Info::connection(), &ewmh_connection);
        xcb_ewmh_init_atoms_replies(&ewmh_connection, cookie, Q_NULLPTR);
        xcb_ewmh_get_atoms_reply_t name;
        xcb_generic_error_t *e;
        xcb_get_property_cookie_t propertyCookie = xcb_ewmh_get_wm_window_type(&ewmh_connection, static_cast<xcb_window_t>(w.winId()));
        bool is_dock = false;
        if (1 == xcb_ewmh_get_wm_window_type_reply(&ewmh_connection,propertyCookie,&name,&e)) {
            for (uint32_t i = 0; i < name.atoms_len; ++i) {
                xcb_atom_t t = name.atoms[i];
                if (t == ewmh_connection._NET_WM_WINDOW_TYPE_DOCK) {
                    is_dock = true;
                }
            }
        }
        EXPECT_EQ(true,is_dock);
    }
}

TEST(xcb,set_window_type_desktop)
{
    QWidget w;
    w.winId();
    EXPECT_NE(nullptr,w.windowHandle());

    Xcb::XcbMisc::instance().set_window_type(w.winId(),Xcb::XcbMisc::Desktop);

    {
        xcb_ewmh_connection_t ewmh_connection;
        xcb_intern_atom_cookie_t *cookie = xcb_ewmh_init_atoms(QX11Info::connection(), &ewmh_connection);
        xcb_ewmh_init_atoms_replies(&ewmh_connection, cookie, Q_NULLPTR);
        xcb_ewmh_get_atoms_reply_t name;
        xcb_generic_error_t *e;
        xcb_get_property_cookie_t propertyCookie = xcb_ewmh_get_wm_window_type(&ewmh_connection, static_cast<xcb_window_t>(w.winId()));
        bool isDesktop = false;
        if (1 == xcb_ewmh_get_wm_window_type_reply(&ewmh_connection,propertyCookie,&name,&e)) {
            for (uint32_t i = 0; i < name.atoms_len; ++i) {
                xcb_atom_t t = name.atoms[i];
                if (t == ewmh_connection._NET_WM_WINDOW_TYPE_DESKTOP) {
                    isDesktop = true;
                }
            }
        }
        EXPECT_EQ(true,isDesktop);
    }
}

TEST(xcb,find_dock_window)
{
    auto docks = Xcb::XcbMisc::instance().find_dock_window();
    EXPECT_EQ(false,docks.isEmpty());
}

TEST(xcb,get_strut_partial)
{
    QWidget w;
    w.setGeometry(20,20,50,50);
    EXPECT_NO_FATAL_FAILURE(Xcb::XcbMisc::instance().get_strut_partial(w.winId()));
}

TEST(xcb,set_window_transparent_input)
{
    QWidget w;
    w.setGeometry(20,20,50,50);

    Xcb::XcbMisc xm;
    EXPECT_NO_FATAL_FAILURE(xm.set_window_transparent_input(w.winId(),false));
    EXPECT_NO_FATAL_FAILURE(xm.set_window_transparent_input(w.winId(),true));
}
