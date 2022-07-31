#include "init_exception.h"

InitException::InitException(const std::string& system, const std::string& error_msg)
:runtime_error("Init exception"), system(system), error_msg(error_msg), msg("") {
	std::ostringstream sout;

	sout << runtime_error::what() << ": " << system << ": " << error_msg;

	std::strcpy(msg, sout.str().c_str());
}

const char* InitException::what() const noexcept {
	return msg;
}
