#!/bin/bash
# build.sh
# Top-level driver.  For every Fedora and Ubuntu version listed in
# versions.conf, run:
#   1. build + ctest + make kit (produces a tagged RPM or DEB in packages/)
#   2. test-install (verifies the kit installs cleanly in a fresh container)
# Failures do not abort the run; a PASS/FAIL table is printed at the end.
set -uo pipefail

HERE="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck source=scripts/lib.sh
source "${HERE}/scripts/lib.sh"

read_versions

summary_reset

run_one() {
    # run_one <distro> <version>
    local distro="$1" version="$2"
    local build_sh test_sh tagged pkg
    case "${distro}" in
        fedora) build_sh=build-fedora.sh;  test_sh=test-install-fedora.sh ;;
        ubuntu) build_sh=build-ubuntu.sh;  test_sh=test-install-ubuntu.sh ;;
        *) echo "unknown distro ${distro}" >&2; return 1 ;;
    esac

    echo
    echo ">>>>>>>>  ${distro} ${version}  <<<<<<<<"

    if tagged="$("${HERE}/scripts/${build_sh}" "${version}")"; then
        summary_add build "${distro}" "${version}" PASS
        pkg="$(basename -- "${tagged}")"
    else
        summary_add build "${distro}" "${version}" FAIL
        summary_add test-install "${distro}" "${version}" SKIP
        return 0
    fi

    if "${HERE}/scripts/${test_sh}" "${version}" "${pkg}"; then
        summary_add test-install "${distro}" "${version}" PASS
    else
        summary_add test-install "${distro}" "${version}" FAIL
    fi
}

for v in ${FEDORA_VERSIONS:-}; do
    run_one fedora "${v}"
done
for v in ${UBUNTU_VERSIONS:-}; do
    run_one ubuntu "${v}"
done

summary_print
