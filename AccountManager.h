#pragma once
#include <vector>
#include <string>

class AccountManager
{
public:
    struct Account {
        std::string mail;
        std::string username;

        bool operator==(const Account& other) const {
            return mail == other.mail && username == other.username;
        }
    };

private:
    std::vector<Account> AccountList;
    std::string currGitUsername;
    const char* filePath;

public:
    bool setup(const char* AccountfilePath);
    void LoadAccount(Account acc);
    std::vector<Account> getAccounts();
    void addAccount(const std::string& mail, const std::string& username);
    void deleteAccount(const Account& acc);
    void saveToFile();
    std::string getCurrGitUsername();

    void readFromFile();
};
