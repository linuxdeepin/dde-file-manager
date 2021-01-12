#!/usr/local/bin/perl -w
use strict;  # we at least try to ;)
use Class::Struct;

# This file is part of the wvWare 2 project
# Copyright (C) 2001-2003 Werner Trobin <trobin@kde.org>

# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Library General Public
# License version 2 as published by the Free Software Foundation.

# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Library General Public License for more details.

# You should have received a copy of the GNU Library General Public License
# along with this library; see the file COPYING.LIB.  If not, write to
# the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
# Boston, MA 02111-1307, USA.


# A small utility to generate the basic classes needed to
# read an write primitive Word structures.
# Usage: perl generate.pl input_file.html Word97
# The input_file.html is the document we want to process,
# 'Word97' is used for various things:
#   - The namespace is called Word97 and all the generated
#     code lives in there
#   - word97_generated.cpp and word97_generated.h are the
#     filenames (note the case)

# A few notes about the form of the HTML document:
# 1) We expect all seven fields in the tables:
#    b10, b16, field, type, size, bitfield, comment
#    If any of them is absent just add empty ones (<td></td>)
# 2) If you want to set an initial value for a (plain!) variable
#    you can add a <!-- initial="50" --> HTML comment to the
#    "entry" (preferably after the "field" tag).
#    Note: It has to be on a separate line, else it won't get
#    picked up!
#    Note 2: We don't check the value, we just assign it, so make
#    sure that this is legal C++ (e.g. inital="true", initial="42+42")!
#    Note 3: Everything else will be set to 0
# 3) In some cases the </table> tag has to be right after the
#    last </tr> tag, so better do that everywhere :)
# 4) An array with a dynamic size can easily be created by editing
#    the "type field." If you add, say "U8[foo]" then this means:
#       - we create a dynamic array of size "foo", where "foo" is
#         some variable of that structure we have already read.
#         Note: We don't do any error checking here, so be careful
#               not to use uninitialized values we didn't read at
#               the time we create the array!
#         Note2: You can even put plain expressions there, or a
#                call to a function you include in the template!
#                Just make sure that it's legal C++ and that it
#                doesn't contain any '[' or ']' as it will probably
#                confuse the parser.
#      - if foo=="", i.e. if you just have "U32[]" then we will
#        just create a plain pointer for you and initialize it with 0.
#         Note: Plain destruction will work as we just delete [] it.
#         Attention: Copy CTOR and assignment operator won't work!!!!
#                    (as we can't know the length) What we do is what
#                    C++ does by default - copy the pointer :}
#   To allow proper comparsions (operator==) we have to know the length
#   of the dynamich structure. Therefore you should add a HTML comment
#   to such items, specifying a way to perform that check.
#   (e.g. <!-- compareSizeLHS="lhs.cb" compareSizeRHS="rhs.cb" -->)
#   Everything between the quotes will be copied verbatim to an if statement
#   (e.g. if((lhs.cb)!=(rhs.cb)) ).
#   If you decide to call a function please ensure it returns something
#   useful we can compare :)
# 5) For all structures which need a way to "apply" a grpprl (e.g. PAP, CHP)
#    we provide a special method you can reimplement if you want to. In the
#    header we simply add a declaration.
# 6) If you need the possibility to share a structure (e.g. SEP, PAP, CHP,...)
#    you can add it to the list in sub selectShared()
# 7) In case you want to use the structure in any of the PL(C)F templates
#    you have to add the "sizeof" comment to the .htm file between the name of
#    the structure and the <table> (like for the DTTM struct)

# If you want to ignore certain structures, please add them to
# the 'cleanStructures' sub.
# If you need the possibility to read a structure from a plain
# pointer, too, please add it to the if statement in parseStructures

# This structure holds one "variable"
struct Item => {
    name => '$',         # The name of this variable
    type => '$',         # The type (e.g. U16, S32[42],...)
    bits => '$',         # The amount of bits (e.g. 3), if any
    comment => '$',      # The comment for this variable
    initial => '$',      # The initial value of this field, if any
    len => '$',          # If the item is a dynamic array we store its length
                         # here. length can be a plain C++ expression.
    compareSizeLHS => '$',  # If the item is a dynamic array we need to compare the
                            # left-hand-side (lhs) and the rhs in their size. This
                            # is a plain C++ expression returning the size of the LHS.
    compareSizeRHS => '$',  # If the item is a dynamic array we need to compare the
                            # left-hand-side (lhs) and the rhs in their size. This
                            # is a plain C++ expression returning the size of the RHS.
    startNew => '$',     # This field is used for debugging purposes. It
                         # is set to 1 if this variable should start a new
                         # bitfield (and close the last one). We simply
                         # check whether we filled the last field completely here
};

struct Structure => {
    name => '$',      # The name of the structure
    comment => '$',   # The comment for this struct
    items => '@',     # All the data members
    hidden => '$',    # Set to "//" if we want to comment that structure out
    dynamic => '$',   # Do we have dynamic memory? Then we need a Copy CTOR,
                      # DTOR, assignment op, op==,...
    readPtr => '$',   # Do we want to be able to construct/read from a pointer?
    shared => '$',    # Whether this structure should be derived from wvWare::Shared'
    sizeOf => '$',    # The size of the structure (not padded, as in the file!)
    dumpCode => '$',      # Whether dumping code should be generated
};


# This array of strings contains the whole HTML
# documentation file.
# All the parsing subs will read/modify that global array
# Note: All the tags we use are already converted to
# uppercase.
my @document;

# The current index in the document-array (used during parsing)
my $i;

# This string holds the name of the namespace to create
my $namespace;

# This array holds all the structures we want to write out
# It's filled during parsing and used heavily afterwards
my @structs;

# The current struct we're working on (only used during parsing)
my $struct;
# The current item we're working on (only used during parsing)
my $item;

# Parses all the structures
sub parseStructures {
    my ($tmp);

    print "Parsing...\n";
    $i=0;
    while($i<=$#document) {
        if($document[$i] =~ m,\</H3\>,) {
            if($document[$i-1] =~ m/\<H3\>/) {  # Safe, as </H3> can't be in the first line
                # looks okay
                $struct=Structure->new();  # create a new structure element
                $document[$i] =~ m,^(.*)\</H3\>,;
                $struct->comment($1);
            }
            elsif($document[$i] =~ m/\<H3\>/) {
                # looks okay, too
                $struct=Structure->new();  # create a new structure element
                $document[$i] =~ m,\<H3\>(.*)\</H3\>,;
                $struct->comment($1);
            }
            else {
                if($document[$i-1] !~ m/Algorithm/) {
                    # huh? Shouldn't happen at all
                    print "####### ERROR #######\n";
                    print $document[$i-1], "\n", $document[$i], "\n";
                }
                $i++; # don't forget that one here :))
                next;
            }
            $struct->comment =~ m,.*\((.*)\),;  # get the name of the structure
            $tmp=$1;              # store it in a $tmp var as I'm too clueless :)
            $tmp =~ s/\s/_/;      # replace the spaces with underscores
            $struct->name($tmp);  # ...and set it as name
            #print "found: name: '", $struct->name, "' comment: '", $struct->comment, "'\n";
            $struct->hidden("");  # initialize that with a sane value

            # We want that readPtr function :)
            if($struct->name eq "BRC" || $struct->name eq "SHD" || $struct->name eq "DCS"
               || $struct->name eq "DTTM" || $struct->name eq "PHE" || $struct->name eq "TLP"
               || $struct->name eq "ANLD" || $struct->name eq "ANLV" || $struct->name eq "OLST"
               || $struct->name eq "TC" || $struct->name eq "PCD" || $struct->name eq "PRM"
               || $struct->name eq "NUMRM") {
                $struct->readPtr(1);
            }

            #print "Checking for a <TABLE> ";
            while($document[$i] !~ m,\<TABLE ,) {
  	        if($document[$i] =~ m,\<\!--\s*sizeOf\s*=\s*\"(.*?)\"\s*--\>,) {
		    #print "found a sizeOf tag for structure " . $struct->name . ": " . $1 . "\n";
		    $struct->sizeOf($1);
		}
                $i++;
                #print ".";
            }
            #print " found\n";
            # parse the <TABLE> we found
            if(parseStructure()) {
                push(@structs, $struct); # append the new structure
            }
            else {
                print "####### ERROR #######\n";
                print "   name: '", $struct->name, "' comment: '", $struct->comment, "'\n";
            }
        }
        $i++;
    }
    # print "Number of structures: ", $#structs+1, "\n";
    print "Done.\n";
}

