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

#ifndef SLUB_CLAZZ_H
#define SLUB_CLAZZ_H

#include "config.h"
#include "constructor.h"
#include "exception.h"
#include "field.h"
#include "function.h"
#include "method.h"
#include "operators.h"
#include "reference.h"
#include "registry.h"
#include "slub_lua.h"
#include "wrapper.h"

#include <utility>

namespace slub {

  struct deleter {
    template<typename T>
    static void delete_(wrapper<T*>* w) {
      if (w->holder != NULL) {
        delete w->holder;
      }
      else {
        delete w->ref();
      }
    }
  };

  struct null_deleter {
    template<typename T>
    static void delete_(wrapper<T*>* w) {
    }
  };

  struct abstract_clazz {
    static int index(lua_State* L);
    static int index(lua_State* L, const string& className, bool fallback);
    
    static int newindex(lua_State* L);
    
    static int callMethod(lua_State* L);
    static int callOperator(lua_State* L);

  protected:
    std::pair<int, int> construct(lua_State * state, registry * reg, char const * name, char const * fqname, int target);
    void add_symbols(lua_State * state, registry * reg, int methods, int metatable);
  };

  template<typename T, typename D = deleter>
  struct clazz : public abstract_clazz {

    clazz(lua_State* L)
    {
      init(L, typeid(T).name(), "slub.invisible", LUA_REGISTRYINDEX);
    }

    clazz(lua_State* L, const string& name, const string& prefix = "", int target = -1)
    {
      init(L, name, prefix, target);
    }

    template<typename B>
    clazz& extends() {
      registry* base = registry::get(typeid(B));
      reg->registerBase(base);
      return *this;
    }

    clazz& constructor() {
      reg->addConstructor(new slub::constructor<T>());
      return *this;
    }
    
    template<typename arg1>
    clazz& constructor() {
      reg->addConstructor(new slub::constructor<T, arg1>());
      return *this;
    }
    
    template<typename arg1, typename arg2>
    clazz& constructor() {
      reg->addConstructor(new slub::constructor<T, arg1, arg2>());
      return *this;
    }
    
    template<typename arg1, typename arg2, typename arg3>
    clazz& constructor() {
      reg->addConstructor(new slub::constructor<T, arg1, arg2, arg3>());
      return *this;
    }
    
    template<typename arg1, typename arg2, typename arg3, typename arg4>
    clazz& constructor() {
      reg->addConstructor(new slub::constructor<T, arg1, arg2, arg3, arg4>());
      return *this;
    }
    
    template<typename F>
    clazz& field(const string& fieldName, F T::*m) {
      reg->addField(fieldName, new slub::field<T, F>(m));
      return *this;
    }

    template<typename F>
    clazz& field(const string& fieldName, F (T::*getter)(), void (T::*setter)(F)) {
      reg->addField(fieldName, new slub::field_method<T, F>(getter, setter));
      return *this;
    }
    
    template<typename F>
    clazz& field(const string& fieldName, F (T::*getter)() const, void (T::*setter)(F)) {
      reg->addField(fieldName, new slub::field_method_const<T, F>(getter, setter));
      return *this;
    }
    
    template<typename F>
    clazz& field(const string& fieldName, F (*getter)(T*), void (*setter)(T*, F)) {
      reg->addField(fieldName, new slub::field_function<T, F>(getter, setter));
      return *this;
    }
    
    template<typename F>
    clazz& field(const string& fieldName, F (*getter)(T*)) {
      reg->addField(fieldName, new slub::field_function<T, F>(getter, &slub::read_only));
      return *this;
    }
    
    template<typename F>
    clazz& field(const string& fieldName, F (T::*getter)()) {
      reg->addField(fieldName, new slub::field_method_read_only<T, F>(getter));
      return *this;
    }
    
    template<typename ret>
    clazz& method(const string& methodName, ret (T::*m)()) {
      reg->addMethod(methodName, new slub::method<T, ret>(m));
      return *this;
    }
    
