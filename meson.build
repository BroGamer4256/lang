project('lang', 'c', 'cpp', version: '1.0.0')
warning_level = 3
debug = true
optimization = 3
b_lto = true
b_pgo = 'use'

cpp = meson.get_compiler('cpp')

add_project_arguments(
	cpp.get_supported_arguments(
		'-D_WIN32_WINNT=_WIN32_WINNT_WIN10',
	),
	language: ['c', 'cpp']
)

add_project_link_arguments(
	cpp.get_supported_arguments(
		'-static',
		'-s',
		'-lshlwapi',
	),
	language: ['c', 'cpp']
)

cmake = import('cmake')
minhook = cmake.subproject('minhook')
abseil = subproject('abseil-cpp')

library(
	'lang',
	include_directories: [
		'src',
		'tomlc99/',
	],
	dependencies : [
		minhook.get_variable('minhook_dep'),
		abseil.get_variable('absl_hash_dep'),
		abseil.get_variable('absl_container_dep'),
	],
	sources : [
		'tomlc99/toml.c',
		'src/dllmain.cpp',
		'src/helpers.c',
		'src/sigscan.c',
	],
	name_prefix: ''
)
