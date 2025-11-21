#!/usr/bin/env python3
"""Test the rendering functionality"""

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

print("\nResetting environment...")
obs, info = env.reset()
print("Reset successful!")

print("\nTaking a few steps...")
for i in range(5):
    action = 0  # NONE action
    obs, reward, terminated, truncated, info = env.step(action)
    print(f"Step {i+1}: reward={reward:.3f}, terminated={terminated}, truncated={truncated}")
    if terminated or truncated:
        break

print("\nRendering city map...")
try:
    env.render(mode="save", output_file="test_city_map.png")
    print("City map rendered successfully! Saved to test_city_map.png")
except Exception as e:
    print(f"Rendering failed: {e}")
    import traceback
    traceback.print_exc()

env.close()
print("\nTest done!")
