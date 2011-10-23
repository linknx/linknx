Name:           linknx
Version:        0.0.1.29
Release:        1
Summary:        KNX home automation platform

Group:          Development/Tools
License:        GPL
URL:            http://sourceforge.net/projects/linknx
Source0:        linknx-%{version}.tar.gz
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

BuildRequires:  pthsem-devel
Requires:       eibd-server, pthsem
Provides:       linknx

%description
Linknx is an automation platform providing high level functionalities to EIB/KNX installation. The rules engine allows execution of actions based on complex logical conditions and timers. Lightweight design allows it to run on embedded Linux (OpenWRT)

# no debug package + strip
%define __spec_install_post %{nil}
%define debug_package %{nil}


%prep
%setup -q


%build
%configure --enable-smtp --with-libcurl
make %{?_smp_mflags}

%install
rm -rf $RPM_BUILD_ROOT
make install DESTDIR=$RPM_BUILD_ROOT


%clean
rm -rf $RPM_BUILD_ROOT


%files
%defattr(-,root,root,-)
/usr/bin/linknx
/usr/share/doc/linknx/linknx.xml
/usr/share/doc/linknx/sample.xml
/usr/share/doc/linknx/linknx_doc.tgz


%changelog
* Tue Nov 27 2007 Jean-Fran�ois Meessen <linknx@ouaye.net> - 0:0.0.0-3
- first version
