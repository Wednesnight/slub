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

#include <iostream>
#include <iosfwd>
#include <iterator>
#include <algorithm>

#include <slub/slub.h>
#include <slub/globals.h>
#include <slub/table.h>

#include "foo.h"

namespace slub {

  reference compile(lua_State* L, const string& chunk, const string& name = "") {
    string code = "return "+ chunk;
    if (luaL_loadbuffer(L, code.c_str(), code.size(), (name.empty() ? chunk.c_str() : name.c_str())) == 0) {
      return call<reference>(reference(L));
    }
    return reference();
  }
  
  struct debugger {

  public:

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
    
  private:

    map<lua_State*, debugger_callback*> callbacks;

    map<lua_State*, list<string> > symbol_breakpoints;
    map<lua_State*, map<string, list<int> > > line_breakpoints;

    map<lua_State*, bool> single_step;

    static debugger* getDebuggerFromState(lua_State* state) {
      debugger* dbg = NULL;
      if (lua_getmetatable(state, LUA_GLOBALSINDEX)) {
        slub::table mt = slub::reference(state);
        dbg = mt["__debugger"].cast<debugger*>();
      }
      return dbg;
    }

    static void __debug_hook(lua_State* state, lua_Debug* ar) {
      debugger* dbg = getDebuggerFromState(state);
      if (dbg != NULL && lua_getinfo(state, "nSlu", ar)) {
        dbg->debug_hook(state, ar);
      }
    }

    void debug_hook(lua_State* state, lua_Debug *ar) {
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

        if (handled && single_step.find(state) != single_step.end() &&
            single_step[state])
        {
          single_step.erase(state);
        }

      }
    }

    void addHook(lua_State* state, int hook, int count = 0) {
      int mask = lua_gethookmask(state);
      if ((mask | hook) != mask) {
        lua_sethook(state, __debug_hook, mask | hook, count);
      }
    }

    void removeHook(lua_State* state, int hook) {
      int mask = lua_gethookmask(state);
      int count = lua_gethookcount(state);
      if ((mask | hook) == mask) {
        lua_sethook(state, __debug_hook, mask ^ hook, count);
      }
    }

