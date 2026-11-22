#!/bin/bash
# build-ubuntu.sh <ubuntu-version>
# Build SoDaLibs inside an Ubuntu container: compile, ctest, cpack -G DEB.
# On success, copies the produced DEB into packages/ with distro tag.
# Prints the tagged DEB path on stdout; log goes to logs/.
set -euo pipefail

HERE="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck source=lib.sh
source "${HERE}/lib.sh"

version="${1:?usage: build-ubuntu.sh <ubuntu-version>}"
image="sodalibs_build_ubuntu_${version//./_}"
log="${LOGS_DIR}/build-ubuntu-${version}.log"
: > "${log}"

out_dir="${CT_ROOT}/.tmp/out-ubuntu-${version}"
rm -rf "${out_dir}"; mkdir -p "${out_dir}"

echo_logged "${log}" "=== build-ubuntu ${version} ==="

run_logged "${log}" ch-image build --rebuild \
    --bind "${SODALIBS_ROOT}:/mnt/src" \
    --bind "${out_dir}:/mnt/out" \
    --build-arg "UBUNTU_VERSION=${version}" \
    -t "${image}" \
    -f "${DOCKERFILES_DIR}/Ubuntu.build" \
    "${CT_ROOT}"

shopt -s nullglob
debs=("${out_dir}"/*.deb)
shopt -u nullglob
if (( ${#debs[@]} == 0 )); then
    echo "no DEB produced for ubuntu ${version}" | tee -a "${log}" >&2
    exit 1
fi

tagged="$(tag_package "${debs[0]}" ubuntu "${version}")"
echo_logged "${log}" "tagged: ${tagged}"
printf '%s\n' "${tagged}"
