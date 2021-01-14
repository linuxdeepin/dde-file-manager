### As you recognized already, "###" at the begin of a line
### marks a comment.

### Everything between start and end will be put into the file
@@license-start@@
/* This file is part of the wvWare 2 project
   Copyright (C) 2001 Werner Trobin <trobin@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

// This code is generated from the Microsoft HTML specification of the
// WinWord format. Do NOT edit this code, but fix the spec or the script
// generating the sources. If you want to add some additional code, some
// includes or any other stuff, please add it to the template file!
// For information about the script and the "hidden features" please read
// the comments at the begin of the script.

// If you find bugs or strange behavior please contact Werner Trobin
// <trobin@kde.org>
@@license-end@@

### Everything between @@includes-start@@ and -end will be added to
### the generated includes (e.g. "#include <string>")
@@includes-start@@
####include <string>  // test
@@includes-end@@

### These are the "borders" of the namespace (e.g. namespace Word97 { .. })
### Everything you add in here will be added to the generated code
@@namespace-start@@
### nothing

// This has been added to the template file, as the mapping is
// non-trivial. Shaheed: Please check the implementation
Word97::BRC toWord97(const Word95::BRC &s) {

    Word97::BRC ret;

    // Check the BRC documentation
    if ( s.dxpLineWidth < 6 ) {
        ret.dptLineWidth = s.dxpLineWidth * 6;
        ret.brcType = s.brcType;
    }
    else if ( s.dxpLineWidth == 6 ) {
        ret.dptLineWidth = 6; // what's the default?
        ret.brcType = 6; // dotted
    }
    else { // s.dxpLineWidth == 7
        ret.dptLineWidth = 6; // what's the default?
        ret.brcType = 7;
    }
    ret.fShadow = s.fShadow;
    ret.ico = s.ico;
    ret.dptSpace = s.dxpSpace;
    return ret;
}

Word97::STSHI toWord97(const Word95::STSHI &s)
{
    Word97::STSHI ret;

    ret.cstd=s.cstd;
    ret.cbSTDBaseInFile=s.cbSTDBaseInFile;
    ret.fStdStylenamesWritten=s.fStdStylenamesWritten;
    ret.unused4_2=s.unused4_2;
    ret.stiMaxWhenSaved=s.stiMaxWhenSaved;
    ret.istdMaxFixedWhenSaved=s.istdMaxFixedWhenSaved;
    ret.nVerBuiltInNamesWhenSaved=s.nVerBuiltInNamesWhenSaved;
    ret.rgftcStandardChpStsh[0]=s.ftcStandardChpStsh;
    ret.rgftcStandardChpStsh[1]=s.ftcStandardChpStsh; // fake them
    ret.rgftcStandardChpStsh[2]=s.ftcStandardChpStsh; // fake them

    return ret;
}
### This tag "expands" to all the structs :)
@@generated-code@@
### nothing
@@namespace-end@@
