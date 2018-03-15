#!/usr/bin/python3

from lxml import etree
import sys
import os
import pystache
import re

DIR = os.path.dirname(os.path.realpath(__file__))
UNICODE_DATA = os.path.join(DIR, "ucd", "ucd.all.flat.xml")

def cp_code(cp):
    try:
        return int(cp, 16)
    except:
        return 0
def to_hex(cp, n = 8):
    return "{0:#0{fill}X}".format(int(cp), fill=n).replace('X', 'x')

class ucd_cp:
    def __init__(self, char):
        self.cp      = cp_code(char.get('cp'))
        self.na      = char.get('na')
        self.dt      = char.get('dt')
        self.hst     = char.get('hst')
        self.dm      = char.get('dm')
        self.block   = char.get("blk")
        self.ccc     = int(char.get("ccc"))
        self.Comp_Ex = char.get("Comp_Ex") == 'Y'

    def has_decomposition(self):
        return self.dm != '#'

    def is_primary_composite(self):
        return self.has_decomposition() and not self.Comp_Ex and len(self.decomposition_sequence()) == 2

    def has_canonical_decomposition(self):
        return self.has_decomposition() and self.dt == 'can'
    def is_hangul(self):
        return self.hst != 'NA'
    def decomposition_sequence(self):
        if self.has_decomposition():
            return list(map(cp_code, self.dm.split(" ")))
        return [self.cp]

class ucd_block:
    def __init__(self, block):
        self.first   = cp_code(block.get('first-cp'))
        self.last    = cp_code(block.get('last-cp'))
        self.name    = block.get('name')

#Blocks (Name, start, end)
blocks_data = {}

#Character decomposition data, sorted by blocks
blocks      = {}

#List of chars that can be decomposed
decomposable_chars = {}

#List of chars that can be decomposed canonically
canonical_decomposable_chars = {}

#List of chararcters with an non-zero combining class
combining_classes = [] # List of <cp, ccc> pairs

def find_block(cp):
    for block in blocks_data.values():
        if(cp >= block.first and cp <= block.last):
            return block
    return None

def char_to_dict(char, replacements):
    dec = char.decomposition_sequence()
    tpl = {"cp" : to_hex(char.cp), "sub": []}
    for sub in range(replacements):
        cp = dec[sub] if len(dec) > sub else None
        v = to_hex(cp) if cp else "nil_cp"
        tpl["sub"].append ({"dec_cp" : v, "comma" : True, "final" : not cp in decomposable_chars })

    if len(tpl["sub"]) > 1:
        del tpl["sub"][-1]["comma"]

    if char.has_canonical_decomposition():
        tpl["canonical"] = True

    return tpl

def unicode_characters_gen():
    context = etree.iterparse(UNICODE_DATA, events=('end',))
    for action, elem in context:
        if elem.tag=='char':
            yield ucd_cp(elem)
        if elem.tag=='block':
            yield ucd_block(elem)
        elem.clear()




def add_to_block(blocks, char):
    block = char.block
    if not block in blocks:
        blocks[block] = []
    blocks[block].append(char)

for item in unicode_characters_gen():
    if isinstance(item, ucd_block):
        blocks_data[item.name] = item
        continue

    if isinstance(item, ucd_cp):
        if item.is_hangul():
            continue
        if item.has_decomposition():
            decomposable_chars[item.cp] = item
            add_to_block(blocks, item)
        if item.has_canonical_decomposition():
            canonical_decomposable_chars[item.cp] = item
        if item.ccc != 0:
            combining_classes.append({"cp" : to_hex(item.cp, 6), "ccc" : item.ccc })


decomposition_data = []
count = 0
total_decompositition_items = 0

#mark recursive transformations as canonical
for _, chars in blocks.items():
    for char in chars:
        if char in canonical_decomposable_chars:
            for cp in char.decomposition_sequence():
                if cp in decomposable_chars:
                    canonical_decomposable_chars[cp] = decomposable_chars[cp]


