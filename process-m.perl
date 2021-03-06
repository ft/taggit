#!/usr/bin/perl
# This is an example of how to process the data that amded emits in
# its list-machine-readable mode (using the -m option).
# The code is written in Perl.

use strict;
use warnings;
use English '-no_match_vars';

# a container for the read data.
my $data;
# handle for the forking open call.
my $amded;

# Read amded output. All of it. At once.
open $amded, '-|', 'amded -m *.mp3' or die "Couldn't for off amded: $!\n";
# locally turn on slurp mode.
local $INPUT_RECORD_SEPARATOR = undef;
$data = <$amded>;
# and turn it off again.
local $INPUT_RECORD_SEPARATOR = "\n";
close $amded;

# Amded uses ASCII end-of-transmission (EOT) bytes to seperate information
# from different files. So First, break up the input at EOT bytes (hex 0x04)
# So let's split at those:
foreach my $fi (split /\x04/, $data) {

    # Just to show that $amded is an array that contains records for
    # each file processed by amded, let's put in a marker line:
    print "-------------------------------------------------------------------\n";

    # Now each file contains a number of tags and each tag=value pair
    # is seperated from each other by ASCII end-of-text (ETX) bytes:
    foreach my $ti (split /\x03/, $fi) {

        # At this point, we got the tag=value pairs in $ti. The
        # tag names are seperated from their value data by ASCII
        # start-of-text (STX) bytes.
        # So, we split again to gain easy access to the actual data:
        my @info = split /\x02/, $ti, 2;
        printf "%15s | %s\n", $info[0], $info[1];
    }
}

exit 0;
