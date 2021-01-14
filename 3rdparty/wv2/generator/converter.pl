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

# A script to generate code which converts Word95 structures to Word97 ones
# as good as possible.
# If you add a convert comment to the Word 6 HTML you can "losen" the
# restrictions a bit:
#   - convert="string(fieldname)" converts between U8[] and XCHAR[]
#             In case you want to limit the string size just write
#             "string(fieldname:XY)" where XY is the length to copy.
#   - convert="type" losens the type restrictions and simply tries to assign
#             even if the types are not exactly the same (U32 <- U16,...)
#   - convert="(fieldname)" relates the fieldnames and losens the type
#             restrictions
#   - convert="unused"  skips this field

###############################################################################
# To discuss with Shaheed:
# - CHP::chse - I think we should map that to the Word97 CHP::fMacChs (70)
# - I disabled (unused) DOP::fReadOnlyRecommended and DOP::fWriteReservation
#   as this should normally go into the Word97 FIB, but well, I doubt we need
#   that flags.
# - I mapped the things like cpnBtePap to the "active" Word97 structures
#   instead of mapping it to the blah_W6 compatibility ones.
# - PAP::fAutoHyph -> ? Maybe it's Word97::PAP::fAutoWrap?
# - PAP::rgdxaTab, PAP::rgtbd?
# - PGD: We should create a PGD2 structure for the 2nd table and have
#   conversion function like for PRM -> PRM2. The we can map Word95::PGD to
#   the Word97::PGD2. For now I disabled the structure.
# - PHE: What to do with the Height/Line field? Have a union for them?
###############################################################################

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
    matched => '$',      # This field is used to indicate that this item already was "matched"
    convert => '$',      # The conversion options - if any
};

struct Structure => {
    name => '$',      # The name of the structure
    comment => '$',   # The comment for this struct
    items => '@',     # All the data members
    hidden => '$',    # Set to "//" if we want to comment that structure out
    dynamic => '$',   # Do we have dynamic memory? Then we need a Copy CTOR,
                      # DTOR, assignment op, op==,...
};


# This array of strings contains the whole HTML
# documentation file. It's used twice when reading the spec in
# All the parsing subs will read/modify that global array
# Note: All the tags we use are already converted to
# uppercase.
my @document;

# The current index in the document-array (used during parsing)
my $i;

# These arrays hold all the structures we want to write out
my @structs95;
my @structs97;

# The current struct we're working on (only used during parsing)
my $struct;
# The current item we're working on (only used during parsing)
my $item;

# Parses all the structures
sub parseStructures {
    my ($doc)=@_;
    my ($tmp);

    print "Parsing $doc...\n";
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

            #print "Checking for a <TABLE> ";
            while($document[$i] !~ m,\<TABLE ,) {
                $i++;
                #print ".";
            }
            #print " found\n";
            # parse the <TABLE> we found
            if(parseStructure()) {
                if($doc eq "Word95") {
                    push(@structs95, $struct); # append the new structure
                }
                elsif($doc eq "Word97") {
                    push(@structs97, $struct);
                }
                else {
                    print "Error: Word95 or Word97?\n";
                }
            }
            else {
                print "####### ERROR #######\n";
                print "   name: '", $struct->name, "' comment: '", $struct->comment, "'\n";
            }
        }
        $i++;
    }
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
        elsif($document[$i] =~ m,\<\!--\s*convert=\"(.*?)\"\s*--\>,) {
            #print "convert found: ", $document[$i], " filtered: ", $1, "\n";
            $item->convert($1);
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
            "PAPX", "CHPX", "FLD", "PLCF", "STD", "BRC", "PGD", "SEPX",
	    "FFN", "STSHI", "TBD");
    foreach (@clean) {
        $index=0;
        $done=0;
        while($index<=$#structs95 && $done==0) {
            if($structs95[$index]->name eq $_) {
                print "Removing: ", $structs95[$index]->name, "\n";
                # Better not really remove, just comment it out by setting "hidden"
                # That way you can copy the declaration for a real implementation
                #splice @structs95,$index,1;
                $structs95[$index]->hidden("//");
                $done=1;
            }
            $index++;
        }
    }
    print "Done.\n";
}

# Generates the conversion header. trivial code, as we just create declarations
# like Word97::FOO toWord97(const Word95::FOO &s), where FOO is some struct
sub generateHeader {
    my($license, $includes, $before, $after, $myState);

    print "Generating the header file...\n";
    open(HEADER, ">convert.h") or die "Couldn't open the header for writing: " . $!;

    ($license, $includes, $before, $after) = parseTemplate("template-conv.h");

    # license section...
    print HEADER $license;
    print HEADER "\n#ifndef CONVERT_H\n#define CONVERT_H\n\n";
    # include section...
    print HEADER "#include <word95_generated.h>\n";
    print HEADER "#include <word97_generated.h>\n";
    print HEADER $includes;
    print HEADER "\nnamespace wvWare {\n\n";
    print HEADER "namespace Word95 {\n";

    # pre
    print HEADER $before . "\n";
    # Fill the empty template
    print HEADER generateDeclarations();
    # post
    print HEADER $after;

    print HEADER "\n} // namespace Word95\n\n";
    print HEADER "} // namespace wvWare\n\n";
    print HEADER "#endif // CONVERT_H\n";
    close(HEADER) or die $!;
    print "Done.\n";
}

