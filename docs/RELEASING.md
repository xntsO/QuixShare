# Release checklist

This checklist applies to QuixShare releases.

## One-time identity work

- [x] Select and reasonably clear a distinctive project name.
- [x] Replace the historical `QuickShare`/`quickshare` working title in the
      desktop file, AppStream metadata, executable, icons, settings/log paths,
      workflows, screenshots, and artifact names.
- [x] Replace every historical `com.google.quickshare` identifier and the
      provisional AppStream ID with a reverse-DNS application ID under a domain
      or GitHub namespace controlled by the maintainer.
- [x] Use original branding and add a prominent independent-project disclaimer.
- [x] Add the homepage, issue tracker, and source URLs to AppStream metadata
      after the public repository exists (the validator checks reachability).

The application identity is `io.github.xntso.quixshare` and the project URL is
`https://github.com/xntsO/QuixShare`.

## Legal and dependency review

- [ ] Retain the root Apache-2.0 license and upstream file notices.
- [ ] Generate an inventory/SBOM from the exact release artifact.
- [ ] Include applicable license texts and notices for every bundled library,
      font, icon, and other asset, including Qt and ICU.
- [ ] Confirm the LGPL requirements for the dynamically linked Qt build,
      including relinking/replacement and corresponding-source obligations.
- [ ] Confirm that AppStream `project_license` describes the application and
      that `metadata_license` describes the metadata file itself.

This checklist is engineering guidance, not legal advice. Obtain qualified
review when trademark or license risk is material.

## Source and CI preflight

- [ ] Start from a clean checkout with recursively initialized submodules.
- [ ] Run `./util/check_release_hygiene.sh` and `git diff --check`.
- [ ] Run the application controller and backend tests.
- [ ] Run focused Bluetooth, BLE, Wi-Fi LAN, shutdown, and packaging tests.
- [ ] Build `//sharing/linux/app:appimage` using the same flags as CI.
- [ ] Confirm GitHub Actions are pinned to reviewed commit SHAs.
- [ ] Review the full diff from the previous release tag.

## Artifact validation

- [ ] Build the AppImage only in CI from the signed release tag.
- [ ] Run the packaged QML smoke test on Wayland and X11/XCB.
- [ ] Inspect the AppImage contents for developer paths, credentials, logs,
      debug dumps, and unexpected host libraries.
- [ ] Validate the desktop file and AppStream metadata with distribution tools.
- [ ] Test send, receive, rejection, cancellation, timeout, and clean shutdown
      on representative hardware.
- [ ] Verify Bluetooth alias, discoverability, and timeout are restored after a
      normal exit and interrupted transfer.
- [ ] Publish SHA-256 checksums and build provenance/attestation.
- [ ] Record supported distributions, known limitations, required inbound
      firewall policy, and upgrade instructions in the release notes.

## Post-release

- [ ] Install the published artifact on a clean supported system.
- [ ] Verify checksums and signatures from the public release page.
- [ ] Monitor crash/security reports and document any withdrawn artifact.
