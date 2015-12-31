#ifndef __SOUND_H__
#define __SOUND_H__

#if APL
	#include <Carbon/Carbon.h>
	#include <OpenAL/al.h>
	#include <OpenAL/alc.h>
#else
#ifdef WINDOWS
	#include <al.h>
	#include <alc.h>
#else
	#include <AL/al.h>
	#include <AL/alc.h>
#endif
#endif

#include <string>
#include <sstream>
#include <fstream>
#include <assert.h>

#if defined(__cplusplus)
extern "C" {
#endif

#include "libavionics.h"

// sound engine handler
struct SaslAlSound;

// initialize sound engine
SaslAlSound* sasl_init_al_sound(SASL sasl);

// destroy sound engine
void sasl_done_al_sound(SaslAlSound *sound);

#if defined(__cplusplus)
};
#endif

class ALChecker {
public:
	static void setSASL(SASL inSASL) {
		sasl = inSASL;
	}

	static inline std::string alErrorString(const std::string& inFunction, const std::string& inLine, ALenum inALError);
	static inline void checkAL(const std::string& inFunction, int inLine);
	static inline std::string alcErrorString(ALCdevice* inDevice, const std::string& inFunction, const std::string& inLine, ALenum inALCError);
	static inline void checkALC(ALCdevice* inDevice, const std::string& inFunction, int inLine);
private:
	static SASL sasl;
};

inline std::string ALChecker::alErrorString(const std::string& inFunction, const std::string& inLine, ALenum inALError) {
	const ALchar* errorIdentifier = alGetString(inALError);
	std::string error = std::string("AL: ") + errorIdentifier + std::string(" <Function>: ") + 
		inFunction + std::string(" <Line>: ") + inLine;
	return error;
}

inline void ALChecker::checkAL(const std::string& inFunction, int inLine) {
	ALenum alError = alGetError();
	if (alError != AL_NO_ERROR) {
		std::stringstream lineStream;
		lineStream << inLine;
		sasl_log_error(sasl, alErrorString(inFunction, lineStream.str(), alError).c_str());
	}
}

inline std::string ALChecker::alcErrorString(ALCdevice* inDevice, const std::string& inFunction, const std::string& inLine, ALenum inALCError) {
	const ALchar* errorIdentifier = alcGetString(inDevice, inALCError);
	std::string error = std::string("ALC: ") + errorIdentifier + std::string(" <Function>: ") + 
		inFunction + std::string(" <Line>: ") + inLine;
	return error;
}

inline void ALChecker::checkALC(ALCdevice* inDevice, const std::string& inFunction, int inLine) {
	ALenum alError = alcGetError(inDevice);
	if (alError != ALC_NO_ERROR) {
		std::stringstream lineStream;
		lineStream << inLine;
		sasl_log_error(sasl, alcErrorString(inDevice, inFunction, lineStream.str(), alError).c_str());
	}
}

#ifdef ALDEBUG
#	define alCheck(ALcode) ALcode; ALChecker::checkAL(__FUNCTION__, __LINE__);
#	define alcCheck(ALCcode, device) ALCcode; ALChecker::checkALC(device, __FUNCTION__, __LINE__);
#else
#	define alCheck(ALcode) ALcode
#	define alcCheck(ALCcode, device) ALCcode
#endif

#endif

