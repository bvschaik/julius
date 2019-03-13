build_dir="$(pwd)/build"

cat > "bintray-vita.json" <<EOF
{
  "package": {
    "subject": "bvschaik",
    "repo": "julius",
    "name": "vita",
    "licenses": ["AGPL-V3"],
    "vcs_url": "https://github.com/bvschaik/julius.git"
  },

  "version": {
    "name": "$TRAVIS_BUILD_NUMBER",
    "released": "$(date +'%Y-%m-%d')",
    "desc": "Automated vita build for Travis-CI job: $TRAVIS_JOB_WEB_URL\nClick on the \"Files\" tab in order to download this version."
  },

  "files": [
    {
      "includePattern": "${build_dir}/julius.vpk",
      "uploadPattern": "julius-$(date +'%Y-%m-%d')-$(git rev-parse --short --verify HEAD).vpk",
      "matrixParams": {
        "override": 1,
        "publish": 1
      }
    }
  ],

  "publish": true
}
EOF
