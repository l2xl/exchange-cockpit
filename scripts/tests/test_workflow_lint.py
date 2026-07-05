# XCockpit
# Copyright (c) 2026 l2xl (l2xl/at/proton.me)
# Distributed under the Intellectual Property Reserve License, v2 (IPRL)

"""Tests for the requirements-gate GitHub Actions workflow -- [INFRA-066] [INFRA-067] [INFRA-068] [INFRA-069]."""

import shutil
import subprocess
from pathlib import Path

import pytest
import yaml

REPO_ROOT = Path(__file__).resolve().parent.parent.parent
WORKFLOW = REPO_ROOT / ".github" / "workflows" / "requirements-gate.yml"


def _steps():
    doc = yaml.safe_load(WORKFLOW.read_text())
    job = doc["jobs"]["requirements"]
    return [f"{s.get('name', '')} {s.get('uses', '')} {s.get('run', '')}" for s in job["steps"]]


def test_workflow_has_build_and_ctest_steps():
    steps = " | ".join(_steps())
    assert "pytest" in steps.lower()
    assert "cmake --build" in steps
    assert "ctest" in steps
    assert "-LE live" in steps


def test_workflow_runs_status_rollup_over_both_junit_reports_and_publishes():
    steps = " | ".join(_steps())
    status_step = next(s for s in _steps() if "req_status.py" in s)
    assert "--junit pytest-results.xml" in status_step
    assert "--junit build-ci/ctest-results.xml" in status_step
    assert "publish_check_run" in steps


def test_workflow_publishes_status_as_run_artifact():
    doc = yaml.safe_load(WORKFLOW.read_text())
    uploads = [s for s in doc["jobs"]["requirements"]["steps"] if "upload-artifact" in s.get("uses", "")]
    assert any("req_status.json" in s.get("with", {}).get("path", "") for s in uploads)


def test_workflow_publishes_requirements_tree_on_the_commit():
    steps = " | ".join(_steps())
    assert "publish_check_run.py --status req_status.json" in steps


def test_workflow_has_no_github_pages_publication():
    doc = yaml.safe_load(WORKFLOW.read_text())
    assert list(doc["jobs"]) == ["requirements"]
    steps = " | ".join(_steps())
    assert "pages" not in steps.lower()


@pytest.mark.skipif(shutil.which("actionlint") is None, reason="actionlint not installed")
def test_actionlint_passes():
    result = subprocess.run(["actionlint", str(WORKFLOW)], capture_output=True, text=True)
    assert result.returncode == 0, result.stdout + result.stderr
