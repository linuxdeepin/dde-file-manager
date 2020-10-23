%global repo dde-file-manager

Name:           deepin-file-manager
Version:        5.2.0.37
Release:        1%{?dist}
Summary:        Deepin File Manager
License:        GPLv3
URL:            https://github.com/linuxdeepin/dde-file-manager
Source0:        %{url}/archive/%{version}/%{repo}-%{version}.tar.gz

BuildRequires:  gcc-c++
BuildRequires:  desktop-file-utils
BuildRequires:  deepin-gettext-tools
BuildRequires:  deepin-dock-devel
BuildRequires:  file-devel
BuildRequires:  jemalloc-devel
BuildRequires:  zlib-devel
BuildRequires:  disomaster-devel
BuildRequires:  openssl-devel
BuildRequires:  cmake(KF5Codecs)
BuildRequires:  pkgconfig(atk)
BuildRequires:  pkgconfig(dtkwidget) >= 2.0.6
BuildRequires:  pkgconfig(dframeworkdbus) >= 2.0
BuildRequires:  udisks2-qt5-devel >= 5.0.3
BuildRequires:  glib2-devel
BuildRequires:  libicu-devel
BuildRequires:  lucene++-devel
BuildRequires:  libxml2-devel
BuildRequires:  htmlcxx-devel
BuildRequires:  mimetic-devel
BuildRequires:  boost-devel
BuildRequires:  pkgconfig(libgsf-1)
BuildRequires:  pkgconfig(libmediainfo)
BuildRequires:  pkgconfig(gsettings-qt)
BuildRequires:  pkgconfig(libsecret-1)
BuildRequires:  pkgconfig(poppler-cpp)
BuildRequires:  pkgconfig(polkit-agent-1)
BuildRequires:  pkgconfig(polkit-qt5-1)
BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  pkgconfig(Qt5Concurrent)
BuildRequires:  pkgconfig(Qt5DBus)
BuildRequires:  pkgconfig(Qt5Gui)
BuildRequires:  pkgconfig(Qt5Svg)
BuildRequires:  pkgconfig(Qt5Multimedia)
BuildRequires:  pkgconfig(Qt5X11Extras)
BuildRequires:  pkgconfig(Qt5Xdg)
BuildRequires:  qt5-qtbase-private-devel
%{?_qt5:Requires: %{_qt5}%{?_isa} = %{_qt5_version}}
BuildRequires:  pkgconfig(taglib)
BuildRequires:  pkgconfig(uchardet)
BuildRequires:  pkgconfig(xcb-util)
BuildRequires:  pkgconfig(xcb-ewmh)
BuildRequires:  pkgconfig(gio-qt)
BuildRequires:  qt5-linguist

# run command by QProcess
Requires:       deepin-shortcut-viewer
Requires:       deepin-terminal
Requires:       deepin-desktop
Requires:       file-roller
Requires:       gvfs-client
Requires:       samba
Requires:       xdg-user-dirs
Recommends:     deepin-manual

%description
File manager front end of Deepin OS.

%package devel
Summary:        Development package for %{name}
Requires:       %{name}%{?_isa} = %{version}-%{release}

%description devel
Header files and libraries for %{name}.

%package -n deepin-desktop
Summary:        Deepin desktop environment - desktop module
Requires:       %{name}%{?_isa} = %{version}-%{release}
Requires:       deepin-dock
Requires:       deepin-launcher
Requires:       deepin-session-ui

%description -n deepin-desktop
Deepin desktop environment - desktop module.

%prep
%autosetup -p1 -n %{repo}-%{version}

# fix file permissions
find -type f -perm 775 -exec chmod 644 {} \;
sed -i 's|systembusconf.path = /etc/dbus-1/system.d|systembusconf.path = %{_datadir}/dbus-1/system.d|' dde-file-manager-daemon/dde-file-manager-daemon.pro
sed -i '/target.path/s|lib|%{_lib}|' dde-dock-plugins/disk-mount/disk-mount.pro
sed -i '/deepin-daemon/s|lib|libexec|' dde-zone/mainwindow.h
sed -i 's|lib/gvfs|libexec|' %{repo}-lib/gvfs/networkmanager.cpp

%build
export PATH=%{_qt5_bindir}:$PATH
%qmake_qt5 PREFIX=%{_prefix} QMAKE_CFLAGS_ISYSTEM= CONFIG+="DISABLE_FFMPEG DISABLE_ANYTHING" filemanager.pro
%make_build

%install
%make_install INSTALL_ROOT=%{buildroot}

%check
desktop-file-validate %{buildroot}%{_datadir}/applications/%{repo}.desktop
desktop-file-validate %{buildroot}%{_datadir}/applications/dde-computer.desktop
desktop-file-validate %{buildroot}%{_datadir}/applications/dde-trash.desktop
desktop-file-validate %{buildroot}%{_datadir}/applications/dde-home.desktop
desktop-file-validate %{buildroot}%{_datadir}/applications/dde-open.desktop

%files
%doc README.md
%license LICENSE
%{_datadir}/dbus-1/system.d/com.deepin.filemanager.daemon.conf
%{_bindir}/%{repo}
%{_bindir}/%{repo}-daemon
%{_bindir}/%{repo}-pkexec
%{_bindir}/dde-property-dialog
%{_libdir}/lib%{repo}.so.*
%{_libdir}/dde-dock/plugins/
%{_libdir}/%{repo}/
%{_datadir}/%{repo}/
%{_datadir}/deepin/%{repo}/
%{_datadir}/icons/hicolor/scalable/apps/*.svg
%{_datadir}/applications/%{repo}.desktop
%{_datadir}/applications/dde-open.desktop
%{_datadir}/dbus-1/interfaces/com.deepin.filemanager.filedialog.xml
%{_datadir}/dbus-1/interfaces/com.deepin.filemanager.filedialogmanager.xml
%{_datadir}/dbus-1/services/com.deepin.filemanager.filedialog.service
%{_datadir}/dbus-1/services/org.freedesktop.FileManager.service
%{_datadir}/dbus-1/system-services/com.deepin.filemanager.daemon.service
%dir %{_datadir}/dde-disk-mount-plugin
%{_datadir}/dde-disk-mount-plugin/translations/
%{_unitdir}/dde-filemanager-daemon.service
%{_datadir}/polkit-1/actions/com.deepin.*.policy
%{_datadir}/glib-2.0/schemas/*

%files devel
%{_includedir}/%{repo}/
%{_libdir}/pkgconfig/%{repo}.pc
%{_libdir}/lib%{repo}.so

%files -n deepin-desktop
%{_bindir}/dde-desktop
%{_datadir}/applications/dde-computer.desktop
%{_datadir}/applications/dde-trash.desktop
%{_datadir}/applications/dde-home.desktop
%dir %{_datadir}/dde-desktop
%{_datadir}/dde-desktop/translations/
%{_datadir}/dbus-1/services/com.deepin.dde.desktop.service

%changelog