    void updateHooks(lua_State* state) {
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
    int traceback (lua_State *L)
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

  public:
    
    debugger() {
    }

    void attach(lua_State* state, debugger_callback* callback) {
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

    void detach(lua_State* state) {
      if (lua_getmetatable(state, LUA_GLOBALSINDEX)) {
        slub::table mt = slub::reference(state);
        mt["__debugger"] = NULL;
        slub::converter<const slub::table&>::push(state, mt);
        lua_setmetatable(state, LUA_GLOBALSINDEX);
      }
      callbacks.erase(state);
    }

    int evaluate(lua_State* state, const string& chunk) {
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

    list<string> symbolBreakpoints(lua_State* state) {
      list<string> breakpoints;
      if (symbol_breakpoints.find(state) != symbol_breakpoints.end()) {
        breakpoints.insert(breakpoints.begin(), symbol_breakpoints[state].begin(),
                           symbol_breakpoints[state].end());
      }
      return breakpoints;
    }

    bool toggleSymbolBreakpoint(lua_State* state, string symbol) {
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

    bool toggleLineBreakpoint(lua_State* state, const string& module, int line) {
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

    void step(lua_State* state) {
      single_step[state] = true;
      addHook(state, LUA_MASKLINE);
    }

    string stack(lua_State* state) {
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

    lua_variables variables(lua_State* state, lua_Debug *ar) {
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

    lua_variables locals(lua_State* state, lua_Debug *ar) {
      lua_variables result;
      int index = 1;
      const char* name = NULL;
      while ((name = lua_getlocal(state, ar, index))) {
        result.push_back(lua_variable(state, index++, name));
      }
      return result;
    }

    lua_variables upvalues(lua_State* state, lua_Debug *ar) {
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
    
  };

  struct commandline_debugger : public debugger::debugger_callback {

  private:

    enum Mode {
      DEFAULT = 0,
      CONTINUE = 1,
      STEP = 2,
      RETURN = 3
    };
    
    debugger dbg;
    Mode mode;
    bool initialized;

  public:
    commandline_debugger(lua_State* state)
    : mode(DEFAULT),
      initialized(false)
    {
      dbg.attach(state, this);
      dbg.step(state);
    }

    void slub_debug(lua_State* state, lua_Debug *ar) {
      if (!initialized) {
        initialized = true;
        std::cout << "slub commandline debugger - type help or ? for a list of commands" << std::endl;
      }

      switch (ar->event) {
        case LUA_HOOKCALL: {
          std::cout << "breakpoint (symbol): " << ar->name << std::endl;
          break;
        }
        case LUA_HOOKLINE: {
          std::cout << "breakpoint (line): " << ar->short_src << ", line " << ar->currentline << std::endl;
          break;
        }
      }

      for (;;) {
        switch (mode) {

          case STEP:
          case CONTINUE: {
            mode = DEFAULT;
            break;
          }

          case RETURN: {
            if (ar->event == LUA_HOOKRET) {
              mode = DEFAULT;
            }
            break;
          }

          default: {
            // prompt
            std::cout << "debug>";

            // read next command
            string line;
            if (std::getline(std::cin, line)) {

              // parse command
              istringstream iss(line);
              string command, args;
              iss >> command;
              args = line.substr(command.size());
              for (; !args.empty() && ::isspace(*(args.begin())); args.erase(args.begin()));
              for (; !args.empty() && ::isspace(*(args.end()-1)); args.erase(args.end()-1));
              std::transform(command.begin(), command.end(), command.begin(),
                             ::tolower);

              // handle command
              if (command == "?" || command == "help") {
                std::cout << "slub commandline debugger help" << std::endl << std::endl;
                std::cout << "\t" << "c, continue"                 << "\t\t\t\t\t\t"   << "continue execution"                 << std::endl;
                std::cout << "\t" << "s, step"                     << "\t\t\t\t\t\t\t" << "step into next line of execution"   << std::endl;
                std::cout << "\t" << "t <symbol>, toggle <symbol>" << "\t\t"           << "toggle symbol breakpoint"           << std::endl;
                std::cout << "\t" << "bp, breakpoints"             << "\t\t\t\t\t"     << "show active breakpoints"            << std::endl;
                std::cout << "\t" << "e <code>, eval <code>"       << "\t\t\t"         << "evaluate Lua code within current"   << std::endl
                          << "\t\t\t\t\t\t\t\t\t\t"                                    << "execution context"                  << std::endl;
                std::cout << "\t" << "source"                      << "\t\t\t\t\t\t\t" << "show the source of the current"     << std::endl
                          << "\t\t\t\t\t\t\t\t\t\t"                                    << "execution context"                  << std::endl;
                std::cout << "\t" << "line"                        << "\t\t\t\t\t\t\t" << "show the current line of execution" << std::endl;
                std::cout << "\t" << "what"                        << "\t\t\t\t\t\t\t" << "show the type of the current"       << std::endl
                          << "\t\t\t\t\t\t\t\t\t\t"                                    << "execution context"                  << std::endl;
                std::cout << "\t" << "where"                       << "\t\t\t\t\t\t\t" << "show the detailed location of the"  << std::endl
                          << "\t\t\t\t\t\t\t\t\t\t"                                    << "current execution context"          << std::endl;
                std::cout << "\t" << "stack"                       << "\t\t\t\t\t\t\t" << "show the current stack trace"       << std::endl;
                std::cout << "\t" << "variables"                   << "\t\t\t\t\t\t"   << "show global variables of the"       << std::endl
                          << "\t\t\t\t\t\t\t\t\t\t"                                    << "current execution context"          << std::endl;
                std::cout << "\t" << "locals"                      << "\t\t\t\t\t\t\t" << "show local variables of the current"<< std::endl
                          << "\t\t\t\t\t\t\t\t\t\t"                                    << "execution context"                  << std::endl;
                std::cout << "\t" << "upvalues"                    << "\t\t\t\t\t\t"   << "show upvalues of the current"       << std::endl
                          << "\t\t\t\t\t\t\t\t\t\t"                                    << "execution context"                  << std::endl;
              }

              else if (command == "c" || command == "continue") {
                mode = CONTINUE;
              }

              else if (command == "s" || command == "step") {
                mode = STEP;
                dbg.step(state);
              }

              else if (command == "t" || command == "toggle") {
                dbg.toggleSymbolBreakpoint(state, args);
              }

              else if (command == "bp" || command == "breakpoints") {
                list<string> symbols = dbg.symbolBreakpoints(state);
                if (symbols.size() == 0) {
                  std::cout << "no symbol breakpoints" << std::endl;
                }
                else {
                  std::cout << "symbol breakpoints:" << std::endl;
                  for (list<string>::iterator iter = symbols.begin(); iter != symbols.end(); ++iter) {
                    std::cout << "\t" << *iter << std::endl;
                  }
                }
              }
              
              else if (command == "e" || command == "eval") {
                int numResults = dbg.evaluate(state, args);
                if (numResults > 0) {
                  for (int index = numResults-1; index >= 0; --index) {
                    int top = lua_gettop(state) - index;
                    if (lua_isstring(state, top)) {
                      std::cout << lua_tostring(state, top);
                    }
                    else {
                      std::cout << luaL_typename(state, top);
                    }
                    if (index > 0) {
                      std::cout << ", ";
                    }
                    else {
                      std::cout << std::endl;
                    }
                  }
                  lua_pop(state, numResults);
                }
              }
              
              else if (command == "source") {
                std::cout << (ar->short_src && *ar->short_src
                              ? ar->short_src : "[unknown]") << std::endl;
              }

              else if (command == "line") {
                std::cout << "line " << ar->currentline << std::endl;
              }
              
              else if (command == "what") {
                std::cout << (ar->namewhat && *ar->namewhat ? ar->namewhat + string(" ") : "")
                  << (ar->name && *ar->name ? ar->name : "[unknown]")
                    << (ar->what && *ar->what ? string(" (") + ar->what + string(")") : "")
                      << std::endl;
              }
              
              else if (command == "where") {
                std::cout << (ar->short_src && *ar->short_src ? ar->short_src : "[unknown]")
                  << ", line " << ar->currentline << ", "
                    << (ar->namewhat && *ar->namewhat ? ar->namewhat + string(" ") : "")
                      << (ar->name && *ar->name ? ar->name : "[unknown]")
                        << (ar->what && *ar->what ? string(" (") + ar->what + string(")") : "")
                          << std::endl;
              }

              else if (command == "stack") {
                std::cout << dbg.stack(state) << std::endl;
              }

              else if (command == "variables") {
                debugger::lua_variables variables = dbg.variables(state, ar);
                if (variables.size() == 0) {
                  std::cout << "no variables defined" << std::endl;
                }
                else {
                  std::cout << "variables:" << std::endl;
                  for (debugger::lua_variables::iterator iter = variables.begin();
                       iter != variables.end(); ++iter)
                  {
                    std::cout << "\t" << iter->name << " = " << iter->value.toString()
                      << " [" << iter->value.typeName() << "]" << std::endl;
                  }
                }
              }

              else if (command == "locals") {
                debugger::lua_variables locals = dbg.locals(state, ar);
                if (locals.size() == 0) {
                  std::cout << "no locals defined" << std::endl;
                }
                else {
                  std::cout << "locals:" << std::endl;
                  for (debugger::lua_variables::iterator iter = locals.begin();
                       iter != locals.end(); ++iter)
                  {
                    std::cout << "\t" << iter->index << ": " << iter->name << " = "
                      << iter->value.toString() << " [" << iter->value.typeName() << "]" << std::endl;
                  }
                }
              }

              else if (command == "upvalues") {
                debugger::lua_variables upvalues = dbg.upvalues(state, ar);
                if (upvalues.size() == 0) {
                  std::cout << "no upvalues defined" << std::endl;
                }
                else {
                  std::cout << "upvalues:" << std::endl;
                  for (debugger::lua_variables::iterator iter = upvalues.begin();
                       iter != upvalues.end(); ++iter)
                  {
                    std::cout << "\t" << iter->index << ": " << iter->name << " = "
                      << iter->value.toString() << " [" << iter->value.typeName() << "]" << std::endl;
                  }
                }
              }
              
              else if (!command.empty()) {
                std::cout << "unknown command: '" << command << "'" << std::endl;
              }

            }
          }

        }

        if (mode != DEFAULT) {
          break;
        }
      }
    }

  };

  struct chunk : public reference {

    chunk(lua_State* state, const string& chunk, const string& name = "")
    : reference()
    {
      string code = "return "+ chunk;
      if (luaL_loadbuffer(state, code.c_str(), code.size(), (name.empty() ? chunk.c_str() : name.c_str())) == 0) {
        reference::operator=(call<reference>(reference(state)));
      }
    }

    template<typename valueType>
    void pushValue(valueType value, const string& name) {
      int index = push();
      lua_getfenv(state, index);
      table env = reference(state);
      env[name] = value;
      converter<table>::push(state, env);
      lua_setfenv(state, index);
      pop();
    }

  };

}

void testing0() {
  std::cout << "testing0()" << std::endl;
}

void testing1(int i) {
  std::cout << "testing1(" << i << ")" << std::endl;
}

void testing2(int i, int j) {
  std::cout << "testing2(" << i << ", " << j << ")" << std::endl;
}

void testing3(int i, int j, int k) {
  std::cout << "testing3(" << i << ", " << j << ", " << k << ")" << std::endl;
}

int testing(int i, int j, int k) {
  return i+j+k;
}

enum enm {
  e1,
  e2
};

std::tr1::shared_ptr<foo> test_null_value() {
  return std::tr1::shared_ptr<foo>();
}

struct invisible {
};

int main (int argc, char * const argv[]) {

  try {
    std::tr1::shared_ptr<baz> b = baz::create(0,0);
    std::cout << typeid(b).name() << std::endl;
    std::cout << typeid(&b).name() << std::endl;
    std::cout << typeid(b.get()).name() << std::endl;
    std::cout << typeid(*b.get()).name() << std::endl;

    lua_State *L = lua_open();

    luaopen_base(L);
    luaopen_table(L);
    luaopen_string(L);
    luaopen_math(L);
    luaopen_debug(L);

/*
    slub::debugger dbg;
    dbg.attach(L);
    dbg.evaluate(L, "for k,v in next,_G do print(k,v) end");
    dbg.toggleSymbolBreakpoint(L, "trigger_breakpoint");
    dbg.toggleSymbolBreakpoint(L, "trigger_breakpoint");
    dbg.toggleSymbolBreakpoint(L, "trigger_breakpoint");
    dbg.toggleSymbolBreakpoint(L, "invalid_symbol_name");
    dbg.toggleLineBreakpoint(L, "[string \"function trigger_breakpoint() end trigger_b...\"]", 1);
    dbg.evaluate(L, "function trigger_breakpoint() end trigger_breakpoint()");
*/
    slub::commandline_debugger dbg(L);

    slub::globals _G(L);

    {
      std::cout << _G["math"].typeName() << std::endl;
      std::cout << _G["math"]["abs"].typeName() << std::endl;
      std::cout << slub::call<int, int>(_G["math"]["abs"], -2) << std::endl;

      _G["foobarbaz"] = "argrml";
      std::cout << _G["foobarbaz"].cast<std::string>() << std::endl;
    }

    slub::package enum_ = slub::package(L, "enum");
    enum_.enumerated("e1", e1);
    enum_.enumerated("e2", e2);

    if (luaL_dostring(L, "print(enum.e1) print(enum.e2) ")) {
      std::cout << lua_tostring(L, -1) << std::endl;
    }
    
    slub::clazz<foo>(L, "foo")
      .constructor<int>()
      .constructor<int, int>()
      .field("bar", &foo::bar)
      .method("doStuff", (void(foo::*)(void))&foo::doStuff)
      .method("doStuff", (void(foo::*)(int))&foo::doStuff)
      .method("getFoo", &foo::getFoo)
      .eq()
      .lt()
      .le()
      .tostring()
      .add<int, int>()
      .sub<int, int>()
      .mul<int, int>()
      .div<int, int>()
      .mod<int, int>()
      .pow<int, int>()
      .unm<int>()
      .function("create", (std::tr1::shared_ptr<foo>(*)(int))&foo::create)
      .function("create", (std::tr1::shared_ptr<foo>(*)(int, int))&foo::create);

    if (luaL_dofile(L, "foo.lua")) {
      std::cout << lua_tostring(L, -1) << std::endl;
    }

    slub::clazz<baz>(L, "baz").extends<foo>()
      .constructor<int, int>()
      .method("get", &baz::get)
      .method("print", &baz::print)
      .function("create", &baz::create);

    if (luaL_dostring(L,
                      "local b = baz.create(10, 11) "
                      "print(tostring(b)) "
                      "print(b.bar) "
                      "b:doStuff() "
                      "b:doStuff(100) "
                      "local f = foo.create(10) "
                      "print(f:getFoo(b)) "
                      "b:print(b:get()) ")) {
      std::cout << lua_tostring(L, -1) << std::endl;
    }

    slub::clazz<std::string>(L, "str").constructor();

    if (luaL_dostring(L, "local s = str()")) {
      std::cout << lua_tostring(L, -1) << std::endl;
    }

    slub::function(L, "testing0", &testing0);
    slub::function(L, "testing1", &testing1);
    slub::function(L, "testing2", &testing2);
    slub::function(L, "testing3", &testing3);
    slub::function(L, "testing", &testing);

    if (luaL_dostring(L,
                      "testing0() "
                      "testing1(1) "
                      "testing2(1, 2) "
                      "testing3(1, 2, 3) "
                      "print(testing(1, 2, 3)) ")) {
      std::cout << lua_tostring(L, -1) << std::endl;
    }

    slub::lua_function<slub::string, slub::string, slub::string, slub::reference> gsub =
      slub::globals(L)["string"]["gsub"];

    slub::chunk cb(L, "function(s) "
                      "  print(s) "
                      "  faz:doStuff() "
                      "  return \"baz\" "
                      "end ", "callback");

    std::tr1::shared_ptr<foo> faz = foo::create(10);
    cb.pushValue(faz, "faz");

    std::cout << gsub("foo bar faz", "faz", cb) << std::endl;
    luaL_dostring(L, "print(type(faz))");

    slub::table myTable(L);
    myTable["foo"] = "bar";
    _G["myTable"] = myTable;
    myTable["bar"] = "baz";
    luaL_dostring(L, "for k,v in next,myTable do print(k, v) end");

    myTable.insert("1string");
    myTable.insert("another");
    myTable.insert(3);
    myTable.insert(false);
    std::tr1::shared_ptr<foo> f = foo::create(10);
    myTable.insert(f);
    std::cout << myTable.concat(", ") << std::endl;
    myTable.remove(2); // "another"
    myTable.remove(3); // false
    std::cout << myTable.concat(", ") << std::endl;
    myTable.insert("another", 2);
    myTable.insert(false, 4);
    std::cout << myTable.concat(", ") << std::endl;
    myTable.remove();
    std::cout << myTable.concat(", ") << std::endl;

    slub::reference nil;
    _G["nilvalue"] = nil;
    luaL_dostring(L, "print(\"type(nilvalue): \"..type(nilvalue))");

    slub::function(L, "test_null_value", &test_null_value);
    luaL_dostring(L, "local null_value = test_null_value() print(type(null_value))");

    // invisible class binding
    slub::clazz<invisible>((lua_State*) L);

    luaL_dostring(L, "function breakpoint()\n"
                     "  print(\"breakpoint()\")\n"
                     "end\n"
                     "breakpoint()");

    lua_gc(L, LUA_GCCOLLECT, 0);
    lua_close(L);
  }
  catch (std::exception& e) {
    std::cout << e.what() << std::endl;
  }

  return 0;
}
