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

#include "../../../include/slub/config.h"
#include "../../../include/slub/debug/debugger.h"

namespace slub {
  
  namespace debug {
    
    debugger::debugger() {
    }
    
    void debugger::attach(lua_State* state, debugger_callback* callback) {
      slub::clazz<debugger>((lua_State*) state);
      
      slub::table mt(state);
      if (lua_getmetatable(state, LUA_GLOBALSINDEX)) {
        mt = slub::reference(state);
      }
      mt["__debugger"] = this;
      slub::converter<const slub::table&>::push(state, mt);
      lua_setmetatable(state, LUA_GLOBALSINDEX);
      
      callbacks[state] = callback;
    }
    
    void debugger::detach(lua_State* state) {
      if (lua_getmetatable(state, LUA_GLOBALSINDEX)) {
        slub::table mt = slub::reference(state);
        mt["__debugger"] = NULL;
        slub::converter<const slub::table&>::push(state, mt);
        lua_setmetatable(state, LUA_GLOBALSINDEX);
      }
      callbacks.erase(state);
    }
    
    int debugger::evaluate(lua_State* state, const string& chunk) {
      // disable hook while evaluating
      int mask = lua_gethookmask(state);
      int count = lua_gethookcount(state);
      lua_sethook(state, NULL, 0, 0);
      int result = lua_gettop(state);
      luaL_dostring(state, chunk.c_str());
      result = lua_gettop(state) - result;
      lua_sethook(state, __debug_hook, mask, count);
      return result;
    }
    
    list<string> debugger::symbolBreakpoints(lua_State* state) {
      list<string> breakpoints;
      if (symbol_breakpoints.find(state) != symbol_breakpoints.end()) {
        breakpoints.insert(breakpoints.begin(), symbol_breakpoints[state].begin(),
                           symbol_breakpoints[state].end());
      }
      return breakpoints;
    }
    
    bool debugger::toggleSymbolBreakpoint(lua_State* state, string symbol) {
      bool wasActive = false;
      if (symbol_breakpoints.find(state) != symbol_breakpoints.end()) {
        for (list<string>::iterator iter = symbol_breakpoints[state].begin();
             iter != symbol_breakpoints[state].end(); ++iter)
        {
          if (*iter == symbol) {
            symbol_breakpoints[state].erase(iter);
            if (symbol_breakpoints[state].size() == 0) {
              symbol_breakpoints.erase(state);
              removeHook(state, LUA_MASKCALL);
            }
            wasActive = true;
            break;
          }
        }
      }
      if (!wasActive) {
        symbol_breakpoints[state].push_back(symbol);
        addHook(state, LUA_MASKCALL);
      }
      return !wasActive;
    }
    
    bool debugger::toggleLineBreakpoint(lua_State* state, const string& module, int line) {
      bool wasActive = false;
      if (line_breakpoints.find(state) != line_breakpoints.end() &&
          line_breakpoints[state].find(module) != line_breakpoints[state].end())
      {
        for (list<int>::iterator iter = line_breakpoints[state][module].begin();
             iter != line_breakpoints[state][module].end(); ++iter)
        {
          if (*iter == line) {
            line_breakpoints[state][module].erase(iter);
            if (line_breakpoints[state][module].size() == 0) {
              line_breakpoints[state].erase(module);
              if (line_breakpoints[state].size() == 0) {
                line_breakpoints.erase(state);
                removeHook(state, LUA_MASKLINE);
              }
            }
            wasActive = true;
            break;
          }
        }
      }
      if (!wasActive) {
        line_breakpoints[state][module].push_back(line);
        addHook(state, LUA_MASKLINE);
      }
      return !wasActive;
    }
    
    void debugger::step(lua_State* state) {
      single_step[state] = true;
      addHook(state, LUA_MASKLINE);
    }
    
    string debugger::stack(lua_State* state) {
      // disable hook while getting stack
      int mask = lua_gethookmask(state);
      int count = lua_gethookcount(state);
      lua_sethook(state, NULL, 0, 0);
      string result;
      const char* s = lua_tostring(state, traceback(state));
      if (s != NULL) {
        result = s;
      }
      lua_sethook(state, __debug_hook, mask, count);
      return result;
    }
    
    lua_variables debugger::variables(lua_State* state, lua_Debug *ar) {
      lua_variables result;
      if (lua_getinfo(state, "f", ar)) {
        lua_getfenv(state, lua_gettop(state));
        int env = lua_gettop(state);
        if (lua_type(state, env) == LUA_TTABLE) {
          int index = 1;
          lua_pushnil(state);  /* first key */
          while (lua_next(state, env)) {
            reference value(state);
            reference name(state, lua_gettop(state));
            result.push_back(lua_variable(index++, name.toString(), value));
          }
        }
        lua_pop(state, 2);
      }
      return result;
    }
    
