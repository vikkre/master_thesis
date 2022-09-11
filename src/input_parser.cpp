#include "input_parser.h"

////////////////
// InputEntry //
////////////////

InputEntry::InputEntry(const std::string& name)
:name(name), data() {}

InputEntry::~InputEntry() {}

void InputEntry::insert(const RawInputEntry& rawInputEntry) {
	data.insert(rawInputEntry);
}

bool InputEntry::keyExists(const std::string& key) const {
	return data.count(key) > 0;
}

Rotation InputEntry::getRotation(const std::string& key) const {
	return Rotation(Vector3f({
		std::stof(data.at(key).at(0)),
		std::stof(data.at(key).at(1)),
		std::stof(data.at(key).at(2))
	}), std::stof(data.at(key).at(3)));
}


/////////////////
// InputParser //
/////////////////

InputParser::InputParser(const std::string& path)
:path(path), inputEntries() {}

InputParser::~InputParser() {}

void InputParser::parse() {
	std::ifstream istream(path);

	if (istream.fail())	throw InitException("InputParser", std::string("failed to open file \"") + path + "\"!");

	InputEntry currentInputEntry("");

	while(istream.good()) {
		char c = istream.get();
		if (c == '\n') {
			continue;
		} else if (c == '#') {
			std::string tmp;
			getline(istream, tmp);
		} else if (c == '\t') {
			RawInputEntry rawInputEntry = parseInputEntry(istream);
			currentInputEntry.insert(rawInputEntry);
		} else if (c != ' ') {
			istream.unget();
			if (currentInputEntry.name != "") inputEntries.push_back(currentInputEntry);

			std::string inputEntryName;
			std::getline(istream, inputEntryName);
			currentInputEntry = InputEntry(inputEntryName);
		}
	}

	if (currentInputEntry.name != "") inputEntries.push_back(currentInputEntry);

	istream.close();
}

RawInputEntry InputParser::parseInputEntry(std::ifstream& istream) {
	std::string name;
	std::getline(istream, name, '(');

	std::vector<std::string> values;
	std::string current = "";
	while(istream.good()) {
		char c = istream.get();

		if (c == ')') {
			istream.get();
			break;
		} else if (c == ',') {
			values.push_back(current);
			current = "";
		} else if (c != ' ') {
			current += c;
		}
	}

	values.push_back(current);

	return {name, values};
}

unsigned int InputParser::size() const {
	return inputEntries.size();
}

const InputEntry& InputParser::getInputEntry(unsigned int index) const {
	return inputEntries.at(index);
}
