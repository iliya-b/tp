#include <stdio.h>
#include "../YY.utest/utest.h"


#include "cpu.h"
#include "memory.h"

struct CpuTestFixture {
    rz_cpu_p cpu;
};

UTEST_F_SETUP(CpuTestFixture) {
    utest_fixture->cpu = rz_create_cpu();
}


UTEST_F(CpuTestFixture, Test_Factorial_10){
    FILE *code_file = fopen("tests/factorial10.o", "rb");
    fread(mem_access(TEXT_OFFSET), 1, TEXT_SIZE, code_file);
    fclose(code_file);

    while(rz_cycle(utest_fixture->cpu));

    ASSERT_STREQ(rz_stdout(utest_fixture->cpu), "3628800");
}


UTEST_F_TEARDOWN(CpuTestFixture) {
    rz_free_cpu(utest_fixture->cpu);
}


UTEST_MAIN();