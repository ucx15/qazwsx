#include <iostream>
#include <string>
#include <fstream>

#include "nlohmann_json/json.hpp" // downloaded from https://github.com/nlohmann/json

#include "settings.hpp"


using json = nlohmann::json;


Settings::Settings() {
	FAR_CLIP = 1E8F;  // Far clip
	NEAR_CLIP = 1E-3F;  // Near clip

	EPSILON  = 1E-8F; // Epsilon

	AOV = 45;         // Angle of View

	W = 640;
	H = 480;
	ASR = (float) W/H;

	FPS = 240;

	UPDATE_TIME = 2.f;  // in sec
	DEBUG = true;
};

Settings::~Settings() {
}


bool Settings::loadFromJSON(const char* path) {
	std::ifstream file(path);
	if (!file.is_open()) {
		std::cerr << "Failed to open scene file: " << path << std::endl;
		return false;
	}

	json data;

	try {
		file >> data;
	}

	catch (const json::parse_error& e) {
		std::cerr << "JSON parse error: " << e.what() << std::endl;
		return false;
	}

	if (data.is_null() || !data.is_object()) {
		return false;
	}

	FAR_CLIP = data.value("FAR_CLIP", FAR_CLIP);
	NEAR_CLIP = data.value("NEAR_CLIP", NEAR_CLIP);
	EPSILON  = data.value("EPSILON", EPSILON);

	AOV = data.value("AOV", AOV);

	W = data.value("W", W);
	H = data.value("H", H);
	ASR = (float) W/H;

	FPS = data.value("FPS", FPS);
	UPDATE_TIME = data.value("UPDATE_TIME", UPDATE_TIME);
	DEBUG = data.value("DEBUG", DEBUG);


	std::cout << "\nSettings Loaded from " << path << ":\n"
			  << "\tFAR_CLIP: " << FAR_CLIP << "\n"
			  << "\tNEAR_CLIP: " << NEAR_CLIP << "\n"
			  << "\tEPSILON: "  << EPSILON  << "\n"
			  << "\tAOV: "      << AOV      << "\n"
			  << "\tW: "        << W        << "\n"
			  << "\tH: "        << H        << "\n"
			  << "\tASR: "      << ASR      << "\n"
			  << "\tFPS: "      << FPS      << "\n"
			  << "\tUPDATE_TIME: " << UPDATE_TIME << "\n"
			  << "\tDEBUG: "    << (DEBUG ? "true" : "false") << "\n"
			  << std::endl;

	return true;
}

bool Settings::saveToJSON(const char* path) {
	json data;

	data["FAR_CLIP"] = FAR_CLIP;
	data["NEAR_CLIP"] = NEAR_CLIP;
	data["EPSILON"] = EPSILON;
	data["AOV"] = AOV;
	data["W"] = W;
	data["H"] = H;
	data["ASR"] = ASR;
	data["FPS"] = FPS;
	data["UPDATE_TIME"] = UPDATE_TIME;

	std::ofstream file(path);
	if (!file.is_open()) {
		std::cerr << "Failed to open scene file for writing: " << path << std::endl;
		return false;
	}

	file << data.dump(4);  // Write JSON with 4-space indentation
	return true;
}
