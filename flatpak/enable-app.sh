#!/bin/sh

set -eu

if [ "$#" -ne 1 ]; then
    echo "Usage: $0 FLATPAK_APP_ID" >&2
    exit 2
fi

app_id=$1
runtime=$(flatpak info --show-runtime "${app_id}")
case "${runtime}" in
    org.kde.Platform/*) ;;
    *)
        echo "${app_id} uses ${runtime}, not org.kde.Platform." >&2
        echo "Applications that bundle Qt need an application-specific build." >&2
        exit 1
        ;;
esac

runtime_branch=${runtime##*/}
data_home=${XDG_DATA_HOME:-"${HOME}/.local/share"}
install_root=${QT6CT_FLATPAK_ROOT:-"${data_home}/qt6ct-flatpak"}
install_dir="${install_root}/${runtime_branch}"
plugin="${install_dir}/lib/plugins/platformthemes/libqt6ct.so"
config_file=${QT6CT_CONFIG:-"${XDG_CONFIG_HOME:-${HOME}/.config}/qt6ct/qt6ct.conf"}

if [ ! -f "${plugin}" ]; then
    echo "No qt6ct build was found for KDE runtime ${runtime_branch}." >&2
    echo "Run: $(dirname -- "$0")/build-local.sh ${runtime_branch}" >&2
    exit 1
fi

if [ ! -f "${config_file}" ]; then
    echo "The qt6ct configuration does not exist: ${config_file}" >&2
    exit 1
fi

flatpak override --user \
    --filesystem="${install_dir}:ro" \
    --filesystem="$(dirname -- "${config_file}"):ro" \
    --env="QT_PLUGIN_PATH=${install_dir}/lib/plugins" \
    --env="QT_QPA_PLATFORMTHEME=qt6ct" \
    --env="QT6CT_CONFIG=${config_file}" \
    "${app_id}"

echo "Enabled qt6ct ${runtime_branch} for ${app_id}. Restart the application to apply it."
