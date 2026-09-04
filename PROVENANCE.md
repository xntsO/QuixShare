# Project provenance

## Source lineage

This repository descends from Google's open-source
[`google/nearby`](https://github.com/google/nearby) repository. The retained
history and copyright headers identify the upstream authors and contributors.
The upstream project is distributed under Apache-2.0 and states that it is not
an officially supported Google product.

The Linux platform work in this fork includes work based on
[`google/nearby#2098`](https://github.com/google/nearby/pull/2098), originally
submitted by Vibhav Pant and with contributions credited in the fork history.
The current repository also contains substantial independent Linux desktop,
packaging, and transport work added after the upstream fork.

The Rust project
[`Martichou/rquickshare`](https://github.com/Martichou/rquickshare) is a
separate implementation. No rquickshare source or release artifacts were
identified in this repository during the September 2026 release audit. Do not
describe this codebase as a fork of rquickshare unless that provenance changes.
Its user interface was used as visual inspiration and is credited in the
application's About & project help section.

## License and notices

Repository source is distributed under the Apache License 2.0 in `LICENSE`.
Existing file-level copyright and attribution notices must be retained. New
third-party code, fonts, icons, and other assets must have documented,
distribution-compatible terms.

The AppImage bundles shared Qt and ICU libraries from the Qt distribution.
Those components retain their own licenses. Before publishing a binary, the
release must include the applicable license texts, copyright notices, and a
reviewable dependency/SBOM inventory. This repository's Apache-2.0 `LICENSE`
does not replace those obligations.

## Names and trademarks

Protocol interoperability does not grant rights to another party's product
name, logo, trade dress, or reverse-DNS namespace. Google, Nearby, Quick Share,
Android, Samsung, and related marks belong to their respective owners.

The Linux application is independent and is not endorsed by Google or Samsung.
It is published as QuixShare with the application ID
`io.github.xntso.quixshare` and project artwork supplied by the maintainer.
The taskbar-safe logo is a high-contrast derivative of that supplied artwork.
Compatibility may be described factually in documentation without presenting
the application as an official client.

The QuixShare name and both logo files are owned by xntsO and excluded from the
Apache-2.0 source-code license. Their permitted uses are defined separately in
[`BRANDING.md`](BRANDING.md). QuixShare is claimed as a project mark; no claim
of trademark registration is made.
