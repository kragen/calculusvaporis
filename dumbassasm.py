#!/usr/bin/python
"""Assembler for the Dumbass CPU."""
import sys, struct, re

def words(fileobj):
    for line in fileobj:
        line = re.sub(';.*', '', line)
        for word in line.split():
            yield word

instructions = { '.': 0, '-': 1, '|': 2, '@': 3, '!': 4, 'nop': 5 }

nbits = 12

def bit(n):
    return 1 << n

immediate = bit(nbits-1)

def bits(n):
    return bit(n) -1

def is_integer(word):
    try:
        int(word)
        return True
    except ValueError:
        return False

class Relocation:
    def __init__(self, encoding, destination):
        self.encoding = encoding
        self.destination = destination
    def resolve(self, program, resolution):
        program.memory[self.destination] = self.encoding(resolution)

def encode_immediate(value):
    return (value & bits(nbits-1)) | immediate

def encode_normal(value):
    return value

class Program:
    def __init__(self):
        self.memory = []
        self.labels = {}
        self.backpatches = {}
    def assemble_words(self, words):
        for word in words:
            self.assemble_word(word)
    def assemble_word(self, word):
        if word in instructions:
            self.assemble(instructions[word] << (nbits - 4))
        elif word.startswith('$'):
            self.assemble_reference(encode_immediate, word[1:])
        elif word.endswith(':'):
            self.assemble_label(word[:-1])
        else:
            self.assemble_reference(encode_normal, word)
    def assemble_reference(self, encoding, text):
        if is_integer(text):
            return self.assemble(encoding(int(text)))
        elif text in self.labels:
            return self.assemble(encoding(self.labels[text]))
        else:
            rel = Relocation(encoding, len(self.memory))
            self.backpatches.setdefault(text, []).append(rel)
            return self.assemble(0)
    def assemble(self, number):
        assert isinstance(number, int)
        self.memory.append(number)
    def assemble_label(self, label):
        self.resolve(label, len(self.memory))
    def resolve(self, label, value):
        if label in self.backpatches:
            for item in self.backpatches[label]:
                item.resolve(self, value)
            del self.backpatches[label]
        self.labels[label] = value
    def warn_undefined_labels(self):
        for label in self.backpatches:
            sys.stderr.write('WARNING: label %r undefined\n' % label)
    def dump(self, outfile):
        self.warn_undefined_labels()
        for number in self.memory:
            outfile.write(struct.pack('>l', number))

if __name__ == '__main__':
    import cgitb
    cgitb.enable(format='text')
    p = Program()
    p.assemble_words(words(sys.stdin))
    p.dump(sys.stdout)
