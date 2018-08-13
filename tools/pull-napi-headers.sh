#!/usr/bin/env bash
set -e

# Usage:
# $ tools/pull-napi-headers.sh v10.8.0

sed_command=sed
if test $(uname) = 'Darwin'; then
  sed_command=gsed
  if ! type $sed_command &>/dev/null; then
    printf "!! gnu-sed not found. \
      Try install gnu-sed with 'brew install gnu-sed'\n"
    exit 1
  fi
fi

if ! type jq &>/dev/null; then
  printf "!! jq not found. \
    Try install jq with 'brew install jq'\n"
  exit 1
fi

tag=$1

echo "Fetching Tag information for tag $tag"

tag_sha=$(curl -s \
  "https://api.github.com/repos/nodejs/node/git/refs/tags/$tag" | \
  jq -r '.object.sha')

if test $tag_sha = 'null'; then
  echo "Tag $tag not found"
  exit 1
fi

files_to_pull="node_api.h node_api_types.h"

for file in $files_to_pull; do
  echo "Fetching file $file"

  curl -so "include/$file" \
    "https://raw.githubusercontent.com/nodejs/node/$tag/src/$file"

  # Prepend a git hash to the file
  command $sed_command -i \
    "1s/^/\/\/ Pulled from nodejs\/node#$tag_sha $tag\n\n/" \
    "include/$file"

  echo "Downloaded file $file"
done
