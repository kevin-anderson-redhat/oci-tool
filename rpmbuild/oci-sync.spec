Name:           oci-sync
Version:        1.0
Release:        1.0
Summary:        A tool to synchronize OpenShift Container Images from public repositories to self-hosted repositories.

License: BSD-3-Clause
URL: https://github.com/gobha-me/oci-tool
Source0: oci-sync

Requires: bash
BuildArch: noarch

%description
OCI Sync is a tool to synchronize OpenShift Container Images from public repositories to self-hosted
repositories on stand-alone or airgapped networks.  It supports synchronization of multiple processor architectures. The
tool also supports v1 and v2 OCI manifests.

%prep
%setup

%build
%configure
make %{?_smp_mflags}

%install
install -m0755 %{name} /usr/bin/%{name}
install -m444 %{name}.man /usr/share/man/man1/%{name}.man
install -m444 LICENSE /tmp/LICENSE

%files
%license LICENSE
%{_bindir}/%{name}

%doc
/usr/share/man/man1/oci-sync.man
%changelog
* Fri Mar 05 2021 Jeff Smith <jefsmith@redhat.com> Initial Release
-