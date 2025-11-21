#!/usr/bin/env python3
"""Test multiple resets on JuliusEnv"""

import julius_gym
import argparse
from pathlib import Path

args = argparse.ArgumentParser()
args.add_argument("--data-dir", type=Path, required=True, help="Path to Caesar3 directory")
args.add_argument("--lib-path", type=Path, required=True, help="Path to libjulius_gym")
args = args.parse_args()

assert args.data_dir.exists(), f"Data directory {args.data_dir} does not exist"
assert args.lib_path.exists(), f"Library file {args.lib_path} does not exist"

print("Creating environment...")
env = julius_gym.JuliusEnv(
    data_directory=str(args.data_dir),
    max_ticks=100,
    lib_path=str(args.lib_path),
)

print("Environment created successfully!")

for i in range(5):
    print(f"\nReset {i+1}...")
    obs, info = env.reset()
    print(f"  Population: {obs['population_total']}")

    for j in range(3):
        print(f"  Step {j+1}...")
        obs, reward, terminated, truncated, info = env.step(0)
        print(f"    Reward: {reward}, Done: {terminated or truncated}")

env.close()
print("\nTest passed!")