# Parses one structure (<table>...</table>)
sub parseStructure {

    # eat the first row (headline)
    while($document[$i] !~ m,^\<TR\>$,) {
        $i++;
    }
    while($document[$i] !~ m,^\</TR\>$,) {
        $i++;
    }

    # parse all the variables till we encounter </TABLE>
    while($document[$i] !~ m,^\</TABLE\>$,) {
        if(parseItem()) {
            push(@{$struct->items}, $item);
            $i++;
        }
        else {
            print "####### ERROR #######\n";
            print "   Error while parsing an item!\n";
            return 0; # uh-oh :}
        }
    }
    #print "count: ", $#{$struct->items}+1, "\n";
    return 1;  # success
}

# Parses one row of the table (<tr> ... </tr>) to get one
# data item out of it. Does some trivial error checking
sub parseItem {
    my ($myState, $tmp);

    $myState=0;
    while($document[$i] !~ m,^\<TR\>$,) {
        $i++;
    }
    $item=Item->new();
    while($document[$i] !~ m,^\</TR\>$,) {
        if($document[$i] =~ m,^\<TD\>(.*)\</TD\>$,) {
            if($myState==0) {  # this is used for debugging/sanity checking
                $item->startNew($1);
                #print "   startNew: ", $1, "\n";
            }
            # yes, I left out $myState==1 on purpose
            elsif($myState==2) {
                $item->name($1);
                #print "   name: ", $1, "\n";
            }
            elsif($myState==3) {
                $item->type($1);
                #print "   type: ", $1, "\n";
            }
            elsif($myState==4) {
                $tmp=$1;
                if($tmp =~ m/^:(.*)/) {
                    $item->bits($1);
                    #print "   bits: ", $1, "\n";
                }
                else {
                    #print "   no bits but a plain size attribute!\n";
                }
            }
            # yes, I left out $myState==5 on purpose
            elsif($myState==6) {
                $item->comment($1);
                #print "   (short) comment: ", $1, "\n";
            }
            $myState++;
        }
        # The comment can expand across several lines
        elsif($document[$i] =~ m,^\<TD\>(.*)$, && $myState==6) {
            $tmp=$1;
            # Insert a <BR> for "newlines" (consistency)
            if($document[$i+1] !~ m,\<BR\>,) {
                $tmp .= "<BR>";
            }
            $i++;
            while($document[$i] !~ m,(.*)\</TD\>$,) {
                $tmp .= $document[$i];
                # Insert a <BR> for "newlines" (consistency)
                if($document[$i+1] !~ m,\<BR\>,) {
                    $tmp .= "<BR>";
                }
                $i++;
            }
            $document[$i] =~ m,(.*)\</TD\>$,;
            $tmp .= $1;
            $item->comment($tmp);
            #print "  (long) comment: ", $tmp, "\n";
            $myState++;
        }
        elsif($document[$i] =~ m,\<\!--\s*initial=\"(.*?)\"\s*--\>,) {
            #print "initial found: ", $document[$i], " filtered: ", $1, "\n";
            $item->initial($1);
        }
        elsif($document[$i] =~ m,\<\!--\s+compareSizeLHS=\"(.*?)\"\s+compareSizeRHS=\"(.*?)\"\s+--\>,) {
            #print "compareSize found: ", $document[$i], " filtered: ", $1, ", ", $2, "\n";
            $item->compareSizeLHS($1);
            $item->compareSizeRHS($2);
        }
        elsif($document[$i] =~ m,^\</TABLE\>$,) {
            print "Error: Found a table end where I didn't expect it!\n";
            return 0;
        }
        $i++;
    }
    #print "$myState==7 ? ", $myState==7, "\n";
    return $myState==7;
}

# Removes some structures we can't generate easily.
# Note: We write out the struct in the header and just
# comment it out (that you can copy it for a proper impl.).
sub cleanStructures {
    my($index, @clean, $done);

    print "Cleaning up...\n";
    # Feel free to add your "favorites" here
    # The goal, however, should be to have as much as possible
    # generated, so try to fix the HTML ;)
    @clean=("PAPXFKP", "CHPXFKP",
            "PAPX", "CHPX", "FLD", "PLCF", "STD", "FFN", "TBD");
    foreach (@clean) {
        $index=0;
        $done=0;
        while($index<=$#structs && $done==0) {
            if($structs[$index]->name eq $_) {
                print "Removing: ", $structs[$index]->name, "\n";
                # Better not really remove, just comment it out by setting "hidden"
                # That way you can copy the declaration for a real implementation
                #splice @structs,$index,1;
                $structs[$index]->hidden("//");
                $done=1;
            }
            $index++;
        }
    }
    print "Done.\n";
}

# Moves around some structures to resolve forward references
# in the generated sources
sub hoistStructures {
    my($index, @hoist, $done);

    print "Resolving forward references...\n";
    # Feel free to add your "favorites" here
    # Note: LIFO, at least kind of (the last element here is first afterwards)
    @hoist=("TBD", "TAP", "DPPOLYLINE", "DPTXBX", "DPHEAD", "TC", "TLP", "BRC", "PHE",
            "SHD", "PRM", "PRM2", "DOPTYPOGRAPHY", "DTTM");
    foreach (@hoist) {
        $index=0;
        $done=0;
        while($index<=$#structs && $done==0) {
            if($structs[$index]->name eq $_) {
                print "Moving: ", $structs[$index]->name, "\n";
                #print "before: ", $#structs, "\n";
                unshift @structs, $structs[$index];
                $index++;
                #print "afterwards: ", $#structs, "\n";
                #print "delete: ", $structs[$index]->name, "\n";
                splice @structs,$index,1;
                #print "test: ", $structs[0]->name, "\n";
                $done=1;
            }
            $index++;
        }
    }
    print "Done.\n";
}

# Selects the structures we want to derive from wvWare::Shared.
sub selectShared {
    my($index, @shared, $done);

    print "Selecting shared structures...\n";
    @shared=("SEP", "TAP", "PAP", "CHP", "PICF");
    foreach (@shared) {
        $index=0;
        $done=0;
        while($index<=$#structs && $done==0) {
            if($structs[$index]->name eq $_) {
                print "Sharing: ", $structs[$index]->name, "\n";
                $structs[$index]->shared(1);
                $done=1;
            }
            $index++;
        }
    }
    print "Done.\n";
}

# Selects the structures which should contain a dump() method
sub selectDumped {
    my($index, @dumped, $done);

    print "Selecting structures with a dump() method...\n";
    @dumped=("SEP", "TAP", "PAP", "CHP", "OLST", "BRC", "TLP",
               "SHD", "DTTM", "PHE", "TC", "ANLV", "LSPD", "DCS",
               "NUMRM", "ANLD");
    foreach (@dumped) {
        $index=0;
        $done=0;
        while($index<=$#structs && $done==0) {
            if($structs[$index]->name eq $_) {
                print "Adding dump() to: ", $structs[$index]->name, "\n";
                $structs[$index]->dumpCode(1);
                $done=1;
            }
            $index++;
        }
    }
    print "Done.\n";
}

# The "main" generator function for headers.
sub generateHeader {
    my($tmp, $license, $includes, $before, $after, $myState);

    print "Generating the header file...\n";
    $tmp=lc($namespace);
    $tmp .= "_generated.h";
    open(HEADER, ">$tmp") or die "Couldn't open the header for writing: " . $!;

    ($license, $includes, $before, $after) = parseTemplate("template-$namespace.h");

    $tmp =~ s/.h/_h/;
    $tmp=uc($tmp);
    # license section...
    print HEADER $license;
    print HEADER "\n#ifndef $tmp\n#define $tmp\n\n";
    # include section...
    print HEADER "#include \"global.h\"\n";
    print HEADER "#include \"sharedptr.h\"\n";
    print HEADER "#include \"utilities.h\"\n";
    print HEADER $includes;
    print HEADER "\nnamespace wvWare {\n\n";
    print HEADER "class OLEStreamReader;\n";
    print HEADER "class OLEStreamWriter;\n";
    print HEADER "class StyleSheet;\n";
    print HEADER "class Style;\n\n";

    print HEADER "namespace $namespace {\n\n";

    # pre
    print HEADER $before . "\n";
    # Fill the empty template
    print HEADER generateHeaderStructs();
    # post
    print HEADER $after;

    print HEADER "\n} // namespace $namespace\n\n";
    print HEADER "} // namespace wvWare\n\n";
    print HEADER "#endif // $tmp\n";
    close(HEADER) or die $!;
    print "Done.\n";
}

