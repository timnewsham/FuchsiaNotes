#!/usr/bin/env python
"""
process files to get list of devices and ioctls
"""
import os

tab = {}
name = {}

def proc(fn, pref) :
    fn2 = fn[len(pref):]
    for l in file(fn) :
        l = l.strip()
        if l.startswith('case IOCTL_') :
            if not fn2 in tab :
                tab[fn2] = set()
            nm = l.split('case ', 1)[1].split(':',1)[0]
            #print fn2, nm
            tab[fn2].add(nm)
        if l.startswith('.name = "') :
            name[fn2] = l.split('"')[1]
            

def procDir(dn, pref) :
    for fn in os.listdir(dn) :
        full = os.path.join(dn, fn)
        if fn.endswith('.c') :
            proc(full, pref)
        if os.path.isdir(full) :
            procDir(full, pref)

def show() :
    for fn in sorted(tab.keys()) :
        print fn, name.get(fn)
        for nm in sorted(tab[fn]) :
            print '   ', nm
        print

pref = '../magenta/'
procDir(pref, pref)
show()

