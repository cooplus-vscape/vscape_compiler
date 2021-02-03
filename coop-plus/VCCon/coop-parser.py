#!/usr/bin/env python
import sys, os
import argparse
import logging

# vf_name: (loc, class, short_name, far_field, list_of_overriden)
vf_dict = {}

# vf_name: list_of_overriders
vf_overrider_map = {}

# sizess = {"sizeof":0, "dsize":0, "align":0, "nvsize":0, "nvalign":0}
# fields = {0: "type1", 4: "type2" ...}
# class_name: {sizes: {}, fields: {}}
class_dict = {}

# vf_name: [(loc, class), ...]
callsite_dict = {}

def parse_vf(vf_fname):
    with open(vf_fname, "r") as vf_file:
        while True:
            line = vf_file.readline()
            if not line:
                break
            if len(line.strip()) == 0:
                continue
            if not line.startswith("Virtual function LOC:"):
                raise Exception("malformed input file, expecting '{}', line:{}".format("Virtual function LOC:", line))
            # next line is LOC
            loc = vf_file.readline().strip()
            logging.debug("loc: {}".format(loc))

            line = vf_file.readline()
            if not line.startswith("Virtual function name:"):
                raise Exception("malformed input file, expecting '{}', line:{}".format("Virtual function name:", line))
            # next line is vf name
            names = vf_file.readline().strip()
            comma_idx = names.find(",")
            vf_name = names[:comma_idx].strip()
            full_name = names[comma_idx+1:].strip()
            idx = full_name.rfind("::")
            if idx<0:
                raise Exception("wrong full vf name: {}".format(names))
            class_name = full_name[:idx]
            short_name = full_name[idx+2:]
            logging.info("parsing vf:{}".format(vf_name))

            line = vf_file.readline()
            if not line.startswith("Overridden functions:"):
                raise Exception("malformed input file, expecting '{}', line:{}".format("Overridden functions:", line))
            # next lines are list of overridden vf
            list_of_overriden = []
            line = vf_file.readline()
            while not line.startswith("Access fields:"):
                line_tokens = line.split()
                logging.debug("overridden: {}".format(line_tokens))
                if not line_tokens[0] == "++":
                    raise Exception("malformed input file, expecting '{}', line:{}".format("++ overridden_vf_name", line))
                overriden_name = line_tokens[1].split(",")[0]
                list_of_overriden.append(overriden_name)

                list_of_overriders = vf_overrider_map.setdefault(overriden_name, [])
                list_of_overriders.append(vf_name)
                line = vf_file.readline()
            
            # next lines are fields access info (maybe empty)
            far_field = 0
            line = vf_file.readline()
            while not line.startswith("Virtual function Finish:"):
                line_tokens = line.split()
                logging.debug("fields: {}".format(line_tokens))
                if not line_tokens[0] == "++":
                    raise Exception("malformed input file, expecting '{}', line:{}".format("++ field_offset", line))
                field = int(line_tokens[1])
                if field > far_field:
                    far_field = field 
                line = vf_file.readline()

            # close entry
            vf_name2 = vf_file.readline().strip().split(",")[0].strip()
            if not vf_name == vf_name2:
                raise Exception("malformed input file, expecting closure '{}', vf_name2:{}, line:{}".format(vf_name, vf_name2, line))


            vf_dict[vf_name] = (loc, class_name, short_name, far_field, list_of_overriden)
            # end line parsing loop.


        with open(vf_fname+".parsed.vf", "w") as f:
            f.write(str(vf_dict))
        with open(vf_fname+".parsed.overrider", "w") as f:
            f.write(str(vf_overrider_map))


