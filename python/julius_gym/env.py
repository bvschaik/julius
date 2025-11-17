"""
JuliusEnv - Gymnasium environment for Julius (Caesar III) city-building game
"""

import ctypes
import os
from ctypes import POINTER, Structure, c_char, c_char_p, c_float, c_int32, c_void_p
from pathlib import Path
from typing import Any, Dict, Optional, Tuple

import gymnasium as gym
import numpy as np
from gymnasium import spaces


# C structure definitions matching gymnasium/observation.h
class CityRatings(Structure):
    _fields_ = [
        ("culture", c_int32),
        ("prosperity", c_int32),
        ("peace", c_int32),
        ("favor", c_int32),
    ]


class Finance(Structure):
    _fields_ = [
        ("treasury", c_int32),
        ("tax_percentage", c_int32),
        ("estimated_tax_income", c_int32),
        ("last_year_income", c_int32),
        ("last_year_expenses", c_int32),
        ("last_year_net", c_int32),
    ]


class Population(Structure):
    _fields_ = [
        ("total", c_int32),
        ("school_age", c_int32),
        ("academy_age", c_int32),
        ("working_age", c_int32),
        ("sentiment", c_int32),
    ]


class Labor(Structure):
    _fields_ = [
        ("workers_needed", c_int32),
        ("workers_employed", c_int32),
        ("unemployment_percentage", c_int32),
        ("wages", c_int32),
    ]


class Resources(Structure):
    _fields_ = [
        ("food_stocks", c_int32),
        ("food_types_available", c_int32),
    ]


class Buildings(Structure):
    _fields_ = [
        ("housing", c_int32),
        ("housing_capacity", c_int32),
    ]


class Migration(Structure):
    _fields_ = [
        ("immigration_amount", c_int32),
        ("emigration_amount", c_int32),
        ("immigration_queue_size", c_int32),
        ("emigration_queue_size", c_int32),
        ("refuse_immigrants_reason", c_int32),
    ]


class Culture(Structure):
    _fields_ = [
        ("entertainment", c_int32),
        ("average_entertainment", c_int32),
        ("average_education", c_int32),
        ("average_health", c_int32),
        ("religion_coverage", c_int32),
    ]


class Time(Structure):
    _fields_ = [
        ("tick", c_int32),
        ("day", c_int32),
        ("month", c_int32),
        ("year", c_int32),
        ("total_months", c_int32),
    ]


class Victory(Structure):
    _fields_ = [
        ("culture_goal", c_int32),
        ("prosperity_goal", c_int32),
        ("peace_goal", c_int32),
        ("favor_goal", c_int32),
        ("population_goal", c_int32),
        ("time_limit_years", c_int32),
    ]


class Observation(Structure):
    """Matches gymnasium_observation_t from observation.h"""

    _fields_ = [
        ("ratings", CityRatings),
        ("finance", Finance),
        ("population", Population),
        ("labor", Labor),
        ("resources", Resources),
        ("buildings", Buildings),
        ("migration", Migration),
        ("culture", Culture),
        ("time", Time),
        ("victory", Victory),
    ]


# C structure definitions matching gymnasium/action.h
class Action(Structure):
    """Matches gymnasium_action_t from action.h"""

    _fields_ = [
        ("type", c_int32),  # gymnasium_action_type_t
        ("x", c_int32),
        ("y", c_int32),
        ("param1", c_int32),
        ("param2", c_int32),
    ]


class ActionResult(Structure):
    """Matches gymnasium_action_result_t from action.h"""

    _fields_ = [
        ("success", c_int32),
        ("reason", c_int32),
        ("message", c_char * 256),
    ]


# C structure definitions matching gymnasium/gymnasium.h
class EnvConfig(Structure):
    """Matches julius_env_config_t from gymnasium.h"""

    _fields_ = [
        ("data_directory", c_char_p),
        ("scenario_file", c_char_p),
        ("max_ticks", c_int32),
        ("tick_speed_ms", c_int32),
    ]


class StepResult(Structure):
    """Matches julius_env_step_result_t from gymnasium.h"""

    _fields_ = [
        ("observation", Observation),
        ("reward", c_float),
        ("terminated", c_int32),
        ("truncated", c_int32),
        ("info", c_char * 256),
    ]


