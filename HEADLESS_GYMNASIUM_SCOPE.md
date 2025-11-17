# Headless Julius for Gymnasium - Scope Document

## Executive Summary

This document outlines the work required to create a headless version of Julius (an open-source re-implementation of Caesar III) that can be used with OpenAI Gymnasium for reinforcement learning research. The headless version would expose game state and accept actions without requiring rendering or user input.

## Project Context

**Julius** is a faithful re-implementation of Caesar III, a city-building game. The current implementation:
- Uses SDL2 for rendering and input
- Maintains 100% save game compatibility with Caesar III
- Has clean separation between game logic and presentation
- Updates at 20 ticks/second with distributed processing

## Current Architecture Analysis

### Core Components

1. **Main Loop** (`src/platform/julius.c`)
   - SDL2 event handling
   - Calls `game_run()` and `game_draw()` each frame
   - Runs at ~30fps

2. **Game Logic** (`src/game/game.c`)
   - `game_run()`: Advances game state based on elapsed ticks
   - `game_draw()`: Renders current window
   - **Clean separation**: Logic and rendering are already separate functions

3. **Game State** (`src/city/data_private.h`)
   - Single global `city_data` struct containing all state
   - Includes: buildings, finance, ratings, population, labor, resources, etc.
   - Well-organized with clear subsystems

4. **Game Tick System** (`src/game/tick.c`)
   - 50ms per tick (20 ticks/second)
   - Distributed processing: different systems update on different ticks
   - Hierarchical time: TICK → DAY (50 ticks) → MONTH (2400 ticks) → YEAR

5. **Module Pattern**
   - Each subsystem (finance, ratings, buildings, etc.) has:
     - Public API in `.h` files
     - Private data in `data_private.h`
     - Implementation in `.c` files

### Key State Components

The `city_data` struct already contains all necessary state:

- **Finance**: Treasury, tax rate, income/expenses
- **Ratings**: Culture, prosperity, peace, favor (0-100 each)
- **Population**: Total, by age, by house level, immigration/emigration
- **Resources**: Food stocks, warehouse inventory, trade goods
- **Buildings**: 2000-building array with all properties
- **Labor**: Workers available/employed/needed by category
- **Military**: Soldiers, legions (can be ignored initially)

### Key Action APIs

Existing APIs can be used or wrapped for actions:

1. **Building Construction**
   - `building_construction_place_building(type, x, y)`
   - Returns success/failure

2. **Building Demolition**
   - Building clearing functions in `building/construction_clear.h`

3. **Tax Adjustment**
   - `city_finance_change_tax_percentage(int change)` in `src/city/finance.h`
   - Current: `city_finance_tax_percentage()`

4. **Wages**
   - `city_labor_*` functions in `src/city/labor.h`

5. **Trade Settings**
   - `city_resource_*` functions for stockpiling/mothballing

## Gymnasium Integration Requirements

### Standard Gymnasium API

A gymnasium environment must implement:

```python
class JuliusEnv(gym.Env):
    def reset(self, seed=None, options=None) -> tuple[observation, info]
    def step(self, action) -> tuple[observation, reward, terminated, truncated, info]
    def render(self) -> None  # Optional
    def close(self) -> None
```

### Observation Space

The observation should include:

**Ratings (4 values, 0-100 each)**
- Culture, Prosperity, Peace, Favor

**Finance (5-10 values)**
- Treasury (denarii)
- Tax rate (0-25%)
- Monthly income/expenses
- Estimated tax income

**Population (5-10 values)**
- Total population
- Working age population
- Workers employed/unemployed
- Immigration/emigration rate
- Sentiment (0-100)

**Resources (per resource type, ~20 resources)**
- Food stocks (months of supply)
- Food types available
- Warehouse stocks for key goods

**Buildings (aggregated)**
- Count by type (houses, farms, industries, services)
- Housing capacity
- Service coverage percentages

