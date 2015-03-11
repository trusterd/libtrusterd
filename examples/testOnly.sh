#!/bin/sh

cwd=`dirname "${0}"`
 
expr  "${0}" : "/.*" > /dev/null || cwd=`(cd "${cwd}" && pwd)`

node $cwd/testOnly.js 



