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

#include "../../../include/slub/config.h"
#include "../../../include/slub/debug/commandline_debugger.h"

#include <string>

namespace slub {
  
  namespace debug {
    
    commandline_debugger::commandline_debugger(lua_State* state)
    : mode(DEFAULT),
    initialized(false)
    {
      dbg.attach(state, this);
      dbg.step(state);

      std::cout << "slub commandline debugger - type help or ? for a list of commands" << std::endl;
    }
    
    void commandline_debugger::slub_debug(lua_State* state, lua_Debug *ar) {
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
            std::string line;
            if (std::getline(std::cin, line)) {
              
              // parse command
              std::istringstream iss(line);
              string command, args;
              std::string cmd;
              iss >> cmd;
              command = cmd.c_str();
              args = line.substr(command.size()).c_str();
              for (; !args.empty() && ::isspace(*(args.begin())); args.erase(args.begin()));
              for (; !args.empty() && ::isspace(*(args.end()-1)); args.erase(args.end()-1));
              eastl::transform(command.begin(), command.end(), command.begin(),
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
                lua_variables variables = dbg.variables(state, ar);
                if (variables.size() == 0) {
                  std::cout << "no variables defined" << std::endl;
                }
                else {
                  std::cout << "variables:" << std::endl;
                  for (lua_variables::iterator iter = variables.begin();
                       iter != variables.end(); ++iter)
                  {
                    std::cout << "\t" << iter->name << " = " << iter->value.toString()
                    << " [" << iter->value.typeName() << "]" << std::endl;
                  }
                }
              }
              
              else if (command == "locals") {
                lua_variables locals = dbg.locals(state, ar);
                if (locals.size() == 0) {
                  std::cout << "no locals defined" << std::endl;
                }
                else {
                  std::cout << "locals:" << std::endl;
                  for (lua_variables::iterator iter = locals.begin();
                       iter != locals.end(); ++iter)
                  {
                    std::cout << "\t" << iter->index << ": " << iter->name << " = "
                    << iter->value.toString() << " [" << iter->value.typeName() << "]" << std::endl;
                  }
                }
              }
              
              else if (command == "upvalues") {
                lua_variables upvalues = dbg.upvalues(state, ar);
                if (upvalues.size() == 0) {
                  std::cout << "no upvalues defined" << std::endl;
                }
                else {
                  std::cout << "upvalues:" << std::endl;
                  for (lua_variables::iterator iter = upvalues.begin();
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
    
  }
  
}