**Map State (optional for now)**
- Grid representation could be added later
- For initial version, aggregated stats sufficient

**Total: ~100-200 numerical values**

### Action Space

Two approaches:

**Option 1: Discrete Action Space**
- Predefined actions (e.g., 100 actions)
- Examples: "Build house at optimal location", "Increase tax by 1%", "Build fountain", etc.
- Easier for simple RL algorithms

**Option 2: Multi-Discrete/Continuous Action Space**
- Action type (build, demolish, adjust tax, etc.)
- Parameters (building type, location, amount)
- More flexible but harder to learn

**Recommended Initial Approach: Discrete with limited action set**
- Adjust tax rate (+/- 1%)
- Build specific building types at "recommended" locations
- Demolish specific building types
- Adjust trade settings for key resources

### Reward Function

Multiple options depending on research goals:

1. **Rating-based**: Weighted sum of culture, prosperity, peace, favor
2. **Population growth**: Reward for increasing population
3. **Economic**: Treasury balance and growth
4. **Victory conditions**: Achieving scenario goals
5. **Composite**: Combination of above

## Technical Approach

### Phase 1: Headless Game Engine (2-3 weeks)

**Goal**: Run Julius without SDL2/rendering

**Tasks**:

1. **Create headless build configuration**
   - Modify `CMakeLists.txt` to add `HEADLESS_BUILD` option
   - Conditional compilation to exclude SDL2, graphics, input
   - Estimated: 1-2 days

2. **Implement headless main loop**
   - New `src/platform/headless.c`
   - Replace SDL event loop with tick-based loop
   - Call `game_run()` without `game_draw()`
   - Estimated: 2-3 days

3. **Mock/stub rendering systems**
   - Provide no-op implementations for:
     - `platform_screen_*` functions
     - `graphics_*` functions
     - `window_draw()`
   - Estimated: 2-3 days

4. **Implement save/load for state management**
   - Julius already has save/load (`src/game/file.c`)
   - Can use this for reset() function
   - Estimated: 1 day

5. **Test headless game runs**
   - Load a save game
   - Run for N ticks
   - Verify state updates without crashes
   - Estimated: 2-3 days

**Challenges**:
- Some game logic may be coupled to window states
- Input validation might assume UI exists
- Need to handle "window stack" without actual windows

### Phase 2: State Observation API (1-2 weeks)

**Goal**: Export game state in structured format

**Tasks**:

1. **Define observation structure**
   - Create `src/gymnasium/observation.h`
   - Define C struct matching gymnasium observation space
   - Estimated: 1 day

2. **Implement state extraction**
   - Create `src/gymnasium/observation.c`
   - Function: `gymnasium_get_observation(observation_t *obs)`
   - Read from existing `city_data` and building arrays
   - Aggregate building counts, resources, etc.
   - Estimated: 3-4 days

3. **Add map state (optional)**
   - Export terrain, building positions
   - Could be 2D arrays or sparse representations
   - Estimated: 2-3 days (if needed)

4. **Test observation accuracy**
   - Compare observations with known game states
   - Verify all values are correctly extracted
   - Estimated: 2 days

**Challenges**:
- Deciding level of detail (raw vs. aggregated)
- Handling variable-size data (building list)
- Efficient representation for ML consumption

### Phase 3: Action API (2-3 weeks)

**Goal**: Allow actions to be taken programmatically

**Tasks**:

1. **Define action interface**
   - Create `src/gymnasium/action.h`
   - Enum for action types
   - Struct for action parameters
   - Estimated: 1 day

2. **Implement action handlers**
   - Create `src/gymnasium/action.c`
   - Function: `gymnasium_take_action(action_t *action)`
   - Wrapper around existing APIs:
     - Building construction
     - Building demolition
     - Tax adjustment
     - Trade settings
   - Estimated: 5-7 days

3. **Action validation**
   - Check if action is legal (enough money, valid position, etc.)
   - Return success/failure and reason
   - Estimated: 2-3 days