    clazz& method(const string& methodName, void (T::*m)()) {
      reg->addMethod(methodName, new slub::method<T>(m));
      return *this;
    }
    
    template<typename ret, typename arg1>
    clazz& method(const string& methodName, ret (T::*m)(arg1)) {
      reg->addMethod(methodName, new slub::method<T, ret, arg1>(m));
      return *this;
    }
    
    template<typename arg1>
    clazz& method(const string& methodName, void (T::*m)(arg1)) {
      reg->addMethod(methodName, new slub::method<T, void, arg1>(m));
      return *this;
    }
    
    template<typename ret, typename arg1, typename arg2>
    clazz& method(const string& methodName, ret (T::*m)(arg1, arg2)) {
      reg->addMethod(methodName, new slub::method<T, ret, arg1, arg2>(m));
      return *this;
    }
    
    template<typename arg1, typename arg2>
    clazz& method(const string& methodName, T* t, void (T::*m)(arg1, arg2)) {
      reg->addMethod(methodName, new slub::method<T, void, arg1, arg2>(m));
      return *this;
    }

    template<typename ret, typename arg1, typename arg2, typename arg3>
    clazz& method(const string& methodName, ret (T::*m)(arg1, arg2, arg3)) {
      reg->addMethod(methodName, new slub::method<T, ret, arg1, arg2, arg3>(m));
      return *this;
    }
    
    template<typename arg1, typename arg2, typename arg3>
    clazz& method(const string& methodName, T* t, void (T::*m)(arg1, arg2, arg3)) {
      reg->addMethod(methodName, new slub::method<T, void, arg1, arg2, arg3>(m));
      return *this;
    }
    
    template<typename ret, typename arg1, typename arg2, typename arg3, typename arg4>
    clazz& method(const string& methodName, ret (T::*m)(arg1, arg2, arg3, arg4)) {
      reg->addMethod(methodName, new slub::method<T, ret, arg1, arg2, arg3, arg4>(m));
      return *this;
    }
    
    template<typename arg1, typename arg2, typename arg3, typename arg4>
    clazz& method(const string& methodName, T* t, void (T::*m)(arg1, arg2, arg3, arg4)) {
      reg->addMethod(methodName, new slub::method<T, void, arg1, arg2, arg3, arg4>(m));
      return *this;
    }
    
    template<typename ret, typename arg1, typename arg2, typename arg3, typename arg4, typename arg5>
    clazz& method(const string& methodName, ret (T::*m)(arg1, arg2, arg3, arg4, arg5)) {
      reg->addMethod(methodName, new slub::method<T, ret, arg1, arg2, arg3, arg4, arg5>(m));
      return *this;
    }
    
    template<typename arg1, typename arg2, typename arg3, typename arg4, typename arg5>
    clazz& method(const string& methodName, T* t, void (T::*m)(arg1, arg2, arg3, arg4, arg5)) {
      reg->addMethod(methodName, new slub::method<T, void, arg1, arg2, arg3, arg4, arg5>(m));
      return *this;
    }
    
    template<typename ret, typename arg1, typename arg2, typename arg3, typename arg4, typename arg5, typename arg6>
    clazz& method(const string& methodName, ret (T::*m)(arg1, arg2, arg3, arg4, arg5, arg6)) {
      reg->addMethod(methodName, new slub::method<T, ret, arg1, arg2, arg3, arg4, arg5, arg6>(m));
      return *this;
    }
    
    template<typename arg1, typename arg2, typename arg3, typename arg4, typename arg5, typename arg6>
    clazz& method(const string& methodName, T* t, void (T::*m)(arg1, arg2, arg3, arg4, arg5, arg6)) {
      reg->addMethod(methodName, new slub::method<T, void, arg1, arg2, arg3, arg4, arg5, arg6>(m));
      return *this;
    }
    
    template<typename ret, typename arg1, typename arg2, typename arg3, typename arg4, typename arg5, typename arg6, typename arg7>
    clazz& method(const string& methodName, ret (T::*m)(arg1, arg2, arg3, arg4, arg5, arg6, arg7)) {
      reg->addMethod(methodName, new slub::method<T, ret, arg1, arg2, arg3, arg4, arg5>(m));
      return *this;
    }
    
