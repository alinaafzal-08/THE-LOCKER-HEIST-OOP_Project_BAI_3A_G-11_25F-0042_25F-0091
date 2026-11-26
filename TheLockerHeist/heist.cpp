// ============================================================
// heist.cpp  --  THE LOCKER HEIST
// Definitions for the types declared in heist.h.
// In Module 1 all types are plain structs and enums; they need
// no non-trivial constructor or method definitions beyond what
// the compiler generates automatically.  This file exists so
// the Visual Studio project always has three real .cpp files
// and so that later commits can add method bodies here without
// restructuring the project.
// ============================================================

#include "heist.h"

// Implementation-only headers live here, not in heist.h, so the
// compiler only sees them when building this translation unit.
#include <fstream>    // std::ifstream / std::ofstream -- for data file I/O (Commit 2+)
#include <cstdlib>    // rand, srand, system() -- for culprit selection and screen-clear
#include <ctime>      // time() -- to seed the random number generator

// ------------------------------------------------------------
// No definitions are needed for Commit 1 because:
//   - Tile, Clue and Location are POD-like structs with
//     in-class initialisers (the compiler-generated default
//     constructor is sufficient).
//   - WordRow, GameState and MenuChoice are enum classes with
//     no methods.
// Later commits will add DataLoader, StringUtils, GameEngine,
// and all class method bodies here.
// ------------------------------------------------------------
