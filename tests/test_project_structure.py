import os
import pytest
from pathlib import Path


def repo_root() -> Path:
    # tests directory is at <repo>/tests
    return Path(__file__).resolve().parent.parent


def test_config_files_exist():
    root = repo_root()
    config = root / "Config"
    expected = [
        "DefaultEditor.ini",
        "DefaultEditorPerProjectUserSettings.ini",
        "DefaultEngine.ini",
        "DefaultGame.ini",
        "DefaultGameplayTags.ini",
        "DefaultInput.ini",
    ]
    assert config.exists(), f"Config directory not found at {config}"
    for name in expected:
        path = config / name
        assert path.exists(), f"Expected config file missing: {path}"


def test_default_engine_contains_settings():
    root = repo_root()
    engine_ini = root / "Config" / "DefaultEngine.ini"
    assert engine_ini.exists(), "DefaultEngine.ini missing"
    text = engine_ini.read_text(encoding="utf-8")

    # Basic sanity checks — ensure maps setting and some renderer flags exist
    assert "GameDefaultMap=" in text, "GameDefaultMap not found in DefaultEngine.ini"
    assert "[/Script/Engine.RendererSettings]" in text or "r.RayTracing" in text, "Renderer settings not present"
    # Check for ray-tracing flag presence (case-insensitive match)
    assert any(k in text for k in ["r.RayTracing", "RayTracing"]) , "Ray tracing flag not found"


def test_content_structure():
    root = repo_root()
    content = root / "Content"
    if not content.exists():
        pytest.skip(f"Content directory not present in checkout ({content}); skipping content structure checks")

    priston = content / "01_PristonRework"
    assert priston.exists(), "Content/01_PristonRework missing"
    blueprints = priston / "Blueprints"
    assert blueprints.exists(), "Blueprints directory missing under Content/01_PristonRework"
