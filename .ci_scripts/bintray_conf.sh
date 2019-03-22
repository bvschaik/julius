build_dir="$(pwd)/build"

VERSION=$(cat res/version.txt)

if [ $DEPLOY = "vita" ]
then
cat > "bintray.json" <<EOF
{
  "package": {
    "subject": "bvschaik",
    "repo": "julius",
    "name": "vita-unstable",
    "licenses": ["AGPL-V3"],
    "vcs_url": "https://github.com/bvschaik/julius.git"
  },

  "version": {
    "name": "$VERSION",
    "released": "$(date +'%Y-%m-%d')",
    "desc": "Automated vita build for Travis-CI job: $TRAVIS_JOB_WEB_URL"
  },

  "files": [
    {
      "includePattern": "${build_dir}/julius.vpk",
      "uploadPattern": "julius-$VERSION.vpk"
    }
  ],

  "publish": true
}
EOF
elif [ $DEPLOY = "switch" ]
then
cat > "bintray.json" <<EOF
{
  "package": {
    "subject": "bvschaik",
    "repo": "julius",
    "name": "switch-unstable",
    "licenses": ["AGPL-V3"],
    "vcs_url": "https://github.com/bvschaik/julius.git"
  },

  "version": {
    "name": "$VERSION",
    "released": "$(date +'%Y-%m-%d')",
    "desc": "Automated Switch build for Travis-CI job: $TRAVIS_JOB_WEB_URL"
  },

  "files": [
    {
      "includePattern": "${build_dir}/julius_switch.zip",
      "uploadPattern": "julius-$VERSION.zip"
    }
  ],

  "publish": true
}
EOF
fi
