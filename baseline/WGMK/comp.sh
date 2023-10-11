./scripts/compileBinary.sh ./source_programs/$1 &&
./scripts/compileJava.sh &&
rm -f src/compiledlib/dov/Cpu_* &&
cp src/com/appcomsci/mips/cpu/cpu.txt bin/com/appcomsci/mips/cpu/cpu.txt &&
./scripts/cpuFactory a.out
