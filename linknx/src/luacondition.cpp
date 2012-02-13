/*
    LinKNX KNX home automation platform
    Copyright (C) 2007 Jean-François Meessen <linknx@ouaye.net>
 
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
 
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
 
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "luacondition.h"

#ifdef HAVE_LUA

extern "C"
{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}
#include <ctime>
#include "services.h"
#include "ioport.h"

LuaMain* LuaMain::instance_m;

LuaMain::LuaMain()
{
    if (pth_mutex_init(&mutex_m))
    {
        Logger::getInstance("LuaMain").debugStream() << "Lua Mutex Initialized" << endlog;
    } else {
        Logger::getInstance("LuaMain").errorStream() << "Error initializing Lua Mutex" << endlog;
    }
}

LuaMain::~LuaMain()
{}

LuaMain* LuaMain::instance()
{
    if (instance_m == 0)
        instance_m = new LuaMain();
    return instance_m;
}

void LuaMain::lock()
{
    pth_mutex_acquire(&instance()->mutex_m, FALSE, NULL);
}

void LuaMain::unlock()
{
    pth_mutex_release(&instance()->mutex_m);
}

LuaCondition::LuaCondition(ChangeListener* cl) : cl_m(cl), l_m(0)
{
    l_m = luaL_newstate();  
    /* stop collector during initialisation
     * http://lua-users.org/lists/lua-l/2008-07/msg00690.html
     */
    lua_gc(l_m, LUA_GCSTOP, 0);    luaL_openlibs(l_m);
    lua_register(l_m, "obj", LuaCondition::obj);  
    lua_register(l_m, "isException", LuaCondition::isException);  
    lua_gc(l_m, LUA_GCRESTART, 0);
}

LuaCondition::~LuaCondition()
{
    lua_close(l_m);
}

bool LuaCondition::evaluate()
{
    LuaMain::lock();
    if (luaL_dostring(l_m, code_m.c_str()) != 0)
    { 
        logger_m.errorStream() << "LuaCondition error: " << lua_tostring(l_m, -1) << endlog;
        return false;
    }
    int ret = lua_toboolean(l_m, -1);  
    logger_m.infoStream() << "LuaCondition evaluated as " << (ret? "true":"false") << endlog;
    lua_settop(l_m, 0);
    LuaMain::unlock();
    return ret;
}

void LuaCondition::importXml(ticpp::Element* pConfig)
{
    code_m = pConfig->GetText();

    infoStream("LuaCondition") << "LuaCondition: Configured code=" << code_m << endlog;
}

void LuaCondition::exportXml(ticpp::Element* pConfig)
{
    pConfig->SetAttribute("type", "script");
    if (code_m.length())
    {
        ticpp::Text pText(code_m);
        pText.SetCDATA(true);
        pConfig->LinkEndChild(&pText);
    }
}

void LuaCondition::statusXml(ticpp::Element* pStatus)
{
    pStatus->SetAttribute("type", "script");
}

int LuaCondition::obj(lua_State *L)
{
    if (lua_gettop(L) != 1 || !lua_isstring(L, 1))
    {
        lua_pushstring(L, "Incorrect argument to 'obj'");
        lua_error(L);
    }
    std::string id(lua_tostring(L, 1));
    debugStream("LuaCondition") << "Getting object with id=" << id << endlog;
    try {
        Object* object = ObjectController::instance()->getObject(id);
        std::string ret = object->getValue();
        object->decRefCount();
        debugStream("LuaCondition") << "Object '" << id << "' has value '" << ret << "'" << endlog;
        lua_pushstring(L, ret.c_str());
    }
    catch( ticpp::Exception& ex )
    {
        lua_pushstring(L, "Error while retrieving object value");
        lua_error(L);
    }
    return 1;
}

int LuaCondition::isException(lua_State *L)
{
    time_t ts;
    if (lua_gettop(L) == 0)
    {
        ts = time(0);
    }
    else if (lua_gettop(L) != 1 || !lua_isnumber(L, 1))
    {
        ts = 0;
        lua_pushstring(L, "Incorrect argument to 'isException'");
        lua_error(L);
    }
    else
    {
        ts = lua_tointeger(L, 1);
    }
    bool ret = Services::instance()->getExceptionDays()->isException(ts);
    debugStream("LuaCondition") << "Is timestamp (" << ts << ") an exception day: " << (ret ? "yes" : "no" ) << endlog;

    lua_pushboolean(L, ret);
    return 1;
}

LuaScriptAction::LuaScriptAction()
{
    l_m = luaL_newstate();
    /* stop collector during initialisation
     * http://lua-users.org/lists/lua-l/2008-07/msg00690.html
     */
    lua_gc(l_m, LUA_GCSTOP, 0);
    luaL_openlibs(l_m);
    lua_register(l_m, "obj", LuaScriptAction::obj);  
    lua_register(l_m, "set", LuaScriptAction::set);  
    lua_register(l_m, "iosend", LuaScriptAction::iosend);  
    lua_register(l_m, "sleep", LuaScriptAction::sleep);
    lua_gc(l_m, LUA_GCRESTART, 0);
}

LuaScriptAction::~LuaScriptAction()
{
    lua_close(l_m);
}

void LuaScriptAction::importXml(ticpp::Element* pConfig)
{
    code_m = pConfig->GetText();
    logger_m.infoStream() << "LuaScriptAction: Configured." << endlog;
}

