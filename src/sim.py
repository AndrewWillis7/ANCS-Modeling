from __future__ import annotations

import argparse
from pathlib import Path
import pprint

from config_loader import load_config

def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="ANC simulation driver")
    parser.add_argument("--config", 
                        type=str, 
                        default="sim_config.yaml", 
                        help="Path to YAML Configuration File",
                        )
    return parser.parse_args()

def main() -> None:
    args = parse_args()
    cfg = load_config(args.config)

    print("\nLoaded Configuration:")
    pprint.pprint(cfg)

    data_dirs = [
        Path(cfg["paths"]["generated_root"]),
        Path(cfg["paths"]["processed_root"]),
        Path(cfg["paths"]["figures_root"]),
    ]
    for d in data_dirs:
        d.mkdir(parents=True, exist_ok=True)
    
    print("\nSimulation bootstrap complete.")
    print(f"Experiment mode: {cfg['run']['experiment']}")
    print(f"Active input: {cfg['run']['active_input']}")

if __name__ == "__main__":
    main()