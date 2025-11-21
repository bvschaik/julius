"""
Observation wrappers for Julius Gym environment

These wrappers help reduce observation space and normalize values for better RL training.
"""

import gymnasium as gym
import numpy as np
from gymnasium import spaces


class FlattenObservation(gym.ObservationWrapper):
    """
    Flatten the nested dictionary observation into a single flat array.
    This is useful for algorithms that require Box observation spaces.
    """

    def __init__(self, env):
        super().__init__(env)

        # Extract all numeric values from nested dict
        self._keys = []
        self._extract_keys(self.env.observation_space.spaces)

        # Create flat box space
        self.observation_space = spaces.Box(
            low=-10000,
            high=100000,
            shape=(len(self._keys),),
            dtype=np.float32
        )

    def _extract_keys(self, space_dict, prefix=""):
        """Recursively extract all keys from nested dict space"""
        for key, value in space_dict.items():
            full_key = f"{prefix}.{key}" if prefix else key
            if isinstance(value, spaces.Dict):
                self._extract_keys(value.spaces, full_key)
            else:
                self._keys.append(full_key)

    def observation(self, obs):
        """Flatten observation dict to array"""
        values = []
        for key in self._keys:
            # Navigate nested dict
            parts = key.split(".")
            val = obs
            for part in parts:
                val = val[part]
            values.append(float(val))
        return np.array(values, dtype=np.float32)


class NormalizeObservation(gym.ObservationWrapper):
    """
    Normalize observation values to roughly [-1, 1] range.
    This helps with neural network training stability.
    """

    def __init__(self, env):
        super().__init__(env)

        # Define normalization ranges for each metric
        self.ranges = {
            "ratings": {"culture": 100, "prosperity": 100, "peace": 100, "favor": 100},
            "finance": {
                "treasury": 10000,
                "tax_percentage": 25,
                "estimated_tax_income": 1000,
                "last_year_income": 5000,
                "last_year_expenses": 5000,
                "last_year_net": 5000,
            },
            "population": {
                "total": 5000,
                "school_age": 1000,
                "academy_age": 1000,
                "working_age": 3000,
                "sentiment": 100,
            },
            "labor": {
                "workers_needed": 3000,
                "workers_employed": 3000,
                "unemployment_percentage": 100,
                "wages": 50,
            },
            "resources": {"food_stocks": 1000, "food_types_available": 4},
            "buildings": {"housing": 500, "housing_capacity": 5000},
            "migration": {
                "immigration_amount": 100,
                "emigration_amount": 100,
                "immigration_queue_size": 100,
                "emigration_queue_size": 100,
            },
            "culture": {
                "entertainment": 100,
                "average_entertainment": 100,
                "average_education": 100,
                "average_health": 100,
                "religion_coverage": 100,
            },
            "time": {
                "tick": 1000,
                "day": 16,
                "month": 12,
                "year": 100,
                "total_months": 1200,
            },
            "victory": {
                "culture_goal": 100,
                "prosperity_goal": 100,
                "peace_goal": 100,
                "favor_goal": 100,
                "population_goal": 10000,
                "time_limit_years": 100,
            },
        }

    def observation(self, obs):
        """Normalize each value by its expected range"""
        normalized = {}
        for category, metrics in obs.items():
            normalized[category] = {}
            for metric, value in metrics.items():
                if category in self.ranges and metric in self.ranges[category]:
                    scale = self.ranges[category][metric]
                    normalized[category][metric] = value / scale
                else:
                    normalized[category][metric] = value
        return normalized


