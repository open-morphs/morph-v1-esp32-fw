#pragma once

#define ATTRIBUTE(type, var) private:\
      type _##var;\
   public:\
      type get_##var() const\
      {\
         return _##var;\
      }\
      void set_##var(const type val)\
      {\
         _##var = val;\
      }