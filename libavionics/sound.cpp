#include "sound.h"

#include <assert.h>

#include "avionics.h"

using namespace xa;


Sound::Sound()
{
    sound = NULL;
}


void Sound::setCallbacks(SaslSoundCallbacks *callbacks)
{
    sound = callbacks;
}


int Sound::loadSample(const char *fileName)
{
    if (sound && sound->load)
        return sound->load(sound, fileName);
    else
        return 0;
}

int Sound::loadSampleReversed(const char *fileName)
{
	if (sound && sound->load_reversed)
		return sound->load_reversed(sound, fileName);
	else
		return 0;
}

void Sound::unloadSample(int sampleId)
{
    if (sound && sound->unload)
        return sound->unload(sound, sampleId);
}


void Sound::playSample(int sampleId, int loop)
{
    if (sound && sound->play)
        sound->play(sound, sampleId, loop);
}


void Sound::stopSample(int sampleId)
{
    if (sound && sound->stop)
        sound->stop(sound, sampleId);
}


void Sound::setSampleGain(int sampleId, int gain)
{
    if (sound && sound->set_gain)
        sound->set_gain(sound, sampleId, gain);
}


void Sound::setSamplePitch(int sampleId, int pitch)
{
    if (sound && sound->set_pitch)
        sound->set_pitch(sound, sampleId, pitch);
}


void Sound::rewindSample(int sampleId)
{
    if (sound && sound->rewind)
        sound->rewind(sound, sampleId);
}


void Sound::setSampleEnv(int sampleId, int env)
{
    if (sound && sound->set_env)
        sound->set_env(sound, sampleId, env);
}


int Sound::getSampleEnv(int sampleId)
{
    if (sound && sound->get_env)
        return sound->get_env(sound, sampleId);
    else
        return 0;
}


void Sound::setSamplePosition(int sampleId, float x, float y, float z)
{
    if (sound && sound->set_position)
        sound->set_position(sound, sampleId, x, y, z);
}


void Sound::getSamplePosition(int sampleId, float &x, float &y, float &z)
{
    if (sound && sound->get_position)
        sound->get_position(sound, sampleId, &x, &y, &z);
}


void Sound::setSampleDirection(int sampleId, float x, float y, float z)
{
    if (sound && sound->set_direction)
        sound->set_direction(sound, sampleId, x, y, z);
}


void Sound::getSampleDirection(int sampleId, float &x, float &y, float &z)
{
    if (sound && sound->get_direction)
        sound->get_direction(sound, sampleId, &x, &y, &z);
}


void Sound::setSampleMaxDistance(int sampleId, float maxDistance)
{
    if (sound && sound->set_max_distance)
        sound->set_max_distance(sound, sampleId, maxDistance);
}


void Sound::setSampleRolloff(int sampleId, float rolloff)
{
    if (sound && sound->set_rolloff)
        sound->set_rolloff(sound, sampleId, rolloff);
}


void Sound::setSampleRefDistance(int sampleId, float distance)
{
    if (sound && sound->set_ref_distance)
        sound->set_ref_distance(sound, sampleId, distance);
}


void Sound::setSampleCone(int sampleId, float outerGain, 
        float innerAngle, float outerAngle)
{
    if (sound && sound->set_cone)
        sound->set_cone(sound, sampleId, outerGain, innerAngle, outerAngle);
}


void Sound::getSampleCone(int sampleId, float &outerGain, 
        float &innerAngle, float &outerAngle)
{
    if (sound && sound->get_cone)
        sound->get_cone(sound, sampleId, &outerGain, &innerAngle, &outerAngle);
}


void Sound::setSampleRelative(int sampleId, bool relative)
{
    if (sound && sound->set_relative)
        sound->set_relative(sound, sampleId, relative); 
}


bool Sound::getSampleRelative(int sampleId)
{
    if (sound && sound->get_relative)
        return sound->get_relative(sound, sampleId);
    else
        return false;
}


