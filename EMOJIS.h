#pragma once

#include <unordered_map>
#include <string>

#ifndef EMOJIS_H
#define EMOJIS_H

std::unordered_map<std::string, std::string> emoji = {
	{"player", "🧍"},
	{"coin", "🪙"},
	{"enemy", "👾"},
	{"hp", "❤️"},
	{"shield", "🛡️"}
};

#endif // EMOJIS_H
