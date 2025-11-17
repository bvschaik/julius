# Julius Gymnasium Environment

OpenAI Gymnasium environment for training reinforcement learning agents on Julius, an open-source re-implementation of Caesar III.

## Installation

### Prerequisites

1. **Build the Julius shared library:**

```bash
cd /path/to/julius
mkdir build && cd build
cmake .. -DBUILD_GYMNASIUM_LIB=ON -DHEADLESS_BUILD=ON
make julius_gym
```

This will create `libjulius_gym.so` in the build directory.

2. **Install Python package:**

```bash
cd python
pip install -e .
```

### Caesar III Data Files

You need the original Caesar III data files to run Julius. The environment expects the path to the directory containing `c3.eng` and other game data files.

## Usage

### Basic Example

```python
import julius_gym

# Create environment
env = julius_gym.JuliusEnv(
    data_directory="/path/to/caesar3/data",
    scenario_file="/path/to/scenario.map",  # Optional
    max_ticks=10000,  # Optional max steps per episode
    lib_path="build/libjulius_gym.so"  # Optional, auto-detected if in standard location
)

# Reset environment
observation, info = env.reset()

# Run episode
done = False
total_reward = 0

while not done:
    # Choose action (0-54)
    action = env.action_space.sample()  # Random action

    # Execute action
    observation, reward, terminated, truncated, info = env.step(action)

    total_reward += reward
    done = terminated or truncated

    print(f"Reward: {reward}, Info: {info}")

print(f"Episode finished with total reward: {total_reward}")
env.close()
```

### With Stable-Baselines3

```python
from stable_baselines3 import PPO
import julius_gym

# Create environment
env = julius_gym.JuliusEnv(
    data_directory="/path/to/caesar3/data",
    max_ticks=5000
)

# Train agent
model = PPO("MultiInputPolicy", env, verbose=1)
model.learn(total_timesteps=100000)

# Save model
model.save("julius_ppo")

# Load and test
model = PPO.load("julius_ppo")
obs, info = env.reset()
for _ in range(1000):
    action, _states = model.predict(obs, deterministic=True)
    obs, reward, terminated, truncated, info = env.step(action)
    if terminated or truncated:
        break
```

## Observation Space

The observation space is a dictionary containing:

- **ratings**: City ratings (culture, prosperity, peace, favor) [0-100]
- **finance**: Treasury, tax percentage, income/expenses
- **population**: Total population, sentiment, working age counts
- **labor**: Employment statistics, wages
- **resources**: Food stocks and availability
- **buildings**: Housing counts and capacity
- **migration**: Immigration/emigration statistics
- **culture**: Entertainment, education, health, religion metrics
- **time**: Current game time (tick, day, month, year)
- **victory**: Scenario victory conditions

## Action Space

Discrete action space with 55 actions:

### Administrative Actions (0-4)
- `NONE` (0): No action
- `ADJUST_TAX_UP` (1): Increase tax by 1%
- `ADJUST_TAX_DOWN` (2): Decrease tax by 1%
- `ADJUST_WAGES_UP` (3): Increase wages
- `ADJUST_WAGES_DOWN` (4): Decrease wages

### Building Actions (5-53)
- Various buildings: housing, roads, infrastructure, services, production, entertainment
- Buildings are placed at default coordinates (50, 50)

### Destruction (54)
- `CLEAR_LAND` (54): Clear land/demolish buildings

## Reward Structure

- **Rating improvements**: +0.1 per rating point increase
- **Maintaining positive treasury**: +0.01
- **Bankruptcy**: -1.0
- **Victory**: +100.0
- **Defeat**: -50.0
- **Failed actions**: -0.1

## Episode Termination

Episodes end when:
- Victory conditions are met (terminated=True)
- Defeat conditions are met (terminated=True)
- Max ticks reached (truncated=True)
- Severe bankruptcy (treasury < -5000)

## API Reference

### `JuliusEnv`

```python
JuliusEnv(
    data_directory: str,
    scenario_file: Optional[str] = None,
    max_ticks: int = 0,
    lib_path: Optional[str] = None
)
```

**Parameters:**
- `data_directory`: Path to Caesar III data files
- `scenario_file`: Optional scenario file to load
- `max_ticks`: Maximum ticks per episode (0 = unlimited)
- `lib_path`: Optional path to libjulius_gym.so

**Methods:**
- `reset()`: Reset environment to initial state
- `step(action)`: Execute action and advance one tick
- `close()`: Clean up resources

## Development

Run tests:
```bash
pytest tests/
```

Format code:
```bash
black julius_gym/
isort julius_gym/
```

## License

AGPL v3 - See LICENSE file for details.
