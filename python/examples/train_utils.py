"""
Training utilities for Julius RL agents

Includes callbacks, evaluation functions, and monitoring tools.
"""

import numpy as np
from stable_baselines3.common.callbacks import BaseCallback
from stable_baselines3.common.evaluation import evaluate_policy


class TensorboardCallback(BaseCallback):
    """
    Custom callback for logging additional metrics to TensorBoard.
    """

    def __init__(self, verbose=0):
        super().__init__(verbose)
        self.episode_rewards = []
        self.episode_lengths = []

    def _on_step(self) -> bool:
        # Log episode rewards and lengths
        for idx, done in enumerate(self.locals["dones"]):
            if done:
                if len(self.locals.get("episode_rewards", [])) > idx:
                    reward = self.locals["episode_rewards"][idx]
                    length = self.locals["episode_lengths"][idx]

                    self.episode_rewards.append(reward)
                    self.episode_lengths.append(length)

                    # Log to TensorBoard
                    self.logger.record("rollout/ep_rew_mean_recent", np.mean(self.episode_rewards[-100:]))
                    self.logger.record("rollout/ep_len_mean_recent", np.mean(self.episode_lengths[-100:]))

        return True


class ProgressCallback(BaseCallback):
    """
    Callback that prints training progress with city metrics.
    """

    def __init__(self, check_freq=1000, verbose=1):
        super().__init__(verbose)
        self.check_freq = check_freq
        self.best_mean_reward = -np.inf

    def _on_step(self) -> bool:
        if self.n_calls % self.check_freq == 0:
            # Evaluate current policy
            mean_reward = np.mean(self.locals.get("rewards", [0]))

            if self.verbose > 0:
                print(f"\nStep: {self.num_timesteps:,}")
                print(f"  Mean reward (last {self.check_freq} steps): {mean_reward:.3f}")

                # Track best performance
                if mean_reward > self.best_mean_reward:
                    self.best_mean_reward = mean_reward
                    print(f"  New best mean reward: {self.best_mean_reward:.3f} 🎉")

        return True


class EarlyStoppingCallback(BaseCallback):
    """
    Stop training if performance doesn't improve after N steps.
    """

    def __init__(self, patience_steps=50000, min_improvement=0.01, verbose=1):
        super().__init__(verbose)
        self.patience_steps = patience_steps
        self.min_improvement = min_improvement
        self.best_mean_reward = -np.inf
        self.steps_without_improvement = 0
        self.episode_rewards = []

    def _on_step(self) -> bool:
        # Collect episode rewards
        for idx, done in enumerate(self.locals["dones"]):
            if done and len(self.locals.get("episode_rewards", [])) > idx:
                self.episode_rewards.append(self.locals["episode_rewards"][idx])

        # Check every 1000 steps
        if len(self.episode_rewards) >= 10:  # Need at least 10 episodes
            mean_reward = np.mean(self.episode_rewards[-10:])

            if mean_reward > self.best_mean_reward + self.min_improvement:
                if self.verbose > 0:
                    print(f"\n[EarlyStopping] Improvement detected: {mean_reward:.3f} > {self.best_mean_reward:.3f}")
                self.best_mean_reward = mean_reward
                self.steps_without_improvement = 0
            else:
                self.steps_without_improvement += 1

            # Check if we should stop
            if self.steps_without_improvement >= self.patience_steps:
                if self.verbose > 0:
                    print(f"\n[EarlyStopping] No improvement for {self.patience_steps} steps. Stopping training.")
                return False

        return True


