# pragma once

class Settings {
public:
	float FAR_CLIP; 	// Far clip
	float NEAR_CLIP; 	// Near clip
	float EPSILON; 		// Epsilon
	float AOV; 			// Angle of View

	int W;
	int H;

	float ASR;

	int FPS;

	float UPDATE_TIME;  // in sec
	bool DEBUG;

public:
	Settings();
	~Settings();

public:
	bool loadFromJSON(const char* path);
	bool saveToJSON(const char* path);
};
