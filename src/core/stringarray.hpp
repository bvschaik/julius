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
// Copyright 2012-2014 Dalerank, dalerankn8@gmail.com

#ifndef __JULIUS_STRINGARRAY_H_INCLUDED__
#define __JULIUS_STRINGARRAY_H_INCLUDED__

#include <string>
#include "core/array.hpp"

class StringArray : public Array<std::string>
{
public:
    StringArray& addIfValid( const std::string& str )
    {
        if (!str.empty())
            _data.push_back(str);

        return *this;
    }

    bool operator ==(const StringArray& a) const
    {
        return _data == a._data;
    }

    inline StringArray& operator << (const std::string& a)
    {
        push_back(a);
        return *this;
    }
};

#endif //__JULIUS_STRINGARRAY_H_INCLUDED__
