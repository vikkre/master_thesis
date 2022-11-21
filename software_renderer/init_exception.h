#pragma once

#include <cstring>
#include <string>
#include <sstream>
#include <stdexcept>


class InitException: public std::runtime_error {
	public:
		InitException(const std::string& system, const std::string& error_msg);
		virtual const char* what() const noexcept;

	private:
		const std::string system;
		const std::string error_msg;

		char msg[255];
};