def parse_layout(layout_fname):
    with open(layout_fname, "r") as vf_file:
        while True:
            line = vf_file.readline()
            if not line:
                break
            if len(line.strip()) == 0:
                continue
            
            if not line.startswith("Class name:"):
                raise Exception("malformed input file, expecting '{}', line:{}".format("Class name:", line))
            # next line is the class name
            class_name = vf_file.readline().strip()
            logging.info("parsing class: {}".format(class_name))

            line = vf_file.readline()
            if not line.startswith("Class layout:"):
                raise Exception("malformed input file, expecting '{}', line:{}".format("Class layout:", line))
            # next lines are field type info
            far_field = 0
            sizes = {"sizeof":0, "dsize":0, "align":0, "nvsize":0, "nvalign":0}
            class_entry = class_dict.setdefault(class_name, {"sizes": sizes, "fields": {}})
            line = vf_file.readline()
            while line.find("|") >= 0:
                tokens = line.split("|")
                left = tokens[0].strip()
                right = tokens[1].strip()

                if not left:
                    right = right.replace("[", "").replace("]", "")
                    tokens = right.split(",")

                    for token in tokens:
                        token = token.strip()
                        if not token:
                            continue
                        t = token.split("=")
                        class_entry['sizes'][t[0].strip()] = int(t[1].strip())
                else:
                    idx = left.find(":")
                    bits = ""
                    if idx >= 0:
                        left = left[:idx]
                        bits = "(bits: " + left[idx+1:] + " )"
                    left = int(left)
                    field = class_entry["fields"].setdefault(left, "")
                    class_entry["fields"][left] = bits + field + " " + right

                line = vf_file.readline()

            # end line parsing loop.

        with open(layout_fname+".parsed", "w") as f:
            f.write(str(class_dict))

def parse_callsite(callsite_fname):
    with open(callsite_fname) as vf_file:
        while True:
            line = vf_file.readline()
            if not line:
                break
            if len(line.strip()) == 0:
                continue

            if not line.startswith("Call site LOC:"):
                raise Exception("malformed input file, expecting '{}', line:{}".format("Call site LOC:", line))
            # next line is LOC
            loc = vf_file.readline().strip()

            line = vf_file.readline()
            if not line.startswith("Target virtual function:"):
                raise Exception("malformed input file, expecting '{}', line:{}".format("Target virtual function:", line))
            # next line is vf name
            names = vf_file.readline().strip()
            comma_idx = names.find(",")
            vf_name = names[:comma_idx].strip()
            full_name = names[comma_idx+1:].strip()
            idx = full_name.rfind("::")
            if idx<0:
                raise Exception("wrong full vf name: {}".format(names))
            class_name = full_name[:idx]
            short_name = full_name[idx+2:]
            logging.info("parsing callsite:{}, vf:{}".format(loc, vf_name))

            line = vf_file.readline()
            if not line.startswith("Target class:"):
                raise Exception("malformed input file, expecting '{}', line:{}".format("Target class:", line))
            class_name2 = vf_file.readline().strip()
            if class_name.find( class_name2 ) < 0:  #TODO: make sure they are equal
                raise Exception("mismatch class name: {} != {}, for callsite {}, vf: {}".format(class_name, class_name2, loc, vf_name))

            vf_callsites = callsite_dict.setdefault(vf_name, [])
            vf_callsites.append((loc, class_name))
            # end line parsing loop.

        with open(callsite_fname + ".parsed", "w") as f:
            f.write(str(callsite_dict))


def analyze():
    for vf_name in callsite_dict:
        callsites = callsite_dict[vf_name]
        for callsite in callsites:
            loc = callsite[0]
            class_name = callsite[1]
            if class_dict.has_key(class_name):
                obj_size = class_dict[class_name]['sizes']['sizeof']
            else:
                obj_size = 0    # abstract base class
            overriders = vf_overrider_map.get(vf_name, [])
            for overrider in overriders:
                far_field = vf_dict[overrider][3]
                if far_field >=  obj_size:
                    print "++++ callsite:{} invokes vf:{}, obj_size:{}, but overrider vf:{} accesses field:{}".format(loc, vf_name, obj_size, overrider, far_field)





if __name__=='__main__':
    parser = argparse.ArgumentParser(description='Parse coop++ related output')
    parser.add_argument('--vf', help='Virtual Function Definition Metadata')
    parser.add_argument('--layout', help='Class Layout Metadata')
    parser.add_argument('--callsite', help='Callsite Metadata')
    
    args = parser.parse_args()

    logger = logging.getLogger()
    logger.setLevel(logging.WARNING)

    if args.vf:
        parse_vf(args.vf)
    if args.layout:
        parse_layout(args.layout)
    if args.callsite:
        parse_callsite(args.callsite)

    if args.vf and args.layout and args.callsite:
        analyze()
