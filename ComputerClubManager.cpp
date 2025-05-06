// ComputerClubManager.cpp: определяет точку входа для приложения.
// ComputerClubManager\build> cmake --build .
// ComputerClubManager\build> ./Debug/task.exe ..\tests\test_file.txt

#include "ComputerClubManager.h"

#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <queue>


int main(int argc, char* argv[])
{
    std::string filename = argv[1];
    std::ifstream input(filename);

    if (!input.is_open()) {
        std::cerr << "Error: Cannot open file " << filename << std::endl;
        exit(1);
    }

    std::string line;
    std::unordered_map<std::string, ClientInfo> clientsMap;   // client name -> info sruct
    std::unordered_map<unsigned int, std::string> tablesMap;    // table num -> client name
    std::queue<std::string> waitQueue;

    //std::unordered_map<std::string, Time> clientSitTime;
    std::unordered_map<unsigned int, unsigned int> tableRevenue;        // table number -> total revenue
    std::unordered_map<unsigned int, unsigned int> tableUsageMinutes;   // table number -> total occupied minutes

    // tabels count
    std::getline(input, line);
    unsigned int tableCount = stoi(line);

    // work time
    std::getline(input, line);
    Time openTime = parseTime(line).first;
    Time closeTime = parseTime(line).second;

    // hour cost
    std::getline(input, line);
    unsigned int price = stoi(line);

    // events
    std::vector<Event> events;
    std::string lastTimeStr;
    Time lastTime = Time();

    int lineNumber = 4;
    while (std::getline(input, line)) {
        try {
            Event e = parseEvent(line);
            if (e.time < lastTime) {
                std::cerr << "Invalid format at line " << lineNumber << ": " << line << std::endl;
                std::cerr << "Events must be in chronological order" << std::endl;
                exit(1);
            }
            lastTime = e.time;
            events.push_back(e);
        }
        catch (const std::exception& ex) {
            std::cerr << "Invalid format at line " << lineNumber << ": " << line << std::endl;
            std::cerr << ex.what() << std::endl;
            exit(1);
        }
        ++lineNumber;
    }

    std::cout << openTime.TimeToString() << std::endl;
    for (const auto& e : events) {
        std::cout << e.time.TimeToString() << " " << e.id;
        for (const std::string& a : e.args) std::cout << " " << a;
        std::cout << std::endl;

        // client entry
        if (e.id == 1) {
            if (e.args.size() != 1) {
                std::cerr << "Invalid args for event id=1\n";
                continue;
            }
            const std::string& name = e.args[0];
            if (!isValidClientName(name)) {
                std::cerr << "Invalid client name format at line " << lineNumber << ": " << line << std::endl;
                exit(1);
            }
            // wromg time
            if (e.time < openTime || e.time >= closeTime) {
                std::cout << e.time.TimeToString() << " 13 " << "NotOpenYet" << std::endl;
                continue;
            }
            // already in club
            if (clientsMap.count(name)) {
                std::cout << e.time.TimeToString() << " 13 " << "YouShallNotPass" << std::endl;
                continue;
            }
            clientsMap[name] = ClientInfo{ 0, {}, false };

        }
        // client sit
        else if (e.id == 2) {
            if (e.args.size() != 2) {
                std::cerr << "Invalid args for event id=2\n";
                continue;
            }
            const std::string& name = e.args[0];
            if (!isValidClientName(name)) {
                std::cerr << "Invalid client name format at line " << lineNumber << ": " << line << std::endl;
                exit(1);
            }
            int table = std::stoi(e.args[1]);
            if (table < 1 || table > tableCount) {
                std::cerr << "Invalid table num at line " << lineNumber << ": " << line << std::endl;
                exit(1);
            }
            if (!clientsMap.count(name)) {
                std::cout << e.time.TimeToString() << " 13 " << "ClientUnknown" << std::endl;
                continue;
            }
            if (tablesMap.count(table)) {
                std::cout << e.time.TimeToString() << " 13 " << "PlaceIsBusy" << std::endl;
                continue;
            }
            if (clientsMap[name].table != 0) {
                tablesMap.erase(clientsMap[name].table);
            }
            clientsMap[name].table = table;
            clientsMap[name].sitTime = e.time;
            clientsMap[name].atTable = true;
            tablesMap[table] = name;
        }
        // client waiting
        else if (e.id == 3) {
            if (e.args.size() != 1) {
                std::cerr << "Invalid args for event id=3\n";
                continue;
            }
            const std::string& name = e.args[0];
            bool hasFree = false;
            for (int i = 1; i <= tableCount; ++i) {
                if (!tablesMap.count(i)) {
                    hasFree = true;
                    break;
                }
            }
            if (hasFree) {
                std::cout << e.time.TimeToString() << " 13 " << "ICanWaitNoLonger!" << std::endl;
                continue;
            }
            if (waitQueue.size() >= static_cast<size_t>(tableCount)) {
                std::cout << e.time.TimeToString() << " 11 " << name << "\n";
                clientsMap.erase(name);
                continue;
            }
            waitQueue.push(name);
        }
        // client leave
        else if (e.id == 4) {
            if (e.args.size() != 1) {
                std::cerr << "Invalid args for event id=4\n";
                continue;
            }
            const std::string& name = e.args[0];
            if (!clientsMap.count(name)) {
                std::cout << e.time.TimeToString() << " 13 " << "ClientUnknown" << std::endl;
                continue;
            }
            unsigned int table = clientsMap[name].table;
            if (table != 0 && clientsMap[name].atTable) {
                Time start = clientsMap[name].sitTime;
                unsigned int duration = Time::duration(start, e.time);
                unsigned int hours = (duration + 59) / 60;
                tableRevenue[table] += hours * price;
                tableUsageMinutes[table] += duration;
                tablesMap.erase(table);
                clientsMap[name].atTable = false;
            }

            if (!waitQueue.empty() && table != 0) {
                std::string next = waitQueue.front(); waitQueue.pop();
                clientsMap[next].table = table;
                clientsMap[next].sitTime = e.time;
                clientsMap[next].atTable = true;
                tablesMap[table] = next;
                std::cout << e.time.TimeToString() << " 12 " << next << " " << table << "\n";
            }


            clientsMap.erase(name);
        }
    }


    std::vector<std::string> clientNames;
    for (const auto& [name, info] : clientsMap) {
        if (info.atTable) {
            unsigned int duration = Time::duration(info.sitTime, closeTime);
            unsigned int hours = (duration + 59) / 60;
            tableRevenue[info.table] += hours * price;
            tableUsageMinutes[info.table] += duration;
        }
        clientNames.push_back(name);
    }
    std::sort(clientNames.begin(), clientNames.end());


    for (const auto& name : clientNames) {
        std::cout << closeTime.TimeToString() << " 11 " << name << "\n";
    }

    std::cout << closeTime.TimeToString() << std::endl;

    for (unsigned int i = 1; i <= tableCount; ++i) {
        std::cout << i << " " << tableRevenue[i] << " " << minsToTime(tableUsageMinutes[i]) << std::endl;
    }



    input.close();

	return 0;
}
