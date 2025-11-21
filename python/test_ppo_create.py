#!/usr/bin/env python3
"""Test PPO model creation only"""

import julius_gym
import argparse
from pathlib import Path
from stable_baselines3 import PPO

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

print("\nCreating PPO model...")
model = PPO("MultiInputPolicy", env, verbose=1)

print("PPO model created successfully!")

print("\nGetting initial observation...")
obs, info = env.reset()
print(f"Observation received: {list(obs.keys())[:5]}...")

print("\nPredicting action...")
action, _states = model.predict(obs, deterministic=True)
print(f"Action: {action}")

env.close()
print("\nTest passed!")
