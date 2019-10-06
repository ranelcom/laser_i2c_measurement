
//
// Logger.h
// Kyle Shores
// 11/19/2016
//
// Defines the interface for writing data to a log file.

#include <string>
#include <iostream>
#include <fstream>
#include <chrono>
#include <ctime>
#include <exception>
#include <boost/filesystem.hpp>
#include <vector>
#include <iomanip>

namespace fs = boost::filesystem;

class Logger
{
public:
    Logger();
    ~Logger();

    // all open functions return true on a successful open or true if the logger is already open
    // false otherwise
    bool open();
    bool open(const char* pPath);
    bool open(const fs::path pPath);

    // write a single message
    void write_log(const std::string& pMessage);
    // write a group of messages
    void write_log(const std::vector<std::string>& pMessages);

    // return true if the stream was successfully closed or if the stream was already closed
    // false otherwise
    bool close();

private:
    bool create_and_open();

private:
    fs::path mDir;
    fs::path mFile;
    fs::path mFull_path;
    std::ofstream mStream;

    // default values
    std::string mDefault_dir = "./";
    std::string mDefault_file = "log.log";
};
