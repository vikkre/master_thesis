#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <sstream>

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

		template <typename T>
		T get(const std::string& key, size_t index=0) const {
			T value;
			std::stringstream(data.at(key).at(index)) >> value;
			return value;
		}

		template <size_t s, typename T>
		Vector<s, T> getVector(const std::string& key) const {
			Vector<s, T> vec;

			for (size_t i = 0; i < s; ++i) {
				vec[i] = get<T>(key, i);
			}

			return vec;
		}

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