# This method is used to actually generate the methods with the pattern
# Word97::FOO toWord97(const Word95::FOO &s), where FOO is some struct
sub generateDeclarations {
    my($index, $string, $n, $tmp);

    for($index=0; $index<=$#structs95; $index++) {
        $n=$structs95[$index]->name;
	if($structs95[$index]->hidden ne "//") {
	    for($tmp=0; $tmp<=$#structs97; $tmp++) {
              if($n eq $structs97[$tmp]->name) {
	        $string .= "Word97::$n toWord97(const Word95::$n &s);\n";
	        last;
              }
	    }
	}
    }
    return $string;
}

# This is the tricky part. It first adds all the template stuff and calls the
# generator method to fill the void ;)
sub generateImplementation {
    my($tmp, $license, $includes, $before, $after);

    print "Generating the source file...\n";
    open(SOURCE, ">convert.cpp") or die "Couldn't open the file for writing: " . $!;

    ($license, $includes, $before, $after) = parseTemplate("template-conv.cpp");

    # license section...
    print SOURCE $license . "\n";
    # include section...
    print SOURCE "#include <convert.h>\n";
    print SOURCE $includes;
    print SOURCE "\nnamespace wvWare {\n";
    print SOURCE "\nnamespace Word95 {\n";

    # pre
    print SOURCE $before . "\n";
    # Fill the empty template
    print SOURCE generateFunctions();
    # post
    print SOURCE $after;

    print SOURCE "} // namespace Word95\n";
    print SOURCE "\n} // namespace wvWare\n";
    close(SOURCE) or die $!;
    print "Done.\n";

}

# Creates the empty template for every conversion function
sub generateFunctions {
    my($index95, $index97, $string, $n, $h);

    for($index95=0; $index95<=$#structs95; $index95++) {
        $n=$structs95[$index95]->name;
        $h=$structs95[$index95]->hidden;
        for($index97=0; $index97<=$#structs97; $index97++) {
            if($n eq $structs97[$index97]->name) {
                if($h eq "//") {
                    $string .= "/* Please check...\n";
                }
                $string .= "Word97::$n toWord97(const Word95::$n &s) {\n\n";
                $string .= "    Word97::$n ret;\n\n";
                $string .= generateConversion($index95, $index97);
                $string .= "\n    return ret;\n";
                if($h eq "//") {
                    $string .= "} */\n\n";
                }
                else {
                    $string .= "}\n\n";
                }
                last;
            }
        }
    }
    return $string;
}