# This subroutine generates the header file's structures
sub generateHeaderStructs {
    my($index, $string, $n, $h, $tmp);

    for($index=0; $index<=$#structs; $index++) {
        $n=$structs[$index]->name;
        $h=$structs[$index]->hidden;
        $string .= "/**\n * " . $structs[$index]->comment . "\n */\n";
        if($h ne "") {
            $string .= "/* This structure has been commented out because we can't handle it correctly\n";
            $string .= " * Please don't try to fix it here in this file, but rather copy this broken\n";
            $string .= " * structure definition and fix it in some auxilliary file. If you want to\n";
            $string .= " * include that aux. file here, please change the template file.\n */\n";
        }
        $string .= $h . "struct $n ";
        if(defined($structs[$index]->shared)) {
            $string .= ": public Shared ";
        }
        $string .= "{\n";
        $string .= $h . "    /**\n";
        $string .= $h . "     * Creates an empty $n structure and sets the defaults\n";
        $string .= $h . "     */\n";
        $string .= $h . "    $n();\n";
        $string .= $h . "    /**\n";
        $string .= $h . "     * Simply calls read(...)\n";
        $string .= $h . "     */\n";
        $string .= $h . "    $n(OLEStreamReader *stream, bool preservePos=false);\n";
        if(defined($structs[$index]->readPtr)) {
            $string .= $h . "    /**\n";
            $string .= $h . "     * Simply calls readPtr(...)\n";
            $string .= $h . "     */\n";
            $string .= $h . "    $n(const U8 *ptr);\n";
        }

        # From here on we first put the text into a temporary variable, as
        # we might have to insert some code at this place. The reason is
        # that we need DTOR, Copy CTOR,... if we have pointers in our struct.
        # Unfortunately we find that out in generateHeaderData and don't know
        # it here.
        $tmp = "\n" . $h . "    /**\n";
        $tmp .= $h . "     * This method reads the $n structure from the stream.\n";
        $tmp .= $h . "     * If  preservePos is true we push/pop the position of\n";
        $tmp .= $h . "     * the stream to save the state. If it's false the state\n";
        $tmp .= $h . "     * of stream will be changed!\n";
        $tmp .= $h . "     */\n";
        $tmp .= $h . "    bool read(OLEStreamReader *stream, bool preservePos=false);\n\n";
        # Special readPtr() method for all the "ultra primitive" structs
        # we sometimes have to read from memory (SPRM parameter,...)
        if(defined($structs[$index]->readPtr)) {
            $tmp .= $h . "    /**\n";
            $tmp .= $h . "     * This method reads the struct from a pointer\n";
            $tmp .= $h . "     */\n";
            $tmp .= $h . "    void readPtr(const U8 *ptr);\n\n";
        }
        $tmp .= $h . "    /**\n";
        $tmp .= $h . "     * Same as reading :)\n";
        $tmp .= $h . "     */\n";
        $tmp .= $h . "    bool write(OLEStreamWriter *stream, bool preservePos=false) const;\n\n";
        $tmp .= $h . "    /**\n";
        $tmp .= $h . "     * Set all the fields to the inital value (default is 0)\n";
        $tmp .= $h . "     */\n";
        $tmp .= $h . "    void clear();\n\n";

        # Special apply() method for all the PAP, CHP,... structs
        # Implement that in an auxilliary file
        if(lc($namespace) eq "word97" && ($n eq "PAP" || $n eq "CHP" || $n eq "TAP" || $n eq "SEP" || $n eq "PICF")) {
            $tmp .= $h . "    /**\n";
            $tmp .= $h . "     * This method applies a grpprl with \@param count elements\n";
            $tmp .= $h . "     */\n";
            $tmp .= $h . "    void apply(const U8 *grpprl, U16 count, const Style* style, OLEStreamReader* dataStream, WordVersion version);\n\n";
            $tmp .= $h . "    /**\n";
            $tmp .= $h . "     * This method applies a whole " . $n . "X to the structure.\n";
            $tmp .= $h . "     * The reason that we only pass a pointer to the start of the exception\n";
            $tmp .= $h . "     * structure is, that we don't know the type in the FKP template :}\n";
            $tmp .= $h . "     */\n";
	    if($n eq "CHP") {  # More than just CHP?
	        $tmp .= $h . "    void applyExceptions(const U8* exceptions, const Style* paragraphStyle, OLEStreamReader* dataStream, WordVersion version);\n\n";
	    }
	    else {
	        $tmp .= $h . "    void applyExceptions(const U8 *exceptions, const StyleSheet *stylesheet, OLEStreamReader* dataStream, WordVersion version);\n\n";
	    }
            $tmp .= $h . "    /**\n";
            $tmp .= $h . "     * This method applies one single SPRM. It returns -1 if it wasn't\n";
            $tmp .= $h . "     * a " . $n . " SPRM and it returns the length of the applied SPRM\n";
            $tmp .= $h . "     * if it was successful.\n";
            $tmp .= $h . "     */\n";
            $tmp .= $h . "    S16 apply" . $n . "SPRM(const U8 *ptr, const Style* style, OLEStreamReader* dataStream, WordVersion version);\n\n";
        }

        # Special toPRM2 method for the PRM struct, implemented in word97_helper.cpp
        # This method is neccessary as we don't want to rely on a "packed" layout of
        # the structure so we can't just do evil casting ;)
        if($n eq "PRM") {
            $tmp .= $h . "    /**\n";
            $tmp .= $h . "     * This method returns a PRM2 created from the current PRM\n";
            $tmp .= $h . "     */\n";
            $tmp .= $h . "    PRM2 toPRM2() const;\n\n";
        }

        if(defined($structs[$index]->dumpCode)) {
	    $tmp .= $h . "    /**\n";
	    $tmp .= $h . "     * Dumps all fields of this structure (for debugging)\n";
	    $tmp .= $h . "     */\n";
	    $tmp .= $h . "    void dump() const;\n\n";

	    $tmp .= $h . "    /**\n";
	    $tmp .= $h . "     * Converts the data structure to a string (for debugging)\n";
	    $tmp .= $h . "     */\n";
	    $tmp .= $h . "    std::string toString() const;\n\n";
	}
	
	if(defined($structs[$index]->sizeOf)) {
	    $tmp .= $h . "    // Size of the structure\n";
	    $tmp .= $h . "    static const unsigned int sizeOf;\n\n";
	}

        $tmp .= $h . "    // Data\n";
        $tmp .= generateHeaderData($index);

        if(defined($structs[$index]->dynamic)) {
            # okay, now we already know what we need, so let's
            # add that stuff (to $string, of course ;)
            $string .= $h . "    /**\n";
            $string .= $h . "     * Attention: This struct allocates memory on the heap\n";
            $string .= $h . "     */\n";
            $string .= $h . "    $n(const $n &rhs);\n";
            $string .= $h . "    ~" . $n . "();\n\n";
            $string .= $h . "    " . $n . " &operator=(const $n &rhs);\n";
        }
        # insert the stuff from above
        $string .= $tmp;

        # If we have dynamic structures we have to be careful
        # with clear()! We simply define that clear() also
        # delete []s all the arrays and clearInternal() just sets
        # everything to 0
        if(defined($structs[$index]->dynamic)) {
            $string .= $h . "private:\n";
            $string .= $h . "    void clearInternal();\n\n";
        }
        $string .= $h . "}; // $n\n";

        # ...and add some more code "outside"
        $string .= "\n" . $h . "bool operator==(const $n &lhs, const $n &rhs);\n";
        $string .= $h . "bool operator!=(const $n &lhs, const $n &rhs);\n\n\n";
    }
    return $string;
}

