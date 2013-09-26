/*
Copyright (c) 2011 Timo Boll, Tony Kostanjsek

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the
following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "../../include/slub/config.h"
#include "../../include/slub/clazz.h"

#include <iostream>
#include <stdexcept>

namespace slub {

  std::pair<int, int> abstract_clazz::construct(lua_State * state, registry * reg, char const * name, char const * fqname, int target)
  {
    lua_newtable(state);
    int methods = lua_gettop(state);
    
    if (luaL_newmetatable(state, fqname) == 0) {
      throw new std::runtime_error("already registered: "+ std::string(fqname));
    }
    int metatable = lua_gettop(state);
    
    // store method table in globals so that
    // scripts can add functions written in Lua.
    lua_pushvalue(state, methods);
    lua_setfield(state, target != -1 ? target : LUA_GLOBALSINDEX, name);
    
    lua_pushliteral(state, "__metatable");
    lua_pushvalue(state, methods);
    lua_settable(state, metatable);  // hide metatable from Lua getmetatable()

    return std::make_pair(methods, metatable);
  }

  void abstract_clazz::add_symbols(lua_State * state, registry * reg, int methods, int metatable)
  {
    lua_pushliteral(state, "__index");
    lua_pushlightuserdata(state, reg);
    lua_pushcclosure(state, index, 1);
    lua_settable(state, metatable);
    
    lua_pushliteral(state, "__newindex");
    lua_pushlightuserdata(state, reg);
    lua_pushcclosure(state, newindex, 1);
    lua_settable(state, metatable);
    
    lua_pushliteral(state, "__eq");
    lua_pushlightuserdata(state, reg);
    lua_pushstring(state, "__eq");
    lua_pushcclosure(state, callOperator, 2);
    lua_settable(state, metatable);
    
    lua_pushliteral(state, "__lt");
    lua_pushlightuserdata(state, reg);
    lua_pushstring(state, "__lt");
    lua_pushcclosure(state, callOperator, 2);
    lua_settable(state, metatable);
    
    lua_pushliteral(state, "__le");
    lua_pushlightuserdata(state, reg);
    lua_pushstring(state, "__le");
    lua_pushcclosure(state, callOperator, 2);
    lua_settable(state, metatable);
    
    lua_pushliteral(state, "__tostring");
    lua_pushlightuserdata(state, reg);
    lua_pushstring(state, "__tostring");
    lua_pushcclosure(state, callOperator, 2);
    lua_settable(state, metatable);
    
    lua_pushliteral(state, "__add");
    lua_pushlightuserdata(state, reg);
    lua_pushstring(state, "__add");
    lua_pushcclosure(state, callOperator, 2);
    lua_settable(state, metatable);
    
    lua_pushliteral(state, "__sub");
    lua_pushlightuserdata(state, reg);
    lua_pushstring(state, "__sub");
    lua_pushcclosure(state, callOperator, 2);
    lua_settable(state, metatable);
    
    lua_pushliteral(state, "__mul");
    lua_pushlightuserdata(state, reg);
    lua_pushstring(state, "__mul");
    lua_pushcclosure(state, callOperator, 2);
    lua_settable(state, metatable);
    
    lua_pushliteral(state, "__div");
    lua_pushlightuserdata(state, reg);
    lua_pushstring(state, "__div");
    lua_pushcclosure(state, callOperator, 2);
    lua_settable(state, metatable);
    
    lua_pushliteral(state, "__mod");
    lua_pushlightuserdata(state, reg);
    lua_pushstring(state, "__mod");
    lua_pushcclosure(state, callOperator, 2);
    lua_settable(state, metatable);
    
    lua_pushliteral(state, "__pow");
    lua_pushlightuserdata(state, reg);
    lua_pushstring(state, "__pow");
    lua_pushcclosure(state, callOperator, 2);
    lua_settable(state, metatable);
    
    lua_pushliteral(state, "__unm");
    lua_pushlightuserdata(state, reg);
    lua_pushstring(state, "__unm");
    lua_pushcclosure(state, callOperator, 2);
    lua_settable(state, metatable);
    
    lua_pop(state, 2);  // drop metatable and method table
  }

  // TODO: access by type
  int abstract_clazz::index(lua_State* L) {
    registry* reg = registry::get(*((wrapper_base*) lua_touserdata(L, 1))->type);
    if (reg != NULL) {
      const char* name = lua_tostring(L, -1);

      reg->getInstanceTable(L, ((wrapper_base*) lua_touserdata(L, 1))->raw);
      lua_pushstring(L, name);
      lua_rawget(L, -2);
      if (lua_type(L, lua_gettop(L)) != LUA_TNIL) {
        lua_remove(L, -2);
        return 1;
      }
      else {
        lua_pop(L, 2);

        if (reg->containsField(name)) {
          return reg->getField(lua_touserdata(L, 1), name)->get(L);
        }
        else if (reg->containsMethod(name)) {
          lua_pushvalue(L, -1);
          lua_pushcclosure(L, callMethod, 1);
          return 1;
        }
        else if (reg->containsOperator("__index") && reg->getOperator("__index", L, false) != NULL) {
          int num = lua_gettop(L);
          reg->getOperator("__index", L)->op(L);
          return lua_gettop(L) - num;
        }
        else {
          // get value from Lua table
          luaL_getmetatable(L, reg->getTypeName().c_str());
          lua_getfield(L, -1, "__metatable");
          int methods = lua_gettop(L);
        
          lua_pushvalue(L, -3);
          lua_gettable(L, methods);
          
          return 1;
        }
      }
    }
    return 0;
  }
  
  int abstract_clazz::newindex(lua_State* L) {
    wrapper_base* w = (wrapper_base*) lua_touserdata(L, 1);
    registry* reg = registry::get(*(w)->type);
    if (reg != NULL) {
      string name(lua_tostring(L, -2));
      if (reg->containsField(name)) {
        return reg->getField(lua_touserdata(L, 1), name)->set(L);
      }
      else {
        reg->getInstanceTable(L, w->raw);
        lua_pushvalue(L, -3);
        lua_pushvalue(L, -3);
        lua_rawset(L, -3);
        lua_pop(L, 1);
        return 0;
      }
    }
    return 0;
  }
  
  int abstract_clazz::callMethod(lua_State* L) {
    void* ud = lua_touserdata(L, 1);
    if(!ud) {
       throw std::runtime_error("callMethod failed, did you use '.' instead of ':'?");
    }
    registry* reg = registry::get(*((wrapper_base*) ud)->type);
    const char* methodName = lua_tostring(L, lua_upvalueindex(1));

    int numParams = lua_gettop(L);

    // try to get value from Lua table
    luaL_getmetatable(L, reg->getTypeName().c_str());
    lua_getfield(L, -1, "__metatable");
    int methods = lua_gettop(L);
      
    lua_pushstring(L, methodName);
    lua_gettable(L, methods);
    lua_remove(L, -2);
    lua_remove(L, -2);

    if (lua_isfunction(L, lua_gettop(L))) {
      slub::call(L, numParams, LUA_MULTRET);
    }
    else {
      lua_pop(L, 1);
      reg->getMethod(methodName, L)->call(L);
    }

    return lua_gettop(L) - numParams;
  }
  
  int abstract_clazz::callOperator(lua_State* L) {
    registry* reg = registry::get(*((wrapper_base*) lua_touserdata(L, 1))->type);
    if (reg != NULL) {
      int num = lua_gettop(L);
      reg->getOperator(lua_tostring(L, lua_upvalueindex(2)), L)->op(L);
      return lua_gettop(L) - num;
    }
    return 0;
  }
  
}