void LuaScriptAction::exportXml(ticpp::Element* pConfig)
{
    pConfig->SetAttribute("type", "script");
    if (code_m.length())
    {
        ticpp::Text pText(code_m);
        pText.SetCDATA(true);
        pConfig->LinkEndChild(&pText);
    }
    Action::exportXml(pConfig);
}

void LuaScriptAction::Run (pth_sem_t * stop)
{
    if (Action::sleep(delay_m, stop))
        return;
    logger_m.infoStream() << "Execute LuaScriptAction" << endlog;
    LuaMain::lock();
    lua_pushlightuserdata(l_m, stop);
    lua_setglobal(l_m, "__linknx_stop");
    if (luaL_dostring(l_m, code_m.c_str()) != 0)
    {
        std::string error(lua_tostring(l_m, -1));
        if (error == "Action interrupted")
            logger_m.infoStream() << "LuaScriptAction canceled" << endlog;
        else
            logger_m.errorStream() << "LuaScriptAction error: " << lua_tostring(l_m, -1) << endlog;
    }
    lua_settop(l_m, 0);
    LuaMain::unlock();
}

int LuaScriptAction::obj(lua_State *L)
{
    if (lua_gettop(L) != 1 || !lua_isstring(L, 1))
    {
        lua_pushstring(L, "Incorrect argument to 'obj'");
        lua_error(L);
    }
    std::string id(lua_tostring(L, 1));
    debugStream("LuaScriptAction") << "Getting object with id=" << id << endlog;
    try {
        Object* object = ObjectController::instance()->getObject(id);
        std::string ret = object->getValue();
        object->decRefCount();
        debugStream("LuaScriptAction") << "Object '" << id << "' has value '" << ret << "'" << endlog;
        lua_pushstring(L, ret.c_str());
    }
    catch( ticpp::Exception& ex )
    {
        lua_pushstring(L, "Error while retrieving object value");
        lua_error(L);
    }
    return 1;
}

int LuaScriptAction::set(lua_State *L)
{
    const char *val = lua_tostring(L, 2);
    if (lua_gettop(L) != 2 || !lua_isstring(L, 1))
    {
        lua_pushstring(L, "Incorrect argument to 'set'");
        lua_error(L);
    }
    if (val == NULL)
    {
        lua_pushstring(L, "Incorrect value for 'set'");
        lua_error(L);
    }
    std::string id(lua_tostring(L, 1));
    std::string value(val);
    debugStream("LuaScriptAction") << "Setting object with id=" << id << endlog;
    try {
        Object* object = ObjectController::instance()->getObject(id);
        object->setValue(value);
        object->decRefCount();
        debugStream("LuaScriptAction") << "Object '" << id << "' set to value '" << value << "'" << endlog;
    }
    catch( ticpp::Exception& ex )
    {
        lua_pushstring(L, "Error while setting object value");
        lua_error(L);
    }

    return 0;
}

int LuaScriptAction::iosend(lua_State *L)
{
    const char *val = lua_tostring(L, 2);
    if (lua_gettop(L) != 2 || !lua_isstring(L, 1))
    {
        lua_pushstring(L, "Incorrect argument to 'iosend'");
        lua_error(L);
    }
    if (val == NULL)
    {
        lua_pushstring(L, "Incorrect value for 'iosend'");
        lua_error(L);
    }
    std::string id(lua_tostring(L, 1));
    std::string value(val);
    debugStream("LuaScriptAction") << "Sending on ioport " << id << endlog;
    try
    {
        IOPort* port = IOPortManager::instance()->getPort(id);
        if (!port)
            throw ticpp::Exception("IO Port ID not found.");
        const uint8_t* data = reinterpret_cast<const uint8_t*>(value.c_str());
        int len = value.length();
        int ret = port->send(data, len);
        if (ret != len) {
            ret = port->send(data, len);
            if (ret != len)
                throw ticpp::Exception("Unable to send data.");
        }
        debugStream("LuaScriptAction") << "Sent '" << value << "' on ioport " << id << endlog;
    }
    catch( ticpp::Exception& ex )
    {
        lua_pushstring(L, "Error while sending on io port");
        lua_error(L);
    }

    return 0;
}

int LuaScriptAction::sleep(lua_State *L)
{
    lua_Number delay, ret=0;
    if (lua_gettop(L) != 1 || !lua_isnumber(L, 1))
    {
        lua_pushstring(L, "Incorrect argument to 'sleep'");
        lua_error(L);
    }
    delay = lua_tonumber(L, 1);
    debugStream("LuaScriptAction") << "Sleep for '" << delay << "' seconds" << endlog;
    lua_getglobal(L, "__linknx_stop");
    if (lua_islightuserdata (L, -1))
    {
        pth_sem_t * stop = (pth_sem_t *)lua_touserdata(L, -1);
        LuaMain::unlock();
        if (Action::sleep(delay*1000, stop))
        {
            LuaMain::lock();
            lua_pushstring(L, "Action interrupted");
            lua_error(L);
            return 0;
        }
        LuaMain::lock();
    }
    else
    {
        LuaMain::unlock();
        if (delay < 1)
            ret = pth_usleep(delay*1000000);
        else
            ret = pth_sleep(delay);
        LuaMain::lock();
    }
    lua_pushnumber(L, ret);
    return 1;
}

#endif // HAVE_LUA

