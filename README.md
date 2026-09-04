<p align="center">
  <img src="sharing/linux/app/icons/quixshare-taskbar.png" alt="QuixShare logo" width="128">
</p>

<h1 align="center">QuixShare</h1>

<p align="center">
  An independent nearby file-sharing client for Linux.
</p>

This repository contains a community-maintained Linux desktop client for
discovering compatible nearby devices and transferring files over local
Bluetooth and network transports. It also carries the open-source Nearby
Connections and Sharing code on which the client is built.

> This is an independent project. It is not an official Google or Samsung
> product, and those companies do not endorse or support it. QuixShare is an
> independent compatibility client with its own name and artwork.

See [PROVENANCE.md](PROVENANCE.md) for the code lineage and trademark boundary.

## Current status

The Linux application can send and receive files with compatible Android
devices. It uses Qt for the desktop interface, BlueZ for Bluetooth, and the
Linux Nearby platform implementation in this repository.

The project is still pre-release software. Hardware, Bluetooth controller,
desktop-session, and firewall combinations vary substantially across Linux
systems, so review the known limitations before relying on it.

## Installation

Tagged releases produce an x86-64 AppImage and a matching SHA-256 checksum.
Download both files from the repository's Releases page, verify the checksum,
then make the AppImage executable:

```bash
sha256sum --check ./*.AppImage.sha256
chmod +x ./*.AppImage
./*.AppImage
```

The current CI targets Ubuntu 24.04. Fedora 43 is also used for hardware
testing. Other distributions may work but are not yet part of the release
test matrix.

Runtime prerequisites include:

- a working BlueZ Bluetooth service;
- systemd and D-Bus;
- NetworkManager for network-state integration; and
- a desktop session with either Wayland or X11/XCB support.

The AppImage does not install or modify firewall policy. Fast transfers can
require an inbound local-network TCP connection; if that connection is
blocked, the transfer may remain on Bluetooth or fail during a bandwidth
upgrade. Do not expose the application to untrusted networks without reviewing
the listener and firewall policy for the release.

## Building

Clone recursively because the repository uses Git submodules:

```bash
git clone --recurse-submodules https://github.com/xntsO/QuixShare.git
cd QuixShare
bazel test \
  --@com_google_protobuf//bazel/toolchains:prefer_prebuilt_protoc=true \
  --copt=-DGITHUB_BUILD \
  //sharing/linux/app:application_controller_test \
  //sharing/linux/app:backend_test
bazel build \
  --@com_google_protobuf//bazel/toolchains:prefer_prebuilt_protoc=true \
  --copt=-DGITHUB_BUILD \
  //sharing/linux/app:appimage
```

The GitHub workflows are the canonical dependency and build reference. For
platform architecture and local development guidance, see
[LINUX_CONTRIBUTING.md](LINUX_CONTRIBUTING.md).

## Logs and bug reports

Native application logs are stored at:

```text
$XDG_STATE_HOME/quixshare/logs/quixshare.log
```

When `XDG_STATE_HOME` is not an absolute path, the fallback is
`$HOME/.local/state/quixshare/logs`. Logs rotate locally. Review logs before
attaching them to a public issue because device names, local addresses, file
names, and other environment details may be sensitive.

Please use the issue templates and include the distribution, desktop session,
Bluetooth hardware, reproduction steps, and a redacted log excerpt.

## Known limitations

- Bluetooth Classic fallback is much slower than a successful network
  bandwidth upgrade.
- Linux radio and firewall behavior differs by distribution and hardware.
- The packaged application intentionally does not reconfigure the active Wi-Fi
  connection unless a developer-only opt-in is used in a source build.
- Packaging and dependency notices must pass the
  checks in [docs/RELEASING.md](docs/RELEASING.md) before a public release.

## Contributing and security

See [CONTRIBUTING.md](CONTRIBUTING.md) for contribution requirements and
[SECURITY.md](SECURITY.md) for private vulnerability reporting.

## License

The repository is licensed under the [Apache License 2.0](LICENSE). Bundled
third-party components retain their own licenses; release artifacts must carry
the corresponding notices and license materials.