4. **Smart action helpers (optional)**
   - "Build house at optimal location"
   - AI to find good building spots
   - Would simplify action space
   - Estimated: 3-5 days (if desired)

**Challenges**:
- Ensuring actions don't break game state
- Handling asynchronous effects (construction takes time)
- Deciding granularity (low-level vs. high-level actions)

### Phase 4: C/Python Bridge (1-2 weeks)

**Goal**: Create Python bindings for gymnasium

**Approaches**:

**Option A: ctypes (simpler, pure Python)**
- Define C API with simple types
- Use Python ctypes to call functions
- Estimated: 3-5 days

**Option B: Cython/CFFI (faster, more complex)**
- Write Cython wrapper
- Better performance for frequent calls
- Estimated: 5-7 days

**Option C: pybind11 (C++ style, modern)**
- Requires C++ wrapper around C code
- More ergonomic Python API
- Estimated: 7-10 days

**Recommended: Start with ctypes, optimize later if needed**

**Tasks**:

1. **Create C library API**
   - `src/gymnasium/gymnasium.h` - main API header
   - Functions:
     ```c
     void* julius_init(const char* data_dir);
     void julius_reset(void* handle, const char* save_file);
     void julius_step(void* handle, int num_ticks);
     void julius_get_obs(void* handle, observation_t* obs);
     int julius_take_action(void* handle, action_t* action);
     void julius_destroy(void* handle);
     ```
   - Estimated: 2-3 days

2. **Build shared library**
   - Modify CMake to build `libjulius_gym.so`
   - Ensure all symbols are exported
   - Estimated: 1-2 days

3. **Create Python wrapper**
   - `python/julius_gym/__init__.py`
   - `python/julius_gym/env.py` - gymnasium.Env implementation
   - Use ctypes to call C functions
   - Estimated: 3-4 days

4. **Define observation/action spaces**
   - Use gymnasium.spaces (Box, Discrete, Dict, etc.)
   - Match C structures
   - Estimated: 1-2 days

**Challenges**:
- Memory management across C/Python boundary
- NumPy array passing for observations
- Error handling and exception translation

### Phase 5: Testing and Integration (1-2 weeks)

**Tasks**:

1. **Unit tests for C components**
   - Test headless engine
   - Test observation extraction
   - Test action handling
   - Estimated: 3-4 days

2. **Integration tests**
   - Full Python environment tests
   - Random action sequences
   - Verify no crashes
   - Estimated: 2-3 days

3. **Example training scripts**
   - PPO or DQN with stable-baselines3
   - Demonstrate basic learning
   - Estimated: 2-3 days

4. **Documentation**
   - API documentation
   - Usage examples
   - Installation guide
   - Estimated: 2-3 days

## Implementation Roadmap

### Minimal Viable Product (6-8 weeks)

1. ✓ Phase 1: Headless engine (3 weeks)
2. ✓ Phase 2: Basic observation API (1.5 weeks)
3. ✓ Phase 3: Basic action API (2 weeks)
4. ✓ Phase 4: Python bindings (1.5 weeks)

**Deliverable**: Working gymnasium environment with basic state/actions

### Extended Features (4-6 weeks additional)

1. Map-based observations (grid representation)
2. Advanced action space (spatial actions)
3. Multiple scenarios support
4. Performance optimization
5. Comprehensive documentation

## Example API Design

### C API (src/gymnasium/gymnasium.h)

