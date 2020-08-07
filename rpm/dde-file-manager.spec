Name:           dde-file-manager
Version:        5.1.0102.20-1	
Release:        1%{?dist}
Summary:        dde file manager
License:        GPLv3
URL:             
Source0:        %{name}_%{version}.orig.tar.xz

BuildRequires:  dtkwidget
BuildRequires:  dtkgui
BuildRequires:  dframeworkdbus
BuildRequires:  xcb
BuildRequires:  xcb-util
BuildRequires:  X11
BuildRequires:  Xext
BuildRequires:  Xtst
BuildRequires:  Xfixes
BuildRequires:  Xcursor
BuildRequires:  procps
BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  pkgconfig(Qt5Widgets)
BuildRequires:  pkgconfig(Qt5Gui)
BuildRequires:  pkgconfig(Qt5Svg)
BuildRequires:  pkgconfig(Qt5Xml)
BuildRequires:  pkgconfig(Qt5Kodecs)
BuildRequires:  pkgconfig(Qt5Network)
BuildRequires:  pkgconfig(Qt5X11extras)
BuildRequires:  pkgconfig(Qt5Dbus)
BuildRequires:  pkgconfig(Qt5Concurrent)
BuildRequires:  pkgconfig(Qt5Multimedia)
BuildRequires:  pkgconfig(Qt5MultimediaWidgets)
Provides:       bundled(CuteLogger)

%description
dde-file-manager

%prep
%setup -q

%build
%qmake_qt5 PREFIX=%{_prefix}
%make_build

%install
%make_install INSTALL_ROOT="%{buildroot}"

%files
%doc README.md
%license LICENSE
%{_bindir}/%{name}

%changelog

