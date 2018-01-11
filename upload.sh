#!/usr/bin/env bash

VERSION=$1
if [[ -z "${VERSION}" ]]; then
  echo "Must provide a version!"
  exit 1
fi

RELEASE_DIR="/srv/http/root/upload/prime-practice-mod"

scp "release/${VERSION}/prime-practice-${VERSION}.zip" "pwootage@new.pwootage.com:${RELEASE_DIR}/prime-practice-${VERSION}.zip"
