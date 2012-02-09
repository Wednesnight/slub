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

namespace slub {

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

    table_entry(const table_entry& r) {
      this->operator=(r);
    }
    
    void operator=(const table_entry& r) {
      reference::operator=(r);
      key = r.key;
      parent = r.parent;
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
    table_entry operator[](indexType index) {
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
  
  struct table : public table_entry {
    
  private:
    
    const reference& ref;
    
  public:
    
    table(const reference& ref)
    : ref(ref)
    {
    }
    
    string concat(string sep = "", int offset = 1, int length = -1) {
      if (length == -1) {
        length = maxn();
      }
      
      string result;
      while (offset <= length) {
        result += (result.empty() ? "" : sep) + this->operator[](offset++).cast<string>();
      }
      return result;
    }
    
    void insert(const reference& value, int index = -1) {
      if (index == -1) {
        index = maxn()+1;
      }
      
      converter<reference>::push(ref.getState(), ref);
      int table_index = lua_gettop(ref.getState());
      
      int pos = maxn();
      int offset = index;
      while (pos >= offset) {
        lua_rawgeti(ref.getState(), table_index, pos);
        lua_rawseti(ref.getState(), table_index, pos+1);
        --pos;
      }
      
      lua_pushinteger(ref.getState(), index);
      converter<reference>::push(value.getState(), value);
      lua_settable(ref.getState(), table_index);
      
      lua_pop(ref.getState(), 1);
    }
    
    size_t maxn() {
      converter<reference>::push(ref.getState(), ref);
      size_t result = lua_objlen(ref.getState(), lua_gettop(ref.getState()));
      lua_pop(ref.getState(), 1);
      return result;
    }
    
    reference remove(int index = -1) {
      if (index == -1) {
        index = maxn();
      }
      
      converter<reference>::push(ref.getState(), ref);
      int table_index = lua_gettop(ref.getState());
      
      lua_pushinteger(ref.getState(), index);
      lua_gettable(ref.getState(), table_index);
      reference result(ref.getState());
      
      int pos = index;
      int length = maxn();
      while (pos < length) {
        lua_rawgeti(ref.getState(), table_index, pos+1);
        lua_rawseti(ref.getState(), table_index, pos);
        --pos;
      }
      
      lua_pushnil(ref.getState());
      lua_rawseti(ref.getState(), table_index, maxn());
      
      lua_pop(ref.getState(), 1);
      return result;
    }
    
    /*
     * TODO: table.sort (table [, comp])
     */
    
  };

}

#endif