# Takes one structure and generates all the fields for it.
# Checks the bit-fields for missing bits and tries to detect
# arrays with non-static size. We use that information all
# over the place :)
sub generateHeaderData {
    my ($index)=@_;
    my ($string, $tmp, $tmp2, $sum, $bits, $h);

    $sum=0;  # no bits counted up to now :)
    $bits=0; # make the first check work

    # write out all the data
    foreach (@{$structs[$index]->items}) {
        $h=$structs[$index]->hidden;
        $string .= prepareComment($_->comment, $h);
        # Check the completeness of the bitfields...
        if($_->startNew ne "") {
            if($bits != $sum) {
                print "   ERROR: Last bitfield incomplete. Current position: ";
                print $structs[$index]->name . " - " . $_->name . "\n";
            }
            # set up a new check (sloppy, only for U8, U16, and U32 bitfields)
            if($_->type =~ m/U(\d+)/ && defined($_->bits)) {
                #print "bitfield..." . $_->name . "\n";
                $bits=$1;
            }
            else {
                $bits=0;
            }
            $sum=0;
        }
        # Handle XCHAR[32] by splitting it up properly
        if($_->type =~ m/(.*)(\[.*\])/) {
            $tmp = "    " . $1 . " " . $_->name . $2;
            #print "Array: '" . $tmp . "'\n";
            # Is it a fixed size array or not?
            if($tmp !~ m/.*\[\d+\]/) {
                $tmp =~ m/    (.+)\[(.*)\]/;
                $tmp2=$1;
                # get the "length" (or the C++ expression ;)
                $_->len($2);
                $tmp2 =~ s/ / \*/;
                $tmp = "    " . $tmp2 . ";   //" . $tmp;
                #print " --- Result: " . $tmp . "\n";
                # okay, we found a dynamic array, so we need some additional
                # code for that struct (Copy CTOR, DTOR,...)
                $structs[$index]->dynamic(1);
                #if(defined($_->len)) {
                #    print "Dynamic: " . $structs[$index]->name . ", length: " . $_->len . "\n";
                #}
            }
            $string .= $h . $tmp;
        }
        else {
            $string .= $h . "    " . $_->type . " " . $_->name;
        }
        if(defined($_->bits)) {
            $string .= ":" . $_->bits;
            $sum += $_->bits;
        }
        $string .= ";\n\n";
    }
    return $string;
}

# This meathod gets a looong comment string. It takes the
# string and splits it at the <BR>s and creates a nice
# comment out of it (not longer than, say 90 cols, as found in
# the HTML spec)
sub prepareComment {
    my($comment, $h)=@_;
    my($string, @tmp);

    if($comment eq "") {
        return "";
    }

    $string = $h . "    /**\n";
    # "unfold" the <BR>'ed comments
    @tmp=split(/\<BR\>/, $comment);
    foreach (@tmp) {
        $string .= $h . "     * $_\n";
    }
    $string .= $h . "     */\n";
    return $string;
}

# Parse the template file
sub parseTemplate {
    my($name) = @_;  # name of the template
    my($license, $includes, $before, $after, $myState);

    open(TEMPLATE, "<$name") or die "Couldn't open the template: " . $!;
    # initialize all the template vars
    $myState=0;
    $license="";
    $includes="";
    $before="";
    $after="";
    # read in the information...
    while(<TEMPLATE>) {
        if(m/^\#\#\#/) {  # ignore comments
            next;
        }
        if(m/^\@\@license-start\@\@$/) {  # license section
            $myState=1;
            next;
        }
        if(m/^\@\@license-end\@\@$/) {  # end of license sect.
            $myState=0;
            next;
        }
        if(m/^\@\@includes-start\@\@$/) {  # includes section
            $myState=2;
            next;
        }
        if(m/^\@\@includes-end\@\@$/) {  # end of includes sect.
            $myState=0;
            next;
        }
        if(m/^\@\@namespace-start\@\@$/) {  # namespace (before)
            $myState=3;
            next;
        }
        if(m/^\@\@generated-code\@\@$/) {  # namespace (after)
            $myState=4;
            next;
        }
        if(m/^\@\@namespace-end\@\@$/) {  # end of namespace
            $myState=0;
            next;
        }

        if($myState==1) {
            $license .= $_;
        }
        elsif($myState==2) {
            $includes .= $_;
        }
        elsif($myState==3) {
            $before .= $_;
        }
        elsif($myState==4) {
            $after .= $_;
        }
    }
    close(TEMPLATE) or die $!;
    return ($license, $includes, $before, $after);
}

# generate the source file
sub generateImplementation {
    my($tmp, $license, $includes, $before, $after, $myState);

    print "Generating the source file...\n";
    $tmp=lc($namespace);
    $tmp .= "_generated.cpp";
    open(SOURCE, ">$tmp") or die "Couldn't open the file for writing: " . $!;

    ($license, $includes, $before, $after) = parseTemplate("template-$namespace.cpp");

    # license section...
    print SOURCE $license . "\n";
    # include section...
    $tmp =~ s/\.cpp/\.h/;
    print SOURCE "#include <$tmp>\n";
    print SOURCE "#include <olestream.h>\n";
    print SOURCE "#include <string.h>  // memset(), memcpy()\n";
    print SOURCE "#include \"wvlog.h\"\n";
    print SOURCE $includes;
    print SOURCE "\nnamespace wvWare {\n";
    print SOURCE "\nnamespace $namespace {\n\n";

    # pre
    print SOURCE $before . "\n";
    # Fill the empty template
    print SOURCE generateImplStructs();
    # post
    print SOURCE $after;

    print SOURCE "\n} // namespace $namespace\n";
    print SOURCE "\n} // namespace wvWare\n";
    close(SOURCE) or die $!;
    print "Done.\n";
}

# Iterare over all structs and generte the necessary code
sub generateImplStructs {
    my($index, $string, $n);

    for($index=0; $index<=$#structs; $index++) {
        if($structs[$index]->hidden ne "") {
            next; # Don't generate useless code
        }
        $n=$structs[$index]->name;
        $string .= "// $n implementation\n\n";

	# Size (if specified)
	if(defined($structs[$index]->sizeOf)) {
	    $string .= "const unsigned int " . $n . "::sizeOf = " . $structs[$index]->sizeOf . ";\n\n";
	}

        # default CTOR
        $string .= $n . "::" . $n . "() ";
        if(defined($structs[$index]->shared)) {
            $string .= ": Shared() ";
        }
        $string .= "{\n";
        if(defined($structs[$index]->dynamic)) {
            $string .= "    clearInternal();\n";
        }
        else {
            $string .= "    clear();\n";
        }
        $string .= "}\n\n";
        # stream CTOR
        $string .= $n . "::" . $n . "(OLEStreamReader *stream, bool preservePos) ";
        if(defined($structs[$index]->shared)) {
            $string .= ": Shared() ";
        }
        $string .= "{\n";
        if(defined($structs[$index]->dynamic)) {
            $string .= "    clearInternal();\n";
        }
        else {
            $string .= "    clear();\n";
        }
        $string .= "    read(stream, preservePos);\n";
        $string .= "}\n\n";
        # readPtr CTOR
        if(defined($structs[$index]->readPtr)) {
            $string .= $n . "::" . $n . "(const U8 *ptr) ";
            if(defined($structs[$index]->shared)) {
                $string .= ": Shared() ";
            }
            $string .= "{\n";
            if(defined($structs[$index]->dynamic)) {
                $string .= "    clearInternal();\n";
            }
            else {
                $string .= "    clear();\n";
            }
            $string .= "    readPtr(ptr);\n";
            $string .= "}\n\n";
        }
        if(defined($structs[$index]->dynamic)) {
            # Copy CTOR
            $string .= $n . "::" . $n . "(const $n &rhs) ";
            if(defined($structs[$index]->shared)) {
                $string .= ": Shared() ";
            }
            $string .= "{\n";
            $string .= generateCopyCTOR($index);
            $string .= "}\n\n";
            # DTOR
            $string .= $n . "::~" . $n . "() {\n";
            $string .= generateDTOR($index);
            $string .= "}\n\n";
            # assignement operator
            $string .= $n . " &" . $n .  "::operator=(const $n &rhs) {\n";
            $string .= generateAssignment($index);
            $string .= "}\n\n";
        }
        # read()
        $string .= "bool " . $n . "::read(OLEStreamReader *stream, bool preservePos) {\n";
        $string .= generateRead($index);
        $string .= "}\n\n";
        # readPtr()?
        if(defined($structs[$index]->readPtr)) {
            $string .= "void " . $n . "::readPtr(const U8 *ptr) {\n";
            $string .= generateReadPtr($index);
            $string .= "}\n\n";
        }
        # write()
        $string .= "bool " . $n . "::write(OLEStreamWriter *stream, bool preservePos) const {\n";
        $string .= generateWrite($index);
        $string .= "}\n\n";
        # clear()
        $string .= "void " . $n . "::clear() {\n";
        $string .= generateClear($index);
        $string .= "}\n\n";

        if(defined($structs[$index]->dumpCode)) {
            $string .= "void " . $n . "::dump() const\n{\n";
            $string .= generateDump($index);
            $string .= "}\n\n";

            $string .= "std::string " . $n . "::toString() const\n{\n";
            $string .= generateToString($index);
            $string .= "}\n\n";
        }

        if(defined($structs[$index]->dynamic)) {
            $string .= "void " . $n . "::clearInternal() {\n";
            $string .= generateClearInternal($index);
            $string .= "}\n\n";
        }

	# It's okay to initialize the const variable in the header
	#if(defined($structs[$index]->sizeOf)) {
	#    $string .= "const unsigned int " . $n . "::sizeOf = " . $structs[$index]->sizeOf . ";\n\n";
	#}

        # operator== and op!=
        $string .= "bool operator==(const $n &lhs, const $n &rhs) {\n";
        $string .= generateEqualityOp($index);
        $string .= "}\n\n";
        $string .= "bool operator!=(const $n &lhs, const $n &rhs) {\n";
        $string .= "    return !(lhs==rhs);\n";
        $string .= "}\n\n\n";
    }
    return $string;
}