void Sound::setListenerEnv(int env)
{
    if (sound && sound->set_listener_env)
        sound->set_listener_env(sound, env); 
}


void Sound::setListenerPosition(float x, float y, float z)
{
    if (sound && sound->set_listener_position)
        sound->set_listener_position(sound, x, y, z); 
}


void Sound::getListenerPosition(float &x, float &y, float &z)
{
    if (sound && sound->get_listener_position)
        sound->get_listener_position(sound, &x, &y, &z); 
}


void Sound::setListenerOrientation(float x, float y, float z,
        float ux, float uy, float uz)
{
    if (sound && sound->set_listener_orientation)
        sound->set_listener_orientation(sound, x, y, z, ux, uy, uz); 
}


void Sound::getListenerOrientation(float &x, float &y, float &z,
        float &ux, float &uy, float &uz)
{
    if (sound && sound->get_listener_orientation)
        sound->get_listener_orientation(sound, &x, &y, &z, &ux, &uy, &uz); 
}


bool Sound::isSamplePlaying(int sampleId)
{
    if (sound && sound->is_playing)
        return sound->is_playing(sound, sampleId);
    else
        return false;
}


void Sound::setMasterGain(int gain)
{
    if (sound && sound->set_master_gain)
        sound->set_master_gain(sound, gain);
}


void Sound::update()
{
    if (sound && sound->update)
        sound->update(sound);
}

void Sound::getContextsState(int& sources, int& context_source_limit)
{
	if (sound && sound->get_contexts_state) {
		sound->get_contexts_state(sound, &sources, &context_source_limit);
	}
}

static int luaLoadSample(lua_State *L)
{
    int sampleId = getAvionics(L)->getSound().loadSample(lua_tostring(L, 1));
    lua_pushnumber(L, sampleId);
    return 1;
}

static int luaLoadSampleInReverse(lua_State *L)
{
	int sampleId = getAvionics(L)->getSound().loadSampleReversed(lua_tostring(L, 1));
	lua_pushnumber(L, sampleId);
	return 1;
}

static int luaUnloadSample(lua_State *L)
{
    getAvionics(L)->getSound().unloadSample(lua_tonumber(L, 1));
    return 0;
}



static int luaSamplePlay(lua_State *L)
{
    getAvionics(L)->getSound().playSample(lua_tonumber(L, 1), 
            lua_tonumber(L, 2));
    return 0;
}


static int luaSampleSetGain(lua_State *L)
{
    getAvionics(L)->getSound().setSampleGain(lua_tonumber(L, 1), 
            lua_tonumber(L, 2));
    return 0;
}


static int luaSampleSetPitch(lua_State *L)
{
    getAvionics(L)->getSound().setSamplePitch(lua_tonumber(L, 1), 
            lua_tonumber(L, 2));
    return 0;
}


static int luaSampleRewind(lua_State *L)
{
    getAvionics(L)->getSound().rewindSample(lua_tonumber(L, 1));
    return 0;
}


static int luaSampleStop(lua_State *L)
{
    getAvionics(L)->getSound().stopSample(lua_tonumber(L, 1));
    return 0;
}


static int luaIsSamplePlaying(lua_State *L)
{
    int playing = getAvionics(L)->getSound().isSamplePlaying(
            lua_tonumber(L, 1));
    lua_pushboolean(L, playing);
    return 1;
}


static int luaSetMasterGain(lua_State *L)
{
    getAvionics(L)->getSound().setMasterGain(lua_tonumber(L, 1));
    return 0;
}


static int luaSetSampleEnv(lua_State *L)
{
    getAvionics(L)->getSound().setSampleEnv(lua_tonumber(L, 1), 
            lua_tonumber(L, 2));
    return 0;
}


static int luaGetSampleEnv(lua_State *L)
{
    int res = getAvionics(L)->getSound().getSampleEnv(lua_tonumber(L, 1));
    lua_pushnumber(L, res);
    return 1;
}


