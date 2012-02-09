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

#ifndef SLUB_FUNCTION_H
#define SLUB_FUNCTION_H

#include "config.h"
#include "slub_lua.h"
#include "converter.h"
#include "reference.h"

namespace slub {

  struct lua_function_base {
  protected:
    reference ref;
  public:
    lua_function_base(const reference& ref) : ref(ref) {}
    void operator=(const reference& ref) { this->ref = ref; }
  };

  template<typename ret, typename arg1, typename arg2, typename arg3>
  struct lua_function : public lua_function_base {
    lua_function(const reference& ref) : lua_function_base(ref) {}
    ret operator()(arg1 a1, arg2 a2, arg3 a3) {
      converter<reference>::push(ref.getState(), ref);
      converter<arg1>::push(ref.getState(), a1);
      converter<arg2>::push(ref.getState(), a2);
      converter<arg3>::push(ref.getState(), a3);
      slub::call(ref.getState(), 3, 1);
      ret r = converter<ret>::get(ref.getState(), -1);
      lua_pop(ref.getState(), 1);
      return r;
    }
  };

  template<typename arg1, typename arg2, typename arg3>
  struct lua_function<void, arg1, arg2, arg3> : public lua_function_base {
    lua_function(const reference& ref) : lua_function_base(ref) {}
    void operator()(arg1 a1, arg2 a2, arg3 a3) {
      converter<reference>::push(ref.getState(), ref);
      converter<arg1>::push(ref.getState(), a1);
      converter<arg2>::push(ref.getState(), a2);
      converter<arg2>::push(ref.getState(), a3);
      slub::call(ref.getState(), 3, 0);
    }
  };
  
  template<typename ret, typename arg1, typename arg2>
  struct lua_function<ret, arg1, arg2, empty> : public lua_function_base {
    lua_function(const reference& ref) : lua_function_base(ref) {}
    ret operator()(arg1 a1, arg2 a2) {
      converter<reference>::push(ref.getState(), ref);
      converter<arg1>::push(ref.getState(), a1);
      converter<arg2>::push(ref.getState(), a2);
      slub::call(ref.getState(), 2, 0);
      ret r = converter<ret>::get(ref.getState(), -1);
      lua_pop(ref.getState(), 1);
      return r;
    }
  };
  
  template<typename arg1, typename arg2>
  struct lua_function<void, arg1, arg2, empty> : public lua_function_base {
    lua_function(const reference& ref) : lua_function_base(ref) {}
    void operator()(arg1 a1, arg2 a2) {
      converter<reference>::push(ref.getState(), ref);
      converter<arg1>::push(ref.getState(), a1);
      converter<arg2>::push(ref.getState(), a2);
      slub::call(ref.getState(), 2, 0);
    }
  };
  
  template<typename ret, typename arg1>
  struct lua_function<ret, arg1, empty, empty> : public lua_function_base {
    lua_function(const reference& ref) : lua_function_base(ref) {}
    ret operator()(arg1 a1) {
      converter<reference>::push(ref.getState(), ref);
      converter<arg1>::push(ref.getState(), a1);
      slub::call(ref.getState(), 1, 1);
      ret r = converter<ret>::get(ref.getState(), -1);
      lua_pop(ref.getState(), 1);
      return r;
    }
  };
  
  template<typename arg1>
  struct lua_function<void, arg1, empty, empty> : public lua_function_base {
    lua_function(const reference& ref) : lua_function_base(ref) {}
    void operator()(arg1 a1) {
      converter<reference>::push(ref.getState(), ref);
      converter<arg1>::push(ref.getState(), a1);
      slub::call(ref.getState(), 1, 0);
    }
  };
  
  template<typename ret>
  struct lua_function<ret, empty, empty, empty> : public lua_function_base {
    lua_function(const reference& ref) : lua_function_base(ref) {}
    ret operator()() {
      converter<reference>::push(ref.getState(), ref);
      slub::call(ref.getState(), 0, 1);
      ret r = converter<ret>::get(ref.getState(), -1);
      lua_pop(ref.getState(), 1);
      return r;
    }
  };
  
  template<>
  struct lua_function<void, empty, empty, empty> : public lua_function_base {
    lua_function(const reference& ref) : lua_function_base(ref) {}
    void operator()() {
      converter<reference>::push(ref.getState(), ref);
      slub::call(ref.getState(), 0, 0);
    }
  };



