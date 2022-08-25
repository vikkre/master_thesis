#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <fstream>

#include "init_exception.h"
#include "math/vector.h"
#include "math/rotation.h"

#define RawInputEntry std::pair<std::string, std::vector<std::string>>

class InputEntry {
	public:
		InputEntry(const std::string& name);
		~InputEntry();

		void insert(const RawInputEntry& rawInputEntry);
		bool keyExists(const std::string& key) const;
		
		int getInt(const std::string& key) const;
		float getFloat(const std::string& key) const;
		Vector3f getVector3f(const std::string& key) const;
		Rotation getRotation(const std::string& key) const;

		std::string name;
	private:
		std::unordered_map<std::string, std::vector<std::string>> data;
};

class InputParser {
	public:
		InputParser(const std::string& path);
		~InputParser();

		void parse();
		unsigned int size() const;
		const InputEntry& getInputEntry(unsigned int index) const;

	private:
		RawInputEntry parseInputEntry(std::ifstream& istream);

		std::string path;
		std::vector<InputEntry> inputEntries;
};
