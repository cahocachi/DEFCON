# -*- Makefile -*- for Defcon

#
# In the parent directory of source/ contrib/ targets/
#
# make -f targets/linux/Makefile.debug
# 

#CXX=g++
#CC=gcc
CXXFLAGS_BASE=-w -g -O3 -pg
SUFFIX=-profile
LDFLAGS=-pg

#CPPFLAGS_BASE=-D_DEBUG

-include targets/linux/Makefile