  template<typename ret, typename arg1, typename arg2>
  static inline ret call(const reference& r, arg1 a1, arg2 a2) {
    lua_function<ret, arg1, arg2, empty> f(r);
    return f(a1, a2);
  }
  
  template<typename arg1, typename arg2>
  static inline void call(const reference& r, arg1 a1, arg2 a2) {
    lua_function<void, arg1, arg2, empty> f(r);
    f(a1, a2);
  }
  
  template<typename ret, typename arg1>
  static inline ret call(const reference& r, arg1 a1) {
    lua_function<ret, arg1, empty, empty> f(r);
    return f(a1);
  }
  
  template<typename arg1>
  static inline void call(const reference& r, arg1 a1) {
    lua_function<void, arg1, empty, empty> f(r);
    f(a1);
  }
  
  template<typename ret>
  static inline ret call(const reference& r) {
    lua_function<ret, empty, empty, empty> f(r);
    return f();
  }

  static inline void call(const reference& r) {
    lua_function<void, empty, empty, empty> f(r);
    f();
  }



  struct abstract_function_wrapper {
    virtual bool check(lua_State* L) = 0;
    virtual int call(lua_State* L) = 0;
  };
  
  struct function_holder {
    
    static function_holder instance;
    map<string, list<abstract_function_wrapper*> > functions;
    
    ~function_holder();
    static void add(lua_State* L, const string& name, abstract_function_wrapper* f, const string& prefix, int target);
    static int call(lua_State* L);
    
  };

  template<typename R, typename arg1, typename arg2, typename arg3, typename arg4, typename arg5, typename arg6, typename arg7>
  struct function_wrapper : public abstract_function_wrapper {
    
    R (*f)(arg1, arg2, arg3, arg4, arg5, arg6, arg7);
    
    function_wrapper(R (*f)(arg1, arg2, arg3, arg4, arg5, arg6, arg7)) : f(f) {
    }
    
    bool check(lua_State* L) {
      return lua_gettop(L) == 7 && converter<arg1>::check(L, -7) && converter<arg2>::check(L, -6) && converter<arg3>::check(L, -5)
          && converter<arg4>::check(L, -4) && converter<arg5>::check(L, -3) && converter<arg6>::check(L, -2) && converter<arg7>::check(L, -1);
    }
    
    int call(lua_State* L) {
      return converter<R>::push(L, f(converter<arg1>::get(L, -7), converter<arg2>::get(L, -6), converter<arg3>::get(L, -5),
                                     converter<arg4>::get(L, -4), converter<arg5>::get(L, -3), converter<arg6>::get(L, -2),
                                     converter<arg7>::get(L, -1)));
    }
    
  };
  
  template<>
  struct function_wrapper<void, empty, empty, empty, empty, empty, empty, empty> : public abstract_function_wrapper {
    
    void (*f)();
    
    function_wrapper(void (*f)()) : f(f) {
    }
    
    bool check(lua_State* L) {
      return lua_gettop(L) == 0;
    }
    
    int call(lua_State* L) {
      f();
      return 0;
    }
    
  };

  template<typename arg1>
  struct function_wrapper<void, arg1, empty, empty, empty, empty, empty, empty> : public abstract_function_wrapper {
    
    void (*f)(arg1);
    
    function_wrapper(void (*f)(arg1)) : f(f) {
    }
    
    bool check(lua_State* L) {
      return lua_gettop(L) == 1 && converter<arg1>::check(L, -1);
    }
    
    int call(lua_State* L) {
      f(converter<arg1>::get(L, -1));
      return 0;
    }
    
  };
  
  template<typename arg1, typename arg2>
  struct function_wrapper<void, arg1, arg2, empty, empty, empty, empty, empty> : public abstract_function_wrapper {
    
    void (*f)(arg1, arg2);
    
    function_wrapper(void (*f)(arg1, arg2)) : f(f) {
    }
    
    bool check(lua_State* L) {
      return lua_gettop(L) == 2 && converter<arg1>::check(L, -2) && converter<arg2>::check(L, -1);
    }
    
    int call(lua_State* L) {
      f(converter<arg1>::get(L, -2), converter<arg2>::get(L, -1));
      return 0;
    }
    
  };
  
  template<typename arg1, typename arg2, typename arg3>
  struct function_wrapper<void, arg1, arg2, arg3, empty, empty, empty, empty> : public abstract_function_wrapper {
    
    void (*f)(arg1, arg2, arg3);
    
    function_wrapper(void (*f)(arg1, arg2, arg3)) : f(f) {
    }
    
