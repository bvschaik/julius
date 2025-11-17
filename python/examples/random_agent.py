#!/usr/bin/env python3
"""
Random Agent Example

Demonstrates basic usage of JuliusEnv with a random agent.
"""

import argparse
import sys

try:
    import julius_gym
except ImportError:
    print("Error: julius_gym not installed. Run: pip install -e .")
    sys.exit(1)


def main():
    parser = argparse.ArgumentParser(description="Run Julius with random agent")
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
        default=1000,
        help="Maximum ticks per episode (default: 1000)",
    )
    parser.add_argument(
        "--lib-path",
        help="Path to libjulius_gym.so (auto-detected if not specified)",
    )
    args = parser.parse_args()

    # Create environment
    print(f"Creating Julius environment...")
    print(f"  Data directory: {args.data_dir}")
    if args.scenario:
        print(f"  Scenario: {args.scenario}")
    print(f"  Max ticks: {args.max_ticks}")

    try:
        env = julius_gym.JuliusEnv(
            data_directory=args.data_dir,
            scenario_file=args.scenario,
            max_ticks=args.max_ticks,
            lib_path=args.lib_path,
        )
    except Exception as e:
        print(f"Error creating environment: {e}")
        sys.exit(1)

    print(f"Environment created successfully!")
    print(f"  Observation space: {env.observation_space}")
    print(f"  Action space: {env.action_space}")
    print()

    # Reset environment
    print("Resetting environment...")
    observation, info = env.reset()
    print(f"Initial observation:")
    print(f"  Culture rating: {observation['ratings']['culture']}")
    print(f"  Prosperity rating: {observation['ratings']['prosperity']}")
    print(f"  Treasury: {observation['finance']['treasury']}")
    print(f"  Population: {observation['population']['total']}")
    print()

    # Run random agent
    print(f"Running random agent for up to {args.max_ticks} ticks...")
    done = False
    step_count = 0
    total_reward = 0

    while not done:
        # Sample random action
        action = env.action_space.sample()

        # Execute action
        observation, reward, terminated, truncated, info = env.step(action)

        step_count += 1
        total_reward += reward

        # Print progress every 100 steps
        if step_count % 100 == 0:
            print(f"Step {step_count}:")
            print(f"  Reward: {reward:.3f}")
            print(f"  Total reward: {total_reward:.3f}")
            print(f"  Treasury: {observation['finance']['treasury']}")
            print(f"  Population: {observation['population']['total']}")
            print(f"  Year: {observation['time']['year']}, Month: {observation['time']['month']}")
            print(f"  Info: {info['message']}")
            print()

        done = terminated or truncated

    # Print final results
    print("=" * 60)
    print("Episode finished!")
    print(f"  Total steps: {step_count}")
    print(f"  Total reward: {total_reward:.3f}")
    print(f"  Terminated: {terminated}")
    print(f"  Truncated: {truncated}")
    print(f"  Final info: {info['message']}")
    print()
    print(f"Final state:")
    print(f"  Culture: {observation['ratings']['culture']}")
    print(f"  Prosperity: {observation['ratings']['prosperity']}")
    print(f"  Peace: {observation['ratings']['peace']}")
    print(f"  Favor: {observation['ratings']['favor']}")
    print(f"  Treasury: {observation['finance']['treasury']}")
    print(f"  Population: {observation['population']['total']}")

    env.close()
    print("\nEnvironment closed.")


if __name__ == "__main__":
    main()
