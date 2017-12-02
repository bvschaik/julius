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

#ifndef __JULIUS_PLATFORM_H_INCLUDED__
#define __JULIUS_PLATFORM_H_INCLUDED__

#if defined(__WIN32__) || defined(_WIN32)
#define JULIUS_PLATFORM_WIN
#define JULIUS_PLATFORM_WIN32
#define JULIUS_PLATFORM_NAME "win32"
#elif defined(WIN64) || defined(_WIN64)
#define JULIUS_PLATFORM_WIN
#define JULIUS_PLATFORM_WIN64
#define JULIUS_PLATFORM_NAME "win64"
#elif defined(__APPLE_CC__) || defined(__APPLE__) || defined(__APPLE_CPP__) || defined(__MACOS_CLASSIC__)
#define JULIUS_PLATFORM_UNIX
#define JULIUS_PLATFORM_MACOSX
#define JULIUS_PLATFORM_NAME "macosx"
#elif defined(__FreeBSD__) || defined(__OpenBSD__)
#define JULIUS_PLATFORM_UNIX
#define JULIUS_PLATFORM_XBSD
#define JULIUS_PLATFORM_NAME "freebsd"
#elif defined(__HAIKU__) || defined(HAIKU)
#define JULIUS_PLATFORM_BEOS
#define JULIUS_PLATFORM_HAIKU
#define JULIUS_PLATFORM_NAME "haiku"
#elif defined(ANDROID)
#define JULIUS_PLATFORM_UNIX
#define JULIUS_PLATFORM_ANDROID
#define JULIUS_PLATFORM_NAME "android"
#else
#define JULIUS_PLATFORM_UNIX
#define JULIUS_PLATFORM_LINUX
#define JULIUS_PLATFORM_NAME "linux"
#endif

#endif //__JULIUS_PLATFORM_H_INCLUDED__