    bool check(lua_State* L) {
      return lua_gettop(L) == 3 && converter<arg1>::check(L, -3) && converter<arg2>::check(L, -2) && converter<arg3>::check(L, -1);
    }
    
    int call(lua_State* L) {
      f(converter<arg1>::get(L, -3), converter<arg2>::get(L, -2), converter<arg3>::get(L, -1));
      return 0;
    }
    
  };
  
  template<typename arg1, typename arg2, typename arg3, typename arg4>
  struct function_wrapper<void, arg1, arg2, arg3, arg4, empty, empty, empty> : public abstract_function_wrapper {
    
    void (*f)(arg1, arg2, arg3, arg4);
    
    function_wrapper(void (*f)(arg1, arg2, arg3, arg4)) : f(f) {
    }
    
    bool check(lua_State* L) {
      return lua_gettop(L) == 4 && converter<arg1>::check(L, -4) && converter<arg2>::check(L, -3) && converter<arg3>::check(L, -2)
           && converter<arg4>::check(L, -1);
    }
    
    int call(lua_State* L) {
      f(converter<arg1>::get(L, -4), converter<arg2>::get(L, -3), converter<arg3>::get(L, -2), converter<arg4>::get(L, -1));
      return 0;
    }
    
  };
  
  template<typename arg1, typename arg2, typename arg3, typename arg4, typename arg5>
  struct function_wrapper<void, arg1, arg2, arg3, arg4, arg5, empty, empty> : public abstract_function_wrapper {
    
    void (*f)(arg1, arg2, arg3, arg4, arg5);
    
    function_wrapper(void (*f)(arg1, arg2, arg3, arg4, arg5)) : f(f) {
    }
    
    bool check(lua_State* L) {
      return lua_gettop(L) == 5 && converter<arg1>::check(L, -5) && converter<arg2>::check(L, -4) && converter<arg3>::check(L, -3)
           && converter<arg4>::check(L, -2) && converter<arg5>::check(L, -1);
    }
    
    int call(lua_State* L) {
      f(converter<arg1>::get(L, -5), converter<arg2>::get(L, -4), converter<arg3>::get(L, -3), converter<arg4>::get(L, -2),
        converter<arg5>::get(L, -1));
      return 0;
    }
    
  };
  
  template<typename arg1, typename arg2, typename arg3, typename arg4, typename arg5, typename arg6>
  struct function_wrapper<void, arg1, arg2, arg3, arg4, arg5, arg6, empty> : public abstract_function_wrapper {
    
    void (*f)(arg1, arg2, arg3, arg4, arg5, arg6);
    
    function_wrapper(void (*f)(arg1, arg2, arg3, arg4, arg5, arg6)) : f(f) {
    }
    
    bool check(lua_State* L) {
      return lua_gettop(L) == 6 && converter<arg1>::check(L, -6) && converter<arg2>::check(L, -5) && converter<arg3>::check(L, -4)
      && converter<arg4>::check(L, -3) && converter<arg5>::check(L, -2) && converter<arg6>::check(L, -1);
    }
    
    int call(lua_State* L) {
      f(converter<arg1>::get(L, -6), converter<arg2>::get(L, -5), converter<arg3>::get(L, -4), converter<arg4>::get(L, -3),
        converter<arg5>::get(L, -2), converter<arg6>::get(L, -1));
      return 0;
    }
    
  };
  
  template<typename arg1, typename arg2, typename arg3, typename arg4, typename arg5, typename arg6, typename arg7>
  struct function_wrapper<void, arg1, arg2, arg3, arg4, arg5, arg6, arg7> : public abstract_function_wrapper {
    
    void (*f)(arg1, arg2, arg3, arg4, arg5, arg6, arg7);
    
    function_wrapper(void (*f)(arg1, arg2, arg3, arg4, arg5, arg6, arg7)) : f(f) {
    }
    
    bool check(lua_State* L) {
      return lua_gettop(L) == 7 && converter<arg1>::check(L, -7) && converter<arg2>::check(L, -6) && converter<arg3>::check(L, -5)
      && converter<arg4>::check(L, -4) && converter<arg5>::check(L, -3) && converter<arg6>::check(L, -2) && converter<arg7>::check(L, -1);
    }
    
    int call(lua_State* L) {
      f(converter<arg1>::get(L, -7), converter<arg2>::get(L, -6), converter<arg3>::get(L, -5), converter<arg4>::get(L, -4),
        converter<arg5>::get(L, -3), converter<arg6>::get(L, -2), converter<arg7>::get(L, -1));
      return 0;
    }
    
  };
  
