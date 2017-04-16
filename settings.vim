let s:cache_dir = $HOME . '/.cache/java'
let s:project_root = expand('<sfile>:p:h')
let s:sdk_root = $HOME . '/Android/Sdk'

let s:project_jars = [
    \ s:sdk_root .
    \ "/platforms/android-25/android.jar",
    \ ]

let s:ndk_includes = [
    \ s:sdk_root .
    \ '/ndk-bundle/sources/cxx-stl/llvm-libc++/include/',
    \ s:sdk_root .
    \ "/ndk-bundle/platforms/android-23/arch-x86_64/usr/include/",
    \ s:sdk_root .
    \ '/ndk-bundle/sources/android/support/include/',
    \ ]

" additional directories to include
let s:project_includes = [
    \ ]

" project java modules
let s:project_modules = [
    \ s:project_root . '/app/src/main/java',
    \ ]

" module names for intermediates
let s:modules_names = [
    \ 'app'
    \ ]

" support libraries
let s:support_path = s:sdk_root . 
    \ "/extras/android/m2repository/com/android/support"
let s:sup_ver = "25.3.1"
let s:support_libs = [
    \ "appcompat-v7", "support-compat" ,
    \ "support-core-ui", "support-core-utils" ,
    \ "support-v4", "support-core-utils" ,
    \ "support-fragment"
    \ ]

let $LC_ALL = "C"
call extend(g:ale_linters, {'c': ['gcc'], 'cpp': ['g++']})

function CollectSourceDirs(module_source_path)
  let l:javas = glob(a:module_source_path, '**/*.java')
  let l:javas = split(l:javas, '\n')
  let l:ret = []

  for l:item in l:javas
    let l:item_source_path = fnamemodify(l:item, ':p:h')
    if (index(l:ret, l:item_source_path) < 0)
      let l:ret =
          \ add(l:ret, l:item_source_path)
    endif
  endfor
  return l:ret
endfunction

function SetupJava()
  let l:project_sources = s:project_jars
  for l:module in s:project_modules
    let l:project_module_sources = CollectSourceDirs(l:module)
    let l:project_sources =
        \ l:project_sources + l:project_module_sources
  endfor
  " source files and libs for auto imports
  let g:JavaImpPaths = join(l:project_sources, ',')

  " support
  let l:support_classes = []
  for l:lib in s:support_libs
    let l:sup_class = s:cache_dir . "/support/" 
        \ . l:lib . "/" . s:sup_ver . "/classes.jar"
    if filereadable(l:sup_class)
      let l:support_classes = l:support_classes
          \ + [ l:sup_class ]
    endif
  endfor
  let g:JavaImpPaths = g:JavaImpPaths . ',' .
    \ join(l:support_classes, ',')

  " source files and libs for linter
  let l:intermediates = []
  for module_name in s:modules_names
    let l:interbuild_path = "/build/intermediates/classes/"
    let l:intermediates = l:intermediates +
      \ [s:project_root . '/' . module_name
      \ . interbuild_path . 'release']
    let l:intermediates = l:intermediates +
      \ [s:project_root . '/' . module_name
      \ . interbuild_path . 'debug']
  endfor

  let l:class_paths = join(s:project_jars, ':') .
      \ ':' . join(l:intermediates, ':') .
      \ ':' . join(l:support_classes, ':')

  let g:ale_java_javac_classpath = l:class_paths
  let g:ale_java_javac_options 
      \ = '-sourcepath ' . join(s:project_modules, ':')
      \ . ' -d ' . g:JavaImpDataDir
endfunction
call SetupJava()

function SetupNative()
  let l:includes = s:project_includes + s:ndk_includes
  let l:flat_includes = '-I' . join(l:includes, ' -I')

  let g:ale_c_gcc_options =
      \ '-Wall ' . l:flat_includes

  let g:ale_cpp_gcc_options =
      \ '-std=c++11 -Wall -g -no-canonical-prefixes ' .
      \ '-ffunction-sections -funwind-tables -DANDROID '
      \ . l:flat_includes
  let g:ale_c_clang_options = g:ale_c_gcc_options
  let g:ale_cpp_clang_options = g:ale_cpp_gcc_options
endfunction
call SetupNative()


" command to generate support classes
function AndroidGenProj()
  let l:support_classes = []
  for l:lib in s:support_libs
    let l:aar = s:support_path . "/"
        \ . l:lib . "/" . s:sup_ver . "/"
        \ . l:lib . "-" . s:sup_ver . ".aar"
    let l:extract_dir = s:cache_dir . "/support/"
        \ . l:lib . "/" . s:sup_ver . "/"
    silent execute "!mkdir -p " . l:extract_dir
    silent execute "!unzip -u " . l:aar .
      \ ' classes.jar -d ' l:extract_dir
    let l:support_classes = l:support_classes
        \ + [ l:extract_dir . '/classes.jar' ]
  endfor
  let g:JavaImpPaths = g:JavaImpPaths . ',' .
    \ join(l:support_classes, ',')
  JavaImpGenerate
  redraw!
endfunction
command AndroidGenProj call AndroidGenProj()

