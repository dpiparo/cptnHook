# CptnHook
A profiler to investigate the usage patterns of mathematical functions capturing the input values and the stacktraces which lead to it.
No code instrumentation nor recompilation is needed: the principle behind CtpnHook is library preload.
The principle is to save in a profile for every mathematical function (both for single and double precision) a two column n-tuple containing the input value and a stacktrace ID. The map between the IDs and the stacktraces themselves is saved as well.
This version of cptnHook is inspired by https://github.com/emyrto/cptnHook but is standalone: it does not depend on any external library such as cctlib or PIN.

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
This will generate a directory called `myFirstProfile` containing the raw profile. The information

### List of intercepted functions
The list of intercepted functions is the following (both single and double precision):
* **sin**, **cos**, **exp**, **tan**, **sinh**, **cosh**, **tanh**
 * The inverse of the above ones
* **cbrt**

## How to produce a report
~~~
cptnhook-analyze myFirstProfile
~~~


### Converting to [ROOT](http://root.cern.ch) format
Make sure to have [ROOT](http://root.cern.ch) installed. Then there are two ways of converting to [ROOT](http://root.cern.ch) format the raw profile.
Either manually with a script
~~~
cptnhook-analyze --root myFirstProfile
~~~
or directly specifying this in the invocation of cptnhook
~~~
cptnhook -o myFirstProfile --root -- myprogram --arg1 --arg2 arg3
~~~

## [MetaLibm](http://www.metalibm.org/ANRMetaLibm/)
CptnHook has been developed in the context of the [MetaLibm](http://www.metalibm.org/ANRMetaLibm/) project, financed by [French National Research Agency (ANR)](http://www.agence-nationale-recherche.fr/en/).
