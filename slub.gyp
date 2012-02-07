#
# Copyright (c) 2011 Timo Boll, Tony Kostanjsek
#
# Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the
# "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish,
# distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the
# following conditions:
#
# The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
# CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
# OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#
{

  'targets': [
    {

      'target_name': 'slub',
      'type': 'none',

      'all_dependent_settings': {

        'include_dirs': [
          './include',
        ],

      },

      'direct_dependent_settings': {

        'sources': [
          './include/slub/call.h',
          './include/slub/constructor.h',
          './include/slub/field.h',
          './include/slub/globals.h',
          './include/slub/package.h',
          './include/slub/slub.h',
          './include/slub/clazz.h',
          './include/slub/converter.h',
          './include/slub/forward.h',
          './include/slub/method.h',
          './include/slub/reference.h',
          './include/slub/slub_lua.h',
          './include/slub/config.h',
          './include/slub/exception.h',
          './include/slub/function.h',
          './include/slub/operators.h',
          './include/slub/registry.h',
          './include/slub/wrapper.h',

          './src/slub/call.cpp',
          './src/slub/clazz.cpp',
          './src/slub/function.cpp',
          './src/slub/registry.cpp',
        ],

      },

    },

  ],

}
