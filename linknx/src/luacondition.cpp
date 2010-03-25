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

LuaCondition::LuaCondition(ChangeListener* cl) : cl_m(cl), l_m(0)
{
    l_m = luaL_newstate();  
    luaL_openlibs(l_m);
    lua_register(l_m, "obj", LuaCondition::obj);  
    lua_register(l_m, "isException", LuaCondition::isException);  
}

LuaCondition::~LuaCondition()
{
    lua_close(l_m);
}

bool LuaCondition::evaluate()
{
    if (luaL_dostring(l_m, code_m.c_str()) != 0)
    { 
        logger_m.errorStream() << "LuaCondition error: " << lua_tostring(l_m, -1) << endlog;
        return false;
    }
    int ret = lua_toboolean(l_m, -1);  
    logger_m.infoStream() << "LuaCondition evaluated as " << (ret? "true":"false") << endlog;
    lua_settop(l_m, 0);      
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
        pConfig->SetText(code_m);
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
    luaL_openlibs(l_m);
    lua_register(l_m, "obj", LuaScriptAction::obj);  
    lua_register(l_m, "set", LuaScriptAction::set);  
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
        pConfig->SetText(code_m);
    Action::exportXml(pConfig);
}

void LuaScriptAction::Run (pth_sem_t * stop)
{
    pth_sleep(delay_m);
    logger_m.infoStream() << "Execute LuaScriptAction" << endlog;
    if (luaL_dostring(l_m, code_m.c_str()) != 0) 
        logger_m.errorStream() << "LuaScriptAction error: " << lua_tostring(l_m, -1) << endlog;
    lua_settop(l_m, 0);      
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
    if (lua_gettop(L) != 2 || !lua_isstring(L, 1))
    {
        lua_pushstring(L, "Incorrect argument to 'set'");
        lua_error(L);
    }
    std::string id(lua_tostring(L, 1));
    std::string value(lua_tostring(L, 2));
    debugStream("LuaScriptAction") << "Setting object with id=" << id << endlog;
    try {
        Object* object = ObjectController::instance()->getObject(id);
        object->setValue(value);
        debugStream("LuaScriptAction") << "Object '" << id << "' set to value '" << value << "'" << endlog;
    }
    catch( ticpp::Exception& ex )
    {
        lua_pushstring(L, "Error while setting object value");
        lua_error(L);
    }

    return 0;
}

#endif // HAVE_LUA

