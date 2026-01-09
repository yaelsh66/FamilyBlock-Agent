#pragma once
#include <unordered_set>
#include <string>

class HostsBlocker
{
private:
	std::unordered_set<std::string> websiteNames;


public:
	void setBlockedWebsites(const std::unordered_set<std::string>& set);
	std::string getHostsPath();
	void applyBlock();
	void removeBlock();

};

