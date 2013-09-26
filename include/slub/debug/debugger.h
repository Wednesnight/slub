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

#ifndef SLUB_DEBUG_DEBUGGER_H
#define SLUB_DEBUG_DEBUGGER_H

#include "../clazz.h"
#include "../reference.h"
#include "../slub_lua.h"
#include "../table.h"

namespace slub {

  namespace debug {

    struct debugger_callback {
      virtual void slub_debug(lua_State*, lua_Debug*) = 0;
    };
    
    struct lua_variable {
      
    private:
      friend struct debugger;
      
      lua_variable(lua_State* state, int index, const string& name)
      : index(index),
      name(name),
      value(state)
      {        
      }
      
      lua_variable(int index, const string& name,
                   const reference& value)
      : index(index),
      name(name),
      value(value)
      {        
      }
      
    public:
      int index;
      string name;
      reference value;
      
    };
    
    typedef list<lua_variable> lua_variables;
    
    struct debugger {
      
    public:
      
      debugger();
      
      void attach(lua_State* state, debugger_callback* callback);
      void detach(lua_State* state);
      
      int evaluate(lua_State* state, const string& chunk);
      
      list<string> symbolBreakpoints(lua_State* state);
      bool toggleSymbolBreakpoint(lua_State* state, string symbol);
      
      bool toggleLineBreakpoint(lua_State* state, const string& module, int line);
      
      void step(lua_State* state);
      
      string stack(lua_State* state);
      
      lua_variables variables(lua_State* state, lua_Debug *ar);      
      lua_variables locals(lua_State* state, lua_Debug *ar);
      lua_variables upvalues(lua_State* state, lua_Debug *ar);
      
    private:
      
      map<lua_State*, debugger_callback*> callbacks;
      map<lua_State*, list<string> > symbol_breakpoints;
      map<lua_State*, map<string, list<int> > > line_breakpoints;
      map<lua_State*, bool> single_step;
      
      static debugger* getDebuggerFromState(lua_State* state);
      
      static void __debug_hook(lua_State* state, lua_Debug* ar);
      
      void debug_hook(lua_State* state, lua_Debug *ar);
      
      void addHook(lua_State* state, int hook, int count = 0);      
      void removeHook(lua_State* state, int hook);
      void updateHooks(lua_State* state);
      
      // traceback function from lua.c
      int traceback (lua_State *L);
      
    };

  }

}

#endif
