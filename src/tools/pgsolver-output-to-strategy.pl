#!/usr/bin/perl

while (<>)
{
	# Read (partial) strategy:
	if (/^with strategy$/)
	{
		$line = <>;
		$line =~ /^  \[(.*)\]$/ or die;
		for $arc (split /,/, $1)
		{
			$arc =~ /^([0-9]+)->([0-9]+)$/ or die;
			$strat{$1} = $2;
		}
	}
}

# Print full strategy, ordered by predecessor vertex index.
@v = keys %strat;
sort @v;
for $v (@v)
{
	print "$v->$strat{$v}\n";
}
