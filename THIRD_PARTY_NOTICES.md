# Third-party notices and release review

This file is an attribution and release-review aid. It is not legal advice, is
not a warranty of license compliance, and is not necessarily a complete list of
every transitive component in a built artifact. Release maintainers should
generate an artifact-level software bill of materials and verify each exact
version before publication.

## Project provenance

The core source derives from [Google Nearby](https://github.com/google/nearby)
and is licensed under Apache License 2.0. QuixShare began from the community
fork `https://github.com/kidfromjupiter/nearby.git`. Linux UI, platform, and
packaging files include community modifications. See `LICENSE` and `NOTICE`.

## Direct components identified in the build

The root `MODULE.bazel`, submodules, and AppImage packaging currently identify
the following major components. License names below summarize the license files
present in the pinned source distributions; the license files themselves are
authoritative.

| Component | Role | License file or review note |
| --- | --- | --- |
| Abseil | C++ runtime library | Apache-2.0 (`LICENSE` in pinned source) |
| Protocol Buffers | Serialization/runtime | BSD-style license (`LICENSE`) |
| BoringSSL | Cryptography | Mixed permissive notices; review its compound `LICENSE` |
| UKEY2 / SecureMessage | Authentication protocol | Apache-2.0 plus upstream `NOTICE` |
| nlohmann/json | JSON support | MIT (`LICENSE.MIT`) |
| sdbus-c++ | Linux D-Bus support | LGPL-2.1 with project exception; review `COPYING` and `COPYING-LGPL-Exception` |
| Qt 6.8.3 modules | GUI and QML runtime bundled by AppImage | LGPL/GPL/commercial terms vary by module; include applicable Qt license texts and preserve user replacement/relinking rights where required |
| ICU 73 | Unicode runtime bundled by AppImage | Unicode/ICU license; include the exact distribution notice |
| systemd/libsystemd | Host D-Bus dependency | LGPL-2.1-or-later; normally resolved from the host, not copied by the minimal AppImage script |
| FTXUI | CLI/TUI support | MIT; verify whether included in the release artifact |
| GoogleTest and build rules | Tests/build tooling | Generally not shipped; verify release closure rather than assuming exclusion |
| Nisaba and protobuf-matchers | Build/source dependencies | Verify pinned-source license and whether code is linked into the artifact |

## Known release blockers and checks

- Run an SBOM/license scanner on the final AppImage. The current table is not a
  substitute for scanning the linked ELF and QML/plugin closure.
- Add the authoritative Qt and ICU license texts for the exact binaries copied
  by the packaging script. The downloaded Qt bundle currently used by the
  build does not expose a complete top-level license bundle in this checkout.
- Confirm that no unlicensed fonts, icons, screenshots, or other media are in
  the release. `googlesans_var.ttf` is deliberately no longer embedded or
  packaged because this repository contains no license granting redistribution.
- Review names, icons, and store metadata for trademark and passing-off risk.
  The current UI and metadata describe the app as independent and unofficial,
  but that wording is not a legal determination.

## Project artwork

The QuixShare penguin-transfer logo was supplied by the project maintainer in
September 2026 as the project's original branding. It is not derived from the
Google Quick Share or RQuickShare icons. The high-contrast taskbar derivative
was prepared from that original mark for legibility on dark desktop panels.
