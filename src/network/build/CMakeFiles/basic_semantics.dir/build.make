# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.5

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/burunuduk/Documents/Geany/C/Problems/GL/Buffer/Boyko_Volodymyr/src/network

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/burunuduk/Documents/Geany/C/Problems/GL/Buffer/Boyko_Volodymyr/src/network/build

# Include any dependencies generated for this target.
include CMakeFiles/basic_semantics.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/basic_semantics.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/basic_semantics.dir/flags.make

CMakeFiles/basic_semantics.dir/tests/basic_semantics.cxx.o: CMakeFiles/basic_semantics.dir/flags.make
CMakeFiles/basic_semantics.dir/tests/basic_semantics.cxx.o: ../tests/basic_semantics.cxx
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/burunuduk/Documents/Geany/C/Problems/GL/Buffer/Boyko_Volodymyr/src/network/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/basic_semantics.dir/tests/basic_semantics.cxx.o"
	/usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/basic_semantics.dir/tests/basic_semantics.cxx.o -c /home/burunuduk/Documents/Geany/C/Problems/GL/Buffer/Boyko_Volodymyr/src/network/tests/basic_semantics.cxx

CMakeFiles/basic_semantics.dir/tests/basic_semantics.cxx.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/basic_semantics.dir/tests/basic_semantics.cxx.i"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/burunuduk/Documents/Geany/C/Problems/GL/Buffer/Boyko_Volodymyr/src/network/tests/basic_semantics.cxx > CMakeFiles/basic_semantics.dir/tests/basic_semantics.cxx.i

CMakeFiles/basic_semantics.dir/tests/basic_semantics.cxx.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/basic_semantics.dir/tests/basic_semantics.cxx.s"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/burunuduk/Documents/Geany/C/Problems/GL/Buffer/Boyko_Volodymyr/src/network/tests/basic_semantics.cxx -o CMakeFiles/basic_semantics.dir/tests/basic_semantics.cxx.s

CMakeFiles/basic_semantics.dir/tests/basic_semantics.cxx.o.requires:

.PHONY : CMakeFiles/basic_semantics.dir/tests/basic_semantics.cxx.o.requires

CMakeFiles/basic_semantics.dir/tests/basic_semantics.cxx.o.provides: CMakeFiles/basic_semantics.dir/tests/basic_semantics.cxx.o.requires
	$(MAKE) -f CMakeFiles/basic_semantics.dir/build.make CMakeFiles/basic_semantics.dir/tests/basic_semantics.cxx.o.provides.build
.PHONY : CMakeFiles/basic_semantics.dir/tests/basic_semantics.cxx.o.provides

CMakeFiles/basic_semantics.dir/tests/basic_semantics.cxx.o.provides.build: CMakeFiles/basic_semantics.dir/tests/basic_semantics.cxx.o


# Object files for target basic_semantics
basic_semantics_OBJECTS = \
"CMakeFiles/basic_semantics.dir/tests/basic_semantics.cxx.o"

# External object files for target basic_semantics
basic_semantics_EXTERNAL_OBJECTS =

basic_semantics: CMakeFiles/basic_semantics.dir/tests/basic_semantics.cxx.o
basic_semantics: CMakeFiles/basic_semantics.dir/build.make
basic_semantics: libCSocket.a
basic_semantics: CMakeFiles/basic_semantics.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/burunuduk/Documents/Geany/C/Problems/GL/Buffer/Boyko_Volodymyr/src/network/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable basic_semantics"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/basic_semantics.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/basic_semantics.dir/build: basic_semantics

.PHONY : CMakeFiles/basic_semantics.dir/build

CMakeFiles/basic_semantics.dir/requires: CMakeFiles/basic_semantics.dir/tests/basic_semantics.cxx.o.requires

.PHONY : CMakeFiles/basic_semantics.dir/requires

CMakeFiles/basic_semantics.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/basic_semantics.dir/cmake_clean.cmake
.PHONY : CMakeFiles/basic_semantics.dir/clean

CMakeFiles/basic_semantics.dir/depend:
	cd /home/burunuduk/Documents/Geany/C/Problems/GL/Buffer/Boyko_Volodymyr/src/network/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/burunuduk/Documents/Geany/C/Problems/GL/Buffer/Boyko_Volodymyr/src/network /home/burunuduk/Documents/Geany/C/Problems/GL/Buffer/Boyko_Volodymyr/src/network /home/burunuduk/Documents/Geany/C/Problems/GL/Buffer/Boyko_Volodymyr/src/network/build /home/burunuduk/Documents/Geany/C/Problems/GL/Buffer/Boyko_Volodymyr/src/network/build /home/burunuduk/Documents/Geany/C/Problems/GL/Buffer/Boyko_Volodymyr/src/network/build/CMakeFiles/basic_semantics.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/basic_semantics.dir/depend

