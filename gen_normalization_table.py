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
        self.NFC_QC  = char.get("NFC_QC") == 'Y'
        self.NFKC_QC = char.get("NFKC_QC") == 'Y'
        self.NFD_QC  = char.get("NFD_QC") == 'Y'
        self.NFKD_QC = char.get("NFKD_QC") == 'Y'

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

#list of all chars
all_chars = []

#List of chars that can be decomposed canonically
canonical_decomposable_chars = {}

#List of hangul syllables
hangul_syllables = {}

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
        if elem.tag=='{http://www.unicode.org/ns/2003/ucd/1.0}char':
            yield ucd_cp(elem)
        if elem.tag=='{http://www.unicode.org/ns/2003/ucd/1.0}block':
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
        all_chars.append(item)
        if item.is_hangul():
            hangul_syllables[item.cp] = item
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


def packed(blocks):
    current_block_size = -1
    packed_blocks = []
    current_block = None
    for block in blocks:
        for char in block:
            if current_block == None or current_block_size != len(char.decomposition_sequence()) or len(current_block) * (current_block_size + 1)  >= 64:
                current_block_size = len(char.decomposition_sequence())
                if current_block:
                    packed_blocks.append(current_block)
                current_block = []
            current_block.append(char)

    if current_block:
        packed_blocks.append(current_block)
    return packed_blocks

for block in packed(list(blocks.values())):
    #chars = [char for char in chars if char.cp in canonical_decomposable_chars]
    if len(block) == 0:
        continue


    #find the name of the actual block
    first_char       = block[0].cp
    block_data       = find_block(first_char)

    replacements = max(len(char.decomposition_sequence()) for char in block)

    #Map to the actual block name
    dec = { "name"  : block_data.name,
            "first" : to_hex(block[0].cp),
            "last"  : to_hex(block[-1].cp),
            "chars" : [char_to_dict(char, replacements) for char in block],
            "start" : total_decompositition_items,
            "size"  : len(block),
            "has_canonical" : "true" if len([c for c in block if c.cp in canonical_decomposable_chars]) > 0 else "false",
            "comma" : True,
            "number_of_replacements": replacements,
    }

    count = count + len(block)
    total_decompositition_items = total_decompositition_items + len(block) * (replacements +1)

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
        canonical = 0
        pos = len(list_of_l_r)
        for l_r in lc_c[key]:
            item = {"l" : to_hex(l_r[0]), "r" : to_hex(l_r[1])}
            if not l_r[1] in canonical_decomposable_chars\
                or not canonical_decomposable_chars[l_r[1]].has_canonical_decomposition():
                    continue
            canonical = canonical + 1
            list_of_l_r.append(item)
        if canonical > 0:
            list_of_c.append({"c" : to_hex(key),
                              "start" : pos,
                              "count" : canonical})
    return(list_of_c, list_of_l_r)


primary_composites =  gen_primary_composites()


def gen_hangul_syllables():
    keys = sorted(list(hangul_syllables.keys()))
    first_cp  = keys[0]
    last_cp   = keys[-1] + 2
    previous_type = 'NA'
    syllables = []

    for cp in range(first_cp, last_cp):

        if cp in hangul_syllables:
            type = hangul_syllables[cp].hst
        else:
            type = 'NA'

        if type == previous_type:
            continue

        if type == 'L':
            typename = "leading_jamo"
        elif type == 'V':
            typename = "vowel_jamo"
        elif type == 'T':
            typename = "trailing_jamo"
        elif type == 'LV':
            typename = "lv_syllable"
        elif type == 'LVT':
            typename = "lvt_syllable"
        elif type == 'NA':
            typename = "invalid"

        syllables.append({"cp" : to_hex(cp), "type": typename })
        previous_type = type

    return syllables


def gen_trie_table(char_list):
    jump_table = []
    for i in range(256):
        jump_table.append([])

    for char in char_list:
        cp = int(char.cp)
        idx = cp & 0xff
        jump_table[idx].append(char)
    pos = 0
    data = []
    starts = []
    for jump_table_item in jump_table:
        starts.append({"idx" : pos })
        pos  += len(jump_table_item)
        for char in jump_table_item:
            data.append({"cp" : to_hex(char.cp >> 8, 4), "ccc" : char.ccc })
    return  {"starts" : starts, "data" : data}

template_data = {
    "total_codepoint_count" : count,
    "total_decompositition_items": total_decompositition_items,
    "blocks" : decomposition_data,
    "block_count" : len(decomposition_data),
    "combining_classes" : gen_trie_table([char for char in all_chars if char.ccc != 0]),
    "composites_c"     : primary_composites[0],
    "composites_l_r"   : primary_composites[1],
    "hangul_syllables" : gen_hangul_syllables(),
    "nfc_qc"  :  gen_trie_table([char for char in all_chars if not char.NFC_QC]) ,
    "nfkc_qc" :  gen_trie_table([char for char in all_chars if not char.NFKC_QC]),
    "nfd_qc"  :  gen_trie_table([char for char in all_chars if not char.NFD_QC and not char.is_hangul()]) ,
    "nfkd_qc" :  gen_trie_table([char for char in all_chars if not char.NFKD_QC and not char.is_hangul()])
}

print("Number of Codepoint  :",  count)
print("Number of Decomposition  :",  total_decompositition_items)
print("Number of Entry in the NFD table  :", sum([1 for char in all_chars if not char.NFD_QC and not char.is_hangul()]))
print("Number of Entry in the NFKK table :", sum([1 for char in all_chars if not char.NFKD_QC and not char.is_hangul()]))
print("Number of Entry in the NFC table  :", sum([1 for char in all_chars if not char.NFD_QC and not char.is_hangul()]))
print("Number of Entry in the NFKD table :", sum([1 for char in all_chars if not char.NFKC_QC and not char.is_hangul()]))



#Source file
import pathlib
pathlib.Path(os.path.join("generated", "include", "ucd", "details")).mkdir(parents=True, exist_ok=True)

with open(os.path.join(DIR, "tpl", "normalization_data.cpp.tpl"), 'r') as f:
    template = f.read()
    with open(os.path.join("generated", "normalization_data.cpp"), 'w') as out:
        out.write(pystache.render(template, template_data))


