```c
// Observation structure
typedef struct {
    // Ratings (0-100)
    int culture;
    int prosperity;
    int peace;
    int favor;

    // Finance
    int treasury;
    int tax_percentage;
    int monthly_income;
    int monthly_expenses;

    // Population
    int population;
    int workers_employed;
    int workers_needed;
    int unemployment_percentage;

    // Resources
    int food_months_supply;
    int food_types;

    // Buildings (counts)
    int num_houses;
    int num_farms;
    int num_industries;
    int housing_capacity;

    // Time
    int year;
    int month;

    // Victory
    int victory_state; // 0=playing, 1=won, -1=lost

} julius_observation_t;

// Action structure
typedef enum {
    ACTION_ADJUST_TAX,
    ACTION_BUILD_HOUSE,
    ACTION_BUILD_FOUNTAIN,
    ACTION_BUILD_PREFECTURE,
    ACTION_BUILD_ENGINEER,
    ACTION_BUILD_MARKET,
    ACTION_BUILD_GRANARY,
    ACTION_BUILD_FARM,
    ACTION_DEMOLISH_BUILDING,
    ACTION_WAIT,
} julius_action_type_t;

typedef struct {
    julius_action_type_t type;
    int param1; // tax delta, building x, etc.
    int param2; // building y, etc.
} julius_action_t;

// API functions
void* julius_env_create(const char* caesar3_data_dir);
void julius_env_destroy(void* env);
void julius_env_reset(void* env, const char* scenario_file);
float julius_env_step(void* env, julius_action_t* action, julius_observation_t* obs);
int julius_env_is_done(void* env);
```

### Python API (python/julius_gym/env.py)

```python
import gymnasium as gym
from gymnasium import spaces
import numpy as np

class JuliusEnv(gym.Env):
    metadata = {"render_modes": []}

    def __init__(self, caesar3_dir: str, scenario: str = "tutorial1"):
        super().__init__()

        # Define observation space
        self.observation_space = spaces.Dict({
            "ratings": spaces.Box(low=0, high=100, shape=(4,), dtype=np.int32),
            "finance": spaces.Box(low=-10000, high=100000, shape=(4,), dtype=np.int32),
            "population": spaces.Box(low=0, high=50000, shape=(4,), dtype=np.int32),
            "resources": spaces.Box(low=0, high=1000, shape=(2,), dtype=np.int32),
            "buildings": spaces.Box(low=0, high=500, shape=(6,), dtype=np.int32),
        })

        # Define action space (discrete for simplicity)
        self.action_space = spaces.Discrete(10)  # 10 different action types

        self._env = None  # C environment handle

    def reset(self, seed=None, options=None):
        super().reset(seed=seed)
        # Call C API: julius_env_reset()
        obs = self._get_observation()
        return obs, {}

    def step(self, action):
        # Call C API: julius_env_step()
        reward = 0.0  # From C function
        terminated = False  # Check victory condition
        truncated = False  # Time limit
        obs = self._get_observation()
        return obs, reward, terminated, truncated, {}
```

## Technical Challenges and Solutions

### Challenge 1: Game State Initialization
- **Problem**: Julius requires Caesar 3 data files and a valid save/scenario
- **Solution**:
  - Provide mechanism to load specific scenario
  - Create "blank slate" scenarios for training
  - Use existing save/load system for reset()

### Challenge 2: Time Scale
- **Problem**: Real Julius runs at 20 ticks/sec, RL needs faster
- **Solution**:
  - Headless mode can run much faster (no 30fps cap)
  - Configurable ticks-per-step
  - Could run at 1000+ ticks/sec headless

### Challenge 3: Action Space Complexity
- **Problem**: Caesar III has hundreds of possible actions
- **Solution**:
  - Start with simplified action space
  - Use high-level actions ("build housing district")
  - Progressively add complexity

### Challenge 4: Stochasticity
- **Problem**: Game has random events (fires, invasions, etc.)
- **Solution**:
  - Seed RNG for reproducibility
  - Option to disable random events initially
  - Already have `random_init()` in `src/core/random.c`

### Challenge 5: Long Episodes
- **Problem**: Caesar III scenarios can take hours
- **Solution**:
  - Define shorter training scenarios
  - Use time-based truncation
  - Focus on specific sub-goals

### Challenge 6: Sparse Rewards
- **Problem**: Victory conditions are long-term
- **Solution**:
  - Design intermediate rewards (population milestones, rating thresholds)
  - Shaped reward based on rating improvements
  - Curriculum learning with easier scenarios first

