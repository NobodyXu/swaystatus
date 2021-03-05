let g:ale_linter_aliases = {'h': 'c', 'hpp': 'cpp', 'cc': 'cpp'}

let g:ale_c_parse_makefile = 1
let g:ale_c_parse_compile_commands = 1

let include_path = system('pkg-config --cflags alsa json-c')

let g:ale_c_clangd_options = '-std=c11' . expand(include_path)
let g:ale_c_clangtidy_options = "-std=c11 " . expand(include_path)
let g:ale_c_clangcheck_options = "-std=c11 " . expand(include_path)

let g:ale_cpp_clangd_options = '-std=c++17' . expand(include_path)
let g:ale_cpp_clangtidy_options = "-std=c++17 " . expand(include_path)
let g:ale_cpp_clangcheck_options = "-std=c++17 " . expand(include_path)
let g:ale_cpp_cppcheck_options = "-std=c++17 " . expand(include_path)
