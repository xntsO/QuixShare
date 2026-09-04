# Contributing

Contributions are welcome through GitHub pull requests and issues.

## Before opening a change

- Search existing issues and pull requests for related work.
- Keep changes focused and preserve copyright, attribution, and license notices
  from upstream files.
- Do not include credentials, private logs, generated build trees, packaged
  binaries, or third-party assets without a compatible license.
- For Linux platform work, read [LINUX_CONTRIBUTING.md](LINUX_CONTRIBUTING.md).

## Pull requests

Describe the problem, the approach, and the exact verification performed. Add
automated regression coverage when practical. User-interface changes should
include screenshots or a short recording; hardware-dependent transport changes
should include redacted logs and identify the tested adapters and devices.

Run the relevant focused tests and the release hygiene check before submitting:

```bash
./util/check_release_hygiene.sh
git diff --check
```

The pull request template contains space for additional test commands.

## Licensing contributions

Unless explicitly stated otherwise, a contribution intentionally submitted to
this repository is provided under the Apache License 2.0 in the root `LICENSE`
file. By submitting a contribution, you confirm that you have the right to do
so and that its dependencies and included assets are compatible with that
license and with distribution of the project.

This independent project does not use Google's contributor license agreement.
Contributing here does not submit the change to Google or any other upstream
project.

## Conduct and security

Participation is governed by [CODE_OF_CONDUCT.md](CODE_OF_CONDUCT.md). Report
security vulnerabilities privately as described in [SECURITY.md](SECURITY.md),
not in a public issue.
