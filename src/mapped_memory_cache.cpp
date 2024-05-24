/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2021 Artem Pavlenko
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *****************************************************************************/

#if defined(MAPNIK_MEMORY_MAPPED_FILE)

// mapnik
#include <mapnik/debug.hpp>
#include <mapnik/util/fs.hpp>
#include <mapnik/mapped_memory_cache.hpp>

#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore.hpp>
#include <boost/assert.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/file_mapping.hpp>
MAPNIK_DISABLE_WARNING_POP

#include <locale>
#include <codecvt>

namespace mapnik {

template class singleton<mapped_memory_cache, CreateStatic>;

void mapped_memory_cache::clear()
{
#ifdef MAPNIK_THREADSAFE
    std::lock_guard<std::mutex> lock(mutex_);
#endif
    return cache_.clear();
}

bool mapped_memory_cache::insert(std::string const& uri, mapped_region_ptr mem)
{
#ifdef MAPNIK_THREADSAFE
    std::lock_guard<std::mutex> lock(mutex_);
#endif
    return cache_.emplace(uri, mem).second;
}

bool mapped_memory_cache::remove(std::string const& key)
{
#ifdef MAPNIK_THREADSAFE
    std::lock_guard<std::mutex> lock(mutex_);
#endif
    return cache_.erase(key) > 0;
}

boost::optional<mapped_region_ptr> mapped_memory_cache::find(std::string const& uri, bool update_cache)
{
#ifdef MAPNIK_THREADSAFE
    std::lock_guard<std::mutex> lock(mutex_);
#endif

    using iterator_type = std::unordered_map<std::string, mapped_region_ptr>::const_iterator;
    boost::optional<mapped_region_ptr> result;
    iterator_type itr = cache_.find(uri);
    if (itr != cache_.end())
    {
        result.reset(itr->second);
        return result;
    }

    if (mapnik::util::exists(uri))
    {
        try
        {
            // boost::interprocess::file_mapping mapping(uri.c_str(), boost::interprocess::read_only);
            // UTF-8字符串
            std::string utf8_string = uri;
            // 创建用于转换的wstring_convert对象
            std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
            // 将UTF-8转换为UTF-16
            std::wstring utf16_string = converter.from_bytes(utf8_string);
            boost::interprocess::file_mapping mapping(utf16_string.c_str(),boost::interprocess::read_only);
            mapped_region_ptr region(
              std::make_shared<boost::interprocess::mapped_region>(mapping, boost::interprocess::read_only));
            result.reset(region);
            if (update_cache)
            {
                cache_.emplace(uri, *result);
            }
            return result;
        }
        catch (std::exception const& ex)
        {
            MAPNIK_LOG_ERROR(mapped_memory_cache)
              << "Error loading mapped memory file: '" << uri << "' (" << ex.what() << ")";
        }
    }
    /*
    else
    {
        MAPNIK_LOG_WARN(mapped_memory_cache) << "Memory region does not exist file: " << uri;
    }
    */
    return result;
}

} // namespace mapnik

#endif
