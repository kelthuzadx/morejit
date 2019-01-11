# morejit
Instruction wrapper to facilitate coding a just in time compiler for intel x86 architecture

# Details
Intel architecture instructions format is as follows:
![](docs/instr_fmt.png)
![](docs/instr_fmt1.png)

# Supported instructions for Intel x86 architecture
|opcode|api|description|
|:----:| ----- | :------|
|mov reg,imm| `instr::mov(jitcode& c, r dest, imm<ImmSize> imm)` |move 8/16/32 bits immediate number to register|

+ `reg` is shorthand for `register`
+ `imm` is shorthand for `immediate`
+ `mem` is shorthand for `memory address`