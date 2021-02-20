let g:ale_linter_aliases = {'h': 'c'}

let include_path = system('pkg-config --cflags upower-glib alsa libnm json-c')

let g:ale_c_gcc_options = "-std=c11 " . expand(include_path)
let g:ale_c_clang_options = "-std=c11 " . expand(include_path)

let g:ale_c_cc_options = "-std=c11 " . expand(include_path)

let g:ale_cpp_gcc_options = "-std=c++17 " . expand(include_path)
let g:ale_cpp_clang_options = "-std=c++17 " . expand(include_path)

let g:ale_cpp_cc_options = "-std=c++17 " . expand(include_path)

let g:ale_c_clangtidy_options = "-std=c11 " . expand(include_path)
let g:ale_c_clangcheck_options = "-std=c11 " . expand(include_path)
let g:ale_c_clangd_options = "-std=c11 " . expand(include_path)

let g:ale_cpp_clangtidy_options = "-std=c++17 " . expand(include_path)
let g:ale_cpp_clangcheck_options = "-std=c++17 " . expand(include_path)
let g:ale_cpp_clangd_options = "-std=c++17 " . expand(include_path)