# Generates a Copy Constructor
sub generateCopyCTOR {
    my($index)=@_;
    my($string);

    foreach (@{$structs[$index]->items}) {
        # is it a dyn. array we know the size of?
        if(defined($_->len) && $_->len ne "") {
            $_->type =~ m/(.*)\[.*\]/;
            $string .= "    " . $_->name . "=new " . $1 . "[" . $_->len . "];\n";
            $string .= "    memcpy(" . $_->name . ", rhs." . $_->name . ", sizeof($1)*(" . $_->len . "));\n";
        }
        elsif($_->type =~ m/.*\[\d+\]/) {
            $string .= "    memcpy(&" . $_->name . ", &rhs." . $_->name . ", sizeof(" . $_->name . "));\n";
        }
        else {
            # "plain" members, no problem here
            $string .= "    " . $_->name . "=rhs." . $_->name . ";\n";
        }
    }
    return $string;
}


# Generates a Destructor
sub generateDTOR {
    my($index)=@_;
    my($string);

    foreach (@{$structs[$index]->items}) {
        # is it a dyn. array (regardless whether we know the size!) ?
        if(defined($_->len)) {
            $string .= "    delete [] " . $_->name . ";\n";
        }
    }
    return $string;
}

# Generates an assignment operator
sub generateAssignment {
    my($index)=@_;
    my($string);

    $string = "\n    // Check for assignment to self\n";
    $string .= "    if(this==&rhs)\n";
    $string .= "        return *this;\n\n";

    foreach (@{$structs[$index]->items}) {
        # is it a dyn. array we know the size of?
        if(defined($_->len) && $_->len ne "") {
            $string .= "    delete [] " . $_->name . ";\n";
            $_->type =~ m/(.*)\[.*\]/;
            $string .= "    " . $_->name . "=new " . $1 . "[" . $_->len . "];\n";
            $string .= "    memcpy(" . $_->name . ", rhs." . $_->name . ", sizeof($1)*(" . $_->len . "));\n";
        }
        elsif($_->type =~ m/.*\[\d+\]/) {
            $string .= "    memcpy(&" . $_->name . ", &rhs." . $_->name . ", sizeof(" . $_->name . "));\n";
        }
        else {
            # "plain" members, no problem here
            $string .= "    " . $_->name . "=rhs." . $_->name . ";\n";
        }
    }
    $string .= "\n    return *this;\n";
    return $string;
}

# Generates the code to read from the stream
sub generateRead {
    my($index)=@_;
    my($needU8, $needU16, $needU32, $string, $sum, $limit, $vars);

    $needU8=0;
    $needU16=0;
    $needU32=0;

    $string = "    if(preservePos)\n";
    $string .= "        stream->push();\n\n";

    foreach (@{$structs[$index]->items}) {
        if(defined($_->bits)) {
            if($_->type eq "U8") {
                $needU8=1;
                $limit=8;
            }
            elsif($_->type eq "U16") {
                $needU16=1;
                $limit=16;
            }
            elsif($_->type eq "U32") {
                $needU32=1;
                $limit=32;
            }
            else {
                print "   ERROR: Don't know how to handle a '" . $_->type . "' bitfield\n";
            }
            # first bit of a bitfield?
            if($_->startNew ne "") {
                $string .= "    shifter" . $_->type . "=stream->read" . $_->type . "();\n";
                $sum=0;
            }
            $string .= "    " . $_->name . "=shifter" . $_->type . ";\n";
            $sum+=$_->bits;
            if($sum<$limit) {
                $string .= "    shifter" . $_->type . ">>=" . $_->bits . ";\n";
            }
        }
        # okay, no bitfields from here on
        else {
            # Array?
            if($_->type =~ m/(.*)\[(.*)\]/) {
                #print "Array: " . $_->name . " -- type: " . $_->type . "\n";
                #print "   1: " . $1 . ", 2: " . $2 . "\n";
                if($2 eq "") {
                    #print "    empty! -> warning\n";
                    $string .= "    // Attention: I don't know how to read " . $_->name . " - " . $_->type . "\n";
                    $string .= "#ifdef __GNUC__\n";
                    $string .= "#warning \"Couldn't generate reading code for " . $structs[$index]->name . "::" . $_->name . "\"\n";
                    $string .= "#endif\n";
                }
                else {
                    # Do we have to allocate the memory first?
                    if(defined($_->len) && $_->len ne "") {
                        #print "   allocating...\n";
                        $_->type =~ m/(.*)\[.*\]/;
                        $string .= "    " . $_->name . "=new " . $1 . "[" . $_->len . "];\n";
                    }
                    $string .= "    for(int _i=0; _i<(" . $2 . "); ++_i)\n";
                    $string .= "    " . readVariable($_->name . "[_i]", $1);
                }
            }
            else {
                $string .= readVariable($_->name, $_->type);
            }
        }
    }

    $vars="\n";
    if($needU8) {
        $vars .= "    U8 shifterU8;\n";
    }
    if($needU16) {
        $vars .= "    U16 shifterU16;\n";
    }
    if($needU32) {
        $vars .= "    U32 shifterU32;\n";
    }

    # looks better IMVHO :)
    if($vars ne "\n") {
        $vars .= "\n";
    }

    $string .= "\n    if(preservePos)\n";
    $string .= "        stream->pop();\n";
    $string .= "    return true;\n";

    return $vars . $string;
}

# Generates the code to read from a pointer
sub generateReadPtr {
    my($index)=@_;
    my($needU8, $needU16, $needU32, $string, $sum, $limit, $vars);

    $needU8=0;
    $needU16=0;
    $needU32=0;

    foreach (@{$structs[$index]->items}) {
        if(defined($_->bits)) {
            if($_->type eq "U8") {
                $needU8=1;
                $limit=8;
            }
            elsif($_->type eq "U16") {
                $needU16=1;
                $limit=16;
            }
            elsif($_->type eq "U32") {
                $needU32=1;
                $limit=32;
            }
            else {
                print "   ERROR: Don't know how to handle a '" . $_->type . "' bitfield\n";
            }
            # first bit of a bitfield?
            if($_->startNew ne "") {
                $string .= "    shifter" . $_->type . "=read" . $_->type . "(ptr);\n";
                $string .= "    ptr+=sizeof(" . $_->type . ");\n";
                $sum=0;
            }
            $string .= "    " . $_->name . "=shifter" . $_->type . ";\n";
            $sum+=$_->bits;
            if($sum<$limit) {
                $string .= "    shifter" . $_->type . ">>=" . $_->bits . ";\n";
            }
        }
        # okay, no bitfields from here on
        else {
            # Array?
            if($_->type =~ m/(.*)\[(.*)\]/) {
                #print "Array: " . $_->name . " -- type: " . $_->type . "\n";
                #print "   1: " . $1 . ", 2: " . $2 . "\n";
                if($2 eq "") {
                    #print "    empty! -> warning\n";
                    $string .= "    // Attention: I don't know how to read " . $_->name . " - " . $_->type . "\n";
                    $string .= "#ifdef __GNUC__\n";
                    $string .= "#warning \"Couldn't generate reading code for " . $structs[$index]->name . "::" . $_->name . "\"\n";
                    $string .= "#endif\n";
                }
                else {
                    # Do we have to allocate the memory first?
                    if(defined($_->len) && $_->len ne "") {
                        #print "   allocating...\n";
                        $_->type =~ m/(.*)\[.*\]/;
                        $string .= "    " . $_->name . "=new " . $1 . "[" . $_->len . "];\n";
                    }
                    $string .= "    for(int _i=0; _i<(" . $2 . "); ++_i) {\n";
                    $string .= readVariablePtr($_->name . "[_i]", $1, "        ");
                    $string .= "    }\n";
                }
            }
            else {
                $string .= readVariablePtr($_->name, $_->type, "    ");
            }
        }
    }

    $vars="\n";
    if($needU8) {
        $vars .= "    U8 shifterU8;\n";
    }
    if($needU16) {
        $vars .= "    U16 shifterU16;\n";
    }
    if($needU32) {
        $vars .= "    U32 shifterU32;\n";
    }

    # looks better IMVHO :)
    if($vars ne "\n") {
        $vars .= "\n";
    }
    return $vars . $string;
}

