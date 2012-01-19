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

#ifndef SLUB_EXCEPTION_H
#define SLUB_EXCEPTION_H

#include "config.h"

#include <stdexcept>

namespace slub {

  class NotFoundException : public std::exception {

    string message;
    
  public:

    NotFoundException(const string& message, const string& name) throw()
    : message(message + name) {}
    
    virtual ~NotFoundException() throw() {}
    
    const char* what() const throw() {
      return message.c_str();
    }
    
  };

  class ConstructorNotFoundException : public NotFoundException {

  public:
    
    ConstructorNotFoundException(const string& className) throw()
    : NotFoundException("No matching constructor found: ", className) {}

  };

  class MethodNotFoundException : public NotFoundException {
    
  public:
    
    MethodNotFoundException(const string& methodName) throw()
    : NotFoundException("No matching method found: ", methodName) {}
    
  };

  class FieldNotFoundException : public NotFoundException {
    
  public:
    
    FieldNotFoundException(const string& fieldName) throw()
    : NotFoundException("No matching field found: ", fieldName) {}
    
  };

  class OperatorNotFoundException : public NotFoundException {
    
  public:
    
    OperatorNotFoundException(const string& fieldName) throw()
    : NotFoundException("No matching operator found: ", fieldName) {}
    
  };
  
  class OverloadNotFoundException : public NotFoundException {
    
  public:
    
    OverloadNotFoundException(const string& overload) throw()
    : NotFoundException("No matching overload found: ", overload) {}
    
  };

  class BaseClassNotFoundException : public NotFoundException {
    
  public:
    
    BaseClassNotFoundException() throw()
    : NotFoundException("Base class was not registered", "") {}
    
  };
  
}

#endif