# This method tries to match fields inside structures, using some basic heuristics
# and hints inside the .html files. Check the documentation at the top of that file
# for further information about the hints and how to use them
sub generateConversion {
    my($index95, $index97)=@_;
    my($i, $j, @items95, @items97, %result, $tmp1, $tmp2, $string);

    print "Trying to match the fields for " . $structs95[$index95]->name . "\n";
    if($structs95[$index95]->hidden eq "//") {
        print "   Note: Hidden structure, implementation will be commented out\n";
    }
    @items95=@{$structs95[$index95]->items};
    @items97=@{$structs97[$index97]->items};
    # First try to find all "direct" matches (type, name, position)
    for($i=0; $i<=$#items95 && $i<=$#items97; $i++) {
        if($items95[$i]->name eq $items97[$i]->name &&
           $items95[$i]->type eq $items97[$i]->type &&
           ((defined($items95[$i]->bits) && defined($items97[$i]->bits) &&
             $items95[$i]->bits eq $items97[$i]->bits) ||
            (not(defined($items95[$i]->bits)) && not(defined($items97[$i]->bits))))) {
            #print "   Direct match for " . $items95[$i]->name . "\n";
            $items95[$i]->matched(1);
            $items97[$i]->matched(1);
            $result{$items95[$i]->name}=$i;
        }
    }
    # Then try to check if we find the same name/type at some other position
    for($i=0; $i<=$#items95; $i++) {
        if(not(defined($items95[$i]->matched))) {
            for($j=0; $j<=$#items97; $j++) {
                if(not(defined($items97[$j]->matched)) &&
                   $items95[$i]->name eq $items97[$j]->name &&
                   $items95[$i]->type eq $items97[$j]->type) {
                    #print "   Indirect match for " . $items95[$i]->name . "\n";
                    $items95[$i]->matched(1);
                    $items97[$j]->matched(1);
                    $result{$items95[$i]->name}=$j;
                }
            }
        }
    }
    # Did the "user" add some hints for us?
    for($i=0; $i<=$#items95; $i++) {
        if(defined($items95[$i]->convert)) {
            if($items95[$i]->convert =~ m/^string\((.*)\)$/) {
                #print "   Hint: string($1)\n";
                $tmp1=$1;
                if($tmp1 =~ m/(.*):(\d+)/) {
                    #print "   Additional length hint: " . $2 . "\n";
                    $tmp1=$1;
                    $tmp2=$2;
                    if($items95[$i]->type =~ m/(.*)\[(.*)\]/) {
                        #print "   Old type: " . $items95[$i]->type . "\n";
                        $items95[$i]->type($1 . "[" . $tmp2 . "]");
                        #print "   New type: " . $items95[$i]->type . "\n";
                    }
                }
                for($j=0; $j<=$#items97; $j++) {
                    if(not(defined($items97[$j]->matched)) &&
                       $tmp1 eq $items97[$j]->name) {
                        #print "   Matched due to string hint: " . $items95[$i]->name . " -> " . $1 . "\n";
                        $items95[$i]->matched(1);
                        $items97[$j]->matched(1);
                        $result{$items95[$i]->name}=$j;
                    }
                }
            }
            elsif($items95[$i]->convert =~ m/^type$/) {
                #print "   Hint: type\n";
                for($j=0; $j<=$#items97; $j++) {
                    if(not(defined($items97[$j]->matched)) &&
                       $items95[$i]->name eq $items97[$j]->name) {
                        #print "   Matched due to type hint: " . $items95[$i]->name . "\n";
                        $items95[$i]->matched(1);
                        $items97[$j]->matched(1);
                        $result{$items95[$i]->name}=$j;
                    }
                }
            }
            elsif($items95[$i]->convert =~ m/^\((.*)\)$/) {
                #print "   Hint: ($1)\n";
                for($j=0; $j<=$#items97; $j++) {
                    if(not(defined($items97[$j]->matched)) &&
                       $1 eq $items97[$j]->name) {
                        #print "   Matched due to mapping hint: " . $items95[$i]->name . " -> " . $1 . "\n";
                        $items95[$i]->matched(1);
                        $items97[$j]->matched(1);
                        $result{$items95[$i]->name}=$j;
                    }
                }
            }
            elsif($items95[$i]->convert =~ m/^unused$/) {
                #print "   Hint: unused\n";
                $items95[$i]->matched(1);
                $result{$items95[$i]->name}=-42; # unused
            }
            else {
                print "   Hint: Didn't understand this hint.\n";
            }
        }
    }
    # What's still missing? (Information)
    foreach(@items95) {
        if(not(defined($_->matched))) {
            print "   -> No match for " . $_->name . "\n";
        }
    }

    # Now that we have a complete map (hopefully ;) let's generate the code
    $string="";
    foreach(@items95) {
        $i=$result{$_->name};
        if(not(defined($i)) || $i == -42) {
            #print "   Skipping item " . $_->name . "\n";
            next;
        }
        $string .= generateMapping($_, $items97[$i]);
    }
    return $string;
}

# Create "one line" of the conversion function. Depending on the type
# this method has to generate a proper assignment operation.
sub generateMapping {
    my($item95, $item97)=@_;
    my($ret, $tmp);

    # is it a dyn. array we know the size of?
    if(defined($item95->len) && $item95->len ne "") {
      $item95->type =~ m/(.*)\[.*\]/;
      $ret .= "    ret." . $item97->name . "=new " . $1 . "[" . $item95->len . "];\n";
      $ret .= "    memcpy(rhs." . $item97->name . ", s." . $item95->name . ", sizeof($1)*(" . $item95->len . "));\n";
    }
    elsif($item95->type =~ m/(.*)\[(\d+)\]/) {
      $ret .= "    for(int i=0;i<($2);++i)\n";
      if(knownType($1)) {
	$ret .= "        ret." . $item97->name . "[i]=toWord97(s." . $item95->name . "[i]);\n";
      }
      else {
	$ret .= "        ret." . $item97->name . "[i]=s." . $item95->name . "[i];\n";
      }
    }
    elsif(knownType($item95->type)) {
      $ret .= "    ret." . $item97->name . "=toWord97(s." . $item95->name . ");\n";
    }
    else {
      # "plain" members, no problem here
      $ret .= "    ret." . $item97->name . "=s." . $item95->name . ";\n";
    }
    return $ret;
}

# Helper method to detect known Word95 structs
sub knownType {
    my($name)=@_;

    foreach (@structs95) {
        if($_->name eq $name) {
            return 1;
        }
    }
    return 0;
}

# Read the whole .html file into an array, line by line
sub readDocument {
    my($name)=@_;
    my $ignore=1;

    open(INPUT, "<$name") or die $!;

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
}

# Reads the HTML files and converts the "interesting" tags
# to uppercase. It also cuts of areas we're not interested in
# from the begin and the end of the file.
sub main {

    readDocument($ARGV[0]);
    parseStructures("Word95");
    $#document=0;
    readDocument($ARGV[1]);
    parseStructures("Word97");
    $#document=0;
    cleanStructures();    # get rid of stuff we don't want to use

    generateHeader();     # generate the header file
    generateImplementation(); # generate the source
}

# We start execution here
if($#ARGV != 1) {
    print "Script to generate C++ code to convert Word95 to Word97 structures";
    print "\nfrom the HTML specs.\n";
    print "Usage: perl converter.pl spec95.html spec97.html\n";
    exit(1);
}

main();
