.. _ldms_csv_time_drops:

===================
ldms_csv_time_drops
===================

-------------------------------
The LDMS CSV data quality check
-------------------------------

:Date:   07 Jul 2022
:Manual section: 7
:Manual group: LDMS

SYNOPSIS
========

| ldms_csv_time_drops <file list>
| ldms_csv_time_drops_range <file list>

DESCRIPTION
===========

LDMS CSV store file quality checker. For each input file, the interval,
gaps and duplicates in the data are reported. When multiple files are
given, they must be given in chronological order of the data contained.

INTERVAL
========

The interval is determined per file by examining the rounded time
differences of sequential samples on each host and taking the most
common value. 0 length intervals are ignored. If more than one 'most
common' interval is found across the hosts of a single file, the maximum
interval seen in any file is reported as 'interval' and the minimum is
reported as 'short_interval'.

GAPS
====

Gaps in the data are computed using the assumption of a uniform sampling
interval across all hosts on the aggregate timestamp data from all the
input files. A missing file or daemon down-time within the range of the
data set will appear a gap.

DUPLICATES
==========

An identical timestamp reappearing on the same host will be reported.
The later time reported for a duplicate is the latest time seen across
any host in the same file preceeding the line location of the duplicate.

INPUT
=====

The LDMS csv store column format is assumed, in particular that the
first column is the timestamp and any row beginning with # is a header
to be ignored. Columns 1-4 are assumed to be

::


   Time,Time_usec,ProducerName,component_id

OUTPUT FORMATS
==============

Per-file summary:

::


   lines <count of valid lines>
   oldest <timestamp>
   newest <timestamp>
   interval <seconds>

If multiple intervals found in a file

::


   short_interval <seconds>

Per gap output for ldms_csv_time_drops_range:

::


   <host> is missing <intervals> between
       <timestamp.start>
   and <timestamp.end>

Per gap output for ldms_csv_time_drops:

::


   <host> missing <timestamp.nominal>

Duplicates are reported as:

::


   <host> <timestamp> written again at <timestamp>

BUGS
====

Sub-second intervals are not supported.

EXAMPLES
========

For input test.csv containing:

::


   1.1,100000,host1,1
   1.1,100000,host2,2
   1.1,100000,host3,3
   2.1,100000,host1,1
   2.1,100000,host2,2
   3.1,100000,host1,1
   3.1,100000,host2,2
   3.1,100000,host3,3
   4.1,100000,host1,1
   4.1,100000,host3,3
   5.1,100000,host1,1
   2.1,100000,host1,1
   5.1,100000,host2,2
   5.1,100000,host3,3

   output of 'ldms_csv_time_drops test.csv'

   lines 14
   oldest 1.100000
   newest 5.100000
   interval 1 seconds
   host1 2.000001 written again at 5.000001
   host2 missing 4
   host3 missing 2

   output of 'ldms_csv_time_drops_range test.csv'

   lines 14
   oldest 1.100000
   newest 5.100000
   interval 1 seconds
   host1 2.100000 written again at 5.100000
   host2 is missing 1 steps between
       3.100000
   and 5.100000
   host3 is missing 1 steps between
       1.100000
   and 3.100000


   Find the interval of data in a file foo.csv

   ldms_csv_time_drops foo.csv |grep ^interval

SEE ALSO
========

:ref:`store_csv(7) <store_csv>`
