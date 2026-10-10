"""Release lifecycle checks with an in-memory GitHub API; no network calls."""

import importlib.util
from pathlib import Path
import sys
import subprocess
from types import SimpleNamespace
import unittest
from unittest.mock import mock_open, patch

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
        self.next_id = max((r["id"] for r in self.releases), default=122) + 1

    def request(self, method, path, data=None):
        if self.error:
            raise self.error
        if method == "GET" and path.startswith("/releases?"):
            page = int(path.split("page=")[-1])
            return self.releases[(page - 1) * 100:page * 100]
        if method == "POST" and path == "/releases":
            release = dict(data, id=self.next_id)
            self.next_id += 1
            self.mutations.append((method, path, data))
            self.releases.append(release)
            return release
        if method == "GET" and path.startswith("/git/ref/tags/"):
            if self.ref is None:
                raise ApiError(404, "Not found")
            return self.ref
        if path.startswith("/releases/"):
            release = next((r for r in self.releases if str(r["id"]) == path.split("/")[-1]), None)
            if release is None:
                raise ApiError(404, "Not found")
            if method == "GET":
                return release
            if method == "PATCH":
                self.mutations.append((method, path, data))
                release.update(data)
                return release
            if method == "DELETE":
                self.mutations.append((method, path, data))
                self.releases.remove(release)
                return None
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

    def test_failed_creation_does_not_grant_ownership(self):
        api = FakeGitHub()
        request = api.request
        def fail_creation(method, path, data=None):
            if method == "POST":
                raise ApiError(422, "Rejected")
            return request(method, path, data)
        api.request = fail_creation
        with self.assertRaises(ApiError):
            self.snapshot.prepare(api)
        self.assertEqual(api.mutations, [])

    def test_retry_after_cleanup_recreates_draft(self):
        api = FakeGitHub([self.release()])
        self.snapshot.cleanup(api, 123)
        recreated = self.snapshot.prepare(api, 123)
        self.assertTrue(recreated["draft"])
        self.assertEqual(recreated["id"], 124)
        self.assertEqual([m[0] for m in api.mutations], ["DELETE", "POST"])

    def test_replaced_draft_cannot_be_claimed_on_retry(self):
        api = FakeGitHub([self.release(id=124)])
        with self.assertRaises(RuntimeError):
            self.snapshot.prepare(api, 123)
        self.assertEqual(api.mutations, [])

    def test_recorded_release_must_match_tag(self):
        api = FakeGitHub([self.release(tag_name="unrelated")])
        with self.assertRaises(RuntimeError):
            self.snapshot.prepare(api, 123)
        self.assertEqual(api.mutations, [])

    def test_cleanup_deletes_only_owned_draft_by_id(self):
        api = FakeGitHub([self.release()])
        self.snapshot.cleanup(api, 123)
        self.assertEqual(api.mutations, [("DELETE", "/releases/123", None)])
        # Repeating cleanup after deletion is harmless.
        self.snapshot.cleanup(api, 123)
        self.assertEqual(len(api.mutations), 1)

    def test_cleanup_preserves_published_release(self):
        api = FakeGitHub([self.release(draft=False)])
        self.snapshot.cleanup(api, 123)
        self.assertEqual(api.mutations, [])

    def test_cleanup_refuses_unrelated_release_or_moved_tag(self):
        cases = [
            FakeGitHub([self.release(body="other run")]),
            FakeGitHub([self.release(target_commitish="wrong")]),
            FakeGitHub([self.release(tag_name="other")]),
            FakeGitHub([self.release()], {"object": {"type": "commit", "sha": "wrong"}}),
        ]
        for api in cases:
            with self.subTest(api=api), self.assertRaises(RuntimeError):
                self.snapshot.cleanup(api, 123)
            self.assertEqual(api.mutations, [])

    def test_missing_or_invalid_release_id_never_deletes(self):
        for release_id in [None, "", "0", "-1", "../123"]:
            api = FakeGitHub([self.release()])
            with self.subTest(release_id=release_id), self.assertRaises(RuntimeError):
                self.snapshot.cleanup(api, release_id)
            self.assertEqual(api.mutations, [])

    def test_cleanup_api_error_does_not_delete(self):
        api = FakeGitHub([self.release()])
        api.error = ApiError(403, "Forbidden")
        with self.assertRaises(ApiError):
            self.snapshot.cleanup(api, 123)
        self.assertEqual(api.mutations, [])

    def asset_files(self):
        return [Path("release-assets", name) for name in self.snapshot.required_assets]

    def test_complete_packages_upload_to_validated_draft(self):
        api = FakeGitHub([self.release()])
        with patch.object(Path, "glob", return_value=self.asset_files()), \
                patch.object(Path, "stat", return_value=SimpleNamespace(st_size=1)), \
                patch.object(module.subprocess, "run") as upload:
            self.snapshot.upload(api, 123, "release-assets")
        args = upload.call_args.args[0]
        self.assertEqual(args[:4], ["gh", "release", "upload", self.snapshot.tag])
        self.assertEqual(args[4:6], ["--repo", self.snapshot.repository])
        self.assertEqual(args[-1], "--clobber")
        self.assertEqual(len(args), 15)

    def test_incomplete_packages_never_upload(self):
        api = FakeGitHub([self.release()])
        with patch.object(Path, "glob", return_value=self.asset_files()[:-1]), \
                patch.object(Path, "stat", return_value=SimpleNamespace(st_size=1)), \
                patch.object(module.subprocess, "run") as upload:
            with self.assertRaises(RuntimeError):
                self.snapshot.upload(api, 123, "release-assets")
            upload.assert_not_called()

    def test_published_release_is_never_uploaded_or_republished(self):
        api = FakeGitHub([self.release(draft=False)])
        with patch.object(module.subprocess, "run") as upload:
            self.snapshot.upload(api, 123, "release-assets")
            self.snapshot.publish(api, 123)
            upload.assert_not_called()
        self.assertEqual(api.mutations, [])

    def test_upload_failure_leaves_draft_for_safe_cleanup(self):
        api = FakeGitHub([self.release()])
        with patch.object(Path, "glob", return_value=self.asset_files()), \
                patch.object(Path, "stat", return_value=SimpleNamespace(st_size=1)), \
                patch.object(module.subprocess, "run", side_effect=subprocess.CalledProcessError(1, "gh")):
            with self.assertRaises(subprocess.CalledProcessError):
                self.snapshot.upload(api, 123, "release-assets")
        self.assertEqual(api.mutations, [])
        self.snapshot.cleanup(api, 123)
        self.assertEqual(api.mutations, [("DELETE", "/releases/123", None)])

    def test_only_complete_uploaded_native_assets_allow_publication(self):
        assets = [{"name": name, "state": "uploaded", "size": 1}
                  for name in self.snapshot.required_assets]
        api = FakeGitHub([self.release(assets=assets)])
        self.snapshot.publish(api, 123)
        self.assertEqual(api.mutations, [("PATCH", "/releases/123", {"draft": False})])

    def test_partial_upload_cannot_be_published(self):
        api = FakeGitHub([self.release(assets=[])])
        with self.assertRaises(RuntimeError):
            self.snapshot.publish(api, 123)
        self.assertEqual(api.mutations, [])

    def test_publication_completed_before_error_survives_cleanup(self):
        assets = [{"name": name, "state": "uploaded", "size": 1}
                  for name in self.snapshot.required_assets]
        api = FakeGitHub([self.release(assets=assets)])
        request = api.request
        def lose_publish_response(method, path, data=None):
            result = request(method, path, data)
            if method == "PATCH":
                raise ApiError(502, "Lost publication response")
            return result
        api.request = lose_publish_response
        with self.assertRaises(ApiError):
            self.snapshot.publish(api, 123)
        self.snapshot.cleanup(api, 123)
        self.assertEqual(api.mutations, [("PATCH", "/releases/123", {"draft": False})])

    def test_rejected_publish_can_clean_up_owned_draft(self):
        assets = [{"name": name, "state": "uploaded", "size": 1}
                  for name in self.snapshot.required_assets]
        api = FakeGitHub([self.release(assets=assets)])
        request = api.request
        def reject_publish(method, path, data=None):
            if method == "PATCH":
                raise ApiError(422, "Publication rejected")
            return request(method, path, data)
        api.request = reject_publish
        with self.assertRaises(ApiError):
            self.snapshot.publish(api, 123)
        self.snapshot.cleanup(api, 123)
        self.assertEqual(api.mutations, [("DELETE", "/releases/123", None)])

    def test_upload_rechecks_release_identity_before_clobber(self):
        api = FakeGitHub([self.release(body="another run")])
        with patch.object(module.subprocess, "run") as upload:
            with self.assertRaises(RuntimeError):
                self.snapshot.upload(api, 123, "release-assets")
            upload.assert_not_called()

    def test_publish_rechecks_release_identity_before_mutation(self):
        api = FakeGitHub([self.release(body="another run")])
        with self.assertRaises(RuntimeError):
            self.snapshot.publish(api, 123)
        self.assertEqual(api.mutations, [])

    def test_required_packages_match_workflow_native_matrix(self):
        import re
        workflow = (Path(__file__).parents[2] / "workflows/snapshots.yml").read_text()
        packages = set(re.findall(r"^            package: (\S+)$", workflow, re.M))
        self.assertEqual(packages | {"asl-freebsd-x86_64"}, set(module.REQUIRED_PACKAGES))

    def test_cancellation_skips_cleanup_and_failed_jobs_keep_artifacts(self):
        workflow = (Path(__file__).parents[2] / "workflows/snapshots.yml").read_text()
        self.assertIn("always() && !cancelled()", workflow)
        self.assertIn("!cancelled() && (failure()", workflow)
        self.assertNotIn("--cleanup-tag", workflow)
        self.assertEqual(workflow.count("compression-level: 0\n          overwrite: true"), 3)

    def main_environment(self):
        return {"GITHUB_REPOSITORY": self.snapshot.repository,
                "GITHUB_RUN_ID": self.snapshot.run_id, "GITHUB_SHA": self.snapshot.sha,
                "SNAPSHOT_TAG": self.snapshot.tag, "SNAPSHOT_BUILD_ID": self.snapshot.build_id,
                "GH_TOKEN": "test-token", "GITHUB_OUTPUT": "unused-mocked-output"}

    def test_only_valid_draft_emits_downstream_release_id(self):
        api = FakeGitHub([self.release()])
        with patch.dict(module.os.environ, self.main_environment()), \
                patch.object(sys, "argv", ["snapshot_release.py", "prepare"]), \
                patch.object(module, "GitHub", return_value=api), \
                patch("builtins.open", mock_open()) as output:
            module.main()
        self.assertEqual(output().write.call_args_list[0].args, ("release_id=123\n",))

    def test_published_release_does_not_emit_mutation_permission(self):
        api = FakeGitHub([self.release(draft=False)])
        with patch.dict(module.os.environ, self.main_environment()), \
                patch.object(sys, "argv", ["snapshot_release.py", "prepare"]), \
                patch.object(module, "GitHub", return_value=api), \
                patch("builtins.open", mock_open()) as output:
            module.main()
        self.assertEqual(output().write.call_args_list[0].args, ("release_id=\n",))
        self.assertEqual(api.mutations, [])

    def test_failed_preparation_emits_no_ownership_output(self):
        api = FakeGitHub([self.release(body="unrelated")])
        with patch.dict(module.os.environ, self.main_environment()), \
                patch.object(sys, "argv", ["snapshot_release.py", "prepare"]), \
                patch.object(module, "GitHub", return_value=api), \
                patch("builtins.open", mock_open()) as output:
            with self.assertRaises(RuntimeError):
                module.main()
            output.assert_not_called()


if __name__ == "__main__":
    unittest.main()
