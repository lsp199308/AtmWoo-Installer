/*
Copyright (c) 2017-2018 Adubbz

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "nx/fs.hpp"

#include <cstring>
#include "util/error.hpp"

namespace nx::fs
{
    // IFile

    IFile::IFile(FsFile& file)
    {
        m_file = file;
    }

    IFile::~IFile()
    {
        fsFileClose(&m_file);
    }

    void IFile::Read(u64 offset, void* buf, size_t size)
    {
        u64 sizeRead;
        ASSERT_OK(fsFileRead(&m_file, offset, buf, size, FsReadOption_None, &sizeRead), "Failed to read file");

        if (sizeRead != size)
        {
            std::string msg = "Size read " + std::string("" + sizeRead) + " doesn't match expected size " + std::string("" + size);
            THROW_FORMAT(msg.c_str());
        }
    }

    s64 IFile::GetSize()
    {
        s64 sizeOut;
        ASSERT_OK(fsFileGetSize(&m_file, &sizeOut), "Failed to get file size");
        return sizeOut;
    }

    // End IFile

    // IDirectory
    IDirectory::IDirectory(FsDir& dir) 
    {
        m_dir = dir;
    }

    IDirectory::~IDirectory()
    {
        fsDirClose(&m_dir);
    }

    void IDirectory::Read(s64 inval, FsDirectoryEntry* buf, size_t numEntries)
    {
        ASSERT_OK(fsDirRead(&m_dir, &inval, numEntries, buf), "Failed to read directory");

        /*if (entriesRead != numEntries)
        {
            std::string msg = "Entries read " + std::string("" + entriesRead) + " doesn't match expected number " + std::string("" + numEntries);
            THROW_FORMAT(msg);
        }*/
    }

    u64 IDirectory::GetEntryCount()
    {
        s64 entryCount = 0;
        ASSERT_OK(fsDirGetEntryCount(&m_dir, &entryCount), "Failed to get entry count");
        return entryCount;
    }

    // End IDirectory

    IFileSystem::IFileSystem() {}

    IFileSystem::~IFileSystem()
    {
        this->CloseFileSystem();
    }

    Result IFileSystem::OpenSdFileSystem()
    {
        ASSERT_OK(fsOpenSdCardFileSystem(&m_fileSystem), "Failed to mount sd card");
        return 0;
    }

    void IFileSystem::OpenFileSystemWithId(std::string path, FsFileSystemType fileSystemType, u64 titleId)
    {
        Result rc = 0;
        if (path.length() >= FS_MAX_PATH)
            THROW_FORMAT("Directory path is too long!");

        // libnx expects a FS_MAX_PATH-sized buffer
        path.reserve(FS_MAX_PATH);

        std::string errorMsg = "Failed to open file system with id: " + path;
        rc = fsOpenFileSystemWithId(&m_fileSystem, titleId, fileSystemType, path.c_str());

        if (rc == 0x236e02)
            errorMsg = "File " + path + " is unreadable! You may have a bad dump, fs_mitm may need to be removed, or your firmware version may be too low to decrypt it.";
        else if (rc == 0x234c02)
            errorMsg = "Failed to open filesystem. Make sure your signature patches are up to date and set up properly!";

        ASSERT_OK(rc, errorMsg.c_str());
    }

    void IFileSystem::CloseFileSystem()
    {
        fsFsClose(&m_fileSystem);
    }

    IFile IFileSystem::OpenFile(std::string path)
    {
        if (path.length() >= FS_MAX_PATH)
            THROW_FORMAT("Directory path is too long!");

        // libnx expects a FS_MAX_PATH-sized buffer
        path.reserve(FS_MAX_PATH);

        FsFile file;
        ASSERT_OK(fsFsOpenFile(&m_fileSystem, path.c_str(), FsOpenMode_Read, &file), ("Failed to open file " + path).c_str());
        return IFile(file);
    }

    IDirectory IFileSystem::OpenDirectory(std::string path, int flags)
    {
        // Account for null at the end of c strings
        if (path.length() >= FS_MAX_PATH)
            THROW_FORMAT("Directory path is too long!");

        // libnx expects a FS_MAX_PATH-sized buffer
        path.reserve(FS_MAX_PATH);

        FsDir dir;
        ASSERT_OK(fsFsOpenDirectory(&m_fileSystem, path.c_str(), flags, &dir), ("Failed to open directory " + path).c_str());
        return IDirectory(dir);
    }
}        
