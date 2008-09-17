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

extern "C"
{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

LuaCondition::LuaCondition(ChangeListener* cl) : cl_m(cl), condition_m(0)
{
    lua_State *l;
    l = lua_open();
//    lua_baselibopen(l);
//    luaopen_table(l);
//    luaopen_io(l);
//    luaopen_string(l);
//    luaopen_math(l);
    luaL_openlibs(l);  
    //run a Lua scrip here  
    luaL_dofile(l,"foo.lua");  
    
    lua_close(l);
    printf("\nBack to C again\n\n");

}

LuaCondition::~LuaCondition()
{
    if (condition_m)
        delete condition_m;
}

bool LuaCondition::evaluate()
{
    
    return true;
}

void LuaCondition::importXml(ticpp::Element* pConfig)
{
    code_m = pConfig->GetText();

    std::cout << "LuaCondition: Configured code=" << code_m << std::endl;
//    condition_m = Condition::create(pConfig->FirstChildElement("condition"), cl_m);
}

void LuaCondition::exportXml(ticpp::Element* pConfig)
{
    pConfig->SetAttribute("type", "lua");
    if (condition_m)
    {
        ticpp::Element pElem("condition");
        condition_m->exportXml(&pElem);
        pConfig->LinkEndChild(&pElem);
    }
}