    template<typename arg1, typename arg2, typename arg3, typename arg4, typename arg5, typename arg6, typename arg7>
    clazz& method(const string& methodName, T* t, void (T::*m)(arg1, arg2, arg3, arg4, arg5, arg6, arg7)) {
      reg->addMethod(methodName, new slub::method<T, void, arg1, arg2, arg3, arg4, arg5, arg6, arg7>(m));
      return *this;
    }
    
    template<typename ret>
    clazz& method(const string& methodName, ret (T::*m)() const) {
      reg->addMethod(methodName, new slub::const_method<T, ret>(m));
      return *this;
    }
    
    clazz& method(const string& methodName, void (T::*m)() const) {
      reg->addMethod(methodName, new slub::const_method<T>(m));
      return *this;
    }
    
    template<typename ret, typename arg1>
    clazz& method(const string& methodName, ret (T::*m)(arg1) const) {
      reg->addMethod(methodName, new slub::const_method<T, ret, arg1>(m));
      return *this;
    }
    
    template<typename arg1>
    clazz& method(const string& methodName, void (T::*m)(arg1) const) {
      reg->addMethod(methodName, new slub::const_method<T, void, arg1>(m));
      return *this;
    }
    
    template<typename ret, typename arg1, typename arg2>
    clazz& method(const string& methodName, ret (T::*m)(arg1, arg2) const) {
      reg->addMethod(methodName, new slub::const_method<T, ret, arg1, arg2>(m));
      return *this;
    }
    
    template<typename arg1, typename arg2>
    clazz& method(const string& methodName, T* t, void (T::*m)(arg1, arg2) const) {
      reg->addMethod(methodName, new slub::const_method<T, void, arg1, arg2>(m));
      return *this;
    }
    
    template<typename ret, typename arg1, typename arg2, typename arg3>
    clazz& method(const string& methodName, ret (T::*m)(arg1, arg2, arg3) const) {
      reg->addMethod(methodName, new slub::const_method<T, ret, arg1, arg2, arg3>(m));
      return *this;
    }
    
    template<typename arg1, typename arg2, typename arg3, typename arg4>
    clazz& method(const string& methodName, T* t, void (T::*m)(arg1, arg2, arg3, arg4) const) {
      reg->addMethod(methodName, new slub::const_method<T, void, arg1, arg2, arg3, arg4>(m));
      return *this;
    }

    template<typename ret, typename arg1, typename arg2, typename arg3, typename arg4>
    clazz& method(const string& methodName, ret (T::*m)(arg1, arg2, arg3, arg4) const) {
      reg->addMethod(methodName, new slub::const_method<T, ret, arg1, arg2, arg3, arg4>(m));
      return *this;
    }
    
    template<typename arg1, typename arg2, typename arg3>
    clazz& method(const string& methodName, T* t, void (T::*m)(arg1, arg2, arg3) const) {
      reg->addMethod(methodName, new slub::const_method<T, void, arg1, arg2, arg3>(m));
      return *this;
    }
    
    template<typename ret>
    clazz& method(const string& methodName, ret (*m)(T*)) {
      reg->addMethod(methodName, new slub::func_method<T, ret>(m));
      return *this;
    }
    
    clazz& method(const string& methodName, void (*m)(T*)) {
      reg->addMethod(methodName, new slub::func_method<T>(m));
      return *this;
    }
    
    template<typename ret, typename arg1>
    clazz& method(const string& methodName, ret (*m)(T*, arg1)) {
      reg->addMethod(methodName, new slub::func_method<T, ret, arg1>(m));
      return *this;
    }
    
    // TODO: add more lua_State* shortcuts
    template<typename ret>
    clazz& method(const string& methodName, ret (*m)(T*, lua_State*)) {
      reg->addMethod(methodName, new slub::func_method<T, ret, lua_State*>(m));
      return *this;
    }
    
