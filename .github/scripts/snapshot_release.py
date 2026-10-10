"""Manage the release belonging to a single snapshot workflow run."""

import argparse
import json
import os
import sys
import urllib.error
import urllib.parse
import urllib.request
from dataclasses import dataclass


class ApiError(RuntimeError):
    def __init__(self, status, message):
        super().__init__(f"GitHub API returned HTTP {status}: {message}")
        self.status = status


class GitHub:
    def __init__(self, repository, token):
        self.base = f"https://api.github.com/repos/{repository}"
        self.token = token

    def request(self, method, path, data=None):
        request = urllib.request.Request(
            self.base + path,
            data=json.dumps(data).encode() if data is not None else None,
            method=method,
            headers={
                "Authorization": f"Bearer {self.token}",
                "Accept": "application/vnd.github+json",
                "Content-Type": "application/json",
                "User-Agent": "asl-snapshot-workflow",
            },
        )
        try:
            with urllib.request.urlopen(request, timeout=60) as response:
                body = response.read()
        except urllib.error.HTTPError as error:
            raise ApiError(error.code, error.read().decode()) from error
        return json.loads(body) if body else None

    def optional(self, path):
        try:
            return self.request("GET", path)
        except ApiError as error:
            if error.status == 404:
                return None
            raise


@dataclass
class Snapshot:
    repository: str
    run_id: str
    sha: str
    tag: str
    build_id: str

    @property
    def marker(self):
        return f"<!-- snapshot-run:{self.repository}:{self.run_id} sha:{self.sha} -->"

    def validate(self, api, release, release_id=None):
        if (
            release.get("tag_name") != self.tag
            or release.get("target_commitish") != self.sha
            or self.marker not in (release.get("body") or "").splitlines()
            or (release_id is not None and str(release.get("id")) != str(release_id))
        ):
            raise RuntimeError("Release identity does not match this snapshot run.")
        if not isinstance(release.get("id"), int) or not isinstance(release.get("draft"), bool):
            raise RuntimeError("Invalid release metadata.")
        self.validate_tag(api)

    def validate_tag(self, api):
        # target_commitish is ignored by GitHub when a tag already exists.
        ref = api.optional("/git/ref/tags/" + urllib.parse.quote(self.tag, safe=""))
        if ref is not None:
            target = ref["object"]
            for _ in range(20):
                if target["type"] != "tag":
                    break
                target = api.request("GET", "/git/tags/" + target["sha"])["object"]
            if target["type"] != "commit" or target["sha"] != self.sha:
                raise RuntimeError("Snapshot tag points at a different commit.")

    def find(self, api):
        # The list API includes drafts for a caller with repository write access.
        page = 1
        while True:
            releases = api.request("GET", f"/releases?per_page=100&page={page}")
            for release in releases:
                if release["tag_name"] == self.tag:
                    return release
            if len(releases) < 100:
                return None
            page += 1

    def prepare(self, api):
        release = self.find(api)
        if release is None:
            # Validate any existing tag before creating a release for it.
            self.validate_tag(api)
            release = api.request("POST", "/releases", {
                "tag_name": self.tag,
                "target_commitish": self.sha,
                "name": f"Snapshot {self.build_id}",
                "draft": True,
                "body": f"Snapshot build for {self.build_id}.\n\n"
                        "This is an automated snapshot build, not a stable release. "
                        "For Apple Silicon binaries, please use the macOS ARM64 build.\n\n"
                        + self.marker,
            })
        self.validate(api, release)
        return release


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("command", choices=["prepare"])
    args = parser.parse_args()
    snapshot = Snapshot(os.environ["GITHUB_REPOSITORY"], os.environ["GITHUB_RUN_ID"],
                        os.environ["GITHUB_SHA"], os.environ["SNAPSHOT_TAG"],
                        os.environ["SNAPSHOT_BUILD_ID"])
    api = GitHub(snapshot.repository, os.environ["GH_TOKEN"])
    if args.command == "prepare":
        release = snapshot.prepare(api)
        with open(os.environ["GITHUB_OUTPUT"], "a", encoding="utf-8") as output:
            # Only a validated draft grants downstream jobs permission to mutate it.
            output.write(f"release_id={release['id'] if release['draft'] else ''}\n")
            output.write(f"published={str(not release['draft']).lower()}\n")
        print("Prepared owned draft." if release["draft"] else "Snapshot already published.")


if __name__ == "__main__":
    try:
        main()
    except (RuntimeError, urllib.error.URLError, KeyError, ValueError) as error:
        print(f"Snapshot release operation failed: {error}", file=sys.stderr)
        sys.exit(1)