# Action type constants (must match gymnasium/action.h)
class ActionType:
    NONE = 0
    ADJUST_TAX_UP = 1
    ADJUST_TAX_DOWN = 2
    ADJUST_WAGES_UP = 3
    ADJUST_WAGES_DOWN = 4
    BUILD_HOUSING = 5
    BUILD_ROAD = 6
    BUILD_PLAZA = 7
    BUILD_GARDEN = 8
    BUILD_STATUE = 9
    BUILD_PREFECTURE = 10
    BUILD_ENGINEERS_POST = 11
    BUILD_DOCTOR = 12
    BUILD_HOSPITAL = 13
    BUILD_BATHHOUSE = 14
    BUILD_BARBER = 15
    BUILD_SCHOOL = 16
    BUILD_ACADEMY = 17
    BUILD_LIBRARY = 18
    BUILD_THEATER = 19
    BUILD_AMPHITHEATER = 20
    BUILD_COLOSSEUM = 21
    BUILD_HIPPODROME = 22
    BUILD_TEMPLE_SMALL = 23
    BUILD_TEMPLE_LARGE = 24
    BUILD_ORACLE = 25
    BUILD_MARKET = 26
    BUILD_GRANARY = 27
    BUILD_WAREHOUSE = 28
    BUILD_FOUNTAIN = 29
    BUILD_WELL = 30
    BUILD_FORUM = 31
    BUILD_SENATE = 32
    BUILD_GOVERNORS_VILLA = 33
    BUILD_GOVERNORS_PALACE = 34
    BUILD_WHEAT_FARM = 35
    BUILD_VEGETABLE_FARM = 36
    BUILD_FRUIT_FARM = 37
    BUILD_OLIVE_FARM = 38
    BUILD_VINES_FARM = 39
    BUILD_PIG_FARM = 40
    BUILD_CLAY_PIT = 41
    BUILD_TIMBER_YARD = 42
    BUILD_IRON_MINE = 43
    BUILD_MARBLE_QUARRY = 44
    BUILD_WINE_WORKSHOP = 45
    BUILD_OIL_WORKSHOP = 46
    BUILD_WEAPONS_WORKSHOP = 47
    BUILD_FURNITURE_WORKSHOP = 48
    BUILD_POTTERY_WORKSHOP = 49
    BUILD_ACTOR_COLONY = 50
    BUILD_GLADIATOR_SCHOOL = 51
    BUILD_LION_HOUSE = 52
    BUILD_CHARIOT_MAKER = 53
    CLEAR_LAND = 54
    COUNT = 55


