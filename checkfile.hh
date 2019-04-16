#ifndef _CHECK_FILE_HH
#define _CHECK_FILE_HH
#include <string>
#include <sys/stat.h>

enum class enFILESTATUS
{
    DIRECTORY,
    FILE_NOT_EMPTY,
    FILE_EMPTY,
    NOT_EXIST,
    FILE_EXIST
};

enFILESTATUS checkfile(const std::string& theFileName)
{
    struct stat statinfo;
    if (stat(theFileName.c_str(), &statinfo) == 0)
    {
        if ((statinfo.st_mode & S_IFMT) == S_IFDIR)
            return enFILESTATUS::DIRECTORY;

        if (statinfo.st_size == 0)
            return enFILESTATUS::FILE_EMPTY;
    }
    else
    {
        if (errno == ENOENT)
            return enFILESTATUS::NOT_EXIST;
    }

    return enFILESTATUS::FILE_NOT_EMPTY;
}
#endif