static int luaSetSamplePosition(lua_State *L)
{
    getAvionics(L)->getSound().setSamplePosition(lua_tonumber(L, 1),
            lua_tonumber(L, 2), lua_tonumber(L, 3), lua_tonumber(L, 4));
    return 0;
}


static int luaGetSamplePosition(lua_State *L)
{
    float x, y, z;
    getAvionics(L)->getSound().getSamplePosition(lua_tonumber(L, 1), x, y, z);
    lua_pushnumber(L, x);
    lua_pushnumber(L, y);
    lua_pushnumber(L, z);
    return 3;
}


static int luaSetSampleDirection(lua_State *L)
{
    getAvionics(L)->getSound().setSampleDirection(lua_tonumber(L, 1),
            lua_tonumber(L, 2), lua_tonumber(L, 3), lua_tonumber(L, 4));
    return 0;
}


static int luaGetSampleDirection(lua_State *L)
{
    float x, y, z;
    getAvionics(L)->getSound().getSampleDirection(lua_tonumber(L, 1), x, y, z);
    lua_pushnumber(L, x);
    lua_pushnumber(L, y);
    lua_pushnumber(L, z);
    return 3;
}


static int luaSetSampleMaxDistance(lua_State *L)
{
    getAvionics(L)->getSound().setSampleMaxDistance(lua_tonumber(L, 1),
            lua_tonumber(L, 2));
    return 0;
}


static int luaSetSampleRolloff(lua_State *L)
{
    getAvionics(L)->getSound().setSampleRolloff(lua_tonumber(L, 1),
            lua_tonumber(L, 2));
    return 0;
}


static int luaSetSampleRefDistance(lua_State *L)
{
    getAvionics(L)->getSound().setSampleRefDistance(lua_tonumber(L, 1),
            lua_tonumber(L, 2));
    return 0;
}


static int luaSetSampleCone(lua_State *L)
{
    getAvionics(L)->getSound().setSampleCone(lua_tonumber(L, 1),
            lua_tonumber(L, 2), lua_tonumber(L, 3), lua_tonumber(L, 4));
    return 0;
}


static int luaGetSampleCone(lua_State *L)
{
    float outerGain, innerAngle, outerAngle;
    getAvionics(L)->getSound().getSampleCone(lua_tonumber(L, 1), 
            outerGain, innerAngle, outerAngle);
    lua_pushnumber(L, outerGain);
    lua_pushnumber(L, innerAngle);
    lua_pushnumber(L, outerAngle);
    return 3;
}


static int luaSetSampleRelative(lua_State *L)
{
    getAvionics(L)->getSound().setSampleRelative(lua_tonumber(L, 1),
            lua_tonumber(L, 2));
    return 0;
}


static int luaGetSampleRelative(lua_State *L)
{
    int res = getAvionics(L)->getSound().getSampleRelative(lua_tonumber(L, 1));
    lua_pushnumber(L, res);
    return 1;
}


static int luaSetListenerEnv(lua_State *L)
{
    getAvionics(L)->getSound().setListenerEnv(lua_tonumber(L, 1));
    return 0;
}


static int luaSetListenerPosition(lua_State *L)
{
    getAvionics(L)->getSound().setListenerPosition(lua_tonumber(L, 1),
            lua_tonumber(L, 2), lua_tonumber(L, 3));
    return 0;
}


static int luaGetListenerPosition(lua_State *L)
{
    float x, y, z;
    getAvionics(L)->getSound().getListenerPosition(x, y, z);
    lua_pushnumber(L, x);
    lua_pushnumber(L, y);
    lua_pushnumber(L, z);
    return 3;
}


