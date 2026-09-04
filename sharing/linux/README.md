# QuixShare

> **Project status:** This is an independent, unofficial Linux compatibility
> client. It is not affiliated with or endorsed by Google or device
> manufacturers. “Quick Share”, “Google”, and other names may be trademarks of
> their respective owners. Compatibility can change when peer implementations
> change.

This Linux application is built from the Apache-2.0-licensed
[Google Nearby](https://github.com/google/nearby) codebase and community Linux
work. The current checkout originated from the community fork recorded in the
Git remote history. The user interface, Linux platform integration, and
packaging contain modifications from the upstream sources.

## Project links

Project repository: <https://github.com/xntsO/QuixShare>

- Use the repository issue tracker for reproducible bugs and compatibility
  reports. Include the Linux distribution, desktop session, phone model, and a
  redacted log excerpt.
- Use pull requests for fixes. See the root `LINUX_CONTRIBUTING.md` for current
  build and contribution guidance.
- Support the project by testing releases, improving documentation, triaging
  issues, or contributing code. No financial-support channel is claimed until
  the maintainers publish one.

## License and notices

The repository's own source is offered under Apache License 2.0; see the root
`LICENSE`. Attribution and provenance notes are in `NOTICE`. Direct dependency
information and release-review caveats are in `THIRD_PARTY_NOTICES.md`.

Dependencies retain their own licenses. An AppImage also bundles Qt, ICU, and
other runtime components, so distributors must review the exact artifact and
include all license texts and notices required by those versions. The inventory
in this repository is a review aid, not a legal opinion or a substitute for an
artifact-level software-composition analysis.

The previously bundled `googlesans_var.ttf` was removed from the application
resources because no redistribution license accompanied it. The application
uses the user's system UI font instead.

## Native application logs

QuixShare writes its native Abseil diagnostics to:

```text
$XDG_STATE_HOME/quixshare/logs/quixshare.log
```

If `XDG_STATE_HOME` is not set to an absolute path, the directory defaults to
`$HOME/.local/state/quixshare/logs`.

Records are stored exactly as Abseil emits them. The active log rotates at
10 MiB, with `quixshare.log.1` through `quixshare.log.4` retaining the four
previous files. The directory and its files are accessible only to the current
user.

After file logging initializes, native Abseil diagnostics are not duplicated
to stderr. If the file cannot be initialized or written, diagnostics fall back
to stderr. Qt and QML messages keep their existing behavior and are not written
to these files.
