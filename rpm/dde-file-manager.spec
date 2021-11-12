%define specrelease 8%{?dist}
%if 0%{?openeuler}
%define specrelease 8
%endif

Name:           dde-file-manager
Version:        5.2.20
Release:        %{specrelease}
Summary:        Deepin File Manager
License:        GPLv3
URL:            https://github.com/linuxdeepin/dde-file-manager
Source0:        %{name}_%{version}.orig.tar.xz	

BuildRequires:  gcc-c++
BuildRequires:  desktop-file-utils
BuildRequires:  deepin-gettext-tools
BuildRequires:  dde-dock-devel
BuildRequires:  file-devel
#BuildRequires:  jemalloc-devel
#BuildRequires:  cmake(KF5Codecs)
BuildRequires:  pkgconfig(atk)
BuildRequires:  dtkgui-devel
BuildRequires:  pkgconfig(dtkwidget) >= 5.1
BuildRequires:  pkgconfig(dframeworkdbus) >= 2.0
BuildRequires:  pkgconfig(gtk+-2.0)
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
BuildRequires:  qt5-qtbase-private-devel
%{?_qt5:Requires: %{_qt5}%{?_isa} = %{_qt5_version}}
BuildRequires:  pkgconfig(taglib)
#BuildRequires:  pkgconfig(uchardet)
BuildRequires:  pkgconfig(xcb-util)
BuildRequires:  pkgconfig(xcb-ewmh)
BuildRequires:  qt5-linguist
BuildRequires:  jemalloc-devel
#BuildRequires:  udisks2-qt5
BuildRequires:  udisks2-qt5-devel
BuildRequires:  disomaster-devel
BuildRequires:  libgio-qt libgio-qt-devel
BuildRequires:  openssl-devel
BuildRequires:  libqtxdg-devel
BuildRequires:  libmediainfo-devel
BuildRequires:  kf5-kcodecs-devel
#BuildRequires:  libudisks2-qt5-devel
BuildRequires:  lucene++-devel
BuildRequires:  htmlcxx-devel
BuildRequires:  libgsf-devel
BuildRequires:  mimetic-devel
BuildRequires:  boost-devel
%ifarch %{ix86} x86_64
BuildRequires:	deepin-anything-devel deepin-anything-server
%endif


# run command by QProcess
#Requires:       deepin-shortcut-viewer
Requires:       deepin-terminal
Requires:       dde-desktop
#Requires:       file-roller
Requires:       jemalloc
Requires:       libglvnd-glx
Requires:       libdde-file-manager
Requires:       cryfs
%ifarch %{ix86} x86_64
Requires:		deepin-anything-dkms deepin-anything-server
%endif
#Requires:       gvfs-client
#Requires:       samba
#Requires:       xdg-user-dirs
#Requires:       gstreamer-plugins-good
Recommends:     deepin-manual

%description
File manager front end of Deepin OS.

%package devel
Summary:        Development package for %{name}
Requires:       %{name}%{?_isa} = %{version}-%{release}

%description devel
Header files and libraries for %{name}.

%package -n libdde-file-manager
Summary:        DDE File Manager library
Requires:       %{name}%{?_isa} = %{version}-%{release}
Requires:       kf5-kcodecs
Requires:       jemalloc
Requires:       poppler-cpp
Requires:       libmediainfo
Requires:       libzen
Requires:       udisks2-qt5
Requires:       taglib
Requires:       libgio-qt
%ifarch %{ix86} x86_64 
Requires:		deepin-anything-libs
%endif

%description -n libdde-file-manager
DDE File Manager library.

%package -n dde-disk-mount-plugin
Summary:        plugin of dde-dock
Requires:       %{name}%{?_isa} = %{version}-%{release}

%description -n dde-disk-mount-plugin
plugin of dde-dock.

%package -n dde-desktop
Summary:        Deepin desktop environment - desktop module
Requires:       %{name}%{?_isa} = %{version}-%{release}
Requires:       dde-dock
Requires:       dde-launcher
Requires:       dde-session-ui

%description -n dde-desktop
Deepin desktop environment - desktop module.

%prep
%setup -q -n %{name}-%{version}

# fix file permissions
find -type f -perm 775 -exec chmod 644 {} \;
#sed -i '/target.path/s|lib|%{_lib}|' src/dde-dock-plugins/disk-mount/disk-mount.pro
sed -i '/deepin-daemon/s|lib|libexec|' src/dde-zone/mainwindow.h
sed -i 's|lib/gvfs|libexec|' src/%{name}-lib/gvfs/networkmanager.cpp
#sed -i 's|%{_datadir}|%{_libdir}|' dde-sharefiles/appbase.pri
sed -i 's|/lib/dde-dock/plugins|/lib64/dde-dock/plugins|' src/dde-dock-plugins/disk-mount/disk-mount.pro

%build
export PATH=%{_qt5_bindir}:$PATH
%qmake_qt5 PREFIX=%{_prefix} DISABLE_JEMALLOC=1 QMAKE_CFLAGS_ISYSTEM= CONFIG+="DISABLE_FFMPEG"  DEFINES+="VERSION=%{version}" filemanager.pro
%make_build

%install
%make_install INSTALL_ROOT=%{buildroot}

