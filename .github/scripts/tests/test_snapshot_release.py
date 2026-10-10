"""Release lifecycle checks with an in-memory GitHub API; no network calls."""

import importlib.util
from pathlib import Path
import sys
import unittest

spec = importlib.util.spec_from_file_location(
    "snapshot_release", Path(__file__).parents[1] / "snapshot_release.py")
module = importlib.util.module_from_spec(spec)
sys.modules[spec.name] = module
spec.loader.exec_module(module)
Snapshot, ApiError = module.Snapshot, module.ApiError


class FakeGitHub:
    def __init__(self, releases=None, ref=None):
        self.releases = list(releases or [])
        self.ref = ref
        self.mutations = []
        self.error = None

    def request(self, method, path, data=None):
        if self.error:
            raise self.error
        if method == "GET" and path.startswith("/releases?"):
            page = int(path.split("page=")[-1])
            return self.releases[(page - 1) * 100:page * 100]
        if method == "POST" and path == "/releases":
            release = dict(data, id=123)
            self.mutations.append((method, path, data))
            self.releases.append(release)
            return release
        if method == "GET" and path.startswith("/git/ref/tags/"):
            if self.ref is None:
                raise ApiError(404, "Not found")
            return self.ref
        raise AssertionError((method, path, data))

    optional = module.GitHub.optional


class SnapshotTests(unittest.TestCase):
    def setUp(self):
        self.snapshot = Snapshot("owner/repo", "456", "abc", "snapshot-date-run456", "date-run456")

    def release(self, **overrides):
        return dict({"id": 123, "tag_name": self.snapshot.tag,
                     "target_commitish": "abc", "draft": True,
                     "body": self.snapshot.marker}, **overrides)

    def test_new_draft_records_run_and_sha(self):
        api = FakeGitHub()
        release = self.snapshot.prepare(api)
        self.assertEqual(release["id"], 123)
        self.assertIn(self.snapshot.marker, release["body"].splitlines())
        self.assertEqual(len(api.mutations), 1)

    def test_retry_reuses_draft_without_mutations(self):
        api = FakeGitHub([self.release()])
        self.assertEqual(self.snapshot.prepare(api)["id"], 123)
        self.assertEqual(api.mutations, [])

    def test_already_published_is_read_only(self):
        api = FakeGitHub([self.release(draft=False)])
        self.assertFalse(self.snapshot.prepare(api)["draft"])
        self.assertEqual(api.mutations, [])

    def test_other_run_cannot_claim_existing_release(self):
        for overrides in [{"body": "other run"}, {"target_commitish": "wrong"}]:
            with self.subTest(overrides=overrides):
                api = FakeGitHub([self.release(**overrides)])
                with self.assertRaises(RuntimeError):
                    self.snapshot.prepare(api)
                self.assertEqual(api.mutations, [])

    def test_tag_target_is_checked_even_when_metadata_matches(self):
        api = FakeGitHub([self.release()], {"object": {"type": "commit", "sha": "wrong"}})
        with self.assertRaises(RuntimeError):
            self.snapshot.prepare(api)
        self.assertEqual(api.mutations, [])

    def test_wrong_existing_tag_prevents_creation(self):
        api = FakeGitHub(ref={"object": {"type": "commit", "sha": "wrong"}})
        with self.assertRaises(RuntimeError):
            self.snapshot.prepare(api)
        self.assertEqual(api.mutations, [])

    def test_api_error_is_not_treated_as_missing_release(self):
        api = FakeGitHub()
        api.error = ApiError(403, "Forbidden")
        with self.assertRaises(ApiError):
            self.snapshot.prepare(api)
        self.assertEqual(api.mutations, [])

    def test_pagination_finds_old_draft(self):
        api = FakeGitHub([self.release(tag_name=f"other-{n}") for n in range(100)] + [self.release()])
        self.assertEqual(self.snapshot.prepare(api)["id"], 123)
        self.assertEqual(api.mutations, [])

    def test_distinct_run_has_distinct_release(self):
        api = FakeGitHub([self.release()])
        other = Snapshot("owner/repo", "789", "abc", "snapshot-date-run789", "date-run789")
        self.assertEqual(other.prepare(api)["tag_name"], other.tag)
        self.assertEqual(len(api.mutations), 1)


if __name__ == "__main__":
    unittest.main()
