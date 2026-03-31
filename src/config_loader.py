from __future__ import annotations

from pathlib import Path
from typing import Any

import yaml

def load_config(config_path: str | Path) -> dict[str, Any]:
    path = Path(config_path)
    if not path.exists():
        raise FileNotFoundError(f"Config file not found: {path}")
    
    with path.open("r", encoding="utf-8") as f:
        cfg = yaml.safe_load(f)

    if not isinstance(cfg, dict):
        raise ValueError("Top-Level YAML config must be a dictionary")
    
    return cfg

def resolve_path(cfg: dict[str, Any], *keys: str) -> Path:
    value: Any = cfg
    for key in keys:
        value = value[key]
    return Path(value).resolve();