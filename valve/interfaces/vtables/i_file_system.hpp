#pragma once

#include "../../../sdk/vfunc/vfunc.hpp"

class i_file_system
{
public:
    bool exists(const char* file_name, const char* path_id)
    {
        return vmt::call_virtual<bool>(this, 21, file_name, path_id);
    }
};
