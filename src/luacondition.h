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

#ifndef LUACONDITION_H
#define LUACONDITION_H

#include <string>
#include "config.h"
#include "ruleserver.h"
#include "ticpp.h"

#ifdef HAVE_LUA

struct lua_State;

class LuaMain
{
public:
    static LuaMain* instance();
    static void reset()
    {
        if (instance_m)
            delete instance_m;
        instance_m = 0;
    };

    static void lock();
    static void unlock();
private:
    LuaMain();
    ~LuaMain();
    pth_mutex_t mutex_m;
    static LuaMain* instance_m;
};

class LuaCondition : public Condition
{
public:
    LuaCondition(ChangeListener* cl);
    virtual ~LuaCondition();

    virtual bool evaluate();
    virtual void importXml(ticpp::Element* pConfig);
    virtual void exportXml(ticpp::Element* pConfig);
    virtual void statusXml(ticpp::Element* pStatus);
    static int obj(lua_State *L);
    static int isException(lua_State *L);
private:
//    Condition* condition_m;
    ChangeListener* cl_m;
    std::string code_m;
    lua_State *l_m;
};

class LuaScriptAction : public Action
{
public:
    LuaScriptAction();
    virtual ~LuaScriptAction();

    virtual void importXml(ticpp::Element* pConfig);
    virtual void exportXml(ticpp::Element* pConfig);
    static int obj(lua_State *L);
    static int set(lua_State *L);
    static int iosend(lua_State *L);
    static int sleep(lua_State *L);
    static int usleep(lua_State *L);

private:
    virtual void Run (pth_sem_t * stop);

    lua_State *l_m;
    std::string code_m;
};

#endif // HAVE_LUA

#endif
