#!/usr/bin/env python3
"""Test the JuliusEnv with flattened observation space"""

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
print(f"Observation space: {env.observation_space}")
print(f"Action space: {env.action_space}")

print("\nResetting environment...")
obs, info = env.reset()

print("Reset successful!")
print(f"Observation keys: {list(obs.keys())}")
print(f"Sample observation values:")
print(f"  ratings_culture: {obs['ratings_culture']}")
print(f"  ratings_prosperity: {obs['ratings_prosperity']}")
print(f"  finance_treasury: {obs['finance_treasury']}")
print(f"  population_total: {obs['population_total']}")

print("\nTaking a step...")
action = 0  # NONE action
obs, reward, terminated, truncated, info = env.step(action)

print("Step successful!")
print(f"Reward: {reward}")
print(f"Terminated: {terminated}")
print(f"Truncated: {truncated}")

env.close()
print("\nTest passed!")
