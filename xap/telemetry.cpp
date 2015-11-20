#include "telemetry.h"

#if defined(WIN32)

std::string TelemetryCollector::mAllTelemetryIgnitorID = "enableTelemetry";
std::string TelemetryCollector::mLogicTelemetryIgnitorID = "enableLogicTelemetry";
std::string TelemetryCollector::mDrawingTelemetryIgnitorID = "enableDrawingTelemetry";

TelemetryCollector::TelemetryCollector(const std::string& inProcessID, 
										const std::string& inPluginShortName,
										const std::string& inPluginSignature,
										const std::string& inPluginName,
										const std::string& inOutputPrefix,
										eTelemetryType inType) {
	mType = inType;
	mProcessID = inProcessID;

	mPluginIdentifiers.push_back(inPluginShortName);
	mPluginIdentifiers.push_back(inPluginSignature);
	mPluginIdentifiers.push_back(inPluginName);

	mEnabled = false; // We don't enable telemetry until we check that we need to
	mFrameCounter = 0;
	mIsFrequencyInitialized = false;

	bool drawingEnableFlagPresent = false;
	bool logicEnableFlagPresent = false;
	bool enableFlagPresent = false;

	char planeName[512];
	char planePath[512];
	XPLMGetNthAircraftModel(0, planeName, planePath);
	std::string productPath(planePath);
	std::size_t pos = productPath.find(planeName);
	productPath = productPath.substr(0, pos);

	checkTelemetryProductConf(productPath, &enableFlagPresent, &logicEnableFlagPresent, &drawingEnableFlagPresent);
	checkTelemetryPluginConf(productPath, &enableFlagPresent, &logicEnableFlagPresent, &drawingEnableFlagPresent);

	if (logicEnableFlagPresent && mType == eTelemetryType::LOGIC ||
		drawingEnableFlagPresent && mType == eTelemetryType::DRAWING ||
		enableFlagPresent) {

		mEnabled = true;

	}

	if (mEnabled) {
		if (mType == eTelemetryType::LOGIC) {
			mOutputStream.open(productPath + "/telemetry/logicTelemetry-" + inOutputPrefix + mPluginIdentifiers[1] + ".csv");
		} else  {
			mOutputStream.open(productPath + "/telemetry/drawingTelemetry-" + inOutputPrefix + mPluginIdentifiers[1] + ".csv");
		}
		// Writes header for output data
		mOutputStream << "[SASL TELEMETRY DATA]" << std::endl;
		mOutputStream << "Frame, "
					    << "Seconds Used, "
						<< "1 / Seconds Used, "
						<< "Comment" << std::endl;
	}

}

TelemetryCollector::~TelemetryCollector() {
	if (mEnabled) {
		mOutputStream.close();
	}
}

void TelemetryCollector::checkTelemetryProductConf(const std::string& inProductPath, bool* outAllEnabled, 
	bool* outLogicEnabled, bool* outDrawingEnabled) {

	checkTelemetryConf(inProductPath, outAllEnabled, outLogicEnabled, outDrawingEnabled);

}

void TelemetryCollector::checkTelemetryPluginConf(const std::string& inProductPath, bool* outAllEnabled,
	bool* outLogicEnabled, bool* outDrawingEnabled) {

	for (std::vector<std::string>::const_iterator it = mPluginIdentifiers.begin();
		it != mPluginIdentifiers.end(); ++it) {

		std::string cur_suffix = "For-" + *it;
		checkTelemetryConf(inProductPath, outAllEnabled, outLogicEnabled, outDrawingEnabled, cur_suffix);

	}

}

void TelemetryCollector::checkTelemetryConf(const std::string& inProductPath, bool* outAllEnabled, 
	bool* outLogicEnabled, bool* outDrawingEnabled, std::string inSuffix /* = "" */) {

	if (isFileExists(inProductPath + mAllTelemetryIgnitorID + inSuffix + ".txt")) {
		*outAllEnabled = true;
	} else if (isFileExists(inProductPath + mLogicTelemetryIgnitorID + inSuffix + ".txt")) {
		*outLogicEnabled = true;
	} else if (isFileExists(inProductPath + mDrawingTelemetryIgnitorID + inSuffix + ".txt")) {
		*outDrawingEnabled = true;
	}

}

bool TelemetryCollector::isEnabled() const {
	return mEnabled;
}

void TelemetryCollector::start() {
	if (mEnabled) {
		if (!mIsFrequencyInitialized) {
			LARGE_INTEGER uLI;
			if (!QueryPerformanceFrequency(&uLI)) {
				mOutputStream << "[SASL] Hardware querying error." << std::endl;
				mOutputStream.close();
				mEnabled = false;
				return;
			} else {
				mHFrequency = (double)(uLI.QuadPart);
			}
			mIsFrequencyInitialized = true;
		}

		LARGE_INTEGER uLI;
		QueryPerformanceCounter(&uLI);
		mTimeStartPoint = uLI.QuadPart;
	}
}

void TelemetryCollector::stop() {
	if (mEnabled) {
		LARGE_INTEGER uLI;
		QueryPerformanceCounter(&uLI);
		mElapsed = (double)(uLI.QuadPart - mTimeStartPoint) / mHFrequency;
	}
}

void TelemetryCollector::write(int* inCurrentFrame /* = NULL */) {
	if (mEnabled) {
		if (inCurrentFrame) {
			mOutputStream << *inCurrentFrame << ", ";
		} else {
			mOutputStream << ++mFrameCounter << ", ";
		}

		mOutputStream << std::setiosflags(std::ios::fixed | std::ios::showpoint) << std::setw(8) << std::setprecision(8) << mElapsed << ", "
			<< std::setw(8) << std::setprecision(2) << 1.0 / mElapsed << ", "
			<< std::setw(8) << mProcessID << std::endl;
	}
}

bool TelemetryCollector::isFileExists(const std::string& inPath) {
	FILE *f = fopen(inPath.c_str(), "rb");
	if (!f) {
		return false;
	}
	fclose(f);
	return true;
}

#endif