class JuliusEnv(gym.Env):
    """
    Gymnasium environment for Julius (Caesar III) city-building game.

    Observation Space:
        Dict containing various city metrics (ratings, finance, population, etc.)

    Action Space:
        Discrete(55) - 55 different action types including:
        - Administrative: adjust taxes, wages
        - Building: construct various buildings at coordinates
        - Destruction: clear land

    Rewards:
        - Positive rewards for improving city ratings
        - Bonus reward for winning scenario (+100)
        - Penalty for losing scenario (-50)
        - Small penalties for failed actions

    Episode Termination:
        - Victory condition achieved
        - Defeat condition met (bankruptcy, time limit)
        - Max steps reached (if configured)
    """

    metadata = {"render_modes": []}

    def __init__(
        self,
        data_directory: str,
        scenario_file: Optional[str] = None,
        max_ticks: int = 0,
        lib_path: Optional[str] = None,
    ):
        """
        Initialize Julius environment.

        Args:
            data_directory: Path to Caesar III data files directory
            scenario_file: Optional path to scenario file to load
            max_ticks: Maximum ticks per episode (0 = unlimited)
            lib_path: Optional path to libjulius_gym.so (auto-detected if None)
        """
        super().__init__()

        # Load the shared library
        if lib_path is None:
            lib_path = self._find_library()

        self.lib = ctypes.CDLL(lib_path)

        # Define C function signatures
        self._setup_library_functions()

        # Create environment configuration
        config = EnvConfig()
        config.data_directory = data_directory.encode("utf-8")
        config.scenario_file = (
            scenario_file.encode("utf-8") if scenario_file else None
        )
        config.max_ticks = max_ticks
        config.tick_speed_ms = 1

        # Create C environment
        self.env_handle = self.lib.julius_env_create(ctypes.byref(config))
        if not self.env_handle:
            raise RuntimeError("Failed to create Julius environment")

        # Define observation space
        self.observation_space = spaces.Dict(
            {
                "ratings": spaces.Dict(
                    {
                        "culture": spaces.Box(0, 100, shape=(), dtype=np.int32),
                        "prosperity": spaces.Box(0, 100, shape=(), dtype=np.int32),
                        "peace": spaces.Box(0, 100, shape=(), dtype=np.int32),
                        "favor": spaces.Box(0, 100, shape=(), dtype=np.int32),
                    }
                ),
                "finance": spaces.Dict(
                    {
                        "treasury": spaces.Box(-10000, 1000000, shape=(), dtype=np.int32),
                        "tax_percentage": spaces.Box(0, 25, shape=(), dtype=np.int32),
                    }
                ),
                "population": spaces.Dict(
                    {
                        "total": spaces.Box(0, 100000, shape=(), dtype=np.int32),
                        "sentiment": spaces.Box(0, 100, shape=(), dtype=np.int32),
                    }
                ),
                "time": spaces.Dict(
                    {
                        "year": spaces.Box(0, 1000, shape=(), dtype=np.int32),
                        "month": spaces.Box(0, 11, shape=(), dtype=np.int32),
                    }
                ),
            }
        )

        # Define action space - discrete actions
        self.action_space = spaces.Discrete(ActionType.COUNT)

        # Store map dimensions for coordinate normalization
        self.map_width = 162
        self.map_height = 162

    def _find_library(self) -> str:
        """Auto-detect libjulius_gym.so location"""
        # Check common locations
        search_paths = [
            "build/libjulius_gym.so",
            "../build/libjulius_gym.so",
            "/usr/local/lib/libjulius_gym.so",
            "/usr/lib/libjulius_gym.so",
        ]

        for path in search_paths:
            if os.path.exists(path):
                return path

        raise RuntimeError(
            "Could not find libjulius_gym.so. Build with "
            "cmake -DBUILD_GYMNASIUM_LIB=ON -DHEADLESS_BUILD=ON"
        )

    def _setup_library_functions(self):
        """Configure ctypes function signatures"""
        # julius_env_create
        self.lib.julius_env_create.argtypes = [POINTER(EnvConfig)]
        self.lib.julius_env_create.restype = c_void_p

        # julius_env_destroy
        self.lib.julius_env_destroy.argtypes = [c_void_p]
        self.lib.julius_env_destroy.restype = None

        # julius_env_reset
        self.lib.julius_env_reset.argtypes = [c_void_p, POINTER(Observation)]
        self.lib.julius_env_reset.restype = c_int32

        # julius_env_step
        self.lib.julius_env_step.argtypes = [
            c_void_p,
            POINTER(Action),
            POINTER(StepResult),
        ]
        self.lib.julius_env_step.restype = c_int32

        # julius_env_get_error
        self.lib.julius_env_get_error.argtypes = [c_void_p]
        self.lib.julius_env_get_error.restype = c_char_p

    def _obs_to_dict(self, obs: Observation) -> Dict[str, Any]:
        """Convert C observation structure to Python dict"""
        return {
            "ratings": {
                "culture": obs.ratings.culture,
                "prosperity": obs.ratings.prosperity,
                "peace": obs.ratings.peace,
                "favor": obs.ratings.favor,
            },
            "finance": {
                "treasury": obs.finance.treasury,
                "tax_percentage": obs.finance.tax_percentage,
                "estimated_tax_income": obs.finance.estimated_tax_income,
                "last_year_income": obs.finance.last_year_income,
                "last_year_expenses": obs.finance.last_year_expenses,
                "last_year_net": obs.finance.last_year_net,
            },
            "population": {
                "total": obs.population.total,
                "school_age": obs.population.school_age,
                "academy_age": obs.population.academy_age,
                "working_age": obs.population.working_age,
                "sentiment": obs.population.sentiment,
            },
            "labor": {
                "workers_needed": obs.labor.workers_needed,
                "workers_employed": obs.labor.workers_employed,
                "unemployment_percentage": obs.labor.unemployment_percentage,
                "wages": obs.labor.wages,
            },
            "resources": {
                "food_stocks": obs.resources.food_stocks,
                "food_types_available": obs.resources.food_types_available,
            },
            "buildings": {
                "housing": obs.buildings.housing,
                "housing_capacity": obs.buildings.housing_capacity,
            },
            "migration": {
                "immigration_amount": obs.migration.immigration_amount,
                "emigration_amount": obs.migration.emigration_amount,
                "immigration_queue_size": obs.migration.immigration_queue_size,
                "emigration_queue_size": obs.migration.emigration_queue_size,
            },
            "culture": {
                "entertainment": obs.culture.entertainment,
                "average_entertainment": obs.culture.average_entertainment,
                "average_education": obs.culture.average_education,
                "average_health": obs.culture.average_health,
                "religion_coverage": obs.culture.religion_coverage,
            },
            "time": {
                "tick": obs.time.tick,
                "day": obs.time.day,
                "month": obs.time.month,
                "year": obs.time.year,
                "total_months": obs.time.total_months,
            },
            "victory": {
                "culture_goal": obs.victory.culture_goal,
                "prosperity_goal": obs.victory.prosperity_goal,
                "peace_goal": obs.victory.peace_goal,
                "favor_goal": obs.victory.favor_goal,
                "population_goal": obs.victory.population_goal,
                "time_limit_years": obs.victory.time_limit_years,
            },
        }

    def _action_to_c(self, action: int) -> Action:
        """Convert discrete action index to C action structure"""
        c_action = Action()
        c_action.type = action
        c_action.x = 50  # Default coordinates (center of map)
        c_action.y = 50
        c_action.param1 = 0
        c_action.param2 = 0
        return c_action

    def reset(
        self, *, seed: Optional[int] = None, options: Optional[dict] = None
    ) -> Tuple[Dict[str, Any], Dict[str, Any]]:
        """
        Reset the environment to initial state.

        Args:
            seed: Random seed (not used in Julius)
            options: Additional options (not used)

        Returns:
            observation: Initial observation
            info: Additional information dict
        """
        super().reset(seed=seed)

        obs = Observation()
        result = self.lib.julius_env_reset(self.env_handle, ctypes.byref(obs))

        if result != 0:
            error = self.lib.julius_env_get_error(self.env_handle)
            raise RuntimeError(f"Failed to reset environment: {error.decode()}")

        return self._obs_to_dict(obs), {}

    def step(
        self, action: int
    ) -> Tuple[Dict[str, Any], float, bool, bool, Dict[str, Any]]:
        """
        Execute one step in the environment.

        Args:
            action: Action index from action_space

        Returns:
            observation: New observation after action
            reward: Reward for this step
            terminated: Whether episode ended (won/lost)
            truncated: Whether episode was cut off (max steps)
            info: Additional information dict
        """
        # Convert action to C structure
        c_action = self._action_to_c(action)

        # Execute step
        result = StepResult()
        ret = self.lib.julius_env_step(
            self.env_handle, ctypes.byref(c_action), ctypes.byref(result)
        )

        if ret != 0:
            error = self.lib.julius_env_get_error(self.env_handle)
            raise RuntimeError(f"Failed to execute step: {error.decode()}")

        # Convert results
        observation = self._obs_to_dict(result.observation)
        reward = result.reward
        terminated = bool(result.terminated)
        truncated = bool(result.truncated)
        info = {"message": result.info.decode("utf-8")}

        return observation, reward, terminated, truncated, info

    def close(self):
        """Clean up environment resources"""
        if hasattr(self, "env_handle") and self.env_handle:
            self.lib.julius_env_destroy(self.env_handle)
            self.env_handle = None

    def __del__(self):
        """Destructor to ensure cleanup"""
        self.close()
