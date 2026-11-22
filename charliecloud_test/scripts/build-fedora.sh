#!/bin/bash
# build-fedora.sh <fedora-version>
# Build SoDaLibs inside a Fedora container: compile, ctest, cpack -G RPM.
# On success, copies the produced RPM into packages/ with distro tag.
# Prints the tagged RPM path on stdout; log goes to logs/.
set -euo pipefail

HERE="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck source=lib.sh
source "${HERE}/lib.sh"

version="${1:?usage: build-fedora.sh <fedora-version>}"
image="sodalibs_build_fedora_${version}"
log="${LOGS_DIR}/build-fedora-${version}.log"
: > "${log}"

# Fresh scratch dir for the Dockerfile's cpack output.
out_dir="${CT_ROOT}/.tmp/out-fedora-${version}"
rm -rf "${out_dir}"; mkdir -p "${out_dir}"

echo_logged "${log}" "=== build-fedora ${version} ==="

run_logged "${log}" ch-image build --rebuild \
    --bind "${SODALIBS_ROOT}:/mnt/src" \
    --bind "${out_dir}:/mnt/out" \
    --build-arg "FEDORA_VERSION=${version}" \
    -t "${image}" \
    -f "${DOCKERFILES_DIR}/Fedora.build" \
    "${CT_ROOT}"

shopt -s nullglob
rpms=("${out_dir}"/*.rpm)
shopt -u nullglob
if (( ${#rpms[@]} == 0 )); then
    echo "no RPM produced for fedora ${version}" | tee -a "${log}" >&2
    exit 1
fi

tagged="$(tag_package "${rpms[0]}" fedora "${version}")"
echo_logged "${log}" "tagged: ${tagged}"
printf '%s\n' "${tagged}"
