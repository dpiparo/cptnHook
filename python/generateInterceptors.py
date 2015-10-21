#! /usr/bin/env python
writerTemplate = 'Writer<%s> %s%sWriter(GetWriterName<%s>("%s"));'
interceptorTemplate = \
'''
auto %s(%s x) -> decltype(x){
   recordInput(x, %s%sWriter);   
   static auto origf = getOriginalSym<%s>("%s");
   return origf(x);
}'''
functions =["exp",
            "log",
            "sin",
            "asin",
            "cos",
            "acos",
            "tan",
            "atan",
            "sinh",
            "asinh",
            "cosh",
            "acosh",
            "tanh",
            "atanh",
            "cbrt"] 

# Print Writers -------------------------------------------
for function in functions:
    print writerTemplate %("double","dp",function.title(),"double",function)

for function in functions:
    functionsp = function+"f"
    print writerTemplate %("float","sp",function.title()+"","float",functionsp)
    
# Print interceptors --------------------------------------
for function in functions:
   print interceptorTemplate %(function, "double", "dp", function.title(), "double", function)

for function in functions:
   functionsp = function+"f"
   print interceptorTemplate %(functionsp, "float", "sp", function.title(), "float", functionsp)
