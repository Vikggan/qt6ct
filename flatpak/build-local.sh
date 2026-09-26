#!/bin/sh

set -eu

runtime_branch=${1:-6.11}
case "${runtime_branch}" in
    6.*) ;;
    *)
        echo "Usage: $0 [KDE runtime branch, for example 6.11]" >&2
        exit 2
        ;;
esac

script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
source_dir=$(CDPATH= cd -- "${script_dir}/.." && pwd)
data_home=${XDG_DATA_HOME:-"${HOME}/.local/share"}
cache_home=${XDG_CACHE_HOME:-"${HOME}/.cache"}
install_root=${QT6CT_FLATPAK_ROOT:-"${data_home}/qt6ct-flatpak"}
install_dir="${install_root}/${runtime_branch}"
build_dir="${cache_home}/qt6ct-flatpak/build-${runtime_branch}"

if ! flatpak info "org.kde.Sdk//${runtime_branch}" >/dev/null 2>&1; then
    echo "The org.kde.Sdk//${runtime_branch} runtime is not installed." >&2
    echo "Install it with: flatpak install flathub org.kde.Sdk//${runtime_branch}" >&2
    exit 1
fi

mkdir -p "${install_root}" "${build_dir}"

flatpak run \
    --command=sh \
    --filesystem="${source_dir}:ro" \
    --filesystem="${install_root}" \
    --filesystem="${build_dir}" \
    "org.kde.Sdk//${runtime_branch}" \
    -eu -c '
        source_dir=$1
        build_dir=$2
        install_dir=$3

        cmake -S "${source_dir}" -B "${build_dir}" \
            -DCMAKE_BUILD_TYPE=Release \
            -DCMAKE_DISABLE_FIND_PACKAGE_Qt6LinguistTools=ON \
            -DCMAKE_INSTALL_PREFIX="${install_dir}" \
            -DCMAKE_INSTALL_LIBDIR=lib \
            -DCMAKE_INSTALL_RPATH="\$ORIGIN/../..;\$ORIGIN/../lib" \
            -DPLUGINDIR="${install_dir}/lib/plugins"
        cmake --build "${build_dir}" --parallel
        cmake --install "${build_dir}"
    ' sh "${source_dir}" "${build_dir}" "${install_dir}"

echo "qt6ct for KDE runtime ${runtime_branch} was installed in ${install_dir}"