    template<typename arg1>
    clazz& method(const string& methodName, void (*m)(T*, arg1)) {
      reg->addMethod(methodName, new slub::func_method<T, void, arg1>(m));
      return *this;
    }
    
    template<typename ret, typename arg1, typename arg2>
    clazz& method(const string& methodName, ret (*m)(T*, arg1, arg2)) {
      reg->addMethod(methodName, new slub::func_method<T, ret, arg1, arg2>(m));
      return *this;
    }
    
    template<typename arg1, typename arg2>
    clazz& method(const string& methodName, T* t, void (*m)(T*, arg1, arg2)) {
      reg->addMethod(methodName, new slub::func_method<T, void, arg1, arg2>(m));
      return *this;
    }
    
    template<typename ret, typename arg1, typename arg2, typename arg3>
    clazz& method(const string& methodName, ret (*m)(T*, arg1, arg2, arg3)) {
      reg->addMethod(methodName, new slub::func_method<T, ret, arg1, arg2, arg3>(m));
      return *this;
    }
    
    template<typename arg1, typename arg2, typename arg3>
    clazz& method(const string& methodName, T* t, void (*m)(T*, arg1, arg2, arg3)) {
      reg->addMethod(methodName, new slub::func_method<T, void, arg1, arg2, arg3>(m));
      return *this;
    }

    template<typename ret, typename arg1, typename arg2, typename arg3, typename arg4>
    clazz& method(const string& methodName, ret (*m)(T*, arg1, arg2, arg3, arg4)) {
      reg->addMethod(methodName, new slub::func_method<T, ret, arg1, arg2, arg3, arg4>(m));
      return *this;
    }
    
    template<typename arg1, typename arg2, typename arg3, typename arg4>
    clazz& method(const string& methodName, T* t, void (*m)(T*, arg1, arg2, arg3, arg4)) {
      reg->addMethod(methodName, new slub::func_method<T, void, arg1, arg2, arg3, arg4>(m));
      return *this;
    }
    
    template<typename ret, typename arg1, typename arg2, typename arg3, typename arg4, typename arg5>
    clazz& method(const string& methodName, ret (*m)(T*, arg1, arg2, arg3, arg4, arg5)) {
      reg->addMethod(methodName, new slub::func_method<T, ret, arg1, arg2, arg3, arg4, arg5>(m));
      return *this;
    }
    
    template<typename arg1, typename arg2, typename arg3, typename arg4, typename arg5>
    clazz& method(const string& methodName, T* t, void (*m)(T*, arg1, arg2, arg3, arg4, arg5)) {
      reg->addMethod(methodName, new slub::func_method<T, void, arg1, arg2, arg3, arg4, arg5>(m));
      return *this;
    }
    
    template<typename ret, typename arg1, typename arg2, typename arg3, typename arg4, typename arg5, typename arg6>
    clazz& method(const string& methodName, ret (*m)(T*, arg1, arg2, arg3, arg4, arg5, arg6)) {
      reg->addMethod(methodName, new slub::func_method<T, ret, arg1, arg2, arg3, arg4, arg5, arg6>(m));
      return *this;
    }
    
    template<typename arg1, typename arg2, typename arg3, typename arg4, typename arg5, typename arg6>
    clazz& method(const string& methodName, T* t, void (*m)(T*, arg1, arg2, arg3, arg4, arg5, arg6)) {
      reg->addMethod(methodName, new slub::func_method<T, void, arg1, arg2, arg3, arg4, arg5, arg6>(m));
      return *this;
    }
    
    template<typename ret, typename arg1, typename arg2, typename arg3, typename arg4, typename arg5, typename arg6, typename arg7>
    clazz& method(const string& methodName, ret (*m)(T*, arg1, arg2, arg3, arg4, arg5, arg6, arg7)) {
      reg->addMethod(methodName, new slub::func_method<T, ret, arg1, arg2, arg3, arg4, arg5, arg6, arg7>(m));
      return *this;
    }
    
