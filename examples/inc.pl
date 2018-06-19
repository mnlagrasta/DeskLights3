#!/usr/bin/env perl

my $i=0;
while ($i <= 255) {
	`dl3 setAll "c=$i"`;
	$i+=1;
}
