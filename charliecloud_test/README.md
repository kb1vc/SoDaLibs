# charliecloud_test

Build and package SoDaLibs inside charliecloud containers for every
Fedora and Ubuntu version listed in `versions.conf`, then verify that
each resulting RPM / DEB installs cleanly in a fresh container.

## How To

Run everything (all distros, all versions, both stages):

```sh
./build.sh
```

A table is printed at the end:

```
================  SUMMARY  ================
step            distro   version  status
--------        ------   -------  ------
build           fedora   44       PASS
test-install    fedora   44       PASS
build           fedora   45       PASS
test-install    fedora   45       PASS
build           ubuntu   24.04    PASS
test-install    ubuntu   24.04    PASS
build           ubuntu   26.04    FAIL
test-install    ubuntu   26.04    SKIP
===========================================
```

Run a single build (produces a tagged kit in `packages/`):

```sh
./scripts/build-fedora.sh 44
./scripts/build-ubuntu.sh 24.04
```

Run a single install test (consumes a tagged kit from `packages/`):

```sh
./scripts/test-install-fedora.sh 44 sodalibs-devel-5.9.1-x86_64.Fedora-44.rpm
./scripts/test-install-ubuntu.sh 24.04 sodalibs-devel-5.9.1-x86_64.Ubuntu-24.04.deb
```

Add or drop a supported distro version: edit `versions.conf`. Nothing
else needs to change — the same file is read by the local scripts and
by `.github/workflows/release.yml`.

Where things land:

- `packages/` — distro-tagged kits (`<basename>.<Fedora-XX|Ubuntu-XX.YY>.<ext>`).
- `logs/` — one log per step per version (`build-fedora-44.log`, `test-install-ubuntu-24.04.log`, …).
- `images/` — charliecloud image storage (created by `ch-image` on first run; do not pre-create).
- `.tmp/` — per-run scratch for cpack output before tagging.

## Scheme

Each (distro, version) goes through two stages:

**Stage 1 — build + ctest + kit.** A container based on the target
distro at the target version is built from `dockerfiles/Fedora.build`
or `dockerfiles/Ubuntu.build`. The host SoDaLibs tree is bind-mounted
at `/mnt/src`; inside the container the Dockerfile `git clone`s from
that path and checks out `dist-test`, then runs `cmake`, `make`,
`ctest`, `make install`, and `cpack`. The resulting package is dropped
into a bind-mounted scratch dir. The driver script copies it into
`packages/` with a distro tag appended before the extension — e.g.
`sodalibs-devel-5.9.1-x86_64.Fedora-44.rpm`.

**Stage 2 — test install.** A fresh container at the same distro
version is built from `dockerfiles/Fedora.testinstall` or
`dockerfiles/Ubuntu.testinstall`. It installs the tagged package from
`/mnt/pkg`, then asks the package itself (`rpm -ql` / `dpkg -L`) what
files it should have installed and verifies each one exists. Missing
files are listed by name.

Guiding rules:

- `versions.conf` is the single source of truth and is treated as read-only.
- Failures never abort the run; the final summary table shows PASS /
  FAIL / SKIP for every (step, distro, version). `test-install` is
  reported as SKIP when its matching `build` failed.
- `git clone` happens *inside* the container (not on the host), so the
  committed state of `dist-test` is what gets built.
- All progress from `ch-image build` is streamed to the terminal *and*
  appended to the per-step log, so a running build is never silent and
  a failed build leaves a complete transcript on disk.

## Files

### `versions.conf`
Shell-fragment file sourced by the scripts. Defines
`FEDORA_VERSIONS` and `UBUNTU_VERSIONS` as space-separated version
lists. Shared with the GitHub Actions release workflow.

### `build.sh`
Top-level driver. Reads `versions.conf`, loops over every Fedora and
Ubuntu version, runs build-then-test-install for each, records
PASS/FAIL/SKIP per step, and prints the summary table. Exits non-zero
if any step failed.

### `scripts/lib.sh`
Common helpers sourced by every script.
- `read_versions` — parses `versions.conf` into shell variables.
- `distro_tag` — `fedora 44` → `Fedora-44`; `ubuntu 24.04` → `Ubuntu-24.04`.
- `tag_package` — copies a cpack output into `packages/` with the
  distro tag inserted before the extension.
- `run_logged` — runs a command, tee'ing its stdout+stderr to both the
  log file and this process's stderr (so terminal progress is visible
  and the caller's stdout stays clean for return values).
- `echo_logged` — writes a line to both stderr and the log file.
- `summary_reset` / `summary_add` / `summary_print` — collect per-step
  results and print the final table.

### `scripts/build-fedora.sh <version>`
Builds `dockerfiles/Fedora.build` with `FEDORA_VERSION=<version>`,
bind-mounting the SoDaLibs tree at `/mnt/src` and a scratch dir at
`/mnt/out`. On success, moves the produced RPM into `packages/` with
a distro tag and prints its path on stdout. Log: `logs/build-fedora-<version>.log`.

### `scripts/build-ubuntu.sh <version>`
Same as above, for Ubuntu and DEB. Builds `dockerfiles/Ubuntu.build`
with `UBUNTU_VERSION=<version>`. Log:
`logs/build-ubuntu-<version>.log`.

### `scripts/test-install-fedora.sh <version> <package-basename>`
Builds `dockerfiles/Fedora.testinstall` with `FEDORA_VERSION=<version>`
and `PACKAGE_NAME=<package-basename>`, bind-mounting `packages/` at
`/mnt/pkg`. Inside, the container installs the named RPM, then checks
that every file `rpm -ql sodalibs-devel` claims exists. Log:
`logs/test-install-fedora-<version>.log`.

### `scripts/test-install-ubuntu.sh <version> <package-basename>`
Same, for Ubuntu and DEB. Inside, the container runs
`apt-get install /mnt/pkg/<name>`, then verifies every file from
`dpkg -L sodalibs-devel`. Log:
`logs/test-install-ubuntu-<version>.log`.

### `dockerfiles/Fedora.build`
Templated on `ARG FEDORA_VERSION`. Starts from `fedora:${FEDORA_VERSION}`,
installs the toolchain and `rpm-build`, clones SoDaLibs from `/mnt/src`,
checks out `dist-test`, runs cmake / make / ctest / make install /
`cpack -G RPM`, and copies the RPM to `/mnt/out`.

### `dockerfiles/Ubuntu.build`
Templated on `ARG UBUNTU_VERSION`. Same shape as `Fedora.build` but
with apt and `cpack -G DEB`.

### `dockerfiles/Fedora.testinstall`
Templated on `ARG FEDORA_VERSION` and `ARG PACKAGE_NAME`. Fresh
`fedora:${FEDORA_VERSION}`; installs the named RPM from `/mnt/pkg`;
confirms with `rpm -q`; verifies every entry in `rpm -ql
sodalibs-devel` exists on disk, listing any that do not.

### `dockerfiles/Ubuntu.testinstall`
Templated on `ARG UBUNTU_VERSION` and `ARG PACKAGE_NAME`. Same as
`Fedora.testinstall` but with `apt-get install` and `dpkg -s` / `dpkg -L`.

### `BuildTestAndKits.txt`
Original task description. Kept for historical reference.
