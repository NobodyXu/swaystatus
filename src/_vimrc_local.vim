let g:ale_linter_aliases = {'h': 'c'}

let include_path = system('pkg-config --cflags upower-glib')

let g:ale_c_gcc_options = expand(include_path)
let g:ale_c_clang_options = expand(include_path)

let g:ale_c_clangtidy_options = expand(include_path)
let g:ale_c_clangcheck_options = expand(include_path)
let g:ale_c_clangd_options = expand(include_path)
