# Copyright (c) 2014, Moritz Nisblé (moritz.nisble@gmx.de)
# All rights reserved.
#
# Melpers - Moritz' little helpers
#
# TODO: Add documentation!!
#
#
# Functions/Macros
#
# melp_add_sources(<source variable> [<source file 1> ... <source file N>])
#
# melp_add_test_executable(TARGET <source file 1> [... <source file N>]
# 							[LIBS <lib 1> ... <lib N>]
# 							[ARGS <test arg 1> ... <test arg  N>])
#
# melp_indent(<variable to indent> <indent count>)
#
# melp_print_list(<list variable>
# 					[<heading string> [FULLINDENT|HALFINDENT]]
# 					[HORIZONTAL] <- FIXME: Stupid! Cause one can simply use message ;)
# 					[SEPERATOR])
#
# function(melp_add_prefix list prefix)
#
# function(melp_add_suffix list suffix)
#
#
# Options
#
# Enable test creation when melp_add_test_executable() is used.
# MELP_TEST_CREATION
#
# Verbosity control
# MELP_QUIET
# MELP_DEBUG
#
#
#
# Changelog:
#            v1.0: Basic versions of add_sources, add_dirs, print_sources, print_includes.
#            v1.1: Replaced print_includes by print_include_property_per_subdir.
#                  Added print_processed_subdirs.
#            v1.2: Added addprefix and addsuffix functions.
#            v1.3: Added print_list function; Renamed prefix/suffix functions.
#                  Declared add_dirs macro (recursive add_subdirectories) as deprecated.
#            v1.4: Fix: add_sources is now able to handle absolute paths.
#                  Moved add_sources into a function to not pollute the global scope.
#                  Added debug output functions.
#                  Added MELP_QUIET/MELP_DEBUG/MELP_QUIET_CMAKE vaiables to control verbosity.
#                  Reworked print list to be able to control output behavior.
#                  Added function that is able to indent a given string.
#                  Prefixed all functions/macros.
#                  Removed deprecated macros/functions.

# Suppress all standard messages by overwriting the message function.
# OK this is crap! So -> DEPRECATED
macro(melp_quiet_cmake)
	# Thanks to Fraser! http://stackoverflow.com/a/10512789
	function(message)
		list(GET ARGV 0 MessageType)
		if(MessageType STREQUAL FATAL_ERROR OR
				MessageType STREQUAL SEND_ERROR OR
				MessageType STREQUAL WARNING OR
				MessageType STREQUAL AUTHOR_WARNING)
			list(REMOVE_AT ARGV 0)
			_message(${MessageType} "${ARGV}")
		endif()
	endfunction()

	if(CMAKE_PROJECT_NAME)
		message(AUTHOR_WARNING "You should call melp_quiet_cmake() from at the \
top of the root CMake source file (before project() command), else Melpers is \
not able to suppress all output.")
	endif()
endmacro()

# Overwrite message when MELP_QUIET_CMAKE was specified on command line.
# DEPRECATED
if(MELP_QUIET_CMAKE)
	melp_quiet_cmake()
endif()

# -- internal --
# Output a debug message
# _debug([<message string 1> ... <message string N>] [PREFIX <prefix string>])
function(_debug)
	# TODO: This is a bit weird. Prefix is useless, or not?
	if(NOT MELP_QUIET AND MELP_DEBUG)
		set(_messages)
		set(_prefixed FALSE)
		foreach(_arg ${ARGN})
			if("x${_arg}" STREQUAL "xPREFIX")
				set(_prefixed TRUE)
			elseif(_prefixed)
				set(_prefix "${_arg} ")
			else()
				# Assume message
				list(APPEND _messages "${_arg}")
			endif()
		endforeach()
		foreach(_msg ${_messages})
			message(STATUS "[MELP] ${_prefix}${_msg}")
		endforeach()
	endif()
endfunction()

# -- internal --
macro(_debug_print_list list)
	if(NOT MELP_QUIET AND MELP_DEBUG)
		melp_print_list(${list} ${ARGN})
	endif()
endmacro()

# -- internal --
function(_add_sources SOURCE_VAR)
	_debug("Invoked by ${CMAKE_CURRENT_LIST_FILE}" PREFIX "melp_add_sources():")
	set(_relPath)

	# Get the realtive path from root to macro calling file
    file(RELATIVE_PATH _relPathToMacro "${CMAKE_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}")
 	_debug("Relative call path: ${_relPathToMacro}" PREFIX "melp_add_sources():")

	# If we have a relative path to macro, we are not in the root dir.
	if(_relPathToMacro)
		# Add relative path to macro to _relPath
		set(_relPath "${_relPathToMacro}/")
	endif()

	# Process passed sources
    foreach(_src ${ARGN})
 		_debug("Adding: ${_src}" PREFIX "melp_add_sources():")

		# Check if source is passed absolute
		if(IS_ABSOLUTE "${_src}")
			# If passed absolute, cut out the rel path between macro and source file
			file(RELATIVE_PATH _relPathToSource "${CMAKE_CURRENT_SOURCE_DIR}" "${_src}")
 			_debug("Relative source path: ${_relPathToSource}" PREFIX "melp_add_sources():")
			# and build the path to source relative to root directory
			list(APPEND ${SOURCE_VAR} "${_relPath}${_relPathToSource}")
		else()
			# If passed relative, simply add the relative path from root to current source dir
			list(APPEND ${SOURCE_VAR} "${_relPath}${_src}")
		endif()
    endforeach()

	# If invoked from subdirectory, propagate variable into parent scope
    if(_relPathToMacro)
        set(${SOURCE_VAR} ${${SOURCE_VAR}} PARENT_SCOPE)
    endif()
 	_debug_print_list(${SOURCE_VAR} "melp_add_sources(): Variable \"${SOURCE_VAR}\":" HALFINDENT)