def evaluate_agent(model, env, n_eval_episodes=10, deterministic=True):
    """
    Evaluate an agent and return detailed statistics.

    Args:
        model: Trained model
        env: Environment to evaluate on
        n_eval_episodes: Number of episodes to run
        deterministic: Use deterministic actions

    Returns:
        Dictionary with evaluation statistics
    """
    episode_rewards = []
    episode_lengths = []
    final_ratings = []
    final_populations = []
    final_treasuries = []

    for episode in range(n_eval_episodes):
        obs, info = env.reset()
        done = False
        episode_reward = 0
        episode_length = 0

        while not done:
            action, _states = model.predict(obs, deterministic=deterministic)
            obs, reward, terminated, truncated, info = env.step(action)

            episode_reward += reward
            episode_length += 1
            done = terminated or truncated

        episode_rewards.append(episode_reward)
        episode_lengths.append(episode_length)

        # Try to extract final city metrics (if using dict observation)
        if isinstance(obs, dict):
            if "ratings" in obs:
                avg_rating = np.mean(list(obs["ratings"].values()))
                final_ratings.append(avg_rating)
            if "population" in obs and "total" in obs["population"]:
                final_populations.append(obs["population"]["total"])
            if "finance" in obs and "treasury" in obs["finance"]:
                final_treasuries.append(obs["finance"]["treasury"])

    stats = {
        "mean_reward": np.mean(episode_rewards),
        "std_reward": np.std(episode_rewards),
        "min_reward": np.min(episode_rewards),
        "max_reward": np.max(episode_rewards),
        "mean_length": np.mean(episode_lengths),
        "std_length": np.std(episode_lengths),
    }

    if final_ratings:
        stats["mean_final_rating"] = np.mean(final_ratings)
        stats["std_final_rating"] = np.std(final_ratings)

    if final_populations:
        stats["mean_final_population"] = np.mean(final_populations)
        stats["std_final_population"] = np.std(final_populations)

    if final_treasuries:
        stats["mean_final_treasury"] = np.mean(final_treasuries)
        stats["std_final_treasury"] = np.std(final_treasuries)

    return stats


def print_evaluation_stats(stats):
    """Pretty print evaluation statistics"""
    print("\n" + "=" * 60)
    print("EVALUATION RESULTS")
    print("=" * 60)

    print(f"\nReward Statistics:")
    print(f"  Mean: {stats['mean_reward']:.3f} ± {stats['std_reward']:.3f}")
    print(f"  Min:  {stats['min_reward']:.3f}")
    print(f"  Max:  {stats['max_reward']:.3f}")

    print(f"\nEpisode Length:")
    print(f"  Mean: {stats['mean_length']:.1f} ± {stats['std_length']:.1f} steps")

    if "mean_final_rating" in stats:
        print(f"\nFinal City Ratings:")
        print(f"  Mean: {stats['mean_final_rating']:.1f} ± {stats['std_final_rating']:.1f}")

    if "mean_final_population" in stats:
        print(f"\nFinal Population:")
        print(f"  Mean: {stats['mean_final_population']:.0f} ± {stats['std_final_population']:.0f}")

    if "mean_final_treasury" in stats:
        print(f"\nFinal Treasury:")
        print(f"  Mean: {stats['mean_final_treasury']:.0f} ± {stats['std_final_treasury']:.0f}")

    print("=" * 60)


def compare_agents(models_dict, env, n_eval_episodes=10):
    """
    Compare multiple trained agents.

    Args:
        models_dict: Dictionary mapping agent names to models
        env: Environment to evaluate on
        n_eval_episodes: Number of episodes per agent

    Returns:
        Dictionary mapping agent names to their statistics
    """
    all_stats = {}

    print("\n" + "=" * 60)
    print("COMPARING AGENTS")
    print("=" * 60)

    for name, model in models_dict.items():
        print(f"\nEvaluating {name}...")
        stats = evaluate_agent(model, env, n_eval_episodes)
        all_stats[name] = stats

        print(f"  Mean reward: {stats['mean_reward']:.3f}")
        print(f"  Mean length: {stats['mean_length']:.1f}")

    # Print comparison table
    print("\n" + "=" * 60)
    print("COMPARISON TABLE")
    print("=" * 60)
    print(f"{'Agent':<20} {'Mean Reward':<15} {'Mean Length':<15}")
    print("-" * 60)

    for name, stats in all_stats.items():
        print(f"{name:<20} {stats['mean_reward']:<15.3f} {stats['mean_length']:<15.1f}")

    # Find best agent
    best_agent = max(all_stats.items(), key=lambda x: x[1]["mean_reward"])
    print(f"\nBest Agent: {best_agent[0]} with mean reward {best_agent[1]['mean_reward']:.3f}")
    print("=" * 60)

    return all_stats
