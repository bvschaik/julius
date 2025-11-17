#!/usr/bin/env python3
"""
Optimized PPO Agent Training Example

PPO with memory-efficient settings for 4GB GPU.
Uses smaller batches and network but still effective.

Memory usage: ~1.5-2.5GB GPU RAM during training
"""

import argparse
import sys
from pathlib import Path

try:
    from stable_baselines3 import PPO
    from stable_baselines3.common.callbacks import CheckpointCallback
    from stable_baselines3.common.monitor import Monitor
    from stable_baselines3.common.vec_env import DummyVecEnv
    import torch
except ImportError:
    print("Error: stable-baselines3 not installed. Run: pip install stable-baselines3[extra]")
    sys.exit(1)

# Add parent directory to path for imports
sys.path.insert(0, str(Path(__file__).parent.parent))

from julius_gym.wrappers import make_efficient_env


def make_env(data_dir, scenario, max_ticks):
    """Create a single environment (for vectorization)"""
    def _init():
        env = make_efficient_env(
            data_directory=data_dir,
            scenario_file=scenario,
            max_ticks=max_ticks,
            simplify=True,
            normalize=True,
            flatten=True,
            shape_rewards=True,
        )
        return Monitor(env)
    return _init


def main():
    parser = argparse.ArgumentParser(description="Train memory-optimized PPO agent on Julius")
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
        default=250000,
        help="Total training timesteps (default: 250000)",
    )
    parser.add_argument(
        "--n-envs",
        type=int,
        default=2,
        help="Number of parallel environments (default: 2)",
    )
    parser.add_argument(
        "--model-path",
        default="models/julius_ppo",
        help="Path to save model (default: models/julius_ppo)",
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

    # Create vectorized environments
    print(f"\nCreating {args.n_envs} parallel Julius environments...")
    print(f"  Data directory: {args.data_dir}")
    print(f"  Max ticks per episode: {args.max_ticks}")

    env = DummyVecEnv([
        make_env(args.data_dir, args.scenario, args.max_ticks)
        for _ in range(args.n_envs)
    ])

    print(f"Environments created!")
    print(f"  Number of parallel envs: {args.n_envs}")
    print(f"  Observation space: {env.observation_space}")
    print(f"  Action space: {env.action_space}")

    if args.load:
        # Load existing model
        print(f"\nLoading model from {args.load}...")
        model = PPO.load(args.load, env=env, device=device)
        print("Model loaded!")
    else:
        # Create new model with memory-efficient hyperparameters
        print("\nCreating PPO model with memory-efficient settings...")
        model = PPO(
            "MlpPolicy",
            env,
            learning_rate=3e-4,
            n_steps=1024,  # Smaller than default 2048
            batch_size=64,  # Small batch for memory efficiency
            n_epochs=4,  # Fewer epochs than default 10
            gamma=0.99,
            gae_lambda=0.95,
            clip_range=0.2,
            clip_range_vf=None,
            normalize_advantage=True,
            ent_coef=0.01,
            vf_coef=0.5,
            max_grad_norm=0.5,
            policy_kwargs=dict(
                net_arch=[dict(pi=[128, 128], vf=[128, 128])],  # Small 2-layer network
                activation_fn=torch.nn.Tanh,  # Tanh is more stable than ReLU
            ),
            verbose=1,
            device=device,
            tensorboard_log=f"./tensorboard/{Path(args.model_path).name}/",
        )

        print(f"\nModel architecture:")
        print(f"  Policy network: [128, 128] with Tanh activation")
        print(f"  Value network: [128, 128]")
        print(f"  N-steps: 1024")
        print(f"  Batch size: 64")
        print(f"  N-epochs: 4")
        print(f"  Parallel environments: {args.n_envs}")
        print(f"  Estimated GPU memory: ~1.5-2.5GB")
        print(f"\nThis is optimized for 4GB GPU but still effective!")

        # Create callbacks
        Path(args.model_path).parent.mkdir(parents=True, exist_ok=True)

        checkpoint_callback = CheckpointCallback(
            save_freq=10000 // args.n_envs,  # Adjust for parallel envs
            save_path=str(Path(args.model_path).parent / "checkpoints"),
            name_prefix="ppo_checkpoint",
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

    # Create single env for evaluation
    eval_env = make_efficient_env(
        data_directory=args.data_dir,
        scenario_file=args.scenario,
        max_ticks=args.max_ticks,
        simplify=True,
        normalize=True,
        flatten=True,
        shape_rewards=True,
    )
    eval_env = Monitor(eval_env)

    n_eval_episodes = 3
    for episode in range(n_eval_episodes):
        obs, info = eval_env.reset()
        done = False
        episode_reward = 0
        step_count = 0

        print(f"\nEpisode {episode + 1}/{n_eval_episodes}")

        while not done and step_count < args.max_ticks:
            action, _states = model.predict(obs, deterministic=True)
            obs, reward, terminated, truncated, info = eval_env.step(action)

            episode_reward += reward
            step_count += 1
            done = terminated or truncated

            if step_count % 500 == 0:
                print(f"  Step {step_count}: reward={reward:.3f}, total={episode_reward:.3f}")

        print(f"\n  Episode finished!")
        print(f"    Steps: {step_count}")
        print(f"    Total reward: {episode_reward:.3f}")
        print(f"    Terminated: {terminated}, Truncated: {truncated}")

    eval_env.close()
    env.close()

    print("\n" + "=" * 60)
    print("Training complete!")
    print(f"Model saved to: {args.model_path}")
    print(f"TensorBoard logs: ./tensorboard/{Path(args.model_path).name}/")
    print(f"\nView training with: tensorboard --logdir=./tensorboard")
    print("=" * 60)


if __name__ == "__main__":
    main()
