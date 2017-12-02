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

#ifndef _JULIUS_VECTOR_EXTENSION_INCLUDE_H_
#define _JULIUS_VECTOR_EXTENSION_INCLUDE_H_

#include <vector>
#include <algorithm>

template<class T>
class Array
{
public:
    typedef std::vector<T> Collection;
    typedef typename Collection::iterator iterator;
    typedef typename Collection::const_iterator const_iterator;
    typedef typename Collection::reference reference;
    typedef typename Collection::const_reference const_reference;
    typedef typename Collection::reverse_iterator reverse_iterator;
    typedef typename Collection::const_reverse_iterator const_reverse_iterator;

    Array& push_back(const T& p)
    {
        _data.push_back(p);
        return *this;
    }
    reference operator[](size_t index)
    {
        return _data[index];
    }
    const_reference operator[](size_t index) const
    {
        return _data[index];
    }
    const_iterator begin() const
    {
        return _data.begin();
    }
    const_reverse_iterator rbegin() const
    {
        return _data.rbegin();
    }
    reverse_iterator rbegin()
    {
        return _data.rbegin();
    }
    const_reverse_iterator rend() const
    {
        return _data.rend();
    }
    reverse_iterator rend()
    {
        return _data.rend();
    }
    iterator erase(iterator it)
    {
        return _data.erase(it);
    }
    void erase(iterator b, iterator e)
    {
        _data.erase(b, e);
    }
    iterator begin()
    {
        return _data.begin();
    }
    iterator end()
    {
        return _data.end();
    }
    const_iterator end() const
    {
        return _data.end();
    }
    size_t size() const
    {
        return _data.size();
    }
    void resize(size_t newSize)
    {
        _data.resize(newSize);
    }
    void resize(size_t newSize, const T& def)
    {
        _data.resize(newSize, def);
    }
    bool empty() const
    {
        return _data.empty();
    }
    void clear()
    {
        _data.clear();
    }
    reference front()
    {
        return _data.front();
    }
    const_reference front() const
    {
        return _data.front();
    }
    reference back()
    {
        return _data.back();
    }
    const_reference back() const
    {
        return _data.back();
    }
    reference at(size_t index)
    {
        return _data.at(index);
    }
    const_reference at(size_t index) const
    {
        return _data.at(index);
    }
    void insert(iterator it, const T& value)
    {
        _data.insert(it, value);
    }
    void reserve(size_t size)
    {
        _data.reserve(size);
    }

    Array& addUnique(const T& value)
    {
        auto it = std::find(_data.begin(), _data.end(), value);
        if (it == end())
            push_back(value);

        return *this;
    }

    Array& push_front( const T& value )
    {
        _data.insert( _data.begin(), value );
        return *this;
    }

    Array& append(const T& item)
    {
        push_back(item);
        return *this;
    }

    Array& append(const Array<T>& other)
    {
        _data.insert(_data.end(), other.begin(), other.end());
        return *this;
    }

    Array& pop_front()
    {
        if (!empty())
            _data.erase( _data.begin() );

        return *this;
    }

    Array& pop_back()
    {
        if (!empty())
            _data.pop_back();

        return *this;
    }

    void remove(const T& value)
    {
        _data.erase( std::remove( _data.begin(), _data.end(), value ) );
    }

    Array() {}

    Array(const std::vector<T>& d)
    {
        _data.resize(d.size());
        std::copy(d.begin(),d.end(),_data.begin());
    }

    Array& operator<<(const T& v)
    {
        _data.push_back(v);
        return *this;
    }

    const T& valueOrEmpty(size_t index) const
    {
        static const T _invalid = T();
        return index < _data.size() ? _data.at(index) : _invalid;
    }

    const T& valueOrDefault(size_t index, const T& def) const
    {
        return index < _data.size() ? _data.at(index) : def;
    }

    bool contain(const T& v) const
    {
        auto it = std::find(_data.begin(), _data.end(), v);
        return it != _data.end();
    }

protected:
    Collection _data;
};


#endif  //_JULIUS_VECTOR_EXTENSION_INCLUDE_H_
