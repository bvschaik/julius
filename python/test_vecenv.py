#!/usr/bin/env python3
"""Test the JuliusEnv with DummyVecEnv wrapper"""

import julius_gym
import argparse
from pathlib import Path
from stable_baselines3.common.vec_env import DummyVecEnv

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

print("\nWrapping in DummyVecEnv...")
vec_env = DummyVecEnv([lambda: env])

print("DummyVecEnv created successfully!")

print("\nResetting vectorized environment...")
obs = vec_env.reset()

print("Reset successful!")
print(f"Observation shape: {obs.shape if hasattr(obs, 'shape') else 'dict'}")

print("\nTaking a step...")
action = [0]  # NONE action
obs, reward, done, info = vec_env.step(action)

print("Step successful!")
print(f"Reward: {reward}")
print(f"Done: {done}")

vec_env.close()
print("\nTest passed!")
