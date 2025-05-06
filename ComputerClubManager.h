// ComputerClubManager.h : включаемый файл для стандартных системных включаемых файлов
// или включаемые файлы для конкретного проекта.

#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <regex>
#include <iomanip>

struct Time {
	int hour, minute;

	Time() {
		hour = 0;
		minute = 0;
	}

	static Time StringToTime(const std::string& str) {
		//ex
		if (!regex_match(str, std::regex(R"(\d{2}:\d{2})")))
			throw std::runtime_error("Invalid time format: " + str);

		Time t;
		sscanf(str.c_str(), "%d:%d", &t.hour, &t.minute);

		//ex
		if (t.hour < 0 || t.hour > 23 || t.minute < 0 || t.minute > 59)
			throw std::runtime_error("Invalid time value: " + str);

		return t;
	}

	std::string TimeToString() const {
		std::ostringstream oss;
		oss << std::setfill('0') << std::setw(2) << hour << ":" << std::setw(2) << minute;
		return oss.str();
	}

	int minutesSinceMidnight() const {
		return hour * 60 + minute;
	}

	static int duration(const Time& from, const Time& to) {
		return to.minutesSinceMidnight() - from.minutesSinceMidnight();
	}

	bool operator<(const Time& other) const {
		return hour < other.hour || (hour == other.hour && minute < other.minute);
	}

	bool operator>(const Time& other) const {
		return hour > other.hour || (hour == other.hour && minute > other.minute);
	}

	bool operator>=(const Time& other) const {
		return hour >= other.hour || (hour == other.hour && minute >= other.minute);
	}

	bool operator==(const Time& other) const {
		return hour == other.hour && minute == other.minute;
	}	
};

std::pair<Time, Time> parseTime(const std::string& line) {
	std::istringstream ss(line);
	std::string startStr, endStr;
	ss >> startStr >> endStr;

	// ex
	if (!ss || !ss.eof()) throw std::runtime_error("Invalid work hours line");

	return { Time::StringToTime(startStr), Time::StringToTime(endStr) };
}

std::string minsToTime(int totalMinutes) {
	int hours = totalMinutes / 60;
	int minutes = totalMinutes % 60;
	std::ostringstream oss;
	oss << std::setfill('0') << std::setw(2) << hours << ":" << std::setw(2) << minutes;
	return oss.str();
}


struct Event {
	Time time;
	int id;
	std::vector<std::string> args;
};

Event parseEvent(const std::string& line) {
	std::istringstream ss(line);
	std::string timeStr;
	int id;
	ss >> timeStr >> id;

	// ex 
	if (!ss) throw std::runtime_error("Invalid event line: " + line);

	std::vector<std::string> args;
	std::string arg;
	while (ss >> arg) {
		args.push_back(arg);
	}

	return Event{ Time::StringToTime(timeStr), id, args };
}


struct ClientInfo {
	unsigned int table = 0;
	Time sitTime;
	bool atTable = false;
};


bool isValidClientName(const std::string& name) {
	return std::regex_match(name, std::regex(R"(^[a-z0-9_-]+$)"));
}