for block, chars in blocks.items():
    #chars = [char for char in chars if char.cp in canonical_decomposable_chars]
    if len(chars) == 0:
        continue


    #find the name of the actual block
    first_char       = chars[0].cp
    block_data       = find_block(first_char)

    replacements = max(len(char.decomposition_sequence()) for char in chars)

    #Map to the actual block name
    dec = { "name"  : block_data.name,
            "first" : to_hex(block_data.first),
            "last"  : to_hex(block_data.last),
            "chars" : [char_to_dict(char, replacements) for char in chars],
            "start" : total_decompositition_items,
            "size"  : len(chars),
            "comma" : True,
            "number_of_replacements": replacements,
    }

    count = count + len(chars)
    total_decompositition_items = total_decompositition_items + len(chars) * (replacements +1)

    decomposition_data.append(dec);
del decomposition_data[-1]["comma"]

def gen_primary_composites():
    lc_c = {}
    for char in decomposable_chars.values():
        if not char.is_primary_composite():
            continue
        seq = char.decomposition_sequence()
        if len(seq) != 2:
            raise ValueError("Unexpected decomposition sequence {} {}".format(char.cp, seq))
        c = seq[1]
        if not c in lc_c :
            lc_c[c] =  []
        lc_c[c].append( (seq[0], char.cp ) )

    #sort each replacement
    for lst in lc_c.keys():
        lc_c[lst] = sorted(lc_c[lst], key=lambda i:(i[0]))

    list_of_c = []
    list_of_l_r = []
    for key in sorted(lc_c.keys()):
        list_of_c.append({"c" : to_hex(key), "start" : len(list_of_l_r), "count" : len(lc_c[key])})
        for l_r in lc_c[key]:
            list_of_l_r.append({"l" : to_hex(l_r[0]), "r" : to_hex(l_r[1])})
    print(max(len(x) for x in lc_c.values()))
    print(len(list_of_l_r))
    return(list_of_c, list_of_l_r)


primary_composites =  gen_primary_composites()

template_data = {
    "total_codepoint_count" : count,
    "total_decompositition_items": total_decompositition_items,
    "blocks" : decomposition_data,
    "block_count" : len(decomposition_data),
    "combining_classes" : combining_classes,
    "composites_c"   : primary_composites[0],
    "composites_l_r" : primary_composites[1]
}


#Source file
import pathlib
pathlib.Path(os.path.join("generated", "include", "ucd", "details")).mkdir(parents=True, exist_ok=True)


with open(os.path.join(DIR, "tpl", "decomposition.cpp.tpl"), 'r') as f:
    template = f.read()
    with open(os.path.join("generated", "decomposition.cpp"), 'w') as out:
        out.write(pystache.render(template, template_data))


def test_to_cpp_value(v):
    return "".join(map(lambda x:"\\U"+x.rjust(8, '0'), v.split(" ")))

class test:
    def __init__(self, c1, c2, c3, c4, c5, comment):
        self.c1  = test_to_cpp_value(c1)
        self.c2  = test_to_cpp_value(c2)
        self.c3  = test_to_cpp_value(c3)
        self.c4  = test_to_cpp_value(c4)
        self.c5  = test_to_cpp_value(c5)
        self.comment = comment

def test_cases():
    regex = re.compile(r'([0-9a-fA-F\s]+);([0-9a-fA-F\s]+);([0-9a-fA-F\s]+);([0-9a-fA-F\s]+);([0-9a-fA-F\s]+);\s*#(.*)$')

    tests = []
    with open(os.path.join(DIR, "ucd", "NormalizationTest.txt"), 'r') as test_file:
        for line in test_file:
            line = line.strip()
            if line == "" or line[0] == '#':
                continue;
            if line[0] == '@':
                continue
            else:
                res = regex.match(line)
                if "HANGUL" in res.group(6):
                    continue
                tests.append(test(res.group(1), res.group(2), res.group(3), res.group(4), res.group(5), res.group(6)))
    return tests


def chunks(l, n):
    """Yield successive n-sized chunks from l."""
    for i in range(0, len(l), n):
        yield l[i:i + n]

def generate_normalization_tests():
    with open(os.path.join(DIR, "tpl", "test_decomposition.cpp.tpl"), 'r') as f:
        template = f.read()
        for idx, tests in enumerate(chunks(test_cases(), 100)):
            with open(os.path.join("generated", "test_decomposition_{}.cpp".format(idx)), 'w') as out:
                out.write(pystache.render(template, {"tests" : tests, "idx" : idx } ))


generate_normalization_tests()


























