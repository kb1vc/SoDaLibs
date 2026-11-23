#!/bin/bash
# test-install-ubuntu.sh <ubuntu-version> <package-basename>
# Verifies that <package-basename> (from packages/) installs cleanly in
# a fresh ubuntu:<version> container.
set -euo pipefail

HERE="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck source=lib.sh
source "${HERE}/lib.sh"

version="${1:?usage: test-install-ubuntu.sh <version> <package-basename>}"
pkg="${2:?usage: test-install-ubuntu.sh <version> <package-basename>}"
image="sodalibs_test_ubuntu_${version//./_}"
log="${LOGS_DIR}/test-install-ubuntu-${version}.log"
: > "${log}"

if [[ ! -f "${PACKAGES_DIR}/${pkg}" ]]; then
    echo_logged "${log}" "missing package: ${PACKAGES_DIR}/${pkg}"
    exit 1
fi

echo_logged "${log}" "=== test-install-ubuntu ${version} / ${pkg} ==="

run_logged "${log}" ch-image build --rebuild \
    --bind "${PACKAGES_DIR}:/mnt/pkg" \
    --build-arg "UBUNTU_VERSION=${version}" \
    --build-arg "PACKAGE_NAME=${pkg}" \
    -t "${image}" \
    -f "${DOCKERFILES_DIR}/Ubuntu.testinstall" \
    "${CT_ROOT}"

echo_logged "${log}" ""
echo_logged "${log}" "YES -- test-install-ubuntu ${version} / ${pkg} PASSED."
