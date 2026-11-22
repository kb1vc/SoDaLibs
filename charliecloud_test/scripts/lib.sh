# shellcheck shell=bash
# Shared helpers for charliecloud_test scripts.
# Source this file; do not execute it directly.

# CT_ROOT is the charliecloud_test directory.
CT_ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
# SODALIBS_ROOT is the SoDaLibs git top-level that holds charliecloud_test.
SODALIBS_ROOT="$(cd -- "${CT_ROOT}/.." && pwd)"

VERSIONS_CONF="${CT_ROOT}/versions.conf"
PACKAGES_DIR="${CT_ROOT}/packages"
LOGS_DIR="${CT_ROOT}/logs"
DOCKERFILES_DIR="${CT_ROOT}/dockerfiles"
IMAGES_DIR="${CT_ROOT}/images"

mkdir -p "${PACKAGES_DIR}" "${LOGS_DIR}"
# ch-image refuses to use a storage dir it didn't create itself, so we must
# NOT pre-mkdir IMAGES_DIR -- just let ch-image populate it on first run.
export CH_IMAGE_STORAGE="${IMAGES_DIR}"

read_versions() {
    # Parse versions.conf.  The file uses shell-style KEY="v1 v2 v3" lines.
    # We deliberately source it in a subshell-safe way by extracting values.
    local conf="${VERSIONS_CONF}"
    if [[ ! -r "${conf}" ]]; then
        echo "versions.conf not readable at ${conf}" >&2
        return 1
    fi
    # shellcheck disable=SC1090
    source "${conf}"
    : "${FEDORA_VERSIONS:=}" "${UBUNTU_VERSIONS:=}"
}

distro_tag() {
    # distro_tag <distro> <version>  ->  "Fedora-44" / "Ubuntu-24.04"
    local distro="$1" version="$2"
    case "${distro}" in
        fedora) printf 'Fedora-%s' "${version}" ;;
        ubuntu) printf 'Ubuntu-%s' "${version}" ;;
        *) echo "unknown distro: ${distro}" >&2; return 1 ;;
    esac
}

tag_package() {
    # tag_package <source-pkg-path> <distro> <version>
    # Copies <source-pkg-path> to packages/<basename>.<DistroTag>.<ext>
    # and prints the new path.
    local src="$1" distro="$2" version="$3"
    local base ext tag dst
    base="$(basename -- "${src}")"
    ext="${base##*.}"
    tag="$(distro_tag "${distro}" "${version}")" || return 1
    dst="${PACKAGES_DIR}/${base%.*}.${tag}.${ext}"
    cp -f -- "${src}" "${dst}"
    printf '%s' "${dst}"
}

summary_reset() {
    SUMMARY_ROWS=()
}

summary_add() {
    # summary_add <step> <distro> <version> <status>
    SUMMARY_ROWS+=("$1|$2|$3|$4")
}

summary_print() {
    echo
    echo "================  SUMMARY  ================"
    printf '%-14s  %-7s  %-7s  %s\n' step distro version status
    printf '%-14s  %-7s  %-7s  %s\n' -------- ------ ------- ------
    local row step distro version status
    local fails=0
    for row in "${SUMMARY_ROWS[@]}"; do
        IFS='|' read -r step distro version status <<< "${row}"
        printf '%-14s  %-7s  %-7s  %s\n' \
               "${step}" "${distro}" "${version}" "${status}"
        [[ "${status}" == PASS ]] || fails=$((fails + 1))
    done
    echo "==========================================="
    if (( fails > 0 )); then
        echo "${fails} failure(s)."
        return 1
    fi
    echo "All green."
    return 0
}

run_logged() {
    # run_logged <log-path> <cmd...>
    # Streams the command's stdout+stderr to BOTH the log file and this
    # process's stderr.  Stderr (not stdout) so that callers can still use
    # stdout for return values via command substitution.
    local log="$1"; shift
    printf '+ %s\n' "$*" >&2
    printf '+ %s\n' "$*" >> "${log}"
    "$@" > >(tee -a "${log}" >&2) 2>&1
}

echo_logged() {
    # echo_logged <log-path> <message...>
    # Writes <message> to stderr (visible) and appends to the log file.
    local log="$1"; shift
    printf '%s\n' "$*" >&2
    printf '%s\n' "$*" >> "${log}"
}
