#!/usr/bin/perl
#
# Execute renamings in a mapping.txt file (usually generated by suggest_identrenaming)
#
# Usage: do_identrenaming listfile mappingfile (incomments|incode|filename|everywhere)
#

$verbose = 1;

$listfile = $ARGV[0];
$mappingfile = $ARGV[1];
$operation = $ARGV[2];

if ($operation ne "source" && $operation ne "filename" && $operation ne "everywhere") {
    print STDERR "usage: do_identrenaming listfile mappingfile (source|filename|everywhere)\n";
    exit(1);
}

# parse listfile
print "reading $listfile...\n" if ($verbose);
$listfilecontents = readfile($listfile);
$listfilecontents =~ s|^\s*(.*?)\s*$|push(@fnames,$1);""|gme;

# parse mappingfile
print "reading $mappingfile...\n" if ($verbose);
$mapping = readfile($mappingfile);
$mapping =~ s|^\s*(.*?)\s*->\s*(.*?)\s*$|$map{"${1}_m"}="${2}_m";""|gme;

# debug: print map
for $i (sort(keys(%map))) {print "$i -> $map{$i}\n";}

# do it
foreach $fname (@fnames)
{
    print "renaming in $fname...\n" if ($verbose);
    $txt = readfile($fname);
    writefile("$fname.bak", $txt);

    # replace in the code
    if ($operation eq "source" || $operation eq "everywhere") {
        for $i (keys(%map)) {
            $txt =~ s|\b$i\b|$map{$i}|gs;
        }
    }

    # replace in the filename
    if ($operation eq "filename" || $operation eq "everywhere") {
        $origFname = $fname;
        for $i (keys(%map)) {
            $fname =~ s|\b$i\b|$map{$i}|gs;
        }
        if ($fname ne $origFname) {
            unlink $origFname;      # delete old file
        }
    }

    writefile($fname, $txt);
}

print "done -- backups saved as .bak\n" if ($verbose);

sub readfile ()
{
    my $fname = shift;
    my $content;
    open FILE, "$fname" || die "cannot open $fname";
    read(FILE, $content, 1000000);
    close FILE;
    $content;
}

sub writefile ()
{
    my $fname = shift;
    my $content = shift;
    open FILE, ">$fname" || die "cannot open $fname for write";
    print FILE $content;
    close FILE;
}