# Is the passed name a known structure? (needed for read())
sub knownType {
    my($name)=@_;

    foreach (@structs) {
        if($_->name eq $name) {
            return 1;
        }
    }
    return 0;
}

# Generates code to read one plain variable (no arrays!)
sub readVariable {
    my($name, $type)=@_;
    my($string);

    if($type =~ m/^[US]\d+$/) {
        $string = "    " . $name . "=stream->read" . $type . "();\n";
    }
    elsif($type eq "FC") {
        $string = "    " . $name . "=stream->readU32();\n";
    }
    elsif($type eq "XCHAR") {
        $string = "    " . $name . "=stream->readU16();\n";
    }
    elsif(knownType($type)) {
        #print "Known: " . $type . "\n";
        $string = "    " . $name . ".read(stream, false);\n";
    }
    elsif($type =~ m/std::vector/ ) {
        print "Found a std::vector, skipping it for reading. I hope you know what you're doing\n";
        $string = "    // skipping the std::vector " . $name . "\n";
    }
    else {
        print "Error: Can't read " . $name . ", " . $type . "\n";
        $string="";  # initialize
    }
    return $string;
}

# Generates code to read one plain variable (no arrays!) from memory
sub readVariablePtr {
    my($name, $type, $indent)=@_;
    my($string);

    if($type =~ m/^[US]\d+$/) {
        $string = $indent . $name . "=read" . $type . "(ptr);\n";
	$string .= $indent . "ptr+=sizeof(" . $type . ");\n";
    }
    elsif($type eq "FC") {
        $string = $indent . $name . "=readU32(ptr);\n";
	$string .= $indent . "ptr+=sizeof(U32);\n";
    }
    elsif($type eq "XCHAR") {
        $string = $indent . $name . "=readU16(ptr);\n";
	$string .= $indent . "ptr+=sizeof(U16);\n";
    }
    elsif(knownType($type)) {
        #print "Known: " . $type . "\n";
        $string = $indent . $name . ".readPtr(ptr);\n";
	$string .= $indent . "ptr+=" . $type . "::sizeOf;\n";
    }
    elsif($type =~ m/std::vector/ ) {
        print "Found a std::vector, skipping it for reading. I hope you know what you're doing\n";
        $string = "    // skipping the std::vector " . $name . "\n";
    }
    else {
        print "Error: Can't read " . $name . ", " . $type . "\n";
        $string="";  # initialize
    }
    return $string;
}

# Generates code to add one plain variable (no arrays!) to the string
sub variableToString {
    my($name, $type)=@_;
    my($string, $output);

    if($name =~ m/(.+)\[(.+)\]/) {
        $output = $1 . "[\" + int2string( " . $2 . " ) + \"]";
    }
    else {
        $output = $name;
    }

    $string = "    s += \"\\n" . $output . "=\";\n";
    if($type =~ m/^U\d+$/ || $type eq "FC" || $type eq "XCHAR") {
        $string .= "    s += uint2string( " . $name . " );\n";
    }
    elsif($type =~ m/^S\d+$/) {
        $string .= "    s += int2string( " . $name . " );\n";
    }
    elsif(knownType($type)) {
        #print "Known: " . $type . "\n";
        $string .= "    s += \"\\n{\" + " . $name . ".toString() + \"}\\n\";\n";
    }
    elsif($type =~ m/std::vector/ ) {
        print "Found a std::vector, skipping it for reading. I hope you know what you're doing\n";
        $string .= "    // skipping the std::vector " . $name . "\n";
    }
    else {
        print "Error: Can't dump " . $name . ", " . $type . "\n";
    }
    return $string;
}

# Generates the code to write to the stream
sub generateWrite {
    my($index)=@_;
    my($needU8, $needU16, $needU32, $string, $position, $limit, $vars);

    $needU8=0;
    $needU16=0;
    $needU32=0;
    $position=0;

    $string = "    if(preservePos)\n";
    $string .= "        stream->push();\n\n";

    foreach (@{$structs[$index]->items}) {
        if(defined($_->bits)) {
            if($_->type eq "U8") {
                $needU8=1;
                $limit=8;
            }
            elsif($_->type eq "U16") {
                $needU16=1;
                $limit=16;
            }
            elsif($_->type eq "U32") {
                $needU32=1;
                $limit=32;
            }
            else {
                print "   ERROR: Don't know how to handle a '" . $_->type . "' bitfield\n";
            }
            # first bit of a bitfield?
            if($_->startNew ne "") {
                # Do we have to write out the last bitfield?
                if($position != 0) {
                    $string .= "    stream->write(shifter" . $_->type . ");\n";
                    $position=0;
                }
                $string .= "    shifter" . $_->type . "=" . $_->name . ";\n";
            }
            else {
                $string .= "    shifter" . $_->type . "|=" . $_->name . " << " . $position . ";\n";
            }
            $position+=$_->bits;
        }
        # Do we have to write out the last bitfield?
        if(defined($limit) && $position == $limit) {
            #print "reached the limit...\n";
            $string .= "    stream->write(shifter" . $_->type . ");\n";
            $position=0;
            undef($limit);
        }
        # okay, no bitfields from here on
        if(!defined($_->bits)) {
            # Array?
            if($_->type =~ m/(.*)\[(.*)\]/) {
                #print "Array: " . $_->name . " -- type: " . $_->type . "\n";
                #print "   1: " . $1 . ", 2: " . $2 . "\n";
                if($2 eq "") {
                    #print "    empty! -> warning\n";
                    $string .= "    // Attention: I don't know how to write " . $_->name . " - " . $_->type . "\n";
                    $string .= "#ifdef __GNUC__\n";
                    $string .= "#warning \"Couldn't generate writing code for " . $structs[$index]->name . "::" . $_->name . "\"\n";
                    $string .= "#endif\n";
                }
                else {
                    $string .= "    for(int _i=0; _i<(" . $2 . "); ++_i)\n";
                    $string .= "    " . writeVariable($_->name . "[_i]", $1);
                }
            }
            else {
                $string .= writeVariable($_->name, $_->type);
            }
        }
    }

    $vars="\n";
    if($needU8) {
        $vars .= "    U8 shifterU8;\n";
    }
    if($needU16) {
        $vars .= "    U16 shifterU16;\n";
    }
    if($needU32) {
        $vars .= "    U32 shifterU32;\n";
    }

    # looks better IMVHO :)
    if($vars ne "\n") {
        $vars .= "\n";
    }

    $string .= "\n    if(preservePos)\n";
    $string .= "        stream->pop();\n";
    $string .= "    return true;\n";

    return $vars . $string;
}

# Generates code to write one plain variable (no arrays!)
sub writeVariable {
    my($name, $type)=@_;
    my($string);

    if($type =~ m/^[US]\d+$/ || $type eq "FC" || $type eq "XCHAR") {
        $string = "    stream->write(" . $name . ");\n";
    }
    elsif(knownType($type)) {
        #print "Known: " . $type . "\n";
        $string = "    " . $name . ".write(stream, false);\n";
    }
    elsif($type =~ m/std::vector/ ) {
        print "Found a std::vector, skipping it for writing. I hope you know what you're doing\n";
        $string = "    // skipping the std::vector " . $name . "\n";
    }
    else {
        print "Error: Can't write " . $name . ", " . $type . "\n";
        $string="";  # initialize
    }
    return $string;
}

