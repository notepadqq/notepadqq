Name:           notepadqq
Version:        1.3.1
Release:        1.3.1%{?dist}
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
%{_prefix}/*


%changelog
* Thu Apr  5 2018 Kimpe Andy
- 