    lua_variables debugger::locals(lua_State* state, lua_Debug *ar) {
      lua_variables result;
      int index = 1;
      const char* name = NULL;
      while ((name = lua_getlocal(state, ar, index))) {
        result.push_back(lua_variable(state, index++, name));
      }
      return result;
    }
    
    lua_variables debugger::upvalues(lua_State* state, lua_Debug *ar) {
      lua_variables result;
      if (ar->nups > 0 && lua_getinfo(state, "f", ar)) {
        int top = lua_gettop(state);
        int index = 1;
        const char* name = NULL;
        while ((name = lua_getupvalue(state, top, index))) {
          result.push_back(lua_variable(state, index++, name));
        }
        lua_pop(state, 1);
      }
      return result;
    }
    
    debugger* debugger::getDebuggerFromState(lua_State* state) {
      debugger* dbg = NULL;
      if (lua_getmetatable(state, LUA_GLOBALSINDEX)) {
        slub::table mt = slub::reference(state);
        dbg = mt["__debugger"].cast<debugger*>();
      }
      return dbg;
    }
    
    void debugger::__debug_hook(lua_State* state, lua_Debug* ar) {
      debugger* dbg = getDebuggerFromState(state);
      if (dbg != NULL && lua_getinfo(state, "nSlu", ar)) {
        dbg->debug_hook(state, ar);
      }
    }
    
    void debugger::debug_hook(lua_State* state, lua_Debug *ar) {
      if (callbacks.find(state) != callbacks.end()) {
        
        bool handled = false;
        switch (ar->event) {
            
          case LUA_HOOKCALL: {
            if (symbol_breakpoints.find(state) != symbol_breakpoints.end() &&
                ar->name != NULL)
            {
              for (list<string>::iterator iter = symbol_breakpoints[state].begin();
                   iter != symbol_breakpoints[state].end(); ++iter)
              {
                if (*iter == ar->name) {
                  handled = true;
                  if (single_step.find(state) != single_step.end() &&
                      single_step[state])
                  {
                    single_step.erase(state);
                  }
                  callbacks[state]->slub_debug(state, ar);
                  break;
                }
              }
            }
            break;
          }
            
          case LUA_HOOKLINE: {
            if (line_breakpoints.find(state) != line_breakpoints.end() &&
                ar->short_src != NULL &&
                line_breakpoints[state].find(ar->short_src) != line_breakpoints[state].end())
            {
              for (list<int>::iterator iter = line_breakpoints[state][ar->short_src].begin();
                   iter != line_breakpoints[state][ar->short_src].end(); ++iter)
              {
                if (*iter == ar->currentline) {
                  handled = true;
                  break;
                }
              }
            }
            
            if (!handled && single_step.find(state) != single_step.end() &&
                single_step[state])
            {
              single_step.erase(state);
              handled = true;
            }
            
            if (handled) {
              callbacks[state]->slub_debug(state, ar);
            }
            
            updateHooks(state);
            
            break;
          }
            
        }
        
      }
    }
    
    void debugger::addHook(lua_State* state, int hook, int count) {
      int mask = lua_gethookmask(state);
      if ((mask | hook) != mask) {
        lua_sethook(state, __debug_hook, mask | hook, count);
      }
    }
    
    void debugger::removeHook(lua_State* state, int hook) {
      int mask = lua_gethookmask(state);
      int count = lua_gethookcount(state);
      if ((mask | hook) == mask) {
        lua_sethook(state, __debug_hook, mask ^ hook, count);
      }
    }
    
    void debugger::updateHooks(lua_State* state) {
      if (symbol_breakpoints.find(state) == symbol_breakpoints.end()) {
        removeHook(state, LUA_MASKCALL);
      }
      if ((single_step.find(state) == single_step.end() || !single_step[state]) &&
          line_breakpoints.find(state) == line_breakpoints.end())
      {
        removeHook(state, LUA_MASKLINE);
      }
    }
    
    // traceback function from lua.c
    int debugger::traceback (lua_State *L)
    {
      // look up Lua's 'debug.traceback' function
      lua_getglobal(L, "debug");
      if (!lua_istable(L, -1))
      {
        lua_pop(L, 1);
        return 1;
      }
      lua_getfield(L, -1, "traceback");
      if (!lua_isfunction(L, -1))
      {
        lua_pop(L, 2);
        return 1;
      }
      lua_call(L, 0, 1);  /* call debug.traceback */
      return lua_gettop(L);
    }
    
  }
  
}
