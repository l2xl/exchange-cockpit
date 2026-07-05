# XCockpit
# Copyright (c) 2026 l2xl (l2xl/at/proton.me)
# Distributed under the Intellectual Property Reserve License, v2 (IPRL)

"""Tests for scripts/check_frozen_tests.py -- [INFRA-035]."""

import doorstop

from check_frozen_tests import run


def _make_reviewed_item(tree_builder, text="def test_x(): pass\n"):
    base, make_document, make_item = tree_builder
    make_document(base, "infra", "INFRA", extensions={"item_sha_required": True})
    (base / "test_thing.py").write_text(text)
    make_item(
        base / "infra", "INFRA-035", "1.4.2", "leaf", normative=True, verify="test",
        references=[{"path": "test_thing.py", "type": "file", "keyword": "INFRA-035"}],
    )
    tree = doorstop.build(root=str(base))
    tree.find_item("INFRA-035").review()
    return base


def test_unmodified_frozen_reference_passes(tree_builder):
    base = _make_reviewed_item(tree_builder)
    assert run(base) == 0


def test_modified_frozen_reference_fails(tree_builder):
    base = _make_reviewed_item(tree_builder)
    (base / "test_thing.py").write_text("def test_x(): pass  # silently edited\n")
    assert run(base) == 1


def test_unreviewed_item_is_ignored_even_if_modified(tree_builder):
    base, make_document, make_item = tree_builder
    make_document(base, "infra", "INFRA", extensions={"item_sha_required": True})
    (base / "test_thing.py").write_text("def test_x(): pass\n")
    make_item(
        base / "infra", "INFRA-035", "1.4.2", "leaf", normative=True, verify="test",
        references=[{"path": "test_thing.py", "type": "file", "keyword": "INFRA-035"}],
    )
    (base / "test_thing.py").write_text("def test_x(): pass  # edited before any review\n")
    assert run(base) == 0
