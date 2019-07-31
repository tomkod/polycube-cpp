# polycube-cpp
Library for executing polycu.be scripts in C++ (work in progress).

Copyright (C) 2019 Tomasz Dobrowolski

# license

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.

# instructions

Before running cmake on this project:
1. Clone glfw from https://github.com/glfw/glfw.git to external/glfw.
2. Modify external/glfw/CMakeLists.txt
   to disable example programs, tests and documentation.
3. Generate glad using https://glad.dav1d.de
   and copy files:
      include/glad to glfw/deps/glad
      include/KHR to glfw/deps/KHR
      src to glfw/deps


