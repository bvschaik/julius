// This file is part of Julius.
//
// Julius is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Julius is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Julius.  If not, see <http://www.gnu.org/licenses/>.
//
// Copyright 2012-2017 Dalerank, dalerankn8@gmail.com

#ifndef __JULIUS_STACKTRACE_H_INCLUDED__
#define __JULIUS_STACKTRACE_H_INCLUDED__

#include "core/platform.hpp"

struct CrashHandler
{
  static void install();
  static void uninstal();
  static void printstack(bool showMessage = true, unsigned int starting_frame = 0, unsigned int max_frames = 63 );
};

#endif //__JULIUS_STACKTRACE_H_INCLUDED__