endfunction()

# Add source macro
# melp_add_sources(<source variable> [<source file 1> ... <source file N>])
macro(melp_add_sources SOURCE_VAR)
	_add_sources(${ARGV})
	set(${SOURCE_VAR} ${${SOURCE_VAR}} PARENT_SCOPE)
endmacro()

# melp_add_test_executable(TARGET <source file 1> [... <source file N>]
# 							[LIBS <lib 1> ... <lib N>]
#							[ARGS <test arg 1> ... <test arg  N>])
function(melp_add_test_executable TARGET)
	if(MELP_TEST_CREATION)
		# Analyse options
		set(_target ${TARGET})
		set(_sources)
		set(_libs)
		set(_args)
		foreach(_arg ${ARGN})
			if("x${_arg}" STREQUAL "xLIBS")
				set(_libs_found TRUE)
				set(_args_found FALSE)
			elseif("x${_arg}" STREQUAL "xARGS")
				set(_args_found TRUE)
				set(_libs_found FALSE)

			elseif(_libs_found)
				list(APPEND _libs "${_arg}")
			elseif(_args_found)
				list(APPEND _args "${_arg}")
			else()
				# Assume source files
				list(APPEND _sources "${_arg}")
			endif()
		endforeach()

		_debug("melp_add_test_executable(): ${_target}")
		_debug_print_list(_sources "melp_add_test_executable(): Sources:" HALFINDENT)
		_debug_print_list(_libs "melp_add_test_executable(): Libs:" HALFINDENT)
		_debug_print_list(_args "melp_add_test_executable(): Test args:" HALFINDENT)

		add_executable(${_target} ${_sources})
		target_link_libraries(${_target} ${_libs})
		add_test(${_target} ${_target} ${_args})
	endif()
endfunction()

# melp_indent(<variable to indent> <indent count>)
function(melp_indent _string _count)
	set(_n ${_count})
	set(_indent "")
	while(${_n} GREATER 0)
		string(CONCAT _indent "${_indent} ")
		math(EXPR _n "${_n}-1")
	endwhile()
	set(${_string} "${_indent}${${_string}}" PARENT_SCOPE)
endfunction()

# melp_print_list(<list variable>
# 					[<heading string> [FULLINDENT|HALFINDENT]]
# 					[HORIZONTAL]
# 					[SEPERATOR])
function(melp_print_list list)
	# Analyse options
	set(_list ${list})
	foreach(_arg ${ARGN})
		if("x${_arg}" STREQUAL "xHORIZONTAL")
			set(_horizontal TRUE)
		elseif("x${_arg}" STREQUAL "xSEPERATOR")
			set(_seperator TRUE)
		elseif("x${_arg}" STREQUAL "xFULLINDENT")
			set(_fullindent TRUE)
		elseif("x${_arg}" STREQUAL "xHALFINDENT")
			set(_halfindent TRUE)
		else()
			# Else assume heading
			set(_heading "${_arg}")
			_debug("melp_print_list(): Heading: \"${_heading}\"")
		endif()
	endforeach()

	if(NOT _quiet)
		if(_seperator)
			message(STATUS "********************************* [MELPERS] *********************************")
		endif()

		if(_heading)
			message(STATUS "${_heading}")
		endif()

		if(_heading)
			if(_fullindent)
				string(LENGTH "${_heading}" _indentcount)
			elseif(_halfindent)
				string(LENGTH "${_heading}" _indentcount)
				math(EXPR _indentcount "${_indentcount}/2")
			endif()
		endif()

		if(_horizontal)
			message(AUTHOR_WARNING "Horizontal output not implemented yet!")
		else()
			foreach(item ${${list}})
				if(_indentcount)
					melp_indent(item ${_indentcount})
				endif()
				message(STATUS "${item}")
			endforeach()
		endif()

		if(_seperator)
			message(STATUS "*****************************************************************************")
		endif()
	endif()
endfunction()

function(melp_add_prefix list prefix)
	foreach(l ${${list}})
		list(APPEND _list ${prefix}${l})
	endforeach()
	set(${list} ${_list} PARENT_SCOPE)
endfunction()

function(melp_add_suffix list suffix)
	foreach(l ${${list}})
		list(APPEND _list ${l}${suffix})
	endforeach()
	set(${list} ${_list} PARENT_SCOPE)
endfunction()
