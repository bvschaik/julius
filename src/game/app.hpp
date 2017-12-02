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


#ifndef __JULIUS_APP_H__
#define __JULIUS_APP_H__

#include <string>
#include "core/platform.hpp"

enum { platform_unknown,
       platform_windows,
       platform_win32,
       platform_win64,
       platform_unix,
       platform_linux,
       platform_macos,
       platform_bsd,
       platform_haiku,
       platform_beos,
       platform_android
     };

class Application
{
public:
    std::string workdir();
    void errordlg( std::string title, std::string text );

    struct Platform
    {
        bool is(int type);
        bool android();
        bool linux();
        bool unix();
        bool mac();
        bool windows();
    } platform;

private:
    std::string _workdir;
};

extern Application app;

#endif //__JULIUS_APP_H__