  template<>
  struct function_wrapper<void, lua_State*, empty, empty, empty, empty, empty, empty> : public abstract_function_wrapper {
    
    void (*f)(lua_State*);
    
    function_wrapper(void (*f)(lua_State*)) : f(f) {
    }
    
    bool check(lua_State* L) {
      return lua_gettop(L) == 0;
    }
    
    int call(lua_State* L) {
      f(L);
      return 0;
    }
    
  };
  
  template<typename arg1>
  struct function_wrapper<void, arg1, lua_State*, empty, empty, empty, empty, empty> : public abstract_function_wrapper {
    
    void (*f)(arg1, lua_State*);
    
    function_wrapper(void (*f)(arg1, lua_State*)) : f(f) {
    }
    
    bool check(lua_State* L) {
      return lua_gettop(L) == 1 && converter<arg1>::check(L, -1);
    }
    
    int call(lua_State* L) {
      f(converter<arg1>::get(L, -1), L);
      return 0;
    }
    
  };
  
  template<typename arg1, typename arg2>
  struct function_wrapper<void, arg1, arg2, lua_State*, empty, empty, empty, empty> : public abstract_function_wrapper {
    
    void (*f)(arg1, arg2, lua_State*);
    
    function_wrapper(void (*f)(arg1, arg2, lua_State*)) : f(f) {
    }
    
    bool check(lua_State* L) {
      return lua_gettop(L) == 2 && converter<arg1>::check(L, -2) && converter<arg2>::check(L, -1);
    }
    
    int call(lua_State* L) {
      f(converter<arg1>::get(L, -2), converter<arg2>::get(L, -1), L);
      return 0;
    }
    
  };
  
  template<typename arg1, typename arg2, typename arg3>
  struct function_wrapper<void, arg1, arg2, arg3, lua_State*, empty, empty, empty> : public abstract_function_wrapper {
    
    void (*f)(arg1, arg2, arg3, lua_State*);
    
    function_wrapper(void (*f)(arg1, arg2, arg3, lua_State*)) : f(f) {
    }
    
    bool check(lua_State* L) {
      return lua_gettop(L) == 3 && converter<arg1>::check(L, -3) && converter<arg2>::check(L, -2) && converter<arg3>::check(L, -1);
    }
    
    int call(lua_State* L) {
      f(converter<arg1>::get(L, -3), converter<arg2>::get(L, -2), converter<arg3>::get(L, -1), L);
      return 0;
    }
    
  };
  
  template<typename arg1, typename arg2, typename arg3, typename arg4>
  struct function_wrapper<void, arg1, arg2, arg3, arg4, lua_State*, empty, empty> : public abstract_function_wrapper {
    
    void (*f)(arg1, arg2, arg3, arg4, lua_State*);
    
    function_wrapper(void (*f)(arg1, arg2, arg3, arg4, lua_State*)) : f(f) {
    }
    
    bool check(lua_State* L) {
      return lua_gettop(L) == 4 && converter<arg1>::check(L, -4) && converter<arg2>::check(L, -3) && converter<arg3>::check(L, -2)
           && converter<arg4>::check(L, -1);
    }
    
    int call(lua_State* L) {
      f(converter<arg1>::get(L, -4), converter<arg2>::get(L, -3), converter<arg3>::get(L, -2), converter<arg4>::get(L, -1), L);
      return 0;
    }
    
  };
  
  template<typename arg1, typename arg2, typename arg3, typename arg4, typename arg5>
  struct function_wrapper<void, arg1, arg2, arg3, arg4, arg5, lua_State*, empty> : public abstract_function_wrapper {
    
    void (*f)(arg1, arg2, arg3, arg4, arg5, lua_State*);
    
    function_wrapper(void (*f)(arg1, arg2, arg3, arg4, arg5, lua_State*)) : f(f) {
    }
    
    bool check(lua_State* L) {
      return lua_gettop(L) == 5 && converter<arg1>::check(L, -5) && converter<arg2>::check(L, -4) && converter<arg3>::check(L, -3)
      && converter<arg4>::check(L, -2) && converter<arg5>::check(L, -1);
    }
    
    int call(lua_State* L) {
      f(converter<arg1>::get(L, -5), converter<arg2>::get(L, -4), converter<arg3>::get(L, -3), converter<arg4>::get(L, -2),
        converter<arg5>::get(L, -1), L);
      return 0;
    }
    
  };
  
