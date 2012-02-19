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

#include <slub/slub.h>
#include <slub/globals.h>
#include <slub/table.h>

#include "foo.h"

namespace slub {

  struct debugger {

  private:

    lua_State* state;
    list<string> func_breakpoints;

    static void __debug_hook(lua_State *L, lua_Debug *ar) {
      if (lua_getmetatable(L, LUA_GLOBALSINDEX)) {
        slub::table mt = slub::reference(L);
        const debugger* dbg = mt["__debugger"].cast<const debugger*>();
        if (dbg != NULL) {
          dbg->debug_hook(ar);
        }
      }
    }

    void debug_hook(lua_Debug *ar) const {
      if (lua_getinfo(state, "nSlu", ar)) {
        std::cout << "call: "
        << (ar->namewhat != 0 ? ar->namewhat + string(" ") : "")
        << (ar->name != 0 ? ar->name + string(" ") : "")
        << " (" << ar->what << ")"
        << std::endl;
      }
    }

  public:
    
    debugger(lua_State* state) : state(state) {
      slub::clazz<debugger>((lua_State*) state);

      slub::table mt(state);
      if (lua_getmetatable(state, LUA_GLOBALSINDEX)) {
        mt = slub::reference(state);
      }
      mt["__debugger"] = (const debugger*) this;
      slub::converter<const slub::table&>::push(state, mt);
      lua_setmetatable(state, LUA_GLOBALSINDEX);
    }

    void evaluate(const char* chunk) {
      luaL_dostring(state, chunk);
    }

    bool toggleBreakpoint(string symbol) {
      for (list<string>::iterator iter = func_breakpoints.begin(); iter != func_breakpoints.end(); ++iter) {
        if (*iter == symbol) {
          func_breakpoints.erase(iter);
          return false;
        }
      }
      func_breakpoints.push_back(symbol);
      if (lua_gethookmask(state) | LUA_MASKCALL) {
        lua_sethook(state, __debug_hook, LUA_MASKCALL, 0);
      }
      return true;
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

  reference compile(lua_State* L, const string& chunk, const string& name = "") {
    string code = "return "+ chunk;
    if (luaL_loadbuffer(L, code.c_str(), code.size(), (name.empty() ? chunk.c_str() : name.c_str())) == 0) {
      return call<reference>(reference(L));
    }
    return reference();
  }
  
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

    slub::debugger dbg(L);
    dbg.evaluate("for k,v in next,_G do print(k,v) end");
    dbg.toggleBreakpoint("trigger_breakpoint");
    dbg.evaluate("function trigger_breakpoint() end trigger_breakpoint()");

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

    slub::reference nil;
    _G["nilvalue"] = nil;
    luaL_dostring(L, "print(\"type(nilvalue): \"..type(nilvalue))");

    slub::function(L, "test_null_value", &test_null_value);
    luaL_dostring(L, "local null_value = test_null_value() print(type(null_value))");

    // invisible class binding
    slub::clazz<invisible>((lua_State*) L);

    lua_gc(L, LUA_GCCOLLECT, 0);
    lua_close(L);
  }
  catch (std::exception& e) {
    std::cout << e.what() << std::endl;
  }

  return 0;
}
