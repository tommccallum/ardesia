set number
set exrc
set secure
set tabstop=4
set softtabstop=4
set shiftwidth=4
set noexpandtab
set colorcolumn=110
highlight ColorColumn ctermbg=darkgray
augroup project
	autocmd!
	autocmd BufRead,BufNewFile *.hpp,*.cpp set filetype=cpp.doxygen
augroup END
let &path.="P/include,P/src,"
set includeexpr=substitute(v:fname,'\\.','/','g')
 " path to directory where library can be found
 let g:clang_library_path='/usr/lib64/llvm/lib'
 " or path directly to the library file
 let g:clang_library_path='/usr/lib64/libclang.so.5'
colorscheme wombat
