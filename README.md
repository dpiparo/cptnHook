# cptnHookLite
A tool tostudy the input value of mathematical functions.
This version of cptnHook is inspired by https://github.com/emyrto/cptnHook but does not use cctlib nor PIN.

## How to build
Clone the repository and then:
~~~
mkdir build
cd build
cmake ../cptnHookLite
make
export PATH=`pwd`:$PATH
~~~

## Obtaining a profile
~~~
cptnhook -o myFirstProfile -- myprogram --arg1 --arg2 arg3
~~~
This will generate a directory called `myFirstProfile` containing the raw profile.

### Converting to ROOT format
Make sure to have ROOT installed. Then there are two ways of converting to ROOT format the raw profile.
Either manually with a script
~~~
cptnhookAnalyze --root myFirstProfile
~~~
or directly specifying this in the invocation of cptnhook
~~~
cptnhook -o myFirstProfile --root -- myprogram --arg1 --arg2 arg3
~~~

## How to produce a report
This is still work inprogress. At the moment you can study the ROOT file by hand.