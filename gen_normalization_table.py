#!/usr/bin/python3

from lxml import etree
import sys
import os
import pystache

DIR = os.path.dirname(os.path.realpath(__file__))
UNICODE_DATA = os.path.join(DIR, "ucd.all.flat.xml")

def cp_code(cp):
    try:
        return int(cp, 16)
    except:
        return 0
def to_hex(cp):
    return "{0:#08X}".format(int(cp)).replace('X', 'x')

class ucd_cp:
    def __init__(self, char):
        self.cp    = cp_code(char.get('cp'))
        self.na    = char.get('na')
        self.dt    = char.get('dt')
        self.hst   = char.get('hst')
        self.dm    = char.get('dm')
        self.block = char.get("blk")

    def has_decomposition(self):
        return self.dm != '#'

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
    chars = [char for char in chars if char.cp in canonical_decomposable_chars]
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

template_data = {
    "total_codepoint_count" : count,
    "total_decompositition_items": total_decompositition_items,
    "blocks" : decomposition_data,
    "block_count" : len(decomposition_data)
}

#Source file
import pathlib
pathlib.Path(os.path.join("generated", "include", "ucd", "details")).mkdir(parents=True, exist_ok=True)


with open(os.path.join(DIR, "tpl", "decomposition.cpp.tpl"), 'r') as f:
    template = f.read()
    with open(os.path.join("generated", "decomposition.cpp"), 'w') as out:
        out.write(pystache.render(template, template_data))

#Header
with open(os.path.join(DIR, "tpl", "decomposition_data.h.tpl"), 'r') as f:
    template = f.read()
    with open(os.path.join("generated", "include", "ucd", "details", "decomposition_data.h"), 'w') as out:
        out.write(pystache.render(template, template_data))




