static int luaSetListenerOrientation(lua_State *L)
{
    getAvionics(L)->getSound().setListenerOrientation(lua_tonumber(L, 1),
            lua_tonumber(L, 2), lua_tonumber(L, 3), lua_tonumber(L, 4),
            lua_tonumber(L, 5), lua_tonumber(L, 6));
    return 0;
}


static int luaGetListenerOrientation(lua_State *L)
{
    float x, y, z, ux, uy, uz;
    getAvionics(L)->getSound().getListenerOrientation(x, y, z,
            ux, uy, uz);
    lua_pushnumber(L, x);
    lua_pushnumber(L, y);
    lua_pushnumber(L, z);
    lua_pushnumber(L, ux);
    lua_pushnumber(L, uy);
    lua_pushnumber(L, uz);
    return 6;
}

static int luaGetContextsState(lua_State *L) 
{
	int sources, source_context_limit;
	getAvionics(L)->getSound().getContextsState(sources, source_context_limit);
	lua_newtable(L);

	int filled_contexts = sources / source_context_limit;
	int in_last_context = sources % source_context_limit;

	for (int i = 0; i < filled_contexts; i++) {
		lua_pushnumber(L, i + 1);
		lua_pushnumber(L, source_context_limit);
		lua_settable(L, -3);
	}

	lua_pushnumber(L, filled_contexts + 1);
	lua_pushnumber(L, in_last_context);
	lua_settable(L, -3);

	return 1;
}


void Sound::exportSoundToLua(Luna &lua)
{
    lua_State *L = lua.getLua();

    LUA_REGISTER(L, "loadSampleFromFile", luaLoadSample);
	LUA_REGISTER(L, "loadSampleInReverse", luaLoadSampleInReverse);
    LUA_REGISTER(L, "unloadSample", luaUnloadSample);
    LUA_REGISTER(L, "playSample", luaSamplePlay);
    LUA_REGISTER(L, "stopSample", luaSampleStop);
    LUA_REGISTER(L, "setSampleGain", luaSampleSetGain);
    LUA_REGISTER(L, "setSamplePitch", luaSampleSetPitch);
    LUA_REGISTER(L, "rewindSample", luaSampleRewind);
    LUA_REGISTER(L, "isSamplePlaying", luaIsSamplePlaying);
    LUA_REGISTER(L, "setMasterGain", luaSetMasterGain);
    LUA_REGISTER(L, "setSampleEnv", luaSetSampleEnv);
    LUA_REGISTER(L, "getSampleEnv", luaGetSampleEnv);
    LUA_REGISTER(L, "setSamplePosition", luaSetSamplePosition);
    LUA_REGISTER(L, "getSamplePosition", luaGetSamplePosition);
    LUA_REGISTER(L, "setSampleDirection", luaSetSampleDirection);
    LUA_REGISTER(L, "getSampleDirection", luaGetSampleDirection);
    LUA_REGISTER(L, "setSampleMaxDistance", luaSetSampleMaxDistance);
    LUA_REGISTER(L, "setSampleRolloff", luaSetSampleRolloff);
    LUA_REGISTER(L, "setSampleRefDistance", luaSetSampleRefDistance);
    LUA_REGISTER(L, "setSampleCone", luaSetSampleCone);
    LUA_REGISTER(L, "getSampleCone", luaGetSampleCone);
    LUA_REGISTER(L, "setSampleRelative", luaSetSampleRelative);
    LUA_REGISTER(L, "getSampleRelative", luaGetSampleRelative);
    LUA_REGISTER(L, "setListenerEnv", luaSetListenerEnv);
    LUA_REGISTER(L, "setListenerPosition", luaSetListenerPosition);
    LUA_REGISTER(L, "getListenerPosition", luaGetListenerPosition);
    LUA_REGISTER(L, "setListenerOrientation", luaSetListenerOrientation);
    LUA_REGISTER(L, "getListenerOrientation", luaGetListenerOrientation);
	LUA_REGISTER(L, "getSoundContextsState", luaGetContextsState);
}

