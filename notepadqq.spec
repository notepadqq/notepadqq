Name:           notepadqq
Version:        1.3.1
Release:        2%{?dist}
Summary:        Notepadqq is a text editor designed from developers, for developers.

License:        GPL3
URL:            https://github.com/notepadqq/notepadqq/
#wget https://github.com/notepadqq/notepadqq/archive/v1.3.1.tar.gz -O notepadqq-1.3.1.tar.gz
Source0:        notepadqq-1.3.1.tar.gz

%define debug_package %{nil}

BuildRequires:  qt5 qt5-devel qt5-qtbase-devel qt5-qttools-devel qt5-qtwebkit-devel qt5-qtsvg-devel
Requires:       qt5 qt5-qtbase qt5-qttools qt5-qtwebkit qt5-qtsvg

%description
Notepadqq is a text editor designed from developers, for developers.

%prep
%setup -q


%build
mkdir -p support_files/shortcuts/save
./configure --prefix=$RPM_BUILD_ROOT/%{_prefix}
make %{?_smp_mflags}


%install
rm -rf $RPM_BUILD_ROOT
make install

%clean
rm -rf $RPM_BUILD_ROOT


%files
%defattr(-,root,root,-)
%doc COPYING
%{_prefix}/share/notepadqq/*
%{_prefix}/bin/notepadqq
%{_prefix}/lib/notepadqq/notepadqq-bin
%{_prefix}/share/applications/notepadqq.desktop
%{_prefix}/share/icons/hicolor/128x128/apps/notepadqq.png
%{_prefix}/share/icons/hicolor/16x16/apps/notepadqq.png
%{_prefix}/share/icons/hicolor/22x22/apps/notepadqq.png
%{_prefix}/share/icons/hicolor/24x24/apps/notepadqq.png
%{_prefix}/share/icons/hicolor/256x256/apps/notepadqq.png
%{_prefix}/share/icons/hicolor/32x32/apps/notepadqq.png
%{_prefix}/share/icons/hicolor/48x48/apps/notepadqq.png
%{_prefix}/share/icons/hicolor/512x512/apps/notepadqq.png
%{_prefix}/share/icons/hicolor/64x64/apps/notepadqq.png
%{_prefix}/share/icons/hicolor/96x96/apps/notepadqq.png
%{_prefix}/share/icons/hicolor/scalable/apps/notepadqq.svg
%{_prefix}/share/metainfo/notepadqq.appdata.xml

%changelog
* Thu Apr  5 2018 Kimpe Andy
- 