  template<typename arg1, typename arg2, typename arg3, typename arg4, typename arg5, typename arg6>
  struct function_wrapper<void, arg1, arg2, arg3, arg4, arg5, arg6, lua_State*> : public abstract_function_wrapper {
    
    void (*f)(arg1, arg2, arg3, arg4, arg5, arg6, lua_State*);
    
    function_wrapper(void (*f)(arg1, arg2, arg3, arg4, arg5, arg6, lua_State*)) : f(f) {
    }
    
    bool check(lua_State* L) {
      return lua_gettop(L) == 6 && converter<arg1>::check(L, -6) && converter<arg2>::check(L, -5) && converter<arg3>::check(L, -4)
      && converter<arg4>::check(L, -3) && converter<arg5>::check(L, -2) && converter<arg6>::check(L, -1);
    }
    
    int call(lua_State* L) {
      f(converter<arg1>::get(L, -6), converter<arg2>::get(L, -5), converter<arg3>::get(L, -4), converter<arg4>::get(L, -3),
        converter<arg5>::get(L, -2), converter<arg6>::get(L, -1), L);
      return 0;
    }
    
  };
  
  template<typename R>
  struct function_wrapper<R, empty, empty, empty, empty, empty, empty, empty> : public abstract_function_wrapper {
    
    R (*f)();
    
    function_wrapper(R (*f)()) : f(f) {
    }
    
    bool check(lua_State* L) {
      return lua_gettop(L) == 0;
    }
    
    int call(lua_State* L) {
      return converter<R>::push(L, f());
    }
    
  };
  
  template<typename R, typename arg1>
  struct function_wrapper<R, arg1, empty, empty, empty, empty, empty, empty> : public abstract_function_wrapper {
    
    R (*f)(arg1);
    
    function_wrapper(R (*f)(arg1)) : f(f) {
    }
    
    bool check(lua_State* L) {
      return lua_gettop(L) == 1 && converter<arg1>::check(L, -1);
    }
    
    int call(lua_State* L) {
      return converter<R>::push(L, f(converter<arg1>::get(L, -1)));
    }
    
  };
  
  template<typename R, typename arg1, typename arg2>
  struct function_wrapper<R, arg1, arg2, empty, empty, empty, empty, empty> : public abstract_function_wrapper {
    
    R (*f)(arg1, arg2);
    
    function_wrapper(R (*f)(arg1, arg2)) : f(f) {
    }
    
    bool check(lua_State* L) {
      return lua_gettop(L) == 2 && converter<arg1>::check(L, -2) && converter<arg2>::check(L, -1);
    }
    
    int call(lua_State* L) {
      return converter<R>::push(L, f(converter<arg1>::get(L, -2), converter<arg2>::get(L, -1)));
    }
    
  };
  
  template<typename R, typename arg1, typename arg2, typename arg3>
  struct function_wrapper<R, arg1, arg2, arg3, empty, empty, empty, empty> : public abstract_function_wrapper {
    
    R (*f)(arg1, arg2, arg3);
    
    function_wrapper(R (*f)(arg1, arg2, arg3)) : f(f) {
    }
    
    bool check(lua_State* L) {
      return lua_gettop(L) == 3 && converter<arg1>::check(L, -3) && converter<arg2>::check(L, -2) && converter<arg3>::check(L, -1);
    }
    
    int call(lua_State* L) {
      return converter<R>::push(L, f(converter<arg1>::get(L, -3), converter<arg2>::get(L, -2), converter<arg3>::get(L, -1)));
    }
    
  };
  
  template<typename R, typename arg1, typename arg2, typename arg3, typename arg4>
  struct function_wrapper<R, arg1, arg2, arg3, arg4, empty, empty, empty> : public abstract_function_wrapper {
    
    R (*f)(arg1, arg2, arg3, arg4);
    
    function_wrapper(R (*f)(arg1, arg2, arg3, arg4)) : f(f) {
    }
    
    bool check(lua_State* L) {
      return lua_gettop(L) == 4 && converter<arg1>::check(L, -4) && converter<arg2>::check(L, -3) && converter<arg3>::check(L, -2)
           && converter<arg4>::check(L, -1);
    }
    
    int call(lua_State* L) {
      return converter<R>::push(L, f(converter<arg1>::get(L, -4), converter<arg2>::get(L, -3), converter<arg3>::get(L, -2),
                                     converter<arg4>::get(L, -1)));
    }
    
  };
  
  template<typename R, typename arg1, typename arg2, typename arg3, typename arg4, typename arg5>
  struct function_wrapper<R, arg1, arg2, arg3, arg4, arg5, empty, empty> : public abstract_function_wrapper {
    