    template<typename arg1, typename arg2, typename arg3, typename arg4, typename arg5, typename arg6, typename arg7>
    clazz& method(const string& methodName, T* t, void (*m)(T*, arg1, arg2, arg3, arg4, arg5, arg6, arg7)) {
      reg->addMethod(methodName, new slub::func_method<T, void, arg1, arg2, arg3, arg4, arg5, arg6, arg7>(m));
      return *this;
    }
    
    clazz& eq() {
      reg->addOperator("__eq", new eq_operator<T, T>());
      return *this;
    }

    template<typename F>
    clazz& eq() {
      reg->addOperator("__eq", new eq_operator<T, F>());
      return *this;
    }
    
    clazz& lt() {
      reg->addOperator("__lt", new lt_operator<T, T>());
      return *this;
    }
    
    template<typename F>
    clazz& lt() {
      reg->addOperator("__lt", new lt_operator<T, F>());
      return *this;
    }
    
    clazz& le() {
      reg->addOperator("__le", new le_operator<T, T>());
      return *this;
    }
    
    template<typename F>
    clazz& le() {
      reg->addOperator("__le", new le_operator<T, F>());
      return *this;
    }
    
    clazz& tostring() {
      reg->addOperator("__tostring", new tostring_operator<T>());
      return *this;
    }
    
    clazz& tostring_default() {
      reg->addOperator("__tostring", new lua_tostring_operator<T>());
      return *this;
    }
    
    template<typename R, typename F>
    clazz& add() {
      reg->addOperator("__add", new add_operator<T, R, F>());
      return *this;
    }
    
    template<typename R, typename F>
    clazz& sub() {
      reg->addOperator("__sub", new sub_operator<T, R, F>());
      return *this;
    }
    
    template<typename R, typename F>
    clazz& mul() {
      reg->addOperator("__mul", new mul_operator<T, R, F>());
      return *this;
    }
    
    template<typename R, typename F>
    clazz& div() {
      reg->addOperator("__div", new div_operator<T, R, F>());
      return *this;
    }
    
    template<typename R, typename F>
    clazz& mod() {
      reg->addOperator("__mod", new mod_operator<T, R, F>());
      return *this;
    }
    
    template<typename R, typename F>
    clazz& pow() {
      reg->addOperator("__pow", new pow_operator<T, R, F>());
      return *this;
    }
    
    clazz& unm() {
      reg->addOperator("__unm", new unm_operator<T, T>());
      return *this;
    }

    template<typename R>
    clazz& unm() {
      reg->addOperator("__unm", new unm_operator<T, R>());
      return *this;
    }
    
    template<typename R, typename F>
    clazz& index() {
      reg->addOperator("__index", new index_operator<T, R, F>());
      return *this;
    }
    
    clazz& function(const string& name, void (*f)()) {
      luaL_getmetatable(state, this->name.c_str());
      lua_getfield(state, -1, "__metatable");
      slub::function(state, name, f, this->name, lua_gettop(state));
      lua_pop(state, 2);
      return *this;
    }
    
    template<typename arg1>
    clazz& function(const string& name, void (*f)(arg1)) {
      luaL_getmetatable(state, this->name.c_str());
      lua_getfield(state, -1, "__metatable");
      slub::function<arg1>(state, name, f, this->name, lua_gettop(state));
      lua_pop(state, 2);
      return *this;
    }
    
    template<typename arg1, typename arg2>
    clazz& function(const string& name, void (*f)(arg1, arg2)) {
      luaL_getmetatable(state, this->name.c_str());
      lua_getfield(state, -1, "__metatable");
      slub::function<arg1, arg2>(state, name, f, this->name, lua_gettop(state));
      lua_pop(state, 2);
      return *this;
    }
    
    template<typename arg1, typename arg2, typename arg3>
    clazz& function(const string& name, void (*f)(arg1, arg2, arg3)) {
      luaL_getmetatable(state, this->name.c_str());
      lua_getfield(state, -1, "__metatable");
      slub::function<arg1, arg2, arg3>(state, name, f, this->name, lua_gettop(state));
      lua_pop(state, 2);
      return *this;
    }
    
