#ifndef TELEMETRY_COLLECTOR_H_
#define TELEMETRY_COLLECTOR_H_

#if defined(WIN32)
#include <string>
#include <fstream>
#include <vector>
#include <iomanip>

#include <cstdlib>
#include <stdio.h>
#include <windows.h>

#include <XPLMPlanes.h>

// Telemetry collector (WIN only, for now)
class TelemetryCollector {
public:
	enum eTelemetryType { LOGIC = 0, DRAWING = 1};
public:
	TelemetryCollector(const std::string& inProcessID, 
						const std::string& inPluginShortName,
						const std::string& inPluginSignature,
						const std::string& inPluginName,
						const std::string& inOutputPrefix = "",
						eTelemetryType inType = eTelemetryType::LOGIC);
	~TelemetryCollector();

	void start();
	void stop();
	void write(int* inCurrentFrame = NULL);

	bool isEnabled() const;
private:
	double mHFrequency;				// Per microsecond
	double mElapsed;
	bool mIsFrequencyInitialized;
	__int64 mTimeStartPoint;

	eTelemetryType mType;
	std::string mProcessID;
	bool mEnabled;
	std::ofstream mOutputStream;
	std::vector<std::string> mPluginIdentifiers;
	unsigned long long mFrameCounter;

	bool isFileExists(const std::string& inPath);

	void checkTelemetryProductConf(const std::string&, bool*, bool*, bool*);
	void checkTelemetryPluginConf(const std::string&, bool*, bool*, bool*);
	void checkTelemetryConf(const std::string&, bool*, bool*, bool*, std::string inSuffix = "");
private:
	static std::string mAllTelemetryIgnitorID;
	static std::string mLogicTelemetryIgnitorID;
	static std::string mDrawingTelemetryIgnitorID;
private:
	TelemetryCollector(const TelemetryCollector&);
	TelemetryCollector& operator=(const TelemetryCollector&);
};


#endif

#endif // TELEMETRY_COLLECTOR_H