class SimplifyObservation(gym.ObservationWrapper):
    """
    Reduce observation space to only the most important metrics.
    This significantly reduces the state space for faster learning.
    """

    def __init__(self, env):
        super().__init__(env)

        # Define simplified observation space with only key metrics
        self.observation_space = spaces.Dict({
            "ratings": spaces.Dict({
                "culture": spaces.Box(0, 100, shape=(), dtype=np.int32),
                "prosperity": spaces.Box(0, 100, shape=(), dtype=np.int32),
                "peace": spaces.Box(0, 100, shape=(), dtype=np.int32),
                "favor": spaces.Box(0, 100, shape=(), dtype=np.int32),
            }),
            "finance": spaces.Dict({
                "treasury": spaces.Box(-10000, 1000000, shape=(), dtype=np.int32),
                "tax_percentage": spaces.Box(0, 25, shape=(), dtype=np.int32),
            }),
            "population": spaces.Dict({
                "total": spaces.Box(0, 100000, shape=(), dtype=np.int32),
                "sentiment": spaces.Box(0, 100, shape=(), dtype=np.int32),
            }),
            "labor": spaces.Dict({
                "unemployment_percentage": spaces.Box(0, 100, shape=(), dtype=np.int32),
            }),
            "resources": spaces.Dict({
                "food_stocks": spaces.Box(0, 10000, shape=(), dtype=np.int32),
            }),
        })

    def observation(self, obs):
        """Extract only key metrics"""
        # Handle both flat and nested observation spaces
        if "ratings" in obs:
            # Nested dict format
            return {
                "ratings": obs["ratings"],
                "finance": {
                    "treasury": obs["finance"]["treasury"],
                    "tax_percentage": obs["finance"]["tax_percentage"],
                },
                "population": {
                    "total": obs["population"]["total"],
                    "sentiment": obs["population"]["sentiment"],
                },
                "labor": {
                    "unemployment_percentage": obs["labor"]["unemployment_percentage"],
                },
                "resources": {
                    "food_stocks": obs["resources"]["food_stocks"],
                },
            }
        else:
            # Flat format (from JuliusEnv)
            return {
                "ratings": {
                    "culture": obs["ratings_culture"],
                    "prosperity": obs["ratings_prosperity"],
                    "peace": obs["ratings_peace"],
                    "favor": obs["ratings_favor"],
                },
                "finance": {
                    "treasury": obs["finance_treasury"],
                    "tax_percentage": obs["finance_tax_percentage"],
                },
                "population": {
                    "total": obs["population_total"],
                    "sentiment": obs["population_sentiment"],
                },
                "labor": {
                    "unemployment_percentage": obs["labor_unemployment_percentage"],
                },
                "resources": {
                    "food_stocks": obs["resources_food_stocks"],
                },
            }


class RewardShaping(gym.Wrapper):
    """
    Shape rewards to encourage specific behaviors.
    This can help guide learning towards better policies.
    """

    def __init__(self, env, population_weight=1.0, rating_weight=1.0, treasury_weight=0.5):
        super().__init__(env)
        self.population_weight = population_weight
        self.rating_weight = rating_weight
        self.treasury_weight = treasury_weight

        self.prev_population = 0
        self.prev_avg_rating = 0
        self.prev_treasury = 0

    def reset(self, **kwargs):
        obs, info = self.env.reset(**kwargs)

        # Initialize previous values
        self.prev_population = obs["population"]["total"]
        self.prev_avg_rating = sum(obs["ratings"].values()) / 4
        self.prev_treasury = obs["finance"]["treasury"]

        return obs, info

    def step(self, action):
        obs, reward, terminated, truncated, info = self.env.step(action)

        # Add shaped rewards
        shaped_reward = reward

        # Reward population growth
        pop_delta = obs["population"]["total"] - self.prev_population
        shaped_reward += pop_delta * self.population_weight * 0.01

        # Reward improving average rating
        avg_rating = sum(obs["ratings"].values()) / 4
        rating_delta = avg_rating - self.prev_avg_rating
        shaped_reward += rating_delta * self.rating_weight * 0.1

        # Small reward for maintaining treasury
        if obs["finance"]["treasury"] > 1000:
            shaped_reward += self.treasury_weight * 0.01

        # Update previous values
        self.prev_population = obs["population"]["total"]
        self.prev_avg_rating = avg_rating
        self.prev_treasury = obs["finance"]["treasury"]

        return obs, shaped_reward, terminated, truncated, info


def make_efficient_env(data_directory, scenario_file=None, max_ticks=5000,
                       lib_path=None, simplify=True, normalize=True, flatten=True, shape_rewards=True):
    """
    Create an environment with wrappers optimized for efficient RL training.

    Args:
        data_directory: Path to Caesar III data
        scenario_file: Optional scenario file
        max_ticks: Max ticks per episode
        lib_path: Optional path to libjulius_gym library
        simplify: Use simplified observation space
        normalize: Normalize observations
        flatten: Flatten to Box space (required for some algorithms)
        shape_rewards: Apply reward shaping

    Returns:
        Wrapped environment ready for training
    """
    import julius_gym

    env = julius_gym.JuliusEnv(
        data_directory=data_directory,
        scenario_file=scenario_file,
        max_ticks=max_ticks,
        lib_path=lib_path,
    )

    if simplify:
        env = SimplifyObservation(env)

    if shape_rewards:
        env = RewardShaping(env)

    if normalize:
        env = NormalizeObservation(env)

    if flatten:
        env = FlattenObservation(env)

    return env
