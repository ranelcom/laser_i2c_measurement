//
// Logger.cpp
// Kyle Shores
// 11/19/2016
//
// Implements the Logger class which writes data to a log file.

#include "Logger.hpp"    

//=============================================================
//                      Public
//=============================================================

Logger::Logger()
{
}

Logger::~Logger()
{
    close();
}

//==================================
//        Open functions
//==================================
bool Logger::open()
{
    if (!mStream.is_open())
    {
        // use the default file and directory
        mDir = mDefault_dir;
        mFile = mDefault_file;
        return create_and_open();
    }
    return true;
}

bool Logger::open(const char* pPath)
{
    if (!mStream.is_open())
    {
        fs::path path{pPath};
        mDir = path.parent_path();
        mFile = path.filename();
        return create_and_open();
    }
    return true;
}

bool Logger::open(const fs::path pPath)
{
    if (!mStream.is_open())
    {
        mDir = pPath.parent_path();
        mFile = pPath.filename();
        return create_and_open();
    }
    return true;
}

//==================================
//        Write log functions
//==================================

void Logger::write_log(const std::string& pMessage)
{
    try
    {
        // get the current time
        //std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
        //time_t tt = std::chrono::system_clock::to_time_t(now);

        // strip the newline passed back by ctime
        //auto tt_stripped = std::strtok( ctime(&tt), "\n");
        
        auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		auto tt_stripped = std::put_time(localtime(&now), "%d/%m/%Y %H:%M:%S");
        
        mStream << "[" << tt_stripped << "] : " << pMessage << std::endl;
    }
    catch (std::exception& e)
    {
        std::cerr << "Error while trying to log a message:\n" << e.what() << std::endl;
    }
}

void Logger::write_log(const std::vector<std::string>& pMessages)
{
    try
    {
        // get the current time
        std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
        time_t tt = std::chrono::system_clock::to_time_t(now);

        // strip the newline passed back by ctime
        auto tt_stripped = std::strtok( ctime(&tt), "\n");
        //auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		//auto tt_stripped = std::put_time(localtime(&now), "%d/%m/%Y %H:%M:%S");
        size_t whitespace = strlen(tt_stripped) + 5;
                                            //^ + 5 to align the text

        for (int i = 0; i < pMessages.size(); ++i)
        {
            if (i == 0)
                mStream << "[" << tt_stripped << "] : " << pMessages[i] << std::endl;
            else
                mStream << std::string(whitespace, ' ') << pMessages[i] << std::endl;
        }
    }
    catch (std::exception& e)
    {
        std::cerr << "Error while trying to log a message:\n" << e.what() << std::endl;
    }
}

//==================================
//        Close function
//==================================

bool Logger::close()
{
    try
    {
        if (mStream.is_open())
        {
            mStream.flush();
            mStream.close();
            return !mStream.is_open();
        }
        else
            return false;
    }
    catch (std::exception& e)
    {
            std::cerr << "Error: " << e.what() <<std::endl;
            return false;
    }
}

//=============================================================
//                      Private
//=============================================================

bool Logger::create_and_open()
{
    // open a log with the specified directory and file
    try
    { 
        // try to create any intermediate directories requested to host the file
        if (!fs::exists(mDir))
        {
            if (!fs::create_directories(mDir))
            {
                std::cerr << "Error creating directories to open the file." << std::endl;
                return false;
            }
        }

        // create the full path and open the file
        mFull_path = mDir / mFile;
        mStream.open(mFull_path.native(), std::ios_base::app | std::ios_base::in);

        if (!mStream.is_open())
        {
            std::cerr << "Error opening log file (" << mFull_path << "):\n" << std::endl;
            return false;
        }

        return true;
    }
    catch (std::exception& e)
    {
        std::cerr << "Error opening log file (" << mFile << "):\n" << e.what() << std::endl;
        return false;
    }
}
