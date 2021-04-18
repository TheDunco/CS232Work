RAM_SIZE = 1024


class RAM:
    def __init__(self, size=RAM_SIZE):
        self._minAddr = 0
        self._maxAddr = RAM_SIZE - 1
        self._memory = []   # a list of values.  Could be #s or instructions.
        for i in range(size):
            self._memory.append(0)

    def __getitem__(self, addr):
        return self._memory[addr]

    def __setitem__(self, addr, val):
        self._memory[addr] = val

    def is_legal_addr(self, addr):
        return self._minAddr <= addr <= self._maxAddr

class MMU:
    def __init__(self, ram):
        self.ram = ram
        self._reloc_reg = 0;
        self._limit_reg = 0;
    
    def getMemValue(self, addr):
        self.check_valid_addr(addr)
            # Code will automatically break without explicity generating trap?
        return self.ram.__getitem__(addr + self._reloc_reg) 

    def setMemValue(self, addr, val):
        if self.check_valid_addr(addr):
            self.ram.__setitem__(addr + self._reloc_reg, val) 

    def set_reloc_register(self, addr):
        self._reloc_reg = addr
    
    def get_reloc_register(self):
        return self._reloc_reg

    def set_limit_register(self, addr):
        self._limit_reg = addr
    
    def get_limit_register(self):
        return self._limit_reg

    def check_valid_addr(self, addr):
        if addr + self._reloc_reg < self._limit_reg:
            return True
        else:
            print("Invalid memory access {}: too high! Reloc: {}, Limit: {}".format(addr, self._reloc_reg, self._limit_reg))
            return False

        