# Generates the code to compare structs with dynamic members
sub generateEqualityOp {
    my($index)=@_;
    my($string, $first, $tmp);

    # first check the arrays
    # Note: We don't check the 0-sized ones!
    foreach (@{$structs[$index]->items}) {
        if($_->type =~ m/.*\[(.*)\]/) {
            $tmp=$1;
            if($tmp eq "") {
                #print "    empty! -> warning\n";
                $string .= "    // Attention: I don't know how to compare " . $_->name . " - " . $_->type . "\n";
                $string .= "#ifdef __GNUC__\n";
                $string .= "#warning \"Can't compare " . $structs[$index]->name . "::" . $_->name . " items\"\n";
                $string .= "#endif\n";
                next;
            }
            # okay, not a plain array, so we have to compare the size, too :}
            if($_->type !~ m/.*\[(\d+)\]/) {
                if(defined($_->compareSizeLHS) && defined($_->compareSizeRHS)) {
                    #print " ---> compareSize: " . $_->compareSizeLHS . ", " . $_->compareSizeRHS . "\n";
                    $string .= "\n    if((" . $_->compareSizeLHS . ")!=(" . $_->compareSizeRHS . "))\n";
                    $string .= "        return false;\n";
                    $string .= "    for(int _i=0; _i<(" . $_->compareSizeLHS . "); ++_i) {\n";
                    $string .= "        if(lhs." . $_->name . "[_i]!=rhs." . $_->name . "[_i])\n";
                    $string .= "            return false;\n";
                    $string .= "    }\n";
                }
                else {
                    $string .= "#ifdef __GNUC__\n";
                    $string .= "#warning \"Please provide a compareSize* comment for " . $structs[$index]->name . "::" . $_->name . "\"\n";
                    $string .= "#endif\n";
                }
            }
            else {
                $string .= "\n    for(int _i=0; _i<(" . $tmp . "); ++_i) {\n";
                $string .= "        if(lhs." . $_->name . "[_i]!=rhs." . $_->name . "[_i])\n";
                $string .= "            return false;\n";
                $string .= "    }\n";
            }
        }
    }
    # ... then create a nice return statement ;)
    $string .= "\n    return";
    $first=1;  # first line?
    foreach (@{$structs[$index]->items}) {
        # don't forget to exclude the arrays!
        if($_->type !~ m/.*\[.*\]/) {
            # special case: first line
            if($first==1) {
                $string .= " lhs." . $_->name . "==rhs." . $_->name;
                $first=0;
            }
            else {
                $string .= " &&\n           lhs." . $_->name . "==rhs." . $_->name;
            }
        }
    }
    $string .= ";\n";
    return $string;
}

# Generates a proper clear() method depending whether we have
# dynamic members of not
sub generateClear {
    my($index)=@_;
    my($string);

    if(defined($structs[$index]->dynamic)) {
        # delete [] the dynamic memory!
        foreach (@{$structs[$index]->items}) {
            if(defined($_->len)) {
                $string .= "    delete [] " . $_->name . ";\n";
            }
        }
        $string .= "    clearInternal();\n";
    }
    else {
        $string=generateClearInternal($index);
    }
    return $string;
}

# Generates the code to dump a structure
sub generateDump {
    my($index, $tmp) = @_;
    my($string);

    $string .= "    wvlog << \"Dumping " . $structs[$index]->name . ":\" << std::endl;\n";
    $string .= "    wvlog << toString().c_str() << std::endl;\n";
    $string .= "    wvlog << \"\\nDumping " . $structs[$index]->name . " done.\" << std::endl;\n";
    return $string;
}

# Generates the code to convert a structure to a string
sub generateToString {
    my($index, $tmp) = @_;
    my($string);

    $string .= "    std::string s( \"" . $structs[$index]->name . ":\" );\n";
    foreach (@{$structs[$index]->items}) {
        # Array?
        if($_->type =~ m/(.*)\[(.*)\]/) {
	    #print "Array: " . $_->name . " -- type: " . $_->type . "\n";
	    #print "   1: " . $1 . ", 2: " . $2 . "\n";
	    if($2 eq "") {
	        #print "    empty! -> warning\n";
	        $string .= "    // Attention: I don't know how to turn " . $_->name . " - " . $_->type . " to a string\n";
		$string .= "#ifdef __GNUC__\n";
		$string .= "#warning \"Couldn't generate toString code for " . $structs[$index]->name . "::" . $_->name . "\"\n";
		$string .= "#endif\n";
	    }
	    else {
	        $string .= "    for(int _i=0; _i<(" . $2 . "); ++_i) {\n";
		$string .= "    " . variableToString($_->name . "[_i]", $1);
		$string .= "    }\n";
	    }
	}
	else {
	    $string .= variableToString($_->name, $_->type);
	}
    }
    $string .= "    s += \"\\n" . $structs[$index]->name . " Done.\";\n";
    $string .= "    return s;\n";
    return $string;
}

# Generates the code to initialize all fields
sub generateClearInternal {
    my($index, $tmp) = @_;
    my($string);

    foreach (@{$structs[$index]->items}) {
        if($_->type =~ m/.*\[(.*)\]/) {
            $tmp=$1;
            # fine, just a pointer
            if($tmp eq "") {
                $string .= "    " . $_->name . "=0;\n";
                next;
            }
            # okay, also a pointer
            if($_->type !~ m/.*\[(\d+)\]/) {
                $string .= "    " . $_->name . "=0;\n";
            }
            else {
                $string .= "    for(int _i=0; _i<(" . $tmp . "); ++_i)\n";
                $_->type =~ m/(.*)\[.*\]/;
                $string .= "        " . $_->name . "[_i]" . initValue($_, $1) . ";\n";
            }
        }
        else {
            $string .= "    " . $_->name . initValue($_, $_->type) . ";\n";
        }
    }
    return $string;
}

sub initValue {
    my($item, $type)=@_;
    my($string);

    if(defined($item->initial)) {
        $string = "=" . $item->initial;
    }
    else {
        if($type =~ m/^[US]\d+$/ || $type eq "FC" || $type eq "XCHAR") {
            $string = "=0";
        }
        elsif(knownType($type)) {
            #print "Known: " . $type . "\n";
            $string = ".clear()";
        }
	elsif($type =~ m/std::vector/ ) {
	    $string = ".clear()";
	}
        else {
            print "Error: Can't initialize " . $item->name . ", " . $type . "\n";
            $string="";  # initialize
        }
    }
    return $string;
}

# Generates some testcases
sub generateTest {
    my($tmp);

    print "Generating the testcases...\n";
    $tmp=lc($namespace);
    open(TEST, ">$tmp" . "_test.cpp") or die "Couldn't open the file for writing: " . $!;

    print TEST "// This file contains generated testing code. We do some basic tests\n";
    print TEST "// like writing and reading from/to streams. Of course these tests are\n";
    print TEST "// neither complete nor very advanced, so watch out :)\n\n";
    print TEST "#include <" . $tmp . "_generated.h>\n";
    print TEST "#include <olestream.h>\n";
    print TEST "#include <iostream>\n";
    print TEST "#include <stdlib.h>   // rand(), srand()\n\n";
    print TEST "#include <time.h>   // time()\n\n";

    print TEST "using namespace wvWare;\n";
    print TEST "using namespace " . $namespace .";\n\n";

    print TEST "int main(int, char**) {\n\n";
    print TEST "    // First we have to create some infrastructure\n";
    print TEST "    system(\"rm " . $tmp . "_test.doc &> /dev/null\");\n";
    print TEST "    OLEStorage storage(\"" . $tmp . "_test.doc\");\n";
    print TEST "    if(!storage.open(OLEStorage::WriteOnly)) {\n";
    print TEST "        std::cout << \"Error: Couldn't open the storage!\" << std::endl;\n";
    print TEST "        ::exit(1);\n";
    print TEST "    }\n\n";
    print TEST "    OLEStreamWriter *writer=storage.createStreamWriter(\"TestStream\");\n";
    print TEST "    if(!writer || !writer->isValid()) {\n";
    print TEST "        std::cout << \"Error: Couldn't open a stream for writing!\" << std::endl;\n";
    print TEST "        ::exit(1);\n";
    print TEST "    }\n\n";
    print TEST "    // Initialize the random number generator\n";
    print TEST "    srand( time( 0 ) );\n\n";

    print TEST "    // Some \"global\" variables...\n";
    print TEST "    int *ptr=0;  // used to \"initialize\" the structs\n";
    print TEST "    int tmp;\n";

    print TEST "    std::cout << \"Testing the $namespace structures...\" << std::endl;\n";

    print TEST generateTestStructures();

    print TEST "    std::cout << \"Done.\" << std::endl;\n";
    print TEST "    // Clean up\n";
    print TEST "    storage.close();\n";
    print TEST "}\n";

    close(TEST) or die $!;
    print "Done.\n";
}