%check
desktop-file-validate %{buildroot}%{_datadir}/applications/%{name}.desktop
desktop-file-validate %{buildroot}%{_datadir}/applications/dde-computer.desktop ||:
desktop-file-validate %{buildroot}%{_datadir}/applications/dde-trash.desktop ||:
desktop-file-validate %{buildroot}%{_datadir}/applications/dde-home.desktop ||:

%ldconfig_scriptlets

%files
%doc README.md
%license LICENSE
%config(noreplace) %{_sysconfdir}/dbus-1/system.d/com.deepin.filemanager.daemon.conf
%{_bindir}/%{name}
%{_bindir}/%{name}-daemon
%{_bindir}/%{name}-pkexec
%ifnarch x86_64
%{_bindir}/*.sh
%{_sysconfdir}/xdg/autostart/dde-file-manager-autostart.desktop
%endif
%{_bindir}/dde-property-dialog
/usr/lib/systemd/system/dde-filemanager-daemon.service

%{_datadir}/applications/dde-open.desktop
%{_datadir}/applications/%{name}.desktop
%{_datadir}/dbus-1/interfaces/com.deepin.filemanager.filedialog.xml
%{_datadir}/dbus-1/interfaces/com.deepin.filemanager.filedialogmanager.xml
%{_datadir}/dbus-1/services/com.deepin.filemanager.filedialog.service
%{_datadir}/dbus-1/services/org.freedesktop.FileManager.service
%{_datadir}/dbus-1/system-services/com.deepin.filemanager.daemon.service
%{_polkit_qt_policydir}/com.deepin.filemanager.daemon.policy
%{_polkit_qt_policydir}/com.deepin.pkexec.dde-file-manager.policy
%{_datadir}/deepin-manual/manual-assets/application/dde-file-manager
%{_datadir}/applications/context-menus/.readme

%files -n libdde-file-manager
%{_libdir}/dde-file-manager/plugins/previews/libdde-image-preview-plugin.so
%{_libdir}/dde-file-manager/plugins/previews/libdde-music-preview-plugin.so
%{_libdir}/dde-file-manager/plugins/previews/libdde-pdf-preview-plugin.so
%{_libdir}/dde-file-manager/plugins/previews/libdde-text-preview-plugin.so
%{_libdir}/libdde-file-manager.so.1.8.2
%{_datadir}/dde-file-manager/mimetypeassociations/mimetypeassociations.json
%{_datadir}/dde-file-manager/mimetypes/archive.mimetype
%{_datadir}/dde-file-manager/mimetypes/audio.mimetype
%{_datadir}/dde-file-manager/mimetypes/backup.mimetype
%{_datadir}/dde-file-manager/mimetypes/executable.mimetype
%{_datadir}/dde-file-manager/mimetypes/image.mimetype
%{_datadir}/dde-file-manager/mimetypes/text.mimetype
%{_datadir}/dde-file-manager/mimetypes/video.mimetype
%{_datadir}/dde-file-manager/templates/newDoc.wps
%{_datadir}/dde-file-manager/templates/newExcel.et
%{_datadir}/dde-file-manager/templates/newPowerPoint.dps
%{_datadir}/dde-file-manager/templates/newDoc.doc
%{_datadir}/dde-file-manager/templates/newExcel.xls
%{_datadir}/dde-file-manager/templates/newPowerPoint.ppt
%{_datadir}/dde-file-manager/templates/newTxt.txt
%{_datadir}/dde-file-manager/translations/
%{_datadir}/deepin/dde-file-manager/oem-menuextensions/.readme
%{_datadir}/glib-2.0/schemas/com.deepin.dde.filemanager.gschema.xml
%{_datadir}/icons/hicolor/scalable/apps/dde-file-manager.svg
%{_libdir}/libdde-file-manager.so.1
%{_libdir}/libdde-file-manager.so.1.8
%ifarch x86_64
%{_libdir}/deepin-anything-server-lib/plugins/handlers/libdde-anythingmonitor.so
%endif

%files -n dde-disk-mount-plugin
%{_libdir}/dde-dock/plugins/system-trays/libdde-disk-mount-plugin.so
%{_datadir}/dde-disk-mount-plugin/translations
%{_datadir}/glib-2.0/schemas/com.deepin.dde.dock.module.disk-mount.gschema.xml

%files devel
%{_includedir}/%{name}/*.h
%{_includedir}/%{name}/gvfs/
%{_includedir}/%{name}/%{name}-plugins/
%{_includedir}/%{name}/private/
%{_libdir}/pkgconfig/%{name}.pc
%{_libdir}/lib%{name}.so

%files -n dde-desktop
%{_bindir}/dde-desktop
%{_datadir}/applications/dde-computer.desktop
%{_datadir}/applications/dde-home.desktop
%{_datadir}/applications/dde-trash.desktop
%dir %{_datadir}/dde-desktop
%{_datadir}/dde-desktop/translations/
%{_datadir}/dbus-1/services/com.deepin.dde.desktop.service

%changelog
* Mon Jul 06 2020 uoser <uoser@uniontech.com> - 5.1.0.9-7
- Move plug-in library to /usr/lib64 directory

* Wed Jun 17 2020 uoser <uoser@uniontech.com> - 5.1.0.9
- Remove the disk capacity information displayed in the computer properties
