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

#ifndef SLUB_FIELD_H
#define SLUB_FIELD_H

#include "converter.h"
#include "registry.h"
#include "wrapper.h"

namespace slub {

  struct abstract_field {
    virtual int get(lua_State*) = 0;
    virtual int set(lua_State*) = 0;
  };

  template<typename T, typename F>
  struct field : public abstract_field {

    F T::*m;

    field(F T::*m) : m(m) {
    }

    int get(lua_State* L) {
      wrapper<T*>* t = static_cast<wrapper<T*>*>(converter<T>::checkudata(L, 1));
      return converter<F*>::push(L, &(t->ref()->*m));
    }

    int set(lua_State* L) {
      wrapper<T*>* t = static_cast<wrapper<T*>*>(converter<T>::checkudata(L, 1));
      t->ref()->*m = converter<F>::get(L, -1);
      return 0;
    }

  };

  template<typename T, typename F>
  struct field<T, F*> : public abstract_field {
    
    F* T::*m;
    
    field(F* T::*m) : m(m) {
    }
    
    int get(lua_State* L) {
      wrapper<T*>* t = static_cast<wrapper<T*>*>(converter<T>::checkudata(L, 1));
      return converter<F*>::push(L, t->ref()->*m);
    }
    
    int set(lua_State* L) {
      wrapper<T*>* t = static_cast<wrapper<T*>*>(converter<T>::checkudata(L, 1));
      t->ref()->*m = converter<F*>::get(L, -1);
      return 0;
    }
    
  };
  
  template<typename T, typename F>
  struct field<T, const F*> : public abstract_field {
    
    const F* T::*m;
    
    field(const F* T::*m) : m(m) {
    }
    
    int get(lua_State* L) {
      wrapper<T*>* t = static_cast<wrapper<T*>*>(converter<T>::checkudata(L, 1));
      return converter<const F*>::push(L, t->ref()->*m);
    }
    
    int set(lua_State* L) {
      wrapper<T*>* t = static_cast<wrapper<T*>*>(converter<T>::checkudata(L, 1));
      t->ref()->*m = converter<const F*>::get(L, -1);
      return 0;
    }
    
  };
  
  template<typename T, typename F>
  struct field<T, boost::shared_ptr<F> > : public abstract_field {
    
    boost::shared_ptr<F> T::*m;
    
    field(boost::shared_ptr<F> T::*m) : m(m) {
    }
    
    int get(lua_State* L) {
      wrapper<T*>* t = static_cast<wrapper<T*>*>(converter<T>::checkudata(L, 1));
      return converter<boost::shared_ptr<F> >::push(L, t->ref()->*m);
    }
    
    int set(lua_State* L) {
      wrapper<T*>* t = static_cast<wrapper<T*>*>(converter<T>::checkudata(L, 1));
      t->ref()->*m = converter<boost::shared_ptr<F> >::get(L, -1);
      return 0;
    }
    
  };

  template<typename T, typename F>
  struct field<T, boost::shared_ptr<F>&> : field<T, boost::shared_ptr<F> > {};

  template<typename T, typename F>
  struct field<T, const boost::shared_ptr<F>&> : field<T, boost::shared_ptr<F> > {};

  template<typename T, typename F>
  struct field<T, std::tr1::shared_ptr<F> > : public abstract_field {
    
    std::tr1::shared_ptr<F> T::*m;
    
    field(std::tr1::shared_ptr<F> T::*m) : m(m) {
    }
    
    int get(lua_State* L) {
      wrapper<T*>* t = static_cast<wrapper<T*>*>(converter<T>::checkudata(L, 1));
      return converter<std::tr1::shared_ptr<F> >::push(L, t->ref()->*m);
    }
    
    int set(lua_State* L) {
      wrapper<T*>* t = static_cast<wrapper<T*>*>(converter<T>::checkudata(L, 1));
      t->ref()->*m = converter<std::tr1::shared_ptr<F> >::get(L, -1);
      return 0;
    }
    
  };

  template<typename T, typename F>
  struct field<T, std::tr1::shared_ptr<F>&> : field<T, std::tr1::shared_ptr<F> > {};

  template<typename T, typename F>
  struct field<T, const std::tr1::shared_ptr<F>&> : field<T, std::tr1::shared_ptr<F> > {};

  template<typename T>
  struct field<T, bool> : public abstract_field {
    
    bool T::*m;
    
    field(bool T::*m) : m(m) {
    }
    
    int get(lua_State* L) {
      wrapper<T*>* t = static_cast<wrapper<T*>*>(converter<T>::checkudata(L, 1));
      lua_pushboolean(L, t->ref()->*m);
      return 1;
    }
    
    int set(lua_State* L) {
      wrapper<T*>* t = static_cast<wrapper<T*>*>(converter<T>::checkudata(L, 1));
      luaL_checktype(L, -1, LUA_TBOOLEAN);
      t->ref()->*m = lua_toboolean(L, -1);
      return 0;
    }
    
  };
  
  template<typename T>
  struct field<T, int> : public abstract_field {

    int T::*m;
    
    field(int T::*m) : m(m) {
    }
    
