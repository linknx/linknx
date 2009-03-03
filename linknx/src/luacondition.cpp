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

LuaCondition::LuaCondition(ChangeListener* cl) : cl_m(cl), l_m(0) //, condition_m(0)
{
    l_m = luaL_newstate();  
    luaL_openlibs(l_m);
    lua_register(l_m, "obj", LuaCondition::obj);  
}

LuaCondition::~LuaCondition()
{
//    if (condition_m)
//        delete condition_m;
    lua_close(l_m);
}

bool LuaCondition::evaluate()
{
    luaL_dostring(l_m, code_m.c_str());  
    int ret = lua_toboolean(l_m, -1);  
    logger_m.infoStream() << "LuaCondition evaluated as " << (ret? "true":"false") << endlog;
    lua_settop(l_m, 0);      
    return ret;
}

void LuaCondition::importXml(ticpp::Element* pConfig)
{
    code_m = pConfig->GetText();

    infoStream("LuaCondition") << "LuaCondition: Configured code=" << code_m << endlog;
//    condition_m = Condition::create(pConfig->FirstChildElement("condition"), cl_m);
}

void LuaCondition::exportXml(ticpp::Element* pConfig)
{
    pConfig->SetAttribute("type", "script");
    if (code_m.length())
        pConfig->SetText(code_m);
/*    if (condition_m)
    {
        ticpp::Element pElem("condition");
        condition_m->exportXml(&pElem);
        pConfig->LinkEndChild(&pElem);
    }*/
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
    Object* object = ObjectController::instance()->getObject(id);
    if (!object) 
    {
        lua_pushstring(L, "Object id not found");
        lua_error(L);
    }
    std::string ret = object->getValue();
    debugStream("LuaCondition") << "Object '" << id << "' has value '" << ret << "'" << endlog;
    lua_pushstring(L, ret.c_str());

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
    luaL_dostring(l_m, code_m.c_str());  
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
    debugStream("LuaCondition") << "Getting object with id=" << id << endlog;
    Object* object = ObjectController::instance()->getObject(id);
    if (!object) 
    {
        lua_pushstring(L, "Object id not found");
        lua_error(L);
    }
    std::string ret = object->getValue();
    debugStream("LuaCondition") << "Object '" << id << "' has value '" << ret << "'" << endlog;
    lua_pushstring(L, ret.c_str());

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
    debugStream("LuaCondition") << "Setting object with id=" << id << endlog;
    Object* object = ObjectController::instance()->getObject(id);
    if (!object) 
    {
        lua_pushstring(L, "Object id not found");
        lua_error(L);
    }
    object->setValue(value);
    debugStream("LuaCondition") << "Object '" << id << "' set to value '" << value << "'" << endlog;

    return 0;
}

#endif // HAVE_LUA

