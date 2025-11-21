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

    metadata = {"render_modes": ["human", "rgb_array", "save"]}

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

        # Define observation space (flattened for stable-baselines3 compatibility)
        self.observation_space = spaces.Dict(
            {
                # Ratings
                "ratings_culture": spaces.Box(0, 100, shape=(1,), dtype=np.int32),
                "ratings_prosperity": spaces.Box(0, 100, shape=(1,), dtype=np.int32),
                "ratings_peace": spaces.Box(0, 100, shape=(1,), dtype=np.int32),
                "ratings_favor": spaces.Box(0, 100, shape=(1,), dtype=np.int32),
                # Finance
                "finance_treasury": spaces.Box(-10000, 1000000, shape=(1,), dtype=np.int32),
                "finance_tax_percentage": spaces.Box(0, 25, shape=(1,), dtype=np.int32),
                "finance_estimated_tax_income": spaces.Box(0, 100000, shape=(1,), dtype=np.int32),
                "finance_last_year_income": spaces.Box(0, 1000000, shape=(1,), dtype=np.int32),
                "finance_last_year_expenses": spaces.Box(0, 1000000, shape=(1,), dtype=np.int32),
                "finance_last_year_net": spaces.Box(-1000000, 1000000, shape=(1,), dtype=np.int32),
                # Population
                "population_total": spaces.Box(0, 100000, shape=(1,), dtype=np.int32),
                "population_school_age": spaces.Box(0, 100000, shape=(1,), dtype=np.int32),
                "population_academy_age": spaces.Box(0, 100000, shape=(1,), dtype=np.int32),
                "population_working_age": spaces.Box(0, 100000, shape=(1,), dtype=np.int32),
                "population_sentiment": spaces.Box(0, 100, shape=(1,), dtype=np.int32),
                # Labor
                "labor_workers_needed": spaces.Box(0, 100000, shape=(1,), dtype=np.int32),
                "labor_workers_employed": spaces.Box(0, 100000, shape=(1,), dtype=np.int32),
                "labor_unemployment_percentage": spaces.Box(0, 100, shape=(1,), dtype=np.int32),
                "labor_wages": spaces.Box(0, 100, shape=(1,), dtype=np.int32),
                # Resources
                "resources_food_stocks": spaces.Box(0, 1000000, shape=(1,), dtype=np.int32),
                "resources_food_types_available": spaces.Box(0, 10, shape=(1,), dtype=np.int32),
                # Buildings
                "buildings_housing": spaces.Box(0, 10000, shape=(1,), dtype=np.int32),
                "buildings_housing_capacity": spaces.Box(0, 100000, shape=(1,), dtype=np.int32),
                # Migration
                "migration_immigration_amount": spaces.Box(0, 10000, shape=(1,), dtype=np.int32),
                "migration_emigration_amount": spaces.Box(0, 10000, shape=(1,), dtype=np.int32),
                "migration_immigration_queue_size": spaces.Box(0, 10000, shape=(1,), dtype=np.int32),
                "migration_emigration_queue_size": spaces.Box(0, 10000, shape=(1,), dtype=np.int32),
                # Culture
                "culture_entertainment": spaces.Box(0, 100, shape=(1,), dtype=np.int32),
                "culture_average_entertainment": spaces.Box(0, 100, shape=(1,), dtype=np.int32),
                "culture_average_education": spaces.Box(0, 100, shape=(1,), dtype=np.int32),
                "culture_average_health": spaces.Box(0, 100, shape=(1,), dtype=np.int32),
                "culture_religion_coverage": spaces.Box(0, 100, shape=(1,), dtype=np.int32),
                # Time
                "time_tick": spaces.Box(0, 1000000, shape=(1,), dtype=np.int32),
                "time_day": spaces.Box(0, 15, shape=(1,), dtype=np.int32),
                "time_month": spaces.Box(0, 11, shape=(1,), dtype=np.int32),
                "time_year": spaces.Box(0, 1000, shape=(1,), dtype=np.int32),
                "time_total_months": spaces.Box(0, 12000, shape=(1,), dtype=np.int32),
                # Victory
                "victory_culture_goal": spaces.Box(0, 100, shape=(1,), dtype=np.int32),
                "victory_prosperity_goal": spaces.Box(0, 100, shape=(1,), dtype=np.int32),
                "victory_peace_goal": spaces.Box(0, 100, shape=(1,), dtype=np.int32),
                "victory_favor_goal": spaces.Box(0, 100, shape=(1,), dtype=np.int32),
                "victory_population_goal": spaces.Box(0, 100000, shape=(1,), dtype=np.int32),
                "victory_time_limit_years": spaces.Box(0, 1000, shape=(1,), dtype=np.int32),
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

        # julius_env_get_map_data
        self.lib.julius_env_get_map_data.argtypes = [
            c_void_p,
            POINTER(ctypes.c_uint16),
            POINTER(ctypes.c_uint16),
            POINTER(c_int32),
            POINTER(c_int32),
        ]
        self.lib.julius_env_get_map_data.restype = c_int32

    def _obs_to_dict(self, obs: Observation) -> Dict[str, Any]:
        """Convert C observation structure to Python dict (flattened)"""
        return {
            # Ratings
            "ratings_culture": np.array([obs.ratings.culture], dtype=np.int32),
            "ratings_prosperity": np.array([obs.ratings.prosperity], dtype=np.int32),
            "ratings_peace": np.array([obs.ratings.peace], dtype=np.int32),
            "ratings_favor": np.array([obs.ratings.favor], dtype=np.int32),
            # Finance
            "finance_treasury": np.array([obs.finance.treasury], dtype=np.int32),
            "finance_tax_percentage": np.array([obs.finance.tax_percentage], dtype=np.int32),
            "finance_estimated_tax_income": np.array([obs.finance.estimated_tax_income], dtype=np.int32),
            "finance_last_year_income": np.array([obs.finance.last_year_income], dtype=np.int32),
            "finance_last_year_expenses": np.array([obs.finance.last_year_expenses], dtype=np.int32),
            "finance_last_year_net": np.array([obs.finance.last_year_net], dtype=np.int32),
            # Population
            "population_total": np.array([obs.population.total], dtype=np.int32),
            "population_school_age": np.array([obs.population.school_age], dtype=np.int32),
            "population_academy_age": np.array([obs.population.academy_age], dtype=np.int32),
            "population_working_age": np.array([obs.population.working_age], dtype=np.int32),
            "population_sentiment": np.array([obs.population.sentiment], dtype=np.int32),
            # Labor
            "labor_workers_needed": np.array([obs.labor.workers_needed], dtype=np.int32),
            "labor_workers_employed": np.array([obs.labor.workers_employed], dtype=np.int32),
            "labor_unemployment_percentage": np.array([obs.labor.unemployment_percentage], dtype=np.int32),
            "labor_wages": np.array([obs.labor.wages], dtype=np.int32),
            # Resources
            "resources_food_stocks": np.array([obs.resources.food_stocks], dtype=np.int32),
            "resources_food_types_available": np.array([obs.resources.food_types_available], dtype=np.int32),
            # Buildings
            "buildings_housing": np.array([obs.buildings.housing], dtype=np.int32),
            "buildings_housing_capacity": np.array([obs.buildings.housing_capacity], dtype=np.int32),
            # Migration
            "migration_immigration_amount": np.array([obs.migration.immigration_amount], dtype=np.int32),
            "migration_emigration_amount": np.array([obs.migration.emigration_amount], dtype=np.int32),
            "migration_immigration_queue_size": np.array([obs.migration.immigration_queue_size], dtype=np.int32),
            "migration_emigration_queue_size": np.array([obs.migration.emigration_queue_size], dtype=np.int32),
            # Culture
            "culture_entertainment": np.array([obs.culture.entertainment], dtype=np.int32),
            "culture_average_entertainment": np.array([obs.culture.average_entertainment], dtype=np.int32),
            "culture_average_education": np.array([obs.culture.average_education], dtype=np.int32),
            "culture_average_health": np.array([obs.culture.average_health], dtype=np.int32),
            "culture_religion_coverage": np.array([obs.culture.religion_coverage], dtype=np.int32),
            # Time
            "time_tick": np.array([obs.time.tick], dtype=np.int32),
            "time_day": np.array([obs.time.day], dtype=np.int32),
            "time_month": np.array([obs.time.month], dtype=np.int32),
            "time_year": np.array([obs.time.year], dtype=np.int32),
            "time_total_months": np.array([obs.time.total_months], dtype=np.int32),
            # Victory
            "victory_culture_goal": np.array([obs.victory.culture_goal], dtype=np.int32),
            "victory_prosperity_goal": np.array([obs.victory.prosperity_goal], dtype=np.int32),
            "victory_peace_goal": np.array([obs.victory.peace_goal], dtype=np.int32),
            "victory_favor_goal": np.array([obs.victory.favor_goal], dtype=np.int32),
            "victory_population_goal": np.array([obs.victory.population_goal], dtype=np.int32),
            "victory_time_limit_years": np.array([obs.victory.time_limit_years], dtype=np.int32),
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

    def get_map_data(self) -> Tuple[np.ndarray, np.ndarray, int, int]:
        """
        Get the current map data for rendering.

        Returns:
            terrain_map: 162x162 numpy array of terrain flags
            building_map: 162x162 numpy array of building IDs
            width: Actual map width
            height: Actual map height
        """
        # Allocate arrays for map data
        grid_size = 162 * 162
        terrain_data = (ctypes.c_uint16 * grid_size)()
        building_data = (ctypes.c_uint16 * grid_size)()
        width = c_int32()
        height = c_int32()

        # Call C function
        ret = self.lib.julius_env_get_map_data(
            self.env_handle,
            terrain_data,
            building_data,
            ctypes.byref(width),
            ctypes.byref(height),
        )

        if ret != 0:
            error = self.lib.julius_env_get_error(self.env_handle)
            raise RuntimeError(f"Failed to get map data: {error.decode()}")

        # Convert to numpy arrays
        terrain_map = np.array(terrain_data, dtype=np.uint16).reshape(162, 162)
        building_map = np.array(building_data, dtype=np.uint16).reshape(162, 162)

        return terrain_map, building_map, width.value, height.value

    def render(self, mode: str = "human", output_file: Optional[str] = None):
        """
        Render the current city map.

        Args:
            mode: Render mode ("human" displays, "rgb_array" returns array, "save" saves to file)
            output_file: Optional filename to save the rendered map (used with mode="save")
        """
        try:
            import matplotlib.pyplot as plt
            import matplotlib.patches as mpatches
        except ImportError:
            raise ImportError(
                "matplotlib is required for rendering. "
                "Install with: pip install matplotlib"
            )

        # Get map data
        terrain_map, building_map, width, height = self.get_map_data()

        # Use full grid if width/height are 0 (no scenario loaded)
        if width == 0 or height == 0:
            width = 162
            height = 162

        # Create RGB image based on terrain and buildings
        rgb_map = np.zeros((162, 162, 3), dtype=np.uint8)

        # Terrain color mapping
        TERRAIN_WATER = 0x04
        TERRAIN_TREE = 0x01
        TERRAIN_ROCK = 0x02
        TERRAIN_ROAD = 0x40
        TERRAIN_BUILDING = 0x08
        TERRAIN_GARDEN = 0x20
        TERRAIN_AQUEDUCT = 0x100
        TERRAIN_WALL = 0x4000
        TERRAIN_MEADOW = 0x800

        for y in range(162):
            for x in range(162):
                terrain = terrain_map[y, x]
                building = building_map[y, x]

                # Color priority: buildings > water > roads > trees > rocks > meadow
                if building > 0:
                    # Buildings - various shades of gray/brown
                    rgb_map[y, x] = [139, 69, 19]  # Brown for buildings
                elif terrain & TERRAIN_WATER:
                    rgb_map[y, x] = [30, 144, 255]  # Blue for water
                elif terrain & TERRAIN_WALL:
                    rgb_map[y, x] = [128, 128, 128]  # Gray for walls
                elif terrain & TERRAIN_ROAD:
                    rgb_map[y, x] = [192, 192, 192]  # Light gray for roads
                elif terrain & TERRAIN_AQUEDUCT:
                    rgb_map[y, x] = [100, 149, 237]  # Cornflower blue for aqueducts
                elif terrain & TERRAIN_GARDEN:
                    rgb_map[y, x] = [50, 205, 50]  # Lime green for gardens
                elif terrain & TERRAIN_TREE:
                    rgb_map[y, x] = [34, 139, 34]  # Forest green for trees
                elif terrain & TERRAIN_ROCK:
                    rgb_map[y, x] = [105, 105, 105]  # Dim gray for rocks
                elif terrain & TERRAIN_MEADOW:
                    rgb_map[y, x] = [144, 238, 144]  # Light green for meadow
                else:
                    rgb_map[y, x] = [222, 184, 135]  # Burlywood for empty land

        # Crop to actual map size
        rgb_map = rgb_map[:height, :width]

        if mode == "rgb_array":
            return rgb_map

        # Create figure
        fig, ax = plt.subplots(figsize=(12, 12))
        ax.imshow(rgb_map)
        ax.set_title("Julius City Map", fontsize=16)
        ax.axis("off")

        # Create legend
        legend_elements = [
            mpatches.Patch(color=[139/255, 69/255, 19/255], label="Buildings"),
            mpatches.Patch(color=[30/255, 144/255, 255/255], label="Water"),
            mpatches.Patch(color=[192/255, 192/255, 192/255], label="Roads"),
            mpatches.Patch(color=[34/255, 139/255, 34/255], label="Trees"),
            mpatches.Patch(color=[50/255, 205/255, 50/255], label="Gardens"),
            mpatches.Patch(color=[144/255, 238/255, 144/255], label="Meadow"),
        ]
        ax.legend(handles=legend_elements, loc="upper right", bbox_to_anchor=(1.15, 1))

        if mode == "save" or output_file:
            filename = output_file or "city_map.png"
            plt.savefig(filename, bbox_inches="tight", dpi=150)
            print(f"Map saved to {filename}")
        elif mode == "human":
            plt.tight_layout()
            plt.show()

        plt.close()

    def close(self):
        """Clean up environment resources"""
        if hasattr(self, "env_handle") and self.env_handle:
            self.lib.julius_env_destroy(self.env_handle)
            self.env_handle = None

    def __del__(self):
        """Destructor to ensure cleanup"""
        self.close()