## File Structure

Proposed new files:

```
brutus/
├── src/
│   └── gymnasium/
│       ├── gymnasium.h          # Main API header
│       ├── gymnasium.c          # API implementation
│       ├── observation.h        # Observation structures
│       ├── observation.c        # State extraction
│       ├── action.h             # Action structures
│       ├── action.c             # Action handling
│       └── headless.c           # Headless game loop
├── python/
│   └── julius_gym/
│       ├── __init__.py
│       ├── env.py               # Gymnasium environment
│       ├── wrappers.py          # Utility wrappers
│       └── scenarios.py         # Scenario definitions
├── test/
│   └── gymnasium/
│       ├── test_observation.c
│       ├── test_action.c
│       └── test_headless.c
└── examples/
    ├── random_agent.py
    ├── train_ppo.py
    └── evaluate.py
```

## Dependencies

### Build-time
- CMake (already required)
- C compiler (already required)
- Python 3.8+ (new)

### Python Runtime
- gymnasium >= 0.29.0
- numpy >= 1.20.0
- (optional) stable-baselines3 for training examples

### Removed for Headless
- SDL2 (conditional compilation)
- SDL2_mixer (conditional)

## Estimated Effort

| Phase | Estimated Time | Risk Level |
|-------|---------------|------------|
| Phase 1: Headless Engine | 2-3 weeks | Medium |
| Phase 2: Observation API | 1-2 weeks | Low |
| Phase 3: Action API | 2-3 weeks | Medium |
| Phase 4: Python Bindings | 1-2 weeks | Low |
| Phase 5: Testing | 1-2 weeks | Low |
| **Total (MVP)** | **7-12 weeks** | **Medium** |

**Risk Factors**:
- Unexpected coupling between game logic and UI
- Complex scenarios may not work headless
- Performance issues with state extraction
- Memory management in C/Python bridge

## Success Criteria

### Minimal Success
- ✓ Julius runs headless without crashes
- ✓ Can extract basic observations (ratings, treasury, population)
- ✓ Can perform basic actions (build, adjust tax)
- ✓ Python environment follows gymnasium API
- ✓ Can train a simple RL agent (even if it doesn't learn well)

### Full Success
- ✓ All above, plus:
- ✓ Comprehensive observation space (including map)
- ✓ Rich action space (all building types, locations)
- ✓ Multiple scenarios supported
- ✓ Fast execution (>100 steps/sec)
- ✓ Documented and tested
- ✓ Example agents demonstrate learning

## Future Enhancements

1. **Visual Rendering for Debugging**
   - Optional SDL2 rendering of headless state
   - Useful for understanding agent behavior

2. **Multi-Agent Support**
   - Multiple cities competing
   - Cooperative scenarios

3. **Curriculum Learning Support**
   - Progressively harder scenarios
   - Automatic scenario generation

4. **Performance Optimization**
   - Vectorized environments (multiple instances)
   - JIT compilation with PyPy
   - GPU state extraction

5. **Advanced Observations**
   - CNN-friendly map representations
   - Attention masks for valid actions
   - Historical state (time series)

## Conclusion

Creating a headless Julius for gymnasium is feasible due to:
- Clean separation of game logic and rendering
- Well-structured state management
- Existing action APIs
- Modular architecture

The project can be approached incrementally, with a working MVP achievable in 7-12 weeks of focused development. The main challenges involve ensuring the headless mode works reliably and designing appropriate observation/action spaces for effective learning.

## Next Steps

1. **Prototype headless mode** - Verify Julius can run without SDL2
2. **Design observation/action spaces** - Decide on specifics before implementation
3. **Choose Python binding approach** - Start with ctypes for quick iteration
4. **Implement incrementally** - Test each phase before moving to the next
5. **Define training scenarios** - Create focused scenarios for initial experiments
