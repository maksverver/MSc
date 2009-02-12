#!/usr/bin/perl

while (<>)
{
	if (/^Player ([01]) wins from nodes:/)
	{
		$player = $1;
		$line = <>;
		$line =~ s/[^0-9,]//g;
		for $v (split /,/, $line)
		{
			$owners[$v] = 'E' if $player == 0;
			$owners[$v] = 'O' if $player == 1;
		}
	}
}
$output = join '', @owners;
$output =~ s/(.{80}|.$)/\1\n/g;
print $output;