    int get(lua_State* L) {
      wrapper<T*>* t = static_cast<wrapper<T*>*>(converter<T>::checkudata(L, 1));
      lua_pushinteger(L, t->ref()->*m);
      return 1;
    }

    int set(lua_State* L) {
      wrapper<T*>* t = static_cast<wrapper<T*>*>(converter<T>::checkudata(L, 1));
      t->ref()->*m = luaL_checkinteger(L, -1);
      return 0;
    }

  };

  template<typename T>
  struct field<T, unsigned int> : public abstract_field {
    
    unsigned int T::*m;
    
    field(unsigned int T::*m) : m(m) {
    }
    
    int get(lua_State* L) {
      wrapper<T*>* t = static_cast<wrapper<T*>*>(converter<T>::checkudata(L, 1));
      lua_pushinteger(L, t->ref()->*m);
      return 1;
    }
    
    int set(lua_State* L) {
      wrapper<T*>* t = static_cast<wrapper<T*>*>(converter<T>::checkudata(L, 1));
      t->ref()->*m = luaL_checkinteger(L, -1);
      return 0;
    }
    
  };
  
  template<typename T>
  struct field<T, float> : public abstract_field {
    
    float T::*m;
    
    field(float T::*m) : m(m) {
    }
    
    int get(lua_State* L) {
      wrapper<T*>* t = static_cast<wrapper<T*>*>(converter<T>::checkudata(L, 1));
      lua_pushnumber(L, t->ref()->*m);
      return 1;
    }
    
    int set(lua_State* L) {
      wrapper<T*>* t = static_cast<wrapper<T*>*>(converter<T>::checkudata(L, 1));
      t->ref()->*m = luaL_checknumber(L, -1);
      return 0;
    }
    
  };
  
  template<typename T>
  struct field<T, double> : public abstract_field {
    
    double T::*m;
    
    field(double T::*m) : m(m) {
    }
    
    int get(lua_State* L) {
      wrapper<T*>* t = static_cast<wrapper<T*>*>(converter<T>::checkudata(L, 1));
      lua_pushnumber(L, t->ref()->*m);
      return 1;
    }
    
    int set(lua_State* L) {
      wrapper<T*>* t = static_cast<wrapper<T*>*>(converter<T>::checkudata(L, 1));
      t->ref()->*m = luaL_checknumber(L, -1);
      return 0;
    }
    
  };
  
  template<typename T, typename F>
  struct field_method : public abstract_field {
    
    F (T::*getter)();
    void (T::*setter)(F);
    
    field_method(F (T::*getter)(), void (T::*setter)(F)) : getter(getter), setter(setter) {
    }
    
    int get(lua_State* L) {
      wrapper<T*>* t = static_cast<wrapper<T*>*>(converter<T>::checkudata(L, 1));
      return converter<F>::push(L, (t->ref()->*getter)());
    }
    
    int set(lua_State* L) {
      wrapper<T*>* t = static_cast<wrapper<T*>*>(converter<T>::checkudata(L, 1));
      (t->ref()->*setter)(converter<F>::get(L, -1));
      return 0;
    }
    
  };
  
  template<typename T, typename F>
  struct field_method_const : public abstract_field {
    
    F (T::*getter)() const;
    void (T::*setter)(F);
    
    field_method_const(F (T::*getter)() const, void (T::*setter)(F)) : getter(getter), setter(setter) {
    }
    
    int get(lua_State* L) {
      wrapper<T*>* t = static_cast<wrapper<T*>*>(converter<T>::checkudata(L, 1));
      return converter<F>::push(L, (t->ref()->*getter)());
    }
    
    int set(lua_State* L) {
      wrapper<T*>* t = static_cast<wrapper<T*>*>(converter<T>::checkudata(L, 1));
      (t->ref()->*setter)(converter<F>::get(L, -1));
      return 0;
    }
    
  };
  
  template<typename T, typename F>
  struct field_function : public abstract_field {
    
    F (*getter)(T*);
    void (*setter)(T*, F);
    
    field_function(F (*getter)(T*), void (*setter)(T*, F)) : getter(getter), setter(setter) {
    }
    
    int get(lua_State* L) {
      wrapper<T*>* t = static_cast<wrapper<T*>*>(converter<T>::checkudata(L, 1));
      return converter<F>::push(L, (*getter)(t->ref()));
    }
    
    int set(lua_State* L) {
      wrapper<T*>* t = static_cast<wrapper<T*>*>(converter<T>::checkudata(L, 1));
      (*setter)(t->ref(), converter<F>::get(L, -1));
      return 0;
    }
    
  };

  template<typename T, typename F>
  void read_only(T*, F) {}

  template<typename T, typename F>
  struct field_method_read_only : public abstract_field {
    
    F (T::*getter)();
    
    field_method_read_only(F (T::*getter)()) : getter(getter) {
    }
    
    int get(lua_State* L) {
      wrapper<T*>* t = static_cast<wrapper<T*>*>(converter<T>::checkudata(L, 1));
      return converter<F>::push(L, (t->ref()->*getter)());
    }
    
    int set(lua_State* L) {
      wrapper<T*>* t = static_cast<wrapper<T*>*>(converter<T>::checkudata(L, 1));
      read_only(t->ref(), converter<F>::get(L, -1));
      return 0;
    }
    
  };
  
}

#endif