    R (*f)(arg1, arg2, arg3, arg4, arg5);
    
    function_wrapper(R (*f)(arg1, arg2, arg3, arg4, arg5)) : f(f) {
    }
    
    bool check(lua_State* L) {
      return lua_gettop(L) == 5 && converter<arg1>::check(L, -5) && converter<arg2>::check(L, -4) && converter<arg3>::check(L, -3)
      && converter<arg4>::check(L, -2) && converter<arg5>::check(L, -1);
    }
    
    int call(lua_State* L) {
      return converter<R>::push(L, f(converter<arg1>::get(L, -5), converter<arg2>::get(L, -4), converter<arg3>::get(L, -3),
                                     converter<arg4>::get(L, -2), converter<arg5>::get(L, -1)));
    }
    
  };
  
  template<typename R, typename arg1, typename arg2, typename arg3, typename arg4, typename arg5, typename arg6>
  struct function_wrapper<R, arg1, arg2, arg3, arg4, arg5, arg6, empty> : public abstract_function_wrapper {
    
    R (*f)(arg1, arg2, arg3, arg4, arg5, arg6);
    
    function_wrapper(R (*f)(arg1, arg2, arg3, arg4, arg5, arg6)) : f(f) {
    }
    
    bool check(lua_State* L) {
      return lua_gettop(L) == 6 && converter<arg1>::check(L, -6) && converter<arg2>::check(L, -5) && converter<arg3>::check(L, -4)
      && converter<arg4>::check(L, -3) && converter<arg5>::check(L, -2) && converter<arg6>::check(L, -1);
    }
    
    int call(lua_State* L) {
      return converter<R>::push(L, f(converter<arg1>::get(L, -6), converter<arg2>::get(L, -5), converter<arg3>::get(L, -4),
                                     converter<arg4>::get(L, -3), converter<arg5>::get(L, -2), converter<arg6>::get(L, -1)));
    }
    
  };
  
  template<typename R>
  struct function_wrapper<R, lua_State*, empty, empty, empty, empty, empty, empty> : public abstract_function_wrapper {
    
    R (*f)(lua_State*);
    
    function_wrapper(R (*f)(lua_State*)) : f(f) {
    }
    
    bool check(lua_State* L) {
      return lua_gettop(L) == 0;
    }
    
    int call(lua_State* L) {
      return converter<R>::push(L, f(L));
    }
    
  };
  
  template<typename R, typename arg1>
  struct function_wrapper<R, arg1, lua_State*, empty, empty, empty, empty, empty> : public abstract_function_wrapper {
    
    R (*f)(arg1, lua_State*);
    
    function_wrapper(R (*f)(arg1, lua_State*)) : f(f) {
    }
    
    bool check(lua_State* L) {
      return lua_gettop(L) == 1 && converter<arg1>::check(L, -1);
    }
    
    int call(lua_State* L) {
      return converter<R>::push(L, f(converter<arg1>::get(L, -1), L));
    }
    
  };
  
  template<typename R, typename arg1, typename arg2>
  struct function_wrapper<R, arg1, arg2, lua_State*, empty, empty, empty, empty> : public abstract_function_wrapper {
    
    R (*f)(arg1, arg2, lua_State*);
    
    function_wrapper(R (*f)(arg1, arg2, lua_State*)) : f(f) {
    }
    
    bool check(lua_State* L) {
      return lua_gettop(L) == 2 && converter<arg1>::check(L, -2) && converter<arg2>::check(L, -1);
    }
    
    int call(lua_State* L) {
      return converter<R>::push(L, f(converter<arg1>::get(L, -2), converter<arg2>::get(L, -1), L));
    }
    
  };
  
  template<typename R, typename arg1, typename arg2, typename arg3>
  struct function_wrapper<R, arg1, arg2, arg3, lua_State*, empty, empty, empty> : public abstract_function_wrapper {
    
    R (*f)(arg1, arg2, arg3, lua_State*);
    
    function_wrapper(R (*f)(arg1, arg2, arg3, lua_State*)) : f(f) {
    }
    
    bool check(lua_State* L) {
      return lua_gettop(L) == 3 && converter<arg1>::check(L, -3) && converter<arg2>::check(L, -2) && converter<arg3>::check(L, -1);
    }
    
    int call(lua_State* L) {
      return converter<R>::push(L, f(converter<arg1>::get(L, -3), converter<arg2>::get(L, -2), converter<arg3>::get(L, -1), L));
    }
    
  };
  
