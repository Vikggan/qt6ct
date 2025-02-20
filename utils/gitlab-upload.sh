#!/bin/sh

if [ -z $1 ] || [ -z $2 ]; then
   echo Usage: gitlab-upload VERSION TOKEN
   exit
fi

ID=5459
NAME=qt6ct
VERSION=$1
TOKEN=$2
TARBALL=${NAME}-${VERSION}.tar.xz

URL="https://www.opencode.net/api/v4/projects/${ID}/packages/generic/${NAME}/${VERSION}/${TARBALL}"

echo URL = ${URL}

curl --location --header "PRIVATE-TOKEN: ${TOKEN}" \
     --upload-file ../extras/packages/sources/${TARBALL} \
     ${URL}

curl --request POST \
     --header "PRIVATE-TOKEN: ${TOKEN}" \
     --data name="${TARBALL}" \
     --data url=${URL} \
     "https://www.opencode.net/api/v4/projects/${ID}/releases/${VERSION}/assets/links"

echo ""