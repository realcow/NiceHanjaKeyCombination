#pragma once

#include <boost/filesystem.hpp>

#include <Windows.h>
#include <string>

class ProcessActivator
{
public:
    ProcessActivator(const boost::filesystem::path& processPath)
        : processPath(processPath)
    {
        processImageName = processPath.filename().wstring();
    }

    void operator()(WORD key);

private:
    std::wstring processImageName;
    boost::filesystem::path processPath;
};
