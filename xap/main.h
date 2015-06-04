#ifndef __MAIN_H__
#define __MAIN_H__

#include <string>
#include "libavionics.h"


namespace xap {

class Options;

/// Options storage
extern Options options;

extern SASL sasl;

void reloadPanel(bool keepProps);

std::string getAircraftDir();

};



#endif

