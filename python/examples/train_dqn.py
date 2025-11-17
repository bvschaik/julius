#!/usr/bin/env python3
"""
DQN Agent Training Example

Deep Q-Network is memory efficient and works well with discrete action spaces.
This example uses a smaller network suitable for 4GB GPU.

Memory usage: ~1-2GB GPU RAM during training
"""

import argparse
import sys
from pathlib import Path

try:
    from stable_baselines3 import DQN
    from stable_baselines3.common.callbacks import CheckpointCallback, EvalCallback
    from stable_baselines3.common.monitor import Monitor
    import torch
except ImportError:
    print("Error: stable-baselines3 not installed. Run: pip install stable-baselines3[extra]")
    sys.exit(1)

# Add parent directory to path for imports
sys.path.insert(0, str(Path(__file__).parent.parent))

from julius_gym.wrappers import make_efficient_env


def main():
    parser = argparse.ArgumentParser(description="Train DQN agent on Julius")
    parser.add_argument(
        "--data-dir",
        required=True,
        help="Path to Caesar III data directory",
    )
    parser.add_argument(
        "--scenario",
        help="Path to scenario file (optional)",
    )
    parser.add_argument(
        "--max-ticks",
        type=int,
        default=3000,
        help="Maximum ticks per episode (default: 3000)",
    )
    parser.add_argument(
        "--timesteps",
        type=int,
        default=200000,
        help="Total training timesteps (default: 200000)",
    )
    parser.add_argument(
        "--model-path",
        default="models/julius_dqn",
        help="Path to save model (default: models/julius_dqn)",
    )
    parser.add_argument(
        "--load",
        help="Load existing model from path",
    )
    parser.add_argument(
        "--no-gpu",
        action="store_true",
        help="Force CPU training",
    )
    args = parser.parse_args()

    # Check GPU availability
    device = "cpu" if args.no_gpu or not torch.cuda.is_available() else "cuda"
    print(f"Using device: {device}")
    if device == "cuda":
        print(f"GPU: {torch.cuda.get_device_name(0)}")
        print(f"GPU Memory: {torch.cuda.get_device_properties(0).total_memory / 1e9:.2f} GB")

    # Create environment with efficient wrappers
    print(f"\nCreating Julius environment...")
    print(f"  Data directory: {args.data_dir}")
    print(f"  Max ticks: {args.max_ticks}")

    env = make_efficient_env(
        data_directory=args.data_dir,
        scenario_file=args.scenario,
        max_ticks=args.max_ticks,
        simplify=True,  # Use simplified observation space
        normalize=True,  # Normalize values
        flatten=True,    # Flatten for DQN
        shape_rewards=True,  # Add reward shaping
    )

    # Wrap with Monitor for logging
    env = Monitor(env)

    print(f"Environment created!")
    print(f"  Observation space: {env.observation_space}")
    print(f"  Action space: {env.action_space}")
    print(f"  Observation dim: {env.observation_space.shape[0]}")

    if args.load:
        # Load existing model
        print(f"\nLoading model from {args.load}...")
        model = DQN.load(args.load, env=env, device=device)
        print("Model loaded!")
    else:
        # Create new model with memory-efficient hyperparameters
        print("\nCreating DQN model with memory-efficient settings...")
        model = DQN(
            "MlpPolicy",
            env,
            learning_rate=1e-4,
            buffer_size=50000,  # Smaller replay buffer for 4GB GPU
            learning_starts=1000,
            batch_size=32,  # Small batch size
            tau=0.005,
            gamma=0.99,
            train_freq=4,
            gradient_steps=1,
            target_update_interval=1000,
            exploration_fraction=0.2,
            exploration_initial_eps=1.0,
            exploration_final_eps=0.05,
            policy_kwargs=dict(
                net_arch=[128, 128],  # Small network: 2 layers, 128 units each
            ),
            verbose=1,
            device=device,
            tensorboard_log=f"./tensorboard/{Path(args.model_path).name}/",
        )

        print(f"\nModel architecture:")
        print(f"  Policy network: [128, 128]")
        print(f"  Replay buffer: 50,000 transitions")
        print(f"  Batch size: 32")
        print(f"  Estimated GPU memory: ~1-2 GB")

        # Create callbacks
        Path(args.model_path).parent.mkdir(parents=True, exist_ok=True)

        checkpoint_callback = CheckpointCallback(
            save_freq=10000,
            save_path=str(Path(args.model_path).parent / "checkpoints"),
            name_prefix="dqn_checkpoint",
        )

        # Train model
        print(f"\nTraining for {args.timesteps:,} timesteps...")
        print("Press Ctrl+C to stop training and save model\n")

        try:
            model.learn(
                total_timesteps=args.timesteps,
                callback=checkpoint_callback,
                progress_bar=True,
            )
        except KeyboardInterrupt:
            print("\n\nTraining interrupted by user.")

        # Save model
        print(f"\nSaving model to {args.model_path}...")
        model.save(args.model_path)
        print("Model saved!")

    # Evaluate model
    print("\n" + "=" * 60)
    print("Evaluating trained model...")
    print("=" * 60)

    n_eval_episodes = 3
    for episode in range(n_eval_episodes):
        obs, info = env.reset()
        done = False
        episode_reward = 0
        step_count = 0

        print(f"\nEpisode {episode + 1}/{n_eval_episodes}")

        while not done and step_count < args.max_ticks:
            action, _states = model.predict(obs, deterministic=True)
            obs, reward, terminated, truncated, info = env.step(action)

            episode_reward += reward
            step_count += 1
            done = terminated or truncated

            if step_count % 500 == 0:
                print(f"  Step {step_count}: reward={reward:.3f}, total={episode_reward:.3f}")

        print(f"\n  Episode finished!")
        print(f"    Steps: {step_count}")
        print(f"    Total reward: {episode_reward:.3f}")
        print(f"    Terminated: {terminated}, Truncated: {truncated}")

    env.close()

    print("\n" + "=" * 60)
    print("Training complete!")
    print(f"Model saved to: {args.model_path}")
    print(f"TensorBoard logs: ./tensorboard/{Path(args.model_path).name}/")
    print(f"\nView training with: tensorboard --logdir=./tensorboard")
    print("=" * 60)


if __name__ == "__main__":
    main()
