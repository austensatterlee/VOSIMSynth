/*
Copyright 2016, Austen Satterlee

This file is part of VOSIMProject.

VOSIMProject is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

VOSIMProject is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with VOSIMProject. If not, see <http://www.gnu.org/licenses/>.
*/

/**
 *  \file common.h
 *  \brief 
 *  \details
 *  \author Austen Satterlee
 *  \date 12/2016
 */

#pragma once

#if defined(VOSIMSYNTH_SHARED)
    #if defined(_WIN32)
        // Windows compilers need specific (and different) keywords for export and import
        #if defined(VOSIMSynth_EXPORTS)
            #define VOSIMSYNTH_API __declspec(dllexport)
        #else
            #define VOSIMSYNTH_API __declspec(dllimport)
        #endif
    #elif defined(VOSIMSynth_EXPORTS) // Linux, FreeBSD, Mac OS X
        #define VOSIMSYNTH_API __attribute__ ((__visibility__ ("default")))
    #else
        #define VOSIMSYNTH_API
    #endif
#else
    // Static build doesn't need import/export macros
    #define VOSIMSYNTH_API
#endif
