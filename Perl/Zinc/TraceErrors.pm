#
# Copyright (c) 1992-1994 The Regents of the University of California.
# Copyright (c) 1994 Sun Microsystems, Inc.
# Copyright (c) 1995-1999 Nick Ing-Simmons. All rights reserved.
# This program is free software; you can redistribute it and/or
# modify it under the same terms as Perl itself, subject
# to additional disclaimer in Tk/license.terms due to partial
# derivation from Tk8.0 sources.
#
# Copyright (c) 2003 CENA, D.Etienne <etienne@cena.fr> to trace all
# Tk::Zinc errors.
#
# This package overloads the Tk::Zinc::WidgetMethods function in order to
# to trap errors by calling every Tk::Zinc method in an eval() block.
#
# This may be very usefull when your application encounters errors such as
# "error .... at /usr/lib/perl5/Tk.pm line 228". With ZincTraceErrors, the
# module name, the line number and the complete error messages are reported
# for each error.
#
# $Id: TraceErrors.pm,v 1.5 2003/09/15 16:17:05 mertz Exp $
#
# When you have no idea where this happens in your code or when your
# application segfaults, use the Tk::Zinc::Trace package which traces every
# Tk::Zinc method call.
#
# for using this file do some thing like :
# perl -MTk::Zinc::TraceErrors myappli.pl

package Tk::Zinc::TraceErrors;

use vars qw( $VERSION );
($VERSION) = sprintf("%d.%02d", q$Revision: 1.5 $ =~ /(\d+)\.(\d+)/);

use Tk;
use strict;
use Tk::Zinc::TraceUtils;

my $WidgetMethodfunction;
my $bold = "[1m";
my $_bold = "[m";

BEGIN {
    my $bold = "[1m";
    my $_bold = "[m";
    
    if ($ZincTrace::on == 1) {
	print STDERR $bold."Tk::Zinc::TraceErrors: incompatible package Tk::Zinc::Trace is already ".
	    "loaded".$_bold." (exit 1)\n";
	exit 1;
    }
    print $bold."Tk::Zinc::TraceErrors is ON".$_bold."\n";
    $ZincTraceErrors::on = 1;
    select STDOUT; $|=1; ## for flushing the trace output
    # save current Tk::Zinc::InitObject function; it will be invoked in
    # overloaded one (see below)
    use Tk;
    use Tk::Zinc;
    $WidgetMethodfunction = Tk::Zinc->can('WidgetMethod');
    
}

sub Tk::Zinc::WidgetMethod {
    my ($zinc, $name, @args) = @_;
    my ($package, $filename, $line) = caller(1);
    $package="" unless defined $package;
    $filename="" unless defined $filename;
    $line="" unless defined $line;
    # invoke function possibly overloaded in other modules
    my ($res, @res);
    if (wantarray()) {
	eval {@res = &$WidgetMethodfunction(@_) if $WidgetMethodfunction;};
    } else {
	eval {$res = &$WidgetMethodfunction(@_) if $WidgetMethodfunction;};
    }
    if ($@) {
	print $bold."error:".$_bold." $filename line $line $name";
	&printList (@args);
	my $msg = $@;
	$msg =~ s/at .*//g;
	print " ".$bold."returns".$_bold." $msg\n";
    }
    if (wantarray()) {
	return @res;
    } else {
	return $res;
    }
}
    
    

1;



