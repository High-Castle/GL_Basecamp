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
include CMakeFiles/source.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/source.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/source.dir/flags.make

CMakeFiles/source.dir/source/POSIX/Source.cxx.o: CMakeFiles/source.dir/flags.make
CMakeFiles/source.dir/source/POSIX/Source.cxx.o: ../source/POSIX/Source.cxx
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/burunuduk/Documents/Geany/C/Problems/GL/Buffer/Boyko_Volodymyr/src/network/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/source.dir/source/POSIX/Source.cxx.o"
	/usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/source.dir/source/POSIX/Source.cxx.o -c /home/burunuduk/Documents/Geany/C/Problems/GL/Buffer/Boyko_Volodymyr/src/network/source/POSIX/Source.cxx

CMakeFiles/source.dir/source/POSIX/Source.cxx.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/source.dir/source/POSIX/Source.cxx.i"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/burunuduk/Documents/Geany/C/Problems/GL/Buffer/Boyko_Volodymyr/src/network/source/POSIX/Source.cxx > CMakeFiles/source.dir/source/POSIX/Source.cxx.i

CMakeFiles/source.dir/source/POSIX/Source.cxx.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/source.dir/source/POSIX/Source.cxx.s"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/burunuduk/Documents/Geany/C/Problems/GL/Buffer/Boyko_Volodymyr/src/network/source/POSIX/Source.cxx -o CMakeFiles/source.dir/source/POSIX/Source.cxx.s

CMakeFiles/source.dir/source/POSIX/Source.cxx.o.requires:

.PHONY : CMakeFiles/source.dir/source/POSIX/Source.cxx.o.requires

CMakeFiles/source.dir/source/POSIX/Source.cxx.o.provides: CMakeFiles/source.dir/source/POSIX/Source.cxx.o.requires
	$(MAKE) -f CMakeFiles/source.dir/build.make CMakeFiles/source.dir/source/POSIX/Source.cxx.o.provides.build
.PHONY : CMakeFiles/source.dir/source/POSIX/Source.cxx.o.provides

CMakeFiles/source.dir/source/POSIX/Source.cxx.o.provides.build: CMakeFiles/source.dir/source/POSIX/Source.cxx.o


# Object files for target source
source_OBJECTS = \
"CMakeFiles/source.dir/source/POSIX/Source.cxx.o"

# External object files for target source
source_EXTERNAL_OBJECTS =

libsource.a: CMakeFiles/source.dir/source/POSIX/Source.cxx.o
libsource.a: CMakeFiles/source.dir/build.make
libsource.a: CMakeFiles/source.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/burunuduk/Documents/Geany/C/Problems/GL/Buffer/Boyko_Volodymyr/src/network/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX static library libsource.a"
	$(CMAKE_COMMAND) -P CMakeFiles/source.dir/cmake_clean_target.cmake
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/source.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/source.dir/build: libsource.a

.PHONY : CMakeFiles/source.dir/build

CMakeFiles/source.dir/requires: CMakeFiles/source.dir/source/POSIX/Source.cxx.o.requires

.PHONY : CMakeFiles/source.dir/requires

CMakeFiles/source.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/source.dir/cmake_clean.cmake
.PHONY : CMakeFiles/source.dir/clean

CMakeFiles/source.dir/depend:
	cd /home/burunuduk/Documents/Geany/C/Problems/GL/Buffer/Boyko_Volodymyr/src/network/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/burunuduk/Documents/Geany/C/Problems/GL/Buffer/Boyko_Volodymyr/src/network /home/burunuduk/Documents/Geany/C/Problems/GL/Buffer/Boyko_Volodymyr/src/network /home/burunuduk/Documents/Geany/C/Problems/GL/Buffer/Boyko_Volodymyr/src/network/build /home/burunuduk/Documents/Geany/C/Problems/GL/Buffer/Boyko_Volodymyr/src/network/build /home/burunuduk/Documents/Geany/C/Problems/GL/Buffer/Boyko_Volodymyr/src/network/build/CMakeFiles/source.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/source.dir/depend

