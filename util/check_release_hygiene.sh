#!/usr/bin/env bash
set -euo pipefail

readonly SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
readonly WORKSPACE_ROOT="$(realpath "$SCRIPT_DIR/..")"
cd "$WORKSPACE_ROOT"

fail() {
  echo "release hygiene: $*" >&2
  exit 1
}

for required in \
  LICENSE \
  BRANDING.md \
  PROVENANCE.md \
  SECURITY.md \
  docs/RELEASING.md \
  sharing/linux/app/packaging/io.github.xntso.quixshare.desktop \
  sharing/linux/app/packaging/quixshare.metainfo.xml; do
  [[ -s "$required" ]] || fail "missing required file: $required"
done

grep -q "Apache License" LICENSE || fail "LICENSE is not Apache-2.0 text"
grep -q "not licensed under the Apache License 2.0" BRANDING.md ||
  fail "BRANDING.md must distinguish protected branding from source licensing"
grep -qi "independent project" README.md ||
  fail "README must state that this is an independent project"
if grep -q "cla.developers.google.com" CONTRIBUTING.md; then
  fail "fork contribution guide must not direct contributors to Google's CLA"
fi

tracked_artifacts="$({
  git ls-files | grep -E \
    '(^|/)(dist|dist-minimal|build)/|\.(AppImage|deb|rpm|exe|dll|dylib|o|a)$' ||
    true
})"
[[ -z "$tracked_artifacts" ]] || {
  printf '%s\n' "$tracked_artifacts" >&2
  fail "generated release/build artifacts are tracked"
}

unpinned_actions="$({
  grep -RhoE 'uses:[[:space:]]+[^[:space:]#]+' .github \
    --include='*.yml' --include='*.yaml' |
    sed -E 's/^uses:[[:space:]]+//' |
    grep -vE '^\./' |
    grep -vE '@[0-9a-f]{40}$' || true
})"
[[ -z "$unpinned_actions" ]] || {
  printf '%s\n' "$unpinned_actions" >&2
  fail "GitHub Actions must be pinned to full commit SHAs"
}

python3 - <<'PY'
import configparser
import pathlib
import sys
import xml.etree.ElementTree as ET

desktop_path = pathlib.Path(
    "sharing/linux/app/packaging/io.github.xntso.quixshare.desktop"
)
metadata_path = pathlib.Path(
    "sharing/linux/app/packaging/quixshare.metainfo.xml"
)

desktop = configparser.ConfigParser(interpolation=None)
desktop.optionxform = str
desktop.read(desktop_path, encoding="utf-8")
if "Desktop Entry" not in desktop:
    sys.exit("release hygiene: desktop file has no [Desktop Entry]")

entry = desktop["Desktop Entry"]
for key in ("Type", "Name", "Exec", "Icon", "Categories"):
    if not entry.get(key, "").strip():
        sys.exit(f"release hygiene: desktop file is missing {key}")
if entry["Type"] != "Application":
    sys.exit("release hygiene: desktop Type must be Application")

root = ET.parse(metadata_path).getroot()
if root.tag != "component" or root.get("type") != "desktop-application":
    sys.exit("release hygiene: AppStream component type is invalid")

required_elements = (
    "id",
    "metadata_license",
    "project_license",
    "name",
    "summary",
    "description",
    "launchable",
)
for tag in required_elements:
    element = root.find(tag)
    if element is None or not "".join(element.itertext()).strip():
        sys.exit(f"release hygiene: AppStream metadata is missing {tag}")

launchable = root.find("launchable")
if launchable.get("type") != "desktop-id":
    sys.exit("release hygiene: AppStream launchable must be a desktop-id")
if launchable.text.strip() != desktop_path.name:
    sys.exit("release hygiene: AppStream launchable and desktop filename differ")
expected_license = "Apache-2.0 AND LicenseRef-QuixShare-Branding"
if root.findtext("project_license").strip() != expected_license:
    sys.exit(
        "release hygiene: AppStream project_license must distinguish the "
        "Apache-2.0 code from protected QuixShare branding"
    )
PY

if command -v desktop-file-validate >/dev/null 2>&1; then
  desktop-file-validate \
    sharing/linux/app/packaging/io.github.xntso.quixshare.desktop
fi
if command -v appstreamcli >/dev/null 2>&1; then
  appstreamcli validate --no-net \
    sharing/linux/app/packaging/quixshare.metainfo.xml
fi

git diff --check
echo "release hygiene: checks passed"
