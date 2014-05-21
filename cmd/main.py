#!/usr/bin/env python
import argparse
import PyIp
import numpy
from PIL import Image
import time


parser = argparse.ArgumentParser(description="Command line tool for GPU Image Processing")
parser.add_argument("filename", help="Image Processing file *.ip")
parser.add_argument("-p", "--parameter", action="append", nargs = 2, help="Change value of parameter")
parser.add_argument("-v","--verbose", help="Outputs information", action="store_true")

args = parser.parse_args()

ip = None

replaced_args = {}
for p in args.parameter:
    replaced_args[p[0]] = p[1]

with open(args.filename, "r") as text_file:
    s1 = "/* --- Meta Data - START ---\n"
    s2 = "------ Meta Data - END --- */\n"
    f = text_file.read()
                
    meta = f[f.find(s1) + len(s1):f.find(s2)].split("\n")

    d = None
    outdata = []
    if args.verbose:
        print "Meta Data:"
    for p in meta:
        if args.verbose:
            print p
        v = p.split()
        if not len(v):
            continue

        if v[0] == "type":
            wrapper_type = v[1]
            ip = PyIp.Wrapper(wrapper_type)

        name = v[1]
        val = ""
        if len(v) > 2:
            if name in replaced_args:
                val = replaced_args[name]
            else:
                val = v[2]
            val = v[2] if name not in replaced_args else replaced_args[name]
            
        if v[0] == "input_buffer":
            image = Image.open(val)
            data = numpy.array(image).transpose((1,0,2)).copy(order="C")
            ip.AddInputBuffer(data)
            if d is None:
                d = data
        elif v[0] == "output_buffer":
            outdata.append((val,numpy.zeros((d.shape[0], d.shape[1], d.shape[2]), 
                              d.dtype)))
            ip.AddOutputBuffer(outdata[-1][1])
        elif v[0] == "double":
            ip.SetParameterFloat(name, eval(val))
        elif v[0] == "int":
            ip.SetParameterInt(name, eval(val))
    t = time.time()

    if args.verbose:
        if len(replaced_args):
            print "Updated parameter values:"
        for name in replaced_args:
            print "    " + name + " : " + replaced_args[name]
        print ""
 
    ip.Build(f[f.find(s2) + len(s2):])
    if args.verbose:
        dt = time.time() - t
        print "Building kernel took " +str(round(1000*dt,2))+" ms"

    t = time.time()
    ip.Process()
    
    if args.verbose:
        dt = time.time() - t
        print "Running  kernel took " +str(round(1000*dt,2))+" ms"

    for outname,data in outdata:
        out = Image.fromarray(data.transpose(1,0,2))
        out.save(outname)
