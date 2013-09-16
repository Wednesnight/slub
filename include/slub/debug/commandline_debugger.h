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

#ifndef SLUB_DEBUG_COMMANDLINE_DEBUGGER_H
#define SLUB_DEBUG_COMMANDLINE_DEBUGGER_H

#include "../config.h"
#include "debugger.h"

namespace slub {
  
  namespace debug {
    
    struct commandline_debugger : public debugger_callback {
      
    private:      
      enum Mode {
        DEFAULT  = 0,
        CONTINUE = 1,
        STEP     = 2,
        RETURN   = 3
      };
      
      debugger dbg;
      Mode mode;
      
    public:
      commandline_debugger(lua_State* state);      
      void slub_debug(lua_State* state, lua_Debug *ar);

    };
    
  }
  
}

#endif
