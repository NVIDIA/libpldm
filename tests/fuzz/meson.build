executable(
    'fd-fuzz',
    'fd-fuzz.cpp',
    # for __AFL_LOOP
    cpp_args: test_cpp_args + [
        '-Wno-gnu-statement-expression-from-macro-expansion',
    ],
    include_directories: test_include_dirs,
    dependencies: [libpldm_dep],
)
