// Copyright (c) 1997  
// Utrecht University (The Netherlands),
// ETH Zurich (Switzerland),
// INRIA Sophia-Antipolis (France),
// Max-Planck-Institute Saarbruecken (Germany),
// and Tel-Aviv University (Israel).  All rights reserved. 
//
// This file was part of CGAL (www.cgal.org); you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation; either version 3 of the License,
// or (at your option) any later version.
//
// Licensees holding a valid commercial license may use this file in
// accordance with the commercial license agreement provided with the software.
//
// This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
// WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
//
// $URL$
// $Id$
// 
//
// Author(s)     : Geert-Jan Giezeman and Sven Schönherr
// Modifications for R by Simon Urbanek

#include <CGAL/config.h>
#include <CGAL/assertions.h>
#include <CGAL/assertions_behaviour.h>
#include <CGAL/exceptions.h>

#include <CGAL/basic.h>
#include <CGAL/IO/io.h>

#include <sstream>
#include <string>

// This is a replacement for standard CGAL implementation
// such that we use our own errors/warning and can rely
// on headers only

#include <R.h>

namespace CGAL {
// failure functions
// -----------------
void
assertion_fail( const char* expr,
                const char* file,
                int         line,
                const char* msg)
{
  Rf_error("Assertion %s failed in %s line %d: %s\n", expr, file, line, msg);
}

void
precondition_fail( const char* expr,
                   const char* file,
                   int         line,
                   const char* msg)
{
  Rf_error("Precondition %s failed in %s line %d: %s\n", expr, file, line, msg);
}

void
postcondition_fail(const char* expr,
                   const char* file,
                   int         line,
                   const char* msg)
{
  Rf_error("Postcondition %s failed in %s line %d: %s\n", expr, file, line, msg);
}


// warning function
// ----------------
void
warning_fail( const char* expr,
              const char* file,
              int         line,
              const char* msg)
{
  Rf_warning("Warning: %s failed in %s line %d: %s\n", expr, file, line, msg);
}

int IO::mode = std::ios::xalloc();

} //namespace CGAL
