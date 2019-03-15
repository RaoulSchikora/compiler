project('mcc', 'c',
    default_options: [ 'buildtype=release',
                       'c_std=c11',
                       'warning_level=3' ],
    meson_version: '>=0.44.0'
)

# ------------------------------------------------------------------ Generators

flex = find_program('flex')
lgen = generator(flex,
                 output: ['@BASENAME@.c', '@BASENAME@.h'],
                 arguments: [ '--outfile=@OUTPUT0@',
                              '--header-file=@OUTPUT1@',
                              '@INPUT@' ])

bison = find_program('bison')
pgen = generator(bison,
                 output: ['@BASENAME@.tab.c', '@BASENAME@.tab.h'],
                 arguments: [ '-Wall',
                              '--output=@OUTPUT0@',
                              '--defines=@OUTPUT1@',
                              '@INPUT@' ])


# --------------------------------------------------------------------- Library

mcc_inc = include_directories('include')

mcc_src = [ 'src/ast.c',
            'src/ast_print.c',
            'src/ast_visit.c',
            lgen.process('src/scanner.l'),
            pgen.process('src/parser.y') ]

mcc_lib = library('mcc', mcc_src,
                  c_args: '-D_POSIX_C_SOURCE=200809L',
                  include_directories: [mcc_inc, include_directories('src')])

# ---------------------------------------------------------------- Applications

mcc_apps = [ 'mcc', 'mc_ast_to_dot' ]

foreach app : mcc_apps
    executable(app, 'app/' + app + '.c',
               include_directories: mcc_inc,
               link_with: mcc_lib)
endforeach

# ----------------------------------------------------------------------- Tests

mcc_tests = [ 'parser_test' ]

cutest_inc = include_directories('vendor/cutest')

foreach test : mcc_tests
    t = executable(test, 'test/unit/' + test + '.c', 'vendor/cutest/CuTest.c',
                   include_directories: [mcc_inc, cutest_inc],
                   link_with: mcc_lib)
    test(test, t)
endforeach
