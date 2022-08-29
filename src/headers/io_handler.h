#pragma once

#include <vector>
#include <windows.h>    // have to include windows.h to build fileapi.h https://stackoverflow.com/questions/4845198/fatal-error-no-target-architecture-in-visual-studio
#include <fileapi.h>
#include "common.h"

//Returns the last Win32 error, in string format. Returns an empty string if there is no error.
static std::string GetLastErrorAsString()
{
    // //Get the error message ID, if any.
    // DWORD errorMessageID = ::GetLastError();
    // if(errorMessageID == 0) {
    //     return std::string(); //No error message has been recorded
    // }
    
    // LPSTR messageBuffer = nullptr;

    // //Ask Win32 to give us the string version of that message ID.
    // //The parameters we pass in, tell Win32 to create the buffer that holds the message for us (because we don't yet know how long the message string will be).
    // size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
    //                              NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);
    
    // //Copy the error message into a std::string.
    // std::string message(messageBuffer, size);
    
    // //Free the Win32's string's buffer.
    // LocalFree(messageBuffer);
            
    // return message;
    return "";
}

static bool file_exists(const std::string& file_path)
{
  DWORD dwAttrib = GetFileAttributes(file_path.c_str());

  return (dwAttrib != INVALID_FILE_ATTRIBUTES && 
         !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

class io_handler{
    HANDLE _file;
    std::string name;
    int _no_of_reads = 0;
public:
    // io_handler() {}
    io_handler(const std::string& file_path) : name(file_path) {
        // if (_file != INVALID_HANDLE_VALUE) CloseHandle(_file);
        _file = CreateFileA(file_path.c_str(),
                GENERIC_READ | GENERIC_WRITE,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                NULL,
                OPEN_ALWAYS,
                FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH | FILE_FLAG_NO_BUFFERING,
                NULL
            );
        
        if (_file == INVALID_HANDLE_VALUE)
            throw std::exception("File opening failed.\n");
    }

    int no_of_reads() {
        return _no_of_reads;
    }

    void reset_reads() {
        _no_of_reads = 0;
    }

    void seekg(size_t offset) {
        auto ret = SetFilePointer(_file,
            offset,
            NULL,
            FILE_BEGIN
        );

        if (ret == INVALID_SET_FILE_POINTER) {
            auto dw = GetLastErrorAsString();
            std::cout << dw << "\n";
            throw std::exception("Seekg failed\n");
        }
            
    }

    void seekg_to_end() {
        if (!SetEndOfFile(_file)) {
            auto dw = GetLastErrorAsString();
            std::cout << dw << "\n";
            throw std::exception("Seek to end failed\n");
        }
            
    }

    // reads size bytes at and stores the into buffer
    void read(std::vector<int32_t>& buffer, size_t size) {
        // we will always read multiples of 512

        size_t expected_size = buffer.size() + size;
        size_t batch_num = size * sizeof(int32_t) / BLOCK_SIZE + 1 * ((size * sizeof(int32_t)) % BLOCK_SIZE != 0);
        size_t actual_read_size = batch_num * BLOCK_SIZE; // in bytes
        buffer.resize(buffer.size() + actual_read_size);
        
        for (size_t i = 0; i < batch_num; ++i) {
            _no_of_reads++;
            auto ret = ReadFile(
                _file,
                reinterpret_cast<char*>(buffer.data() + buffer.size() - actual_read_size + i * BLOCK_SIZE / 4), // BLOCK_SIZE is in bytes
                BLOCK_SIZE,
                NULL,
                NULL
            );

            if (!ret){
                auto dw = GetLastErrorAsString();
                std::cout << dw << "\n";
                throw std::exception("Failed buff reading.\n");
            }
        }

        if (buffer.size() > expected_size)
            buffer.resize(expected_size);
    }

    void write(std::vector<int32_t>& buffer) {
        size_t old_size = buffer.size();
        
        while (buffer.size() % BLOCK_SIZE)
            buffer.push_back(0);

        auto ret = WriteFile(
            _file,
            reinterpret_cast<char*>(buffer.data()),
            buffer.size(),
            NULL,
            NULL
        );

        buffer.resize(old_size);
    }

    ~io_handler() {
        CloseHandle(_file);
    }

};