  template<typename R, typename arg1, typename arg2, typename arg3, typename arg4>
  struct function_wrapper<R, arg1, arg2, arg3, arg4, lua_State*, empty, empty> : public abstract_function_wrapper {
    
    R (*f)(arg1, arg2, arg3, arg4, lua_State*);
    
    function_wrapper(R (*f)(arg1, arg2, arg3, arg4, lua_State*)) : f(f) {
    }
    
    bool check(lua_State* L) {
      return lua_gettop(L) == 4 && converter<arg1>::check(L, -4) && converter<arg2>::check(L, -3) && converter<arg3>::check(L, -2)
           && converter<arg4>::check(L, -1);
    }
    
    int call(lua_State* L) {
      return converter<R>::push(L, f(converter<arg1>::get(L, -4), converter<arg2>::get(L, -3), converter<arg3>::get(L, -2),
                                     converter<arg4>::get(L, -1), L));
    }
    
  };
  
  template<typename R, typename arg1, typename arg2, typename arg3, typename arg4, typename arg5>
  struct function_wrapper<R, arg1, arg2, arg3, arg4, arg5, lua_State*, empty> : public abstract_function_wrapper {
    
    R (*f)(arg1, arg2, arg3, arg4, arg5, lua_State*);
    
    function_wrapper(R (*f)(arg1, arg2, arg3, arg4, arg5, lua_State*)) : f(f) {
    }
    
    bool check(lua_State* L) {
      return lua_gettop(L) == 5 && converter<arg1>::check(L, -5) && converter<arg2>::check(L, -4) && converter<arg3>::check(L, -3)
      && converter<arg4>::check(L, -2) && converter<arg5>::check(L, -1);
    }
    
    int call(lua_State* L) {
      return converter<R>::push(L, f(converter<arg1>::get(L, -5), converter<arg2>::get(L, -4), converter<arg3>::get(L, -3),
                                     converter<arg4>::get(L, -2), converter<arg5>::get(L, -1), L));
    }
    
  };
  
  template<typename R, typename arg1, typename arg2, typename arg3, typename arg4, typename arg5, typename arg6>
  struct function_wrapper<R, arg1, arg2, arg3, arg4, arg5, arg6, lua_State*> : public abstract_function_wrapper {
    
    R (*f)(arg1, arg2, arg3, arg4, arg5, arg6, lua_State*);
    
    function_wrapper(R (*f)(arg1, arg2, arg3, arg4, arg5, arg6, lua_State*)) : f(f) {
    }
    
    bool check(lua_State* L) {
      return lua_gettop(L) == 6 && converter<arg1>::check(L, -6) && converter<arg2>::check(L, -5) && converter<arg3>::check(L, -4)
      && converter<arg4>::check(L, -3) && converter<arg5>::check(L, -2) && converter<arg6>::check(L, -1);
    }
    
    int call(lua_State* L) {
      return converter<R>::push(L, f(converter<arg1>::get(L, -6), converter<arg2>::get(L, -5), converter<arg3>::get(L, -4),
                                     converter<arg4>::get(L, -3), converter<arg5>::get(L, -2), converter<arg6>::get(L, -1), L));
    }
    
  };
  
  static void function(lua_State* L, const string& name, void (*f)(), const string& prefix = "", int target = -1) {
    function_holder::add(L, name, new function_wrapper<void, empty, empty, empty, empty, empty, empty, empty>(f), prefix, target);
  }
  
  template<typename arg1>
  static void function(lua_State* L, const string& name, void (*f)(arg1), const string& prefix = "", int target = -1) {
    function_holder::add(L, name, new function_wrapper<void, arg1, empty, empty, empty, empty, empty, empty>(f), prefix, target);
  }
  
  template<typename arg1, typename arg2>
  static void function(lua_State* L, const string& name, void (*f)(arg1, arg2), const string& prefix = "", int target = -1) {
    function_holder::add(L, name, new function_wrapper<void, arg1, arg2, empty, empty, empty, empty, empty>(f), prefix, target);
  }
  
  template<typename arg1, typename arg2, typename arg3>
  static void function(lua_State* L, const string& name, void (*f)(arg1, arg2, arg3), const string& prefix = "", int target = -1) {
    function_holder::add(L, name, new function_wrapper<void, arg1, arg2, arg3, empty, empty, empty, empty>(f), prefix, target);
  }
  
