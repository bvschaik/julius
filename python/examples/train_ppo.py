#!/usr/bin/env python3
"""
PPO Training Example

Demonstrates training a PPO agent on Julius using stable-baselines3.
"""

import argparse
import sys

try:
    import julius_gym
except ImportError:
    print("Error: julius_gym not installed. Run: pip install -e .")
    sys.exit(1)

try:
    from stable_baselines3 import PPO
    from stable_baselines3.common.callbacks import EvalCallback
    from stable_baselines3.common.monitor import Monitor
except ImportError:
    print("Error: stable-baselines3 not installed. Run: pip install stable-baselines3")
    sys.exit(1)


def main():
    parser = argparse.ArgumentParser(description="Train PPO agent on Julius")
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
        default=5000,
        help="Maximum ticks per episode (default: 5000)",
    )
    parser.add_argument(
        "--timesteps",
        type=int,
        default=100000,
        help="Total training timesteps (default: 100000)",
    )
    parser.add_argument(
        "--model-path",
        default="julius_ppo_model",
        help="Path to save/load model (default: julius_ppo_model)",
    )
    parser.add_argument(
        "--load",
        action="store_true",
        help="Load existing model instead of training",
    )
    args = parser.parse_args()

    # Create environment
    print(f"Creating Julius environment...")
    print(f"  Data directory: {args.data_dir}")
    if args.scenario:
        print(f"  Scenario: {args.scenario}")
    print(f"  Max ticks: {args.max_ticks}")

    env = julius_gym.JuliusEnv(
        data_directory=args.data_dir,
        scenario_file=args.scenario,
        max_ticks=args.max_ticks,
    )

    # Wrap with Monitor for logging
    env = Monitor(env)

    if args.load:
        # Load existing model
        print(f"\nLoading model from {args.model_path}...")
        model = PPO.load(args.model_path, env=env)
        print("Model loaded successfully!")
    else:
        # Create new model
        print("\nCreating PPO model...")
        model = PPO(
            "MultiInputPolicy",
            env,
            verbose=1,
            learning_rate=3e-4,
            n_steps=2048,
            batch_size=64,
            n_epochs=10,
            gamma=0.99,
            gae_lambda=0.95,
            clip_range=0.2,
            ent_coef=0.01,
        )

        # Train model
        print(f"\nTraining for {args.timesteps} timesteps...")
        model.learn(
            total_timesteps=args.timesteps,
            progress_bar=True,
        )

        # Save model
        print(f"\nSaving model to {args.model_path}...")
        model.save(args.model_path)
        print("Model saved!")

    # Evaluate model
    print("\nEvaluating model...")
    obs, info = env.reset()
    done = False
    episode_reward = 0
    step_count = 0

    while not done and step_count < args.max_ticks:
        action, _states = model.predict(obs, deterministic=True)
        obs, reward, terminated, truncated, info = env.step(action)

        episode_reward += reward
        step_count += 1
        done = terminated or truncated

        if step_count % 100 == 0:
            print(f"Step {step_count}: reward={reward:.3f}, total={episode_reward:.3f}")

    print("\n" + "=" * 60)
    print("Evaluation complete!")
    print(f"  Total steps: {step_count}")
    print(f"  Total reward: {episode_reward:.3f}")
    print(f"  Final ratings:")
    print(f"    Culture: {obs['ratings']['culture']}")
    print(f"    Prosperity: {obs['ratings']['prosperity']}")
    print(f"    Peace: {obs['ratings']['peace']}")
    print(f"    Favor: {obs['ratings']['favor']}")
    print(f"  Final treasury: {obs['finance']['treasury']}")
    print(f"  Final population: {obs['population']['total']}")

    env.close()


if __name__ == "__main__":
    main()