    template<typename arg1, typename arg2, typename arg3, typename arg4>
    clazz& function(const string& name, void (*f)(arg1, arg2, arg3, arg4)) {
      luaL_getmetatable(state, this->name.c_str());
      lua_getfield(state, -1, "__metatable");
      slub::function<arg1, arg2, arg3, arg4>(state, name, f, this->name, lua_gettop(state));
      lua_pop(state, 2);
      return *this;
    }
    
    template<typename arg1, typename arg2, typename arg3, typename arg4, typename arg5>
    clazz& function(const string& name, void (*f)(arg1, arg2, arg3, arg4, arg5)) {
      luaL_getmetatable(state, this->name.c_str());
      lua_getfield(state, -1, "__metatable");
      slub::function<arg1, arg2, arg3, arg4, arg5>(state, name, f, this->name, lua_gettop(state));
      lua_pop(state, 2);
      return *this;
    }
    
    template<typename arg1, typename arg2, typename arg3, typename arg4, typename arg5, typename arg6>
    clazz& function(const string& name, void (*f)(arg1, arg2, arg3, arg4, arg5, arg6)) {
      luaL_getmetatable(state, this->name.c_str());
      lua_getfield(state, -1, "__metatable");
      slub::function<arg1, arg2, arg3, arg4, arg5, arg6>(state, name, f, this->name, lua_gettop(state));
      lua_pop(state, 2);
      return *this;
    }
    
    template<typename arg1, typename arg2, typename arg3, typename arg4, typename arg5, typename arg6, typename arg7>
    clazz& function(const string& name, void (*f)(arg1, arg2, arg3, arg4, arg5, arg6, arg7)) {
      luaL_getmetatable(state, this->name.c_str());
      lua_getfield(state, -1, "__metatable");
      slub::function<arg1, arg2, arg3, arg4, arg5, arg6, arg7>(state, name, f, this->name, lua_gettop(state));
      lua_pop(state, 2);
      return *this;
    }
    
    template<typename R>
    clazz& function(const string& name, R (*f)()) {
      luaL_getmetatable(state, this->name.c_str());
      lua_getfield(state, -1, "__metatable");
      slub::function<R>(state, name, f, this->name, lua_gettop(state));
      lua_pop(state, 2);
      return *this;
    }
    
    template<typename R, typename arg1>
    clazz& function(const string& name, R (*f)(arg1)) {
      luaL_getmetatable(state, this->name.c_str());
      lua_getfield(state, -1, "__metatable");
      slub::function<R, arg1>(state, name, f, this->name, lua_gettop(state));
      lua_pop(state, 2);
      return *this;
    }
    
    template<typename R, typename arg1, typename arg2>
    clazz& function(const string& name, R (*f)(arg1, arg2)) {
      luaL_getmetatable(state, this->name.c_str());
      lua_getfield(state, -1, "__metatable");
      slub::function<R, arg1, arg2>(state, name, f, this->name, lua_gettop(state));
      lua_pop(state, 2);
      return *this;
    }
    
    template<typename R, typename arg1, typename arg2, typename arg3>
    clazz& function(const string& name, R (*f)(arg1, arg2, arg3)) {
      luaL_getmetatable(state, this->name.c_str());
      lua_getfield(state, -1, "__metatable");
      slub::function<R, arg1, arg2, arg3>(state, name, f, this->name, lua_gettop(state));
      lua_pop(state, 2);
      return *this;
    }
    
    template<typename R, typename arg1, typename arg2, typename arg3, typename arg4>
    clazz& function(const string& name, R (*f)(arg1, arg2, arg3, arg4)) {
      luaL_getmetatable(state, this->name.c_str());
      lua_getfield(state, -1, "__metatable");
      slub::function<R, arg1, arg2, arg3, arg4>(state, name, f, this->name, lua_gettop(state));
      lua_pop(state, 2);
      return *this;
    }
    