# Generates all the tests for every single structure
sub generateTestStructures {
    my($string, $index, $n);

    # writing
    for($index=0; $index<=$#structs; $index++) {
        if($structs[$index]->hidden ||
	   $structs[$index]->name eq "DPCALLOUT" || # DPCALLOUT depends on DPPOLY -> dynamic
	   $structs[$index]->name eq "PAP" || # PAP uses std::vector<Foo>
	   $structs[$index]->name eq "SEP" || # SEP uses std::vector<Foo>
	   $structs[$index]->name eq "TAP") { # TAP uses std::vector<Foo>
            next;
        }
        $n=$structs[$index]->name;
        if($structs[$index]->dynamic) {
            $string .= "    std::cout << \"Testing writing for $n:\" << std::endl;\n";
            $string .= "    std::cout << \"  Sorry, testing dynamic structures isn't implemented,\"\n";
            $string .= "              << \" yet.\" << std::endl;\n";
        }
        else {
            $string .= "    // Begin of writing test for $n\n";
            $string .= "    std::cout << \"Testing writing for $n: \";\n";
            $string .= generateWritingTest($index);
            $string .= "    // End of writing test for $n\n\n";
        }
    }

    # mess with the streams...
    $string .= "\n    // Okay, close the stream writer and open it for reading...\n";
    $string .= "    int position=writer->tell();  // store the position for a check\n";
    $string .= "    delete writer;\n";
    $string .= "    storage.close();\n";
    $string .= "    if(!storage.open(OLEStorage::ReadOnly)) {\n";
    $string .= "        std::cout << \"Error: Couldn't open the storage!\" << std::endl;\n";
    $string .= "        ::exit(1);\n";
    $string .= "    }\n";
    $string .= "    OLEStreamReader *reader=storage.createStreamReader(\"TestStream\");\n";
    $string .= "    if(!reader || !reader->isValid()) {\n";
    $string .= "        std::cout << \"Error: Couldn't open a stream for reading!\" << std::endl;\n";
    $string .= "        ::exit(1);\n";
    $string .= "    }\n\n";

    # reading
    for($index=0; $index<=$#structs; $index++) {
        if($structs[$index]->hidden ||
	   $structs[$index]->name eq "DPCALLOUT" || # DPCALLOUT depends on DPPOLY -> dynamic
	   $structs[$index]->name eq "PAP" || # PAP uses std::vector<Foo>
	   $structs[$index]->name eq "SEP" || # SEP uses std::vector<Foo>
	   $structs[$index]->name eq "TAP") { # TAP uses std::vector<Foo>
            next;
        }
        $n=$structs[$index]->name;
        if($structs[$index]->dynamic) {
            $string .= "    std::cout << \"Testing reading for $n:\" << std::endl;\n";
            $string .= "    std::cout << \"  Sorry, testing dynamic structures isn't implemented,\"\n";
            $string .= "              << \" yet.\" << std::endl;\n";
        }
        else {
            $string .= "    // Begin of reading test for $n\n";
            $string .= "    std::cout << \"Testing reading for $n: \";\n";
            $string .= generateReadingTest($index);
            $string .= "    // End of reading test for $n\n\n";
        }
    }

    # check the position in the stream
    $string .= "\n    if(position!=reader->tell())\n";
    $string .= "        std::cout << \"Error: Different amount of bytes read/written!\" << std::endl;\n";
    $string .= "    delete reader;\n\n";
    return $string;
}

# Generates some code to initialize a struct and write it out.
sub generateWritingTest {
    my($index)=@_;
    my($string, $n, $var);

    $n=$structs[$index]->name;
    $var=lc($n) . "1";
    $string .= "    $n " . $var . ";\n";
    $string .= "    // Initilaize the struct with random data\n";
    $string .= "    tmp=sizeof($n)/sizeof(int);\n";
    $string .= "    ptr=reinterpret_cast<int*>( &" . $var . " );\n";
    $string .= "    for(int _i=0; _i<tmp; ++_i)\n";
    $string .= "        *ptr++=rand();\n";
    $string .= "    *ptr |= rand() & (0x00ffffff >> (((sizeof(int)-1)-(sizeof($n) % sizeof(int)))*8));  // yay! :)\n";
    $string .= "    // and write it out...\n";
    $string .= "    if($var.write(writer, false))\n";
    $string .= "        std::cout << \"Passed.\" << std::endl;\n";
    $string .= "    else\n";
    $string .= "        std::cout << \"Failed.\" << std::endl;\n";
    if(defined($structs[$index]->dumpCode)) {
        $string .= "    " . $var . ".dump();\n";
    }
    return $string;
}

sub generateReadingTest {
    my($index)=@_;
    my($string, $n, $var2, $var3);

    $n=$structs[$index]->name;
    $var2=lc($n) . "2";
    $string .= "    $n " . $var2 . ";\n";
    $string .= "    // Read the data from the stream\n";
    $string .= "    if(!$var2.read(reader, false))\n";
    $string .= "        std::cout << \"Failed. \" << std::endl;\n";
    $string .= "    // Test the copy CTOR\n";
    $var3=lc($n) . "3";
    $string .= "    $n " . $var3 . "($var2);\n";
    $string .= "    if(" . lc($n) . "1==$var2 && $var2==$var3)\n";
    $string .= "        std::cout << \"Passed.\" << std::endl;\n";
    $string .= "    else\n";
    $string .= "        std::cout << \"Failed.\" << std::endl;\n";

    return $string;
}

# Reads the HTML file and converts the "interesting" tags
# to uppercase. It also cuts of areas we're not interested in
# from the begin and the end of the file.
sub main {
    # A flag which tells us what part of the HTML to ignore
    my $ignore=1;

    open(INPUT, "<$ARGV[0]") or die $!;
    $namespace=$ARGV[1];

    while(<INPUT>) {

        # Detection of start for Word9x
        if(m,^Structure Definitions\</h[12]\>$,) {
            $ignore=0;
        }
        # Detection of end for Word97
        elsif(m,^Appendix A - Reading a Macintosh PICT Graphic\</h2\>$,) {
            $ignore=1;
        }
        # Detection of end for Word95
        elsif(m,^Appendix A - Changes from version 1\.x to 2\.0\</h1\>$,) {
            $ignore=1;
        }

        if(!$ignore) {
            chomp;
            # convert the important tags we use to uppercase on the fly
            s,\<tr\>,\<TR\>,;
            s,\</tr\>,\</TR\>,;
            s,\<td\>,\<TD\>,;
            s,\</td\>,\</TD\>,;
            s,\<table ,\<TABLE ,;
            s,\</table\>,\</TABLE\>,;
            s,\<br\>,\<BR\>,;
            s,\<h3\>,\<H3\>,;
            s,\</h3\>,\</H3\>,;
            # get rid of that ugly &nbsp; thingies
            s/&nbsp;//g;

            push(@document, $_);
        }
    }
    close(INPUT) or die $!;
    # print "Size of the array: ", $#document+1, "\n";

    parseStructures();    # parse the document
    cleanStructures();    # get rid of stuff we don't want to use
    hoistStructures();    # resolve forward references
    selectShared();       # select the structures we wanted to share
    selectDumped();       # select the structures which should contain a dump() method

    generateHeader();     # generate the header file
    generateImplementation(); # generate the source
    generateTest();       # generate the sythetic test cases
}

# We start execution here
if($#ARGV != 1) {
    print "Script to generate C++ code to read Word structures from the HTML\n";
    print "documentation. Usage: perl generate.pl input_file.html Word9x\n";
    exit(1);
}

main();
