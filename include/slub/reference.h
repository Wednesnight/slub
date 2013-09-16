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

#ifndef SLUB_REFERENCE_H
#define SLUB_REFERENCE_H

#include "config.h"
#include "call.h"
#include "converter.h"

#include <stdexcept>

namespace slub {

  template<typename T>
  struct converter;

  struct reference {

    template<typename T>
    friend struct converter;

    reference() : state(NULL), index(LUA_REFNIL) {
    }
    
    reference(lua_State* state) : state(state), index(luaL_ref(state, LUA_REGISTRYINDEX)) {
    }
    
    reference(lua_State* state, int index) : state(state) {
      lua_pushvalue(state, index);
      this->index = luaL_ref(state, LUA_REGISTRYINDEX);
    }
    
    reference(const reference& r) {
      this->operator=(r);
    }
    
    ~reference() {
      if (index != LUA_REFNIL && state != NULL && lua_status(state) == 0) {
        luaL_unref(state, LUA_REGISTRYINDEX, index);
      }
      state = NULL;
      index = LUA_REFNIL;
    }

    virtual bool operator==(const reference& r) {
      bool result = lua_equal(state, push(), r.push());
      lua_pop(state, 2);
      return result;
    }

    virtual void operator=(const reference& r) {
      this->state = r.state;
      if (r.index != LUA_REFNIL) {
        r.push();
        this->index = luaL_ref(state, LUA_REGISTRYINDEX);
      }
      else {
        this->index = LUA_REFNIL;
      }
    }
    
    int type() const {
      int result = LUA_TNIL;
      if (state != NULL && index != LUA_REFNIL) {
        result = lua_type(state, push());
        pop();
      }
      return result;
    }
    
    string typeName() const {
      // state can be NULL since it is not used in lua_typename()
      return lua_typename(state, type());
    }
    
    template<typename T>
    T cast() const {
      if (state == NULL || index == LUA_REFNIL) {
        throw std::runtime_error("trying to cast a nil value");
      }
      T result = converter<T>::get(state, push());
      pop();
      return result;
    }

    bool isNil() {
      return state == NULL || index == LUA_REFNIL || type() == LUA_TNIL ||
        type() == LUA_TNONE;
    }

    lua_State* getState() const {
      return state;
    }

    string toString() const {
      string result = typeName();
      int index = push();
      if (lua_isstring(state, index)) {
        result = lua_tostring(state, index);
      }
      pop();
      return result;
    }

  protected:

    lua_State* state;
    int index;

    int push() const {
      return push(state);
    }

    int push(lua_State* state) const {
      if (state == NULL && index == LUA_REFNIL) {
        throw std::runtime_error("trying to push a nil value");
      }
      if (index != LUA_REFNIL) {
        lua_rawgeti(state, LUA_REGISTRYINDEX, index);
      }
      else {
        lua_pushnil(state);
      }
      return lua_gettop(state);
    }

    void pop() const {
      pop(state);
    }

    void pop(lua_State* state) const {
      if (state == NULL && index == LUA_REFNIL) {
        throw std::runtime_error("trying to pop a nil value");
      }
      lua_pop(state, 1);
    }

  };

  template<>
  struct converter<reference> {
    
    static bool check(lua_State* L, int index) {
      return true;
    }
    
    static reference get(lua_State* L, int index) {
      lua_pushvalue(L, index);
      return reference(L);
    }
    
    static int push(lua_State* L, const reference& ref) {
      ref.push(L);
      return 1;
    }
    
  };
  
  template<>
  struct converter<reference&> : converter<reference> {};
  
  template<>
  struct converter<const reference&> : converter<reference> {};
  
}

#endif
