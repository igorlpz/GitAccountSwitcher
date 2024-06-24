#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include "AccountManager.h"
#include "filesystem"
#include "windows.h"
#include <cstdio>
#include <cstring>

void AccountManager::LoadAccount(Account acc)
{
    std::string command_name = "git config --global user.name \"" + acc.username + "\"";
    std::string command_email = "git config --global user.email \"" + acc.mail + "\"";

    std::system(command_name.c_str());
    std::system(command_email.c_str());

    currGitUsername = acc.username.c_str();
}

std::vector<AccountManager::Account> AccountManager::getAccounts()
{
    return AccountManager::AccountList;
}

void AccountManager::saveToFile()
{

    std::ofstream out(filePath, std::ofstream::out | std::ofstream::trunc); // Open file for writing and truncate (erase content)

    //This will replace the lines, ignoring if there are already accounts
    for (const auto& acc : AccountList) {
        out << acc.mail << ":" << acc.username << std::endl;
    }
    out.close();
}

std::string AccountManager::getCurrGitUsername()
{
    return currGitUsername;
}

void AccountManager::addAccount(const std::string& mail, const std::string& username)
{
    Account acc;
    acc.mail = mail;
    acc.username = username;

    //If empty don't add 
    if (mail.empty() || username.empty()) {
        return;
    }

    AccountList.push_back(acc);
    saveToFile();
}

void AccountManager::deleteAccount(const Account& acc)
{
    //Deleting from the list
    auto it = std::find(AccountList.begin(), AccountList.end(), acc);
    if (it != AccountList.end()) {
        AccountList.erase(it);
    }

    saveToFile();
}

void AccountManager::readFromFile()
{
    std::ifstream in(filePath);
    std::string LineData;

    AccountList.clear();
    while (std::getline(in, LineData)) {
        if (LineData.empty()) {
            break;
        }

        size_t separator = LineData.find(':');
        if (separator == std::string::npos) {
            continue; // Skip lines without ':'
        }

        std::string mail = LineData.substr(0, LineData.find(":"));
        std::string username = LineData.substr(mail.size() + 1);


        addAccount(mail, username);
    }
    in.close();
}

bool isGitInstalled() {
    // Get the PATH environment variable
    const char* pathEnvVar = std::getenv("PATH");
    if (!pathEnvVar) {
        std::cerr << "PATH environment variable not found." << std::endl;
        return false;
    }

    // Split PATH into individual directories
    std::string pathEnv = pathEnvVar;
    size_t start = 0;
    size_t end = pathEnv.find(';');
    while (end != std::string::npos) {
        std::string dir = pathEnv.substr(start, end - start);
        if (std::filesystem::exists(dir + "\\git.exe")) { // On Windows
            return true;
        }
        start = end + 1;
        end = pathEnv.find(';', start);
    }

    return false;
}

std::string reqCurrentGitUsername() {
    // Use popen directly
    FILE* pipe = nullptr;
    pipe = _popen("git config --global user.name", "r"); // For Windows

    if (!pipe) {
        return "Error";
    }

    constexpr int MAX_BUFFER = 128;
    char buffer[MAX_BUFFER];
    std::string result = "";

    while (!feof(pipe)) {
        if (fgets(buffer, MAX_BUFFER, pipe) != nullptr)
            result += buffer;
    }

    _pclose(pipe);

    // Remove newline characters from the result
    result.erase(std::remove(result.begin(), result.end(), '\n'), result.end());
    result.erase(std::remove(result.begin(), result.end(), '\r'), result.end());

    return result;
}

bool AccountManager::setup(const char* AccountfilePath)
{
    //Allocating console then hiding to fix shit cmd popups when loading
    AllocConsole();
    ShowWindow(GetConsoleWindow(), false);

    currGitUsername = reqCurrentGitUsername();
    filePath = AccountfilePath;
    std::filesystem::path dirPath(AccountfilePath);
    dirPath.remove_filename();  // Remove the file name to get the directory path

    // Check if directory exists, create if it doesn't
    std::error_code ec;
    if (!std::filesystem::exists(dirPath)) {
        std::filesystem::create_directories(dirPath, ec);
    }

    // Check if file exists, create if it doesn't
    std::ofstream outfile(AccountfilePath, std::ios_base::app);

    outfile.close();

    if (!isGitInstalled()){
        return false;
    }
    readFromFile();
    return true;
}
