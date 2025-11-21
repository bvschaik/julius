#!/usr/bin/env python3
"""Test PPO model learn with minimal steps"""

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

print("\nCreating PPO model with small n_steps...")
model = PPO("MultiInputPolicy", env, verbose=1, n_steps=2)

print("PPO model created successfully!")

print("\nTraining for 2 timesteps...")
try:
    model.learn(total_timesteps=2)
    print("Training successful!")
except Exception as e:
    print(f"Training failed: {e}")
    import traceback
    traceback.print_exc()

print("\nRendering city map...")
try:
    # Render and save the city map after training
    env.render(mode="save", output_file="city_map_final.png")
    print("City map rendered successfully!")
except Exception as e:
    print(f"Rendering failed: {e}")
    import traceback
    traceback.print_exc()

env.close()
print("\nTest done!")