    template<typename R, typename arg1, typename arg2, typename arg3, typename arg4, typename arg5>
    clazz& function(const string& name, R (*f)(arg1, arg2, arg3, arg4, arg5)) {
      luaL_getmetatable(state, this->name.c_str());
      lua_getfield(state, -1, "__metatable");
      slub::function<R, arg1, arg2, arg3, arg4, arg5>(state, name, f, this->name, lua_gettop(state));
      lua_pop(state, 2);
      return *this;
    }
    
    template<typename R, typename arg1, typename arg2, typename arg3, typename arg4, typename arg5, typename arg6>
    clazz& function(const string& name, R (*f)(arg1, arg2, arg3, arg4, arg5, arg6)) {
      luaL_getmetatable(state, this->name.c_str());
      lua_getfield(state, -1, "__metatable");
      slub::function<R, arg1, arg2, arg3, arg4, arg5, arg6>(state, name, f, this->name, lua_gettop(state));
      lua_pop(state, 2);
      return *this;
    }
    
    template<typename R, typename arg1, typename arg2, typename arg3, typename arg4, typename arg5, typename arg6, typename arg7>
    clazz& function(const string& name, R (*f)(arg1, arg2, arg3, arg4, arg5, arg6, arg7)) {
      luaL_getmetatable(state, this->name.c_str());
      lua_getfield(state, -1, "__metatable");
      slub::function<R, arg1, arg2, arg3, arg4, arg5, arg6, arg7>(state, name, f, this->name, lua_gettop(state));
      lua_pop(state, 2);
      return *this;
    }
    
    template<typename C>
    clazz& enumerated(const string& constantName, const C& value) {
      return constant<int>(constantName, value);
    }
    
    template<typename C>
    clazz& constant(const string& constantName, const C& value) {
      luaL_getmetatable(state, name.c_str());
      lua_getfield(state, -1, "__metatable");
      converter<C>::push(state, value);
      lua_setfield(state, -2, constantName.c_str());
      lua_pop(state, 2);
      return *this;
    }
    
  private:

    lua_State* state;
    string name;
    registry* reg;
    
    static T* clazz_cast(const reference& ref) {
      return ref.cast<T*>();
    }
    
    void init(lua_State* L, const string& name, const string& prefix = "", int target = -1)
    {
      this->state = L;
      this->name = prefix.size() > 0 ? prefix +"."+ name : name;

      reg = registry::registerType<T>(this->name);
      
      std::pair<int, int> tables = construct(state, reg, name.c_str(), this->name.c_str(), target);
      int methods = tables.first;
      int metatable = tables.second;
      
      lua_newtable(state);                // mt for method table
      int mt = lua_gettop(state);
      lua_pushliteral(state, "__call");
      lua_pushcfunction(state, call);
      lua_settable(state, mt);            // mt.__call = ctor
      lua_setmetatable(state, methods);
      
      lua_pushliteral(state, "__gc");
      lua_pushcfunction(state, gc);
      lua_settable(state, metatable);
      
      add_symbols(state, reg, methods, metatable);
      
      function("cast", clazz_cast);
    }

    static int call(lua_State* L) {
      registry* r = registry::get(typeid(T));
      if (r->containsConstructor()) {
        T* instance = r->getConstructor(L)->newInstance<T>(L);
        wrapper<T*>* w = wrapper<T*>::create(L, typeid(T));
        w->ref(instance);
        w->gc = true;
        luaL_getmetatable(L, r->getTypeName().c_str());
        lua_setmetatable(L, -2);
        return 1;
      }
      return 0;
    }

    static int gc(lua_State* L) {
      wrapper<T*>* w = static_cast<wrapper<T*>*>(luaL_checkudata(L, 1, registry::get(typeid(T))->getTypeName().c_str()));

      registry* r = registry::get(typeid(T));
      if (r != NULL) {
        r->removeInstanceTable(L, w->raw);
      }

      if (w->gc) {
        D::delete_(w);
      }
      else {
        w->ref(NULL);
      }
      return 0;
    }
    
  };

}

#endif
