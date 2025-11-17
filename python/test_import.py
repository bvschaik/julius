#!/usr/bin/env python3
"""
Simple import test for julius_gym package
"""

import sys

try:
    import julius_gym
    print("✓ julius_gym imported successfully")
    print(f"  Version: {julius_gym.__version__}")
    print(f"  Exports: {julius_gym.__all__}")

    from julius_gym import JuliusEnv
    print("✓ JuliusEnv imported successfully")

    # Check if gymnasium is available
    import gymnasium as gym
    print("✓ gymnasium imported successfully")

    # Check if numpy is available
    import numpy as np
    print("✓ numpy imported successfully")

    print("\nAll imports successful! Package structure is correct.")
    sys.exit(0)

except ImportError as e:
    print(f"✗ Import error: {e}")
    sys.exit(1)
except Exception as e:
    print(f"✗ Unexpected error: {e}")
    sys.exit(1)