  template<typename arg1, typename arg2, typename arg3, typename arg4>
  static void function(lua_State* L, const string& name, void (*f)(arg1, arg2, arg3, arg4), const string& prefix = "", int target = -1) {
    function_holder::add(L, name, new function_wrapper<void, arg1, arg2, arg3, arg4, empty, empty, empty>(f), prefix, target);
  }
  
  template<typename arg1, typename arg2, typename arg3, typename arg4, typename arg5>
  static void function(lua_State* L, const string& name, void (*f)(arg1, arg2, arg3, arg4, arg5), const string& prefix = "", int target = -1) {
    function_holder::add(L, name, new function_wrapper<void, arg1, arg2, arg3, arg4, arg5, empty, empty>(f), prefix, target);
  }
  
  template<typename arg1, typename arg2, typename arg3, typename arg4, typename arg5, typename arg6>
  static void function(lua_State* L, const string& name, void (*f)(arg1, arg2, arg3, arg4, arg5, arg6), const string& prefix = "", int target = -1) {
    function_holder::add(L, name, new function_wrapper<void, arg1, arg2, arg3, arg4, arg5, arg6, empty>(f), prefix, target);
  }
  
  template<typename arg1, typename arg2, typename arg3, typename arg4, typename arg5, typename arg6, typename arg7>
  static void function(lua_State* L, const string& name, void (*f)(arg1, arg2, arg3, arg4, arg5, arg6, arg7), const string& prefix = "", int target = -1) {
    function_holder::add(L, name, new function_wrapper<void, arg1, arg2, arg3, arg4, arg5, arg6, arg7>(f), prefix, target);
  }
  
  template<typename R>
  static void function(lua_State* L, const string& name, R (*f)(), const string& prefix = "", int target = -1) {
    function_holder::add(L, name, new function_wrapper<R, empty, empty, empty, empty, empty, empty, empty>(f), prefix, target);
  }
  
  template<typename R, typename arg1>
  static void function(lua_State* L, const string& name, R (*f)(arg1), const string& prefix = "", int target = -1) {
    function_holder::add(L, name, new function_wrapper<R, arg1, empty, empty, empty, empty, empty, empty>(f), prefix, target);
  }
  
  template<typename R, typename arg1, typename arg2>
  static void function(lua_State* L, const string& name, R (*f)(arg1, arg2), const string& prefix = "", int target = -1) {
    function_holder::add(L, name, new function_wrapper<R, arg1, arg2, empty, empty, empty, empty, empty>(f), prefix, target);
  }
  
  template<typename R, typename arg1, typename arg2, typename arg3>
  static void function(lua_State* L, const string& name, R (*f)(arg1, arg2, arg3), const string& prefix = "", int target = -1) {
    function_holder::add(L, name, new function_wrapper<R, arg1, arg2, arg3, empty, empty, empty, empty>(f), prefix, target);
  }
  
  template<typename R, typename arg1, typename arg2, typename arg3, typename arg4>
  static void function(lua_State* L, const string& name, R (*f)(arg1, arg2, arg3, arg4), const string& prefix = "", int target = -1) {
    function_holder::add(L, name, new function_wrapper<R, arg1, arg2, arg3, arg4, empty, empty, empty>(f), prefix, target);
  }
  
  template<typename R, typename arg1, typename arg2, typename arg3, typename arg4, typename arg5>
  static void function(lua_State* L, const string& name, R (*f)(arg1, arg2, arg3, arg4, arg5), const string& prefix = "", int target = -1) {
    function_holder::add(L, name, new function_wrapper<R, arg1, arg2, arg3, arg4, arg5, empty, empty>(f), prefix, target);
  }
  
  template<typename R, typename arg1, typename arg2, typename arg3, typename arg4, typename arg5, typename arg6>
  static void function(lua_State* L, const string& name, R (*f)(arg1, arg2, arg3, arg4, arg5, arg6), const string& prefix = "", int target = -1) {
    function_holder::add(L, name, new function_wrapper<R, arg1, arg2, arg3, arg4, arg5, arg6, empty>(f), prefix, target);
  }
  
  template<typename R, typename arg1, typename arg2, typename arg3, typename arg4, typename arg5, typename arg6, typename arg7>
  static void function(lua_State* L, const string& name, R (*f)(arg1, arg2, arg3, arg4, arg5, arg6, arg7), const string& prefix = "", int target = -1) {
    function_holder::add(L, name, new function_wrapper<R, arg1, arg2, arg3, arg4, arg5, arg6, arg7>(f), prefix, target);
  }
  
}

#endif
