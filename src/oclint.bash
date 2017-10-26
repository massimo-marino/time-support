#!/bin/bash
# 
# File:   oclint.bash
# Author: massimo
#
# Created on May 17, 2017, 3:37:22 PM
#
#
oclint -enable-global-analysis \
       -p ../build/Debug/GNU-Linux/_ext/71149f3b/ \
       *.cpp \
       -- -std=c++11 -m64 -Ofast -pedantic-errors -Wall -Weffc++ -Wextra -Wfatal-errors -c -Werror -s -fPIC -MMD -MP

#

