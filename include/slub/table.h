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

#ifndef slub_table_h
#define slub_table_h

#include "config.h"
#include "reference.h"
#include "slub_lua.h"
//#include "globals.h"

namespace slub {

  struct globals;

  struct table_entry : public reference {
    
  protected:
    
    friend struct globals;

    reference key;
    reference parent;
    
    table_entry() : reference() {
    }
    
    table_entry(lua_State* state) : reference(state) {
    }
    
    table_entry(lua_State* state, int index) : reference(state, index) {
    }
    
    table_entry(const reference& r) : reference(r) {
    }
    
  public:

    /*
     * C++98 requires an accessible copy constructor when binding a reference to
     * a temporary; was protected
     */
    table_entry(const table_entry& r) {
      reference::operator=(r);
      key = r.key;
      parent = r.parent;
    }
    
    void operator=(const table_entry& r) {
      converter<reference>::push(state, parent);
      int table_index = lua_gettop(state);
      converter<reference>::push(state, key);
      converter<reference>::push(state, r);
      lua_settable(state, table_index);
      lua_pop(state, 1);
    }

    template<typename valueType>
    void operator=(valueType value) {
      converter<reference>::push(state, parent);
      int table_index = lua_gettop(state);
      converter<reference>::push(state, key);
      converter<valueType>::push(state, value);
      lua_settable(state, table_index);
      lua_pop(state, 1);
    }
    
    template<typename indexType>
    table_entry operator[](indexType index) const {
      int table_index = push();
      converter<indexType>::push(state, index);
      lua_pushvalue(state, lua_gettop(state));
      lua_gettable(state, table_index);
      table_entry result(state);
      result.key = reference(state);
      result.parent = reference(state);
      return result;
    }

  };
  
  template<>
  struct converter<table_entry> : converter<reference> {};
  
  template<>
  struct converter<table_entry&> : converter<reference> {};
  
  template<>
  struct converter<const table_entry&> : converter<reference> {};
  
  struct table : public table_entry {
    
  public:
    
    table() {}

    table(lua_State* state)
    : table_entry()
    {
      lua_newtable(state);
      reference::operator=(reference(state));
    }
    
    table(const reference& r)
    : table_entry(r)
    {
    }
    
    string concat(string sep = "", int offset = 1, int length = 0) const {
      if (length == 0) {
        length = (int)maxn();
      }
      
      string result;
      while (offset <= length) {
        if (!result.empty()) {
          result += sep;
        }
        reference entry = this->operator[](offset++);
        converter<reference>::push(state, entry);
        int index = lua_gettop(state);
        if (lua_getmetatable(state, index)) {
          table mt = reference(state);
          if (mt["__tostring"].type() == LUA_TFUNCTION) {
            result += call<string, reference>(mt["__tostring"], entry);
          }
          else if (lua_isstring(state, index)) {
            result += lua_tostring(state, index);
          }
          else {
            result += entry.typeName();
          }
        }
        else if (lua_isstring(state, index)) {
          result += lua_tostring(state, index);
        }
        else {
          result += entry.typeName();
        }
        lua_pop(state, 1);
      }
      return result;
    }
    
    template<typename valueType>
    void insert(valueType value, int index = 0) {
      if (index == 0) {
        index = (int)(maxn()+1);
      }
      
      int table_index = push();
      
      int pos = (int)maxn();
      int offset = index;
      while (pos >= offset) {
        lua_rawgeti(state, table_index, pos);
        lua_rawseti(state, table_index, pos+1);
        --pos;
      }
      
      lua_pushinteger(state, index);
      converter<valueType>::push(state, value);
      lua_settable(state, table_index);
      
      lua_pop(state, 1);
    }
    
    // maxn function from ltablib.c
    lua_Number maxn() const {
      lua_Number max = 0;
      int index = push();
      luaL_checktype(state, index, LUA_TTABLE);
      lua_pushnil(state);  /* first key */
      while (lua_next(state, index)) {
        lua_pop(state, 1);  /* remove value */
        if (lua_type(state, -1) == LUA_TNUMBER) {
          lua_Number v = lua_tonumber(state, -1);
          if (v > max) max = v;
        }
      }
      pop();
      return max;
    }
    
    size_t length() const {
      size_t result = lua_objlen(state, push());
      pop();
      return result;
    }

    reference remove(int index = 0) {
      if (index == 0) {
        index = (int)maxn();
      }

      reference result;
      if (index > 0) {
        int table_index = push();
        
        lua_pushinteger(state, index);
        lua_gettable(state, table_index);
        result = reference(state);
        
        int pos = index;
        int length = (int)maxn();
        while (pos < length) {
          lua_rawgeti(state, table_index, pos+1);
          lua_rawseti(state, table_index, pos++);
        }
        
        lua_pushnil(state);
        lua_rawseti(state, table_index, length);
        
        lua_pop(state, 1);
      }
      return result;
    }
    
    /*
     * TODO: table.sort (table [, comp])
     */
    
  };

  template<>
  struct converter<table> : converter<reference> {};
  
  template<>
  struct converter<table&> : converter<reference> {};
  
  template<>
  struct converter<const table&> : converter<reference> {};

}

